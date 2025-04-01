#include "awcotn/hook.h"
#include "awcotn/iomanager.h"
#include "awcotn/log.h"

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("root");
void test_sleep() {
    awcotn::IOManager iom(1);

    iom.schedule([]() {
        sleep(2);
        AWCOTN_LOG_INFO(g_logger) << "sleep 2 second end";
    });

    iom.schedule([]() {
        sleep(3);
        AWCOTN_LOG_INFO(g_logger) << "sleep 3 second end";
    });

    AWCOTN_LOG_INFO(g_logger) << "test_sleep begin";
    
}

int main() {
    test_sleep();
    return 0;
}