#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "thread.h"
#include "hook.h"
namespace awcotn {

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    : m_name(name) {
    AWCOTN_ASSERT(threads > 0);

    if(use_caller) {
        Fiber::GetThis();
        --threads;

        AWCOTN_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        awcotn::Thread::SetName(name);

        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = awcotn::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    } 
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    AWCOTN_ASSERT(m_stopping);
    AWCOTN_LOG_INFO(g_logger) << this << " deconstruction";
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;

    AWCOTN_ASSERT(m_threads.empty());
//    AWCOTN_LOG_INFO(g_logger) << m_threadCount;
    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();

    // if(m_rootFiber) {
    //     //AWCOTN_LOG_INFO(g_logger) << "start root fiber";
    //     m_rootFiber->call();
    //     //m_rootFiber->swapIn();
    //     AWCOTN_LOG_INFO(g_logger) << "call out" << m_rootFiber->getState();
    // }
}

void Scheduler::stop() {
    AWCOTN_LOG_INFO(g_logger) << "Scheduler stop";
    m_autoStop = true;
    if(m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        AWCOTN_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }
    
    if(m_rootThread != -1) {
        AWCOTN_ASSERT(GetThis() == this);
    } else {
        AWCOTN_ASSERT(GetThis() != this);
    }
    //AWCOTN_LOG_INFO(g_logger) << m_threadCount;


    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }
    
    if(m_rootFiber) {
        // while(!stopping()) {
        //     if(m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::EXCEPT) {
        //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //         AWCOTN_LOG_INFO(g_logger) << "root fiber is term, reset";
        //         t_scheduler_fiber = m_rootFiber.get();
        //     }
        //     m_rootFiber->call();
        // }
        if(!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
    AWCOTN_LOG_INFO(g_logger) << this << " stopped";
    // if(stopping()) {
    //     return;
    // }
    
}

void Scheduler::setThis() {
    t_scheduler = this;
}

/**
 * @brief 调度器运行函数，负责协程调度循环
 * @details 
 * 核心工作流程：
 * 1. 设置当前线程的调度器指针
 * 2. 创建专用的空闲协程(idle_fiber)
 * 3. 循环执行：
 *    - 从任务队列获取待执行的协程/回调
 *    - 如果有任务，则执行任务
 *    - 如果没有任务，则执行空闲协程
 * 
 * 空闲协程与主协程分离的必要性：
 * 1. 职责分离：主协程(m_rootFiber)负责调度循环，空闲协程专门处理空闲状态
 * 2. 不同行为模式：
 *    - 主协程需要保持运行以维持调度逻辑
 *    - 空闲协程可以频繁让出执行权并监听事件(特别是在IOManager中)
 * 3. 资源效率：
 *    - 空闲协程可以在没有任务时通过epoll_wait等监听IO事件
 *    - 主协程可以处理调度器的正常工作流
 * 4. 状态管理：
 *    - 让空闲协程单独处理idle状态，简化状态转换逻辑
 */
void Scheduler::run() {
    set_hook_enable(true);
    setThis();
    AWCOTN_LOG_INFO(g_logger) << "run";
    if(awcotn::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    // 创建专门的空闲协程，用于处理线程无任务可调度的情况
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread != awcotn::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }
                AWCOTN_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                AWCOTN_LOG_INFO(g_logger) << "find fiber=" << m_fibers.size();
                break;
            }
            tickle_me = tickle_me || !m_fibers.empty();
        }

        if(tickle_me) {
            tickle();
        }

        // 执行调度的协程(如果有)
        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            
            ft.fiber->swapIn();  // 切换到任务协程
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->setState (Fiber::HOLD);
            }
            ft.reset();
        } else if(ft.cb) {
            // 执行回调函数(如果有)
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            
            cb_fiber->swapIn(); 
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {
                cb_fiber->setState(Fiber::HOLD);
                cb_fiber.reset();
            }
        } else {
            // 没有任务时，执行空闲协程
            // 这里体现了空闲协程的重要性 - 在IOManager中会实现为epoll_wait等待IO事件
            if(idle_fiber->getState() == Fiber::TERM) {
                AWCOTN_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();  // 切换到空闲协程
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->setState(Fiber::HOLD);
            }
            --m_idleThreadCount;
        }
    }
}

void Scheduler::tickle() {
    AWCOTN_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

/**
 * @brief 空闲协程执行函数
 * @details 
 * 在基类Scheduler中，空闲协程只是简单地让出执行权，等待被再次调度
 * 在子类IOManager中，此函数被重写为调用epoll_wait等待IO事件
 * 
 * 空闲协程的价值：
 * 1. 在IOManager中转化为事件循环，等待IO事件发生
 * 2. 避免CPU空转，提高系统资源利用率
 * 3. 可以在没有任务时释放CPU资源给其他进程使用
 */
void Scheduler::idle() {
    AWCOTN_LOG_INFO(g_logger) << "idle";

    AWCOTN_LOG_INFO(g_logger) << m_autoStop << " - " << m_stopping << " - " << m_fibers.empty() << " - " << m_activeThreadCount;
    while(!stopping()) {
        awcotn::Fiber::YieldToHold();
    }
}



}