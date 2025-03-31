#ifndef __AWCOTN_TIMER_H__
#define __AWCOTN_TIMER_H__

#include <memory>
#include "thread.h"
#include <sys/epoll.h>
#include <functional>
#include <string>
#include <list>
#include <set>
#include <unistd.h>
#include <vector>

namespace awcotn {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    typedef Mutex MutexType;

    bool cancel();
    bool refresh();
    bool reset(uint64_t ms, bool from_now);

private:
    Timer(uint64_t ms, std::function<void()> cb, bool recurring = false, TimerManager* manager = nullptr);
    Timer(uint64_t next);

private:
    bool m_recurring = false; // 是否是循环定时器
    uint64_t m_ms; // 定时器的时间间隔
    uint64_t m_next; // 下次触发的时间
    TimerManager* m_manager; // 定时器管理器

    std::function<void()> m_cb; // 定时器到期时执行的回调函数
    
    bool m_cancelled; // 是否被取消
private:
    struct Comparator {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs);
    };
};

class TimerManager {
friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring = false);


    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()>>& cbs);
    bool hasTimer();

protected:
    virtual void onTimerInsertedAtFront() = 0;
    void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

private:
    bool delectClockRollover(uint64_t now_ms);

private:
    RWMutexType m_mutex; // 互斥锁，用于保护定时器列表的访问
    std::set<Timer::ptr,Timer::Comparator> m_timers; // 定时器列表
    bool m_tickled = false; // 是否被唤醒
    uint64_t m_previouseTime = 0; // 上次触发的时间

};

}  // namespace awcotn

#endif