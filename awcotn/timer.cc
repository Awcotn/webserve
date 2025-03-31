#include "timer.h"
#include "util.h"
#include "log.h"

namespace awcotn {

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) {
    if(!lhs && !rhs) {
        return false;
    } else if(!lhs) {
        return true;
    } else if(!rhs) {
        return false;
    }
    if(lhs->m_next < rhs->m_next) {
        return true;
    } else if(lhs->m_next == rhs->m_next) {
        return lhs.get() < rhs.get();
    }
    return false;
}
    
Timer::Timer(uint64_t ms, std::function<void()> cb, 
             bool recurring, TimerManager* manager) 
    : m_recurring(recurring)
    , m_ms(ms)
    , m_next(ms + GetCurrentMS())
    , m_manager(manager)
    , m_cb(cb){
    
}

Timer::Timer(uint64_t next)
    : m_next(next) {
}

bool Timer::cancel() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(m_cb) {
        m_cb = nullptr;
        auto it = m_manager->m_timers.find(shared_from_this());
        m_manager->m_timers.erase(it);
        return true;
    }
    return false;
}

bool Timer::refresh() {
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);
    m_next = GetCurrentMS() + m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}

// 这里的from_now参数表示是否从当前时间开始计算新的时间间隔
// 如果from_now为true，则从当前时间开始计算新的时间间隔
bool Timer::reset(uint64_t ms, bool from_now) {
    // 如果定时器的时间间隔没有变化，直接返回
    if(ms == m_ms && !from_now) {
        return false;
    }
    TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
    if(!m_cb) {
        return false;
    }
    auto it = m_manager->m_timers.find(shared_from_this());
    if(it == m_manager->m_timers.end()) {
        return false;
    }
    m_manager->m_timers.erase(it);

    // 计算新的触发时间
    uint64_t start;
    if(from_now) {
        start = awcotn::GetCurrentMS();
    } else {
        start = m_next - m_ms; 
    }
    m_ms = ms;
    m_next = start + m_ms;
    m_manager->addTimer(shared_from_this(),lock);
    return true;
}

TimerManager::TimerManager() {
    m_previouseTime = GetCurrentMS();
}
TimerManager::~TimerManager() {

}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
    Timer::ptr timer(new Timer(ms, cb, recurring, this));
    RWMutexType::WriteLock lock(m_mutex);
    addTimer(timer, lock);
    // 如果不是最前面的定时器，可能需要重新设置定时器的触发时间


    return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if(tmp) {
        cb();
    }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recurring) {
    return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    m_tickled = false;
    if(m_timers.empty()) {
        return ~0ull;
    }
    const Timer::ptr& next = *m_timers.begin();
    uint64_t now_ms = GetCurrentMS();
    if(now_ms >= next->m_next) {
        return 0;
    }
    return next->m_next - now_ms;
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs) {
    uint64_t now_ms = GetCurrentMS();
    std::vector<Timer::ptr> expired;
    {
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()) {
            return;
        }
    }
    RWMutexType::WriteLock lock(m_mutex);

    // 检查是否发生了时钟回拨
    // 如果发生了时钟回拨，则将所有定时器的触发时间都设置为当前时间
    bool rollover = delectClockRollover(now_ms);
    if(!rollover && (*m_timers.begin())->m_next < now_ms) {
        return;
    }
    
    Timer::ptr now_timer(new Timer(now_ms));
    auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
    while(it != m_timers.end() && (*it)->m_next == now_ms) {
        ++it;
    }
    expired.insert(expired.begin(), m_timers.begin(), it);
    m_timers.erase(m_timers.begin(), it); 
    cbs.reserve(expired.size());
    for(auto& i : expired) {
        cbs.push_back(i->m_cb);
        if(i->m_recurring) {
            i->m_next = now_ms + i->m_ms;
            m_timers.insert(i);
        } else {
            i->m_cb = nullptr;
        }
    }

}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
    auto it = m_timers.insert(val).first;
    bool at_front = (it == m_timers.begin() && !m_tickled);
    if(at_front) {
        m_tickled = true;
    }
    lock.unlock();

    if(at_front) {
        // 如果是最前面的定时器，触发定时器
        // 这里可以添加代码来触发定时器，比如使用epoll等
        onTimerInsertedAtFront();
    }
}

bool TimerManager::delectClockRollover(uint64_t now_ms) {
    bool rollover = false;
    if(now_ms < m_previouseTime && now_ms < (m_previouseTime - 60 * 60 * 1000)) {
        rollover = true;

    }
    m_previouseTime = now_ms;
    return rollover;
}

bool TimerManager::hasTimer() {
    RWMutexType::ReadLock lock(m_mutex);
    return !m_timers.empty();
}


}