#include "iomanager.h"
#include "macro.h"
#include "log.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace awcotn {

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

/**
 * @brief 获取指定事件类型对应的事件上下文
 * @param event 事件类型(READ/WRITE)
 * @return 对应事件类型的上下文引用
 */
IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event) {
     switch(event) {
        case READ:
            return read;
        case WRITE:
            return write;
        default:
            AWCOTN_ASSERT2(false, "getContext");
    };
}

/**
 * @brief 重置事件上下文的状态
 * @param ctx 需要重置的事件上下文
 */
void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

/**
 * @brief 触发指定的事件
 * @param event 要触发的事件类型(READ/WRITE)
 * @details 将对应事件的回调函数或协程调度到调度器中执行，同时从events中清除该事件标志
 */
void IOManager::FdContext::triggerEvent(IOManager::Event event) {
    AWCOTN_ASSERT(events & event);
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);  
    if(ctx.cb) {
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    } 
    ctx.scheduler = nullptr;
    return;
}

/**
 * @brief IOManager构造函数，初始化epoll实例和通知管道
 * @param threads 线程数量
 * @param use_caller 是否使用调用者线程
 * @param name 调度器名称
 * @details 
 * 创建epoll实例，并设置通知机制用于唤醒idle线程
 * 
 * 【多线程共享设计】
 * 整个IOManager只创建一个epoll实例(m_epfd)，
 * 所有工作线程的idle协程都监听这同一个epoll实例，
 * 从而实现多线程协同处理IO事件的高效模型。
 */
IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    : Scheduler(threads, use_caller, name) {
    // 创建epoll实例，参数5000只是一个提示，不是实际限制
    m_epfd = epoll_create(5000);
    AWCOTN_ASSERT(m_epfd > 0);   

    // 创建管道用于通知/唤醒idle线程
    int rt = pipe(m_tickleFds);
    AWCOTN_ASSERT(!rt);

    // 配置epoll监听管道的读端，设置为边缘触发模式(EPOLLET)
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    // 设置管道读端为非阻塞模式
    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    AWCOTN_ASSERT(rt == 0);

    // 将管道读端添加到epoll实例
    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    AWCOTN_ASSERT(!rt);

    // 初始化文件描述符上下文数组
    contextResize(32);
    
    // 启动调度器
    start();
}

/**
 * @brief IOManager析构函数，释放资源
 * @details 停止调度器，关闭epoll实例和管道，释放所有文件描述符上下文
 */
IOManager::~IOManager() {
    AWCOTN_LOG_INFO(g_logger) << "IOManager::~IOManager";
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

/**
 * @brief 重新调整文件描述符上下文数组大小
 * @param size 新的大小
 * @details 确保数组能容纳指定数量的文件描述符，并初始化新增的上下文
 */
void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

/**
 * @brief 向IO事件监听器添加事件
 * @param fd 文件描述符
 * @param event 待添加的事件类型(READ/WRITE)
 * @param cb 事件发生时的回调函数，如不传入，则使用当前协程
 * @return 成功返回0，出错返回-1
 * @details 该函数将一个文件描述符的指定事件注册到epoll中，并设置对应的回调
 */
int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
    // 获取文件描述符对应的上下文对象
    FdContext* fd_ctx = nullptr;
    // 读锁保护，尝试从已有列表获取fd上下文
    RWMutexType::ReadLock lock(m_mutex);
    if(fd < (int)m_fdContexts.size()) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        // 如果fd超出当前容量，需要扩容
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        // 扩容策略：当前大小的1.5倍
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    // 锁定特定fd的上下文，保证fd操作的线程安全
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    // 确保不重复添加同一事件
    if(fd_ctx->events & event) {
        AWCOTN_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
            << " event=" << event
            << " fd_ctx.event=" << fd_ctx->events;
        AWCOTN_ASSERT(!(fd_ctx->events & event));
    }

    // 根据文件描述符当前状态确定epoll操作类型
    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    // - 如果fd_ctx->events已有值(非0)，表示该fd已注册到epoll中，需要修改(MOD)
    // - 如果fd_ctx->events为0，表示该fd未注册，需要添加(ADD)
    
    // 创建epoll事件结构体用于配置
    epoll_event epevent;
    
    // 设置要监听的事件类型，包含三部分:
    epevent.events = EPOLLET | fd_ctx->events | event;
    // - EPOLLET: 设置边缘触发模式(Edge Triggered)，只在状态变化时触发一次，
    //   区别于水平触发模式(Level Triggered)会持续触发直到处理完毕
    // - fd_ctx->events: 保留该fd已有的事件监听设置
    // - event: 添加新的事件类型(如EPOLLIN、EPOLLOUT等)
    
    // 设置用户数据，将fd上下文对象与事件关联
    epevent.data.ptr = fd_ctx;
    // - 当事件触发时，epoll_wait返回该指针，使程序能找回完整的上下文信息
    // - 这比仅存储fd号更灵活，可以关联到复杂的数据结构
    
    // 执行epoll_ctl系统调用，将事件添加到epoll实例
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        // 如果操作失败，记录错误并返回
        AWCOTN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return -1;
    }
    
    // 增加待处理事件计数
    m_pendingEventCount++;
    // 更新文件描述符上下文中的事件标志位
    fd_ctx->events = (Event)(fd_ctx->events | event);
    
    // 获取对应事件类型的上下文
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    // 确保该事件上下文尚未被使用
    AWCOTN_ASSERT(!event_ctx.scheduler
            && !event_ctx.fiber
            && !event_ctx.cb);

    // 关联当前调度器
    event_ctx.scheduler = Scheduler::GetThis();
    if(cb) {
        // 如果提供了回调函数，保存回调函数
        event_ctx.cb.swap(cb);
    } else {
        // 否则，使用当前协程作为回调
        event_ctx.fiber = Fiber::GetThis();
        // 确保当前协程处于运行状态
        AWCOTN_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
    }
    return 0;
}

