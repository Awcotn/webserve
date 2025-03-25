#include "awcotn/awcotn.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();

void test_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "test in fiber";
}

int main() {
    awcotn::Scheduler sc;
    sc.schedule(&test_fiber);
    sc.start();
    
    sc.stop();
    AWCOTN_LOG_INFO(g_logger) << "over";
    return 0;
}