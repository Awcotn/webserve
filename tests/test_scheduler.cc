#include "awcotn/awcotn.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();

void test_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "test in fiber";
    
    static int s_count = 5;
    sleep(1);
    if(--s_count >= 0) {
        AWCOTN_LOG_INFO(g_logger) << "test in fiber s_count = " << s_count;
        awcotn::Scheduler::GetThis()->schedule(&test_fiber);
    }
}

int main() {
    awcotn::Scheduler sc(3,false,"test");
    
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    AWCOTN_LOG_INFO(g_logger) << "over";
    return 0;
}