/**
 * @brief 删除文件描述符上的事件
 * @param fd 文件描述符
 * @param event 要删除的事件(READ/WRITE)
 * @return 成功返回true，失败返回false
 * @details 从epoll实例中删除指定的事件监听，但不触发任何回调
 */
bool IOManager::delEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if(fd >= (int)m_fdContexts.size()) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)) {
        return false;
    }

    // 计算删除事件后的事件集合
    Event new_events = (Event)(fd_ctx->events & ~event);
    // 根据剩余事件决定epoll操作：有则修改，无则删除
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        AWCOTN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 更新待处理事件计数和文件描述符事件标志
    m_pendingEventCount--;
    fd_ctx->events = new_events;
    // 重置对应事件的上下文，但不调度执行任何回调
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}

/**
 * @brief 取消文件描述符上的事件并触发回调
 * @param fd 文件描述符
 * @param event 要取消的事件(READ/WRITE)
 * @return 成功返回true，失败返回false
 * @details 从epoll实例中删除指定的事件监听，并调度执行对应的回调函数或协程
 */
bool IOManager::cancelEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if(fd >= (int)m_fdContexts.size()) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!(fd_ctx->events & event)) {
        return false;
    }

    // 计算取消事件后的事件集合
    Event new_events = (Event)(fd_ctx->events & ~event);
    // 根据剩余事件决定epoll操作：有则修改，无则删除
    int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_events;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        AWCOTN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 更新待处理事件计数
    m_pendingEventCount--;
    // 触发事件对应的回调函数或协程
    fd_ctx->triggerEvent(event);
   
    return true;
}

/**
 * @brief 取消文件描述符上的所有事件并触发回调
 * @param fd 文件描述符
 * @return 成功返回true，失败返回false
 * @details 从epoll实例中删除所有事件监听，并调度执行对应的回调函数或协程
 */
bool IOManager::cancelAll(int fd) {
    RWMutexType::ReadLock lock(m_mutex);
    if(fd >= (int)m_fdContexts.size()) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if(!fd_ctx->events) {
        return false;
    }

    // 从epoll实例中完全删除该文件描述符
    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        AWCOTN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << op << ", " << fd << ", " << epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    // 触发所有注册的事件回调
    if(fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        m_pendingEventCount --;
    }
    if(fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        m_pendingEventCount --;
    }

    AWCOTN_ASSERT(fd_ctx->events == 0);
    return true;
}

/**
 * @brief 获取当前线程的IOManager实例
 * @return 当前线程的IOManager指针，如果当前调度器不是IOManager则返回nullptr
 */
IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

/**
 * @brief 唤醒一个空闲的线程处理待处理事件
 * @details 通过向tickle管道写入数据唤醒epoll_wait阻塞的线程
 */
void IOManager::tickle() {
    if(hasIdleThreads()) {
        return;
    }
    int rt = write(m_tickleFds[1], "T", 1);
    AWCOTN_ASSERT(rt == 1);
}

/**
 * @brief 检查调度器是否应该停止
 * @return 如果调度器应该停止返回true，否则返回false
 */
bool IOManager::stopping() {
    uint64_t timeout = getNextTimer();
    return stopping(timeout);
}

/**
 * @brief 检查调度器是否应该停止
 * @param timeout 下一个定时器的超时时间
 * @return 如果调度器应该停止返回true，否则返回false
 * @details 当没有定时器、没有待处理事件且调度器状态为停止时返回true
 */
bool IOManager::stopping(uint64_t timeout) {
    timeout = getNextTimer();
    return timeout == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();
} 

/**
 * @brief 线程空闲时执行的函数，主要用于等待和处理IO事件
 * @details 
 * 通过epoll_wait等待IO事件，处理定时器回调，并对触发的事件执行对应的回调
 * 
 * 【多线程共享epoll模型】
 * 1. 所有调度线程的idle协程共享同一个epoll实例(m_epfd)
 * 2. 多个线程可以同时调用epoll_wait监听事件
 * 3. 当事件发生时，内核会唤醒其中一个等待的线程处理事件
 * 4. 触发事件后，对应的回调会被放入调度器，可能由任意线程执行
 * 5. 这种设计避免了传统的"每线程一个epoll"模型中的负载不均问题
 * 
 * 【惊群问题的处理】
 * Linux内核在2.6.18后对epoll实现了优化，同一个epoll实例上的
 * epoll_wait不会出现经典的"惊群现象"，即只有一个线程会被唤醒处理事件。
 * 这使得多线程共享epoll实例的设计更加高效。
 */
