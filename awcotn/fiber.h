#ifndef __AWCOTN_FIBER_H__
#define __AWCOTN_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>
#include "thread.h"
#include "mutex.h"

namespace awcotn {

class Scheduler;
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

    void call();
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