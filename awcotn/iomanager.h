#ifndef __AWCOTN_IOMANAGER_H__
#define __AWCOTN_IOMANAGER_H__
#include "scheduler.h"
#include "timer.h"

namespace awcotn {

class IOManager : public Scheduler, public TimerManager {    
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;
    
    enum Event {
        NONE  = 0x0,
        READ  = 0x1,
        WRITE = 0x4
    };

    /**
     * @brief 用于协程调用处理的回调信息结构
     */
    struct CallBackInfo {
        Fiber::ptr caller;      // 调用者协程
        Fiber::ptr callee;      // 被调用协程
        bool is_callee_done;    // 被调用协程是否完成
        Scheduler* scheduler;   // 关联的调度器

        CallBackInfo() : is_callee_done(false), scheduler(nullptr) {}
    };

private:
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext {
            Scheduler* scheduler = nullptr; //事件执行的调度器
            Fiber::ptr fiber;               //事件协程
            std::function<void()> cb;       //事件的回调函数
        };
        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);


        EventContext read;
        EventContext write;
        int fd;
        Event events = NONE;
        MutexType mutex;
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager() noexcept override;

    //0 success. -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);

    static IOManager* GetThis();
    
    /**
     * @brief 协程间调用，处理调用者与被调用者的同步问题
     * @param callee 被调用协程
     * @return 返回被调用协程
     * @details 
     *   1. 如果callee已完成，直接返回
     *   2. 如果callee未完成，挂起caller，等待callee完成
     *   3. 防止caller尚未挂起，而callee已返回的情况
     */
    Fiber::ptr call(Fiber::ptr callee);

protected:
    void tickle() override;
    bool stopping() override;
    bool stopping(uint64_t timeout);
    void idle() override;
    void onTimerInsertedAtFront() override;

    void contextResize(size_t size);

private:
    int m_epfd = 0;
    int m_tickleFds[2];

    std::atomic<size_t> m_pendingEventCount = {0};  //待处理事件数量
    RWMutexType m_mutex;
    std::vector<FdContext*> m_fdContexts;
};

}

#endif