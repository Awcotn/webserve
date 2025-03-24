#include "awcotn/awcotn.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

void run_in_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "run_in_fiber begin";
    awcotn::Fiber::YieldToHold();
    AWCOTN_LOG_INFO(g_logger) << "run_in_fiber end";
    awcotn::Fiber::YieldToHold();
}

void test_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "main begin -1";
    {
        AWCOTN_LOG_INFO(g_logger) << "main begin";
        awcotn::Fiber::GetThis();
        AWCOTN_LOG_INFO(g_logger) << "main begin";
        awcotn::Fiber::ptr fiber(new awcotn::Fiber(run_in_fiber));
        fiber->swapIn();
        AWCOTN_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        AWCOTN_LOG_INFO(g_logger) << "main end";
        fiber->swapIn();
    }
    AWCOTN_LOG_INFO(g_logger) << "main end";
}

int main(int argc, char** argv) {
    awcotn::Thread::SetName("main");
    AWCOTN_LOG_INFO(g_logger) << "main begin";

    std::vector<awcotn::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(awcotn::Thread::ptr(new awcotn::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    AWCOTN_LOG_INFO(g_logger) << "main end";
    return 0;
}