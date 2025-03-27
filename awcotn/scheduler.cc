#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "thread.h"
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

void Scheduler::run() {
    setThis();
    AWCOTN_LOG_INFO(g_logger) << "run";
    if(awcotn::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

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
                
                break;
            }
            tickle_me = tickle_me || !m_fibers.empty();
        }

        if(tickle_me) {
            tickle();
        }

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->setState (Fiber::HOLD);
            }
            ft.reset();
        } else if(ft.cb) {
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
            if(idle_fiber->getState() == Fiber::TERM) {
                AWCOTN_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
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

void Scheduler::idle() {
    AWCOTN_LOG_INFO(g_logger) << "idle";

    AWCOTN_LOG_INFO(g_logger) << m_autoStop << " - " << m_stopping << " - " << m_fibers.empty() << " - " << m_activeThreadCount;
    while(!stopping()) {
        awcotn::Fiber::YieldToHold();
    }
}



} 