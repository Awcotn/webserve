#ifndef __AWCOTN_FIBER_H__
#define __AWCOTN_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"
#include "mutex.h"

namespace awcotn {

/**
 * @brief 协程类
 * @details 封装了协程的创建、切换、让出和恢复等操作
 * 
 * 协程间调用流程说明:
 * 1. 父协程(A)调用子协程(B)的三种主要方式:
 *    a) 使用Scheduler调度: A将B添加到调度器队列，然后让出执行权
 *    b) 直接调用: A直接call() B，A被挂起直到B调用back()返回
 *    c) 使用channel等通信机制: A发送请求到channel，B消费并处理
 * 
 * 2. 直接调用流程 (call/back模式):
 *    - A调用B.call()，切换到B协程执行
 *    - A被挂起，保存上下文到t_threadFiber
 *    - B执行任务，完成后调用back()
 *    - B被挂起，A恢复执行
 *    - 适用于同步调用场景
 * 
 * 3. 调度器调用流程 (schedule模式):
 *    - A将B添加到调度器: scheduler->schedule(B)
 *    - A可以继续执行或让出CPU: Fiber::YieldToReady()
 *    - 调度器选择B执行: B.swapIn()
 *    - B完成后让出CPU: YieldToReady()/YieldToHold()
 *    - 调度器可能调度A或其他协程继续执行
 *    - 适用于异步调用场景
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        INIT, //初始化
        HOLD, //挂起
        EXEC, //执行
        TERM, //结束
        READY, //就绪
        EXCEPT //异常
    };
private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    //重置协程
    void reset(std::function<void()> cb);
    //切换到当前协程
    void swapIn();
    //切换到后台执行
    void swapOut();

    /**
     * @brief 将当前协程切换到运行状态
     * @details 
     * 协程调用方式1: 父协程调用子协程，父协程被挂起
     * 通常用于线程的主协程调用其他协程
     */
    void call();
    
    /**
     * @brief 将当前协程切换到后台
     * @details 
     * 与call()配对使用，子协程调用back()返回到父协程
     */
    void back();

    uint64_t getId() const { return m_id; }

    State getState() const { return m_state;}

    void setState(State s) { m_state = s;}

public:
    //设置当前协程
    static void SetThis(Fiber* f);
    //返回当前协程
    static Fiber::ptr GetThis();
    //切换到后台并Ready状态
    static void YieldToReady();
    //切换到后台并Hold状态
    static void YieldToHold();

    static uint64_t TotalFibers();

    static void MainFunc();

    static void CallerMainFunc();

    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint64_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;  
    void* m_stack = nullptr;

    std::function<void()> m_cb;
};

}


#endif