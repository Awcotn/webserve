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

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    : Scheduler(threads, use_caller, name) {
    m_epfd = epoll_create(5000);
    AWCOTN_ASSERT(m_epfd > 0);   

    int rt = pipe(m_tickleFds);
    AWCOTN_ASSERT(rt);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    AWCOTN_ASSERT(rt == 0);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);

    AWCOTN_ASSERT(rt);

    m_fdContexts.resize(32);
        
    start();

}

IOManager::~IOManager() {
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

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

//1 success, 0 retry. -1 error
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
    if(fd < m_fdContexts.size()) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        // 如果fd超出当前容量，需要扩容
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        // 扩容策略：当前大小的1.5倍
        contextResize(m_fdContexts.size() * 1.5);
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

bool IOManager::delEvent(int fd, Event event) {
    
}

bool IOManager::cancelEvent(int fd, Event event) {

}

bool IOManager::cancelAll(int fd) {

}

IOManager* IOManager::GetThis() {

}
}