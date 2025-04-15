#include "sync_call.h"
#include "log.h"
#include "iomanager.h"

namespace awcotn {

namespace test {

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("test");

/**
 * @brief 测试SyncCall解决协程间时序问题
 * @details 演示如何解决B协程先于A协程挂起就完成的情况
 */
void test_sync_call() {
    AWCOTN_LOG_INFO(g_logger) << "测试开始: 协程同步调用问题";
    
    // 创建同步调用对象
    SyncCall::ptr sc(new SyncCall());
    
    // 模拟调用者协程A
    auto task_a = []() {
        AWCOTN_LOG_INFO(g_logger) << "协程A: 开始执行";
        
        // 创建同步调用对象
        SyncCall::ptr sc(new SyncCall());
        
        // 模拟一些前置处理
        AWCOTN_LOG_INFO(g_logger) << "协程A: 准备调用协程B";
        
        // 调用协程B并等待结果
        sc->call([]() {
            AWCOTN_LOG_INFO(g_logger) << "协程B: 开始执行";
            AWCOTN_LOG_INFO(g_logger) << "协程B: 任务完成";
            // B协程很快就完成了任务
        });
        
        AWCOTN_LOG_INFO(g_logger) << "协程A: 协程B已完成，继续执行";
        AWCOTN_LOG_INFO(g_logger) << "协程A: 任务完成";
    };
    
    // 创建并调度协程A
    IOManager::GetThis()->schedule(task_a);
    
    // 模拟B快速完成的极端情况
    auto task_extreme = []() {
        AWCOTN_LOG_INFO(g_logger) << "极端情况测试: 开始";
        
        // 创建同步调用对象
        SyncCall::ptr sc(new SyncCall());
        
        // 调用极快完成的协程
        sc->call([]() {
            // 目标协程非常快就完成了，甚至在调用者准备挂起之前
            AWCOTN_LOG_INFO(g_logger) << "极速协程: 立即完成";
        });
        
        AWCOTN_LOG_INFO(g_logger) << "极端情况测试: 调用已完成，无需挂起";
    };
    
    // 调度极端情况测试
    IOManager::GetThis()->schedule(task_extreme);
    
    AWCOTN_LOG_INFO(g_logger) << "测试添加完成";
}

/**
 * @brief 比较使用不同调用方式的示例
 * @details 演示标准call/back模式、调度器模式和SyncCall模式的区别
 */
void compare_call_methods() {
    AWCOTN_LOG_INFO(g_logger) << "比较不同协程调用方式:";
    
    // 1. 直接call/back调用模式 (同步阻塞)
    auto call_back_test = []() {
        AWCOTN_LOG_INFO(g_logger) << "1. call/back模式: 开始";
        
        // 创建协程但不执行
        Fiber::ptr fiber_b = std::make_shared<Fiber>([]() {
            AWCOTN_LOG_INFO(g_logger) << "call/back - 协程B: 执行";
            AWCOTN_LOG_INFO(g_logger) << "call/back - 协程B: 完成";
            // 此处会自动back()返回A
        });
        
        AWCOTN_LOG_INFO(g_logger) << "call/back - 协程A: 调用协程B";
        fiber_b->call();  // A阻塞直到B完成
        
        AWCOTN_LOG_INFO(g_logger) << "call/back - 协程A: B已返回，继续执行";
        
        // 但这种方式无法处理B提前完成的情况
    };
    
    // 2. 调度器schedule模式 (完全异步)
    auto schedule_test = []() {
        AWCOTN_LOG_INFO(g_logger) << "2. schedule模式: 开始";
        
        // 创建任务
        auto task_b = []() {
            AWCOTN_LOG_INFO(g_logger) << "schedule - 协程B: 执行";
            AWCOTN_LOG_INFO(g_logger) << "schedule - 协程B: 完成";
            // 这里无法直接通知A协程
        };
        
        AWCOTN_LOG_INFO(g_logger) << "schedule - 协程A: 调度协程B";
        IOManager::GetThis()->schedule(task_b);
        
        AWCOTN_LOG_INFO(g_logger) << "schedule - 协程A: 已调度B，继续执行";
        // A无法知道B何时完成，需要额外同步机制
    };
    
    // 3. SyncCall模式 (同步等待，但解决了时序问题)
    auto sync_call_test = []() {
        AWCOTN_LOG_INFO(g_logger) << "3. SyncCall模式: 开始";
        
        // 创建同步调用对象
        SyncCall::ptr sc(new SyncCall());
        
        AWCOTN_LOG_INFO(g_logger) << "SyncCall - 协程A: 调用协程B";
        
        // 调用B并等待完成
        sc->call([]() {
            AWCOTN_LOG_INFO(g_logger) << "SyncCall - 协程B: 执行";
            AWCOTN_LOG_INFO(g_logger) << "SyncCall - 协程B: 完成";
        });
        
        AWCOTN_LOG_INFO(g_logger) << "SyncCall - 协程A: B已完成，继续执行";
        // 即使B先完成，A也能正确处理
    };
    
    // 调度三种测试
    IOManager::GetThis()->schedule(call_back_test);
    IOManager::GetThis()->schedule(schedule_test);
    IOManager::GetThis()->schedule(sync_call_test);
}

} // namespace test

} // namespace awcotn