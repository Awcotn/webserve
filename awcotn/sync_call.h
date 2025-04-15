#ifndef __AWCOTN_SYNC_CALL_H__
#define __AWCOTN_SYNC_CALL_H__

#include <memory>
#include <mutex>
#include <functional>
#include "fiber.h"
#include "scheduler.h"

namespace awcotn {

/**
 * @brief 协程同步调用封装
 * @details 
 * 解决协程间调用时可能出现的时序问题，特别是被调用协程(B)在调用协程(A)
 * 挂起前就已完成的情况。
 * 
 * 主要解决问题：
 * 1. 信号丢失：确保B的完成信号不会丢失
 * 2. 避免死锁：防止A永久等待已完成的B
 * 3. 资源利用：如果B已完成，A无需挂起
 */
class SyncCall {
public:
    typedef std::shared_ptr<SyncCall> ptr;

    /**
     * @brief 构造函数
     */
    SyncCall() : m_called(false), m_done(false) {}

    /**
     * @brief 调用目标协程并等待完成
     * @param task 目标协程要执行的任务函数
     * @param scheduler 调度器，默认使用当前线程的调度器
     * 
     * 使用流程：
     * 1. 创建新协程执行task
     * 2. 标记已调用状态
     * 3. 如果任务已完成，直接返回
     * 4. 否则，挂起当前协程等待完成
     */
    void call(std::function<void()> task, Scheduler* scheduler = nullptr) {
        if(!scheduler) {
            scheduler = Scheduler::GetThis();
        }
        
        // 加锁检查和设置状态
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_called = true;
            
            // 如果任务已经完成，直接返回
            if(m_done) {
                return;
            }
            
            // 保存当前协程，用于后续唤醒
            m_caller = Fiber::GetThis();
        }
        
        // 创建新协程执行任务
        Fiber::ptr new_fiber = std::make_shared<Fiber>([this, task, scheduler]() {
            // 执行实际任务
            task();
            
            // 任务完成后处理
            std::lock_guard<std::mutex> lock(m_mutex);
            m_done = true;
            
            // 如果调用者已经设置了等待状态，则调度它继续执行
            if(m_called && m_caller) {
                scheduler->schedule(m_caller);
            }
        });
        
        // 调度新协程
        scheduler->schedule(new_fiber);
        
        // 让出执行权等待任务完成
        // 加锁再次检查状态，防止任务已经完成的情况
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            // 任务已完成，无需等待
            if(m_done) {
                return;
            }
        }
        
        // 挂起当前协程
        Fiber::YieldToHold();
    }
    
    /**
     * @brief 检查任务是否已完成
     */
    bool isDone() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_done;
    }

private:
    std::mutex m_mutex;           // 状态保护锁
    bool m_called;                // 是否已调用
    bool m_done;                  // 任务是否已完成
    Fiber::ptr m_caller;          // 调用者协程
};

}

#endif