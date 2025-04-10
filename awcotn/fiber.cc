#include "fiber.h"
#include "macro.h"
#include "config.h"
#include <atomic>
#include "log.h"
#include "scheduler.h"

namespace awcotn {

static Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MalloStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        free(vp);
    }
};

using StackAllocator = MalloStackAllocator;

uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}


Fiber::Fiber() 
    : m_id(s_fiber_id++) {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        AWCOTN_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;
    

    AWCOTN_LOG_DEBUG(g_logger) << "Fiber::Fiber main id=" << m_id;   
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    : m_id(s_fiber_id++)
    , m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        AWCOTN_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    
    AWCOTN_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;  
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        AWCOTN_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        AWCOTN_ASSERT(!m_cb);
        AWCOTN_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id << m_state;
        AWCOTN_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    AWCOTN_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}

//重置协程
void Fiber::reset(std::function<void()> cb) {
    AWCOTN_ASSERT(m_stack);
    AWCOTN_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        AWCOTN_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

//切换到当前协程执行
void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    AWCOTN_LOG_ERROR(g_logger) << getId();
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        AWCOTN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        AWCOTN_ASSERT2(false, "swapcontext");
    }
}

/**
 * @brief 将当前协程切换为运行状态并执行
 * 
 * 该方法作用:
 * 1. 设置当前正在运行的协程为this
 * 2. 确保协程不在EXEC状态(防止重复切换)
 * 3. 将协程状态设为EXEC(执行中)
 * 4. 保存调度器主协程上下文，并切换到当前协程上下文
 * 5. 当协程让出CPU(YieldToReady/YieldToHold)或结束时，会从这里返回到调度器
 * 
 * 调用swapIn后，协程会从上次中断的地方继续执行，包括:
 * - 首次执行时从MainFunc/CallerMainFunc开始
 * - 非首次执行时从上次YieldToReady/YieldToHold的下一条指令继续
 */
void Fiber::swapIn() {
    SetThis(this);
    AWCOTN_ASSERT(m_state != EXEC);
    m_state = EXEC;
    // 保存调度器主协程上下文到Scheduler::GetMainFiber()->m_ctx
    // 并将当前上下文切换为this协程的m_ctx
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        AWCOTN_ASSERT2(false, "swapcontext");
    }
}

/**
 * @brief 将当前协程切换到后台，切回调度器主协程
 * 
 * 该方法作用:
 * 1. 设置当前正在运行的协程为调度器主协程
 * 2. 保存当前协程上下文，并切换回调器主协程上下文
 * 3. 切换后当前协程会挂起，等待下次被swapIn唤醒
 * 
 * 注意: 协程状态(READY/HOLD)的设置需要在调用swapOut前完成
 */
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
    // 保存当前协程上下文到m_ctx
    // 并恢复调度器主协程的上下文继续执行
    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        AWCOTN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

//返回当前协程
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    AWCOTN_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}
//切换到后台并Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}
//切换到后台并Hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    AWCOTN_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& e) {
        cur->m_state = EXCEPT;
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "Fiber Except: " << e.what()
                                            << " fiber_id=" << cur->getId()
                                            << std::endl
                                            << awcotn::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "Fiber Except"
                                            << " fiber_id=" << cur->getId()
                                            << std::endl
                                            << awcotn::BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    AWCOTN_ASSERT2(false, "never reach fiber id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    AWCOTN_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& e) {
        cur->m_state = EXCEPT;
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "Fiber Except: " << e.what()
                                            << " fiber_id=" << cur->getId()
                                            << std::endl
                                            << awcotn::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEPT;
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "Fiber Except"
                                            << " fiber_id=" << cur->getId()
                                            << std::endl
                                            << awcotn::BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();

    AWCOTN_ASSERT2(false, "never reach fiber id=" + std::to_string(raw_ptr->getId()));
}

}