void IOManager::idle() {
    // 分配一个长度为64的epoll_event数组，用于存储从epoll_wait返回的事件
    // 使用()初始化确保所有元素被零初始化
    epoll_event* events = new epoll_event[64]();
    // 使用智能指针管理events的生命周期，确保在函数退出时自动释放内存
    std::shared_ptr<epoll_event> shared_events(events, [](epoll_event* ptr){
        delete[] ptr;
    });

    while(true) {
        // 检查调度器是否应该停止
        // stopping()返回true当且仅当调度器需要停止且没有挂起的事件
        uint64_t next_timeout = 0;
        if(stopping(next_timeout)) {
            AWCOTN_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
            break;
        }

        int rt = 0; // 存储epoll_wait返回的事件数量
        while(true) {
            static const int MAX_TIMEOUT = 1000; // 最大超时时间为1秒(1000毫秒)
            if(next_timeout != ~0ull) {
                // 如果有定时器，计算下一个超时时间
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }

            // 等待epoll事件发生:
            // m_epfd: epoll实例的文件描述符
            // events: 存储返回事件的数组
            // 64: 数组的大小，最多一次处理64个事件
            // MAX_TIMEOUT: 超时时间(毫秒)，如果没有事件发生，最多等待这么长时间
            rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);

            // 处理系统调用被信号中断的情况(EINTR)
            // 如果是因为信号中断导致的返回，则重新调用epoll_wait
            if(rt < 0 && errno == EINTR) {
                // 空操作，继续下一次循环
            } else {
                // 其他情况(包括正常返回事件或错误)跳出循环
                break;
            }
        } 

        std::vector<std::function<void()>> cbs;
        listExpiredCb(cbs); // 获取所有过期的定时器回调函数
        if(!cbs.empty()) {
            schedule(cbs.begin(), cbs.end()); // 调度执行这些回调函数
            cbs.clear(); // 清空回调函数列表
        }
        
        // 处理所有返回的事件，rt是返回的事件数量
        for(int i = 0; i < rt; i++) {
            epoll_event& event = events[i]; // 当前处理的事件
            
            // 检查是否是tickle事件(用于唤醒idle线程的特殊事件)
            if(event.data.fd == m_tickleFds[0]) {
                uint8_t dummy;
                // 清空管道中的所有数据，防止同一事件被多次触发
                while(read(m_tickleFds[0], &dummy, 1) == 1);
                continue; // 继续处理下一个事件
            }
            
            // 获取事件关联的文件描述符上下文
            FdContext* fd_ctx = (FdContext*)event.data.ptr;
            // 锁定该文件描述符的互斥量，确保线程安全
            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            
            // 如果发生了错误(EPOLLERR)或挂起(EPOLLHUP)事件
            // 将其同时视为可读和可写事件，这样可以让应用程序尝试读写并获得具体的错误信息
            if(event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= EPOLLIN | EPOLLOUT;
            }
            
            // 将epoll事件映射到IOManager定义的事件类型(READ/WRITE)
            int real_events = NONE; // 初始化为无事件
            if(event.events & EPOLLIN) {
                real_events |= READ; // 可读事件
            }
            if(event.events & EPOLLOUT) {
                real_events |= WRITE; // 可写事件
            }
            
            // 如果实际触发的事件与fd_ctx注册的事件没有交集，则跳过
            // 这可能发生在事件已被取消但epoll通知尚未处理完的情况
            if((fd_ctx->events & real_events) == NONE) {
                continue;
            }
            
            // 计算剩余事件：从fd_ctx的事件中移除已触发的事件
            int left_events = (fd_ctx->events & ~real_events);
            // 确定epoll操作类型：如果还有剩余事件则修改，否则删除
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            // 设置epoll事件，使用边缘触发模式(EPOLLET)
            event.events = EPOLLET | left_events;
            
            // 更新epoll实例中的事件设置
            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if(rt2) {
                // 如果更新失败，记录错误信息但继续处理其他事件
                AWCOTN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << op << ", " << fd_ctx->fd << ", " << event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }
            
            // 触发读事件回调：如果有READ事件发生且fd注册了READ事件
            if(real_events & READ) {
                fd_ctx->triggerEvent(READ); // 触发注册的读事件回调
                --m_pendingEventCount; // 减少待处理事件计数
            }
            // 触发写事件回调：如果有WRITE事件发生且fd注册了WRITE事件
            if(real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE); // 触发注册的写事件回调
                --m_pendingEventCount; // 减少待处理事件计数
            }
        }

        // 当前协程处理完所有事件后，需要让出执行权
        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();

        // 将当前协程切换出去，让调度器调度其他协程执行
        raw_ptr->swapOut();
    }
}

/**
 * @brief 当定时器被插入到最前面时调用
 * @details 唤醒idle线程以重新计算等待超时时间
 */
void IOManager::onTimerInsertedAtFront() {
    tickle();
}

}