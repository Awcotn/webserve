#include "awcotn/iomanager.h"
#include "awcotn/awcotn.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();

void test_fiber() {
    AWCOTN_LOG_INFO(g_logger) << "test_fiber";
}

void test1() {
    awcotn::IOManager iom;
    iom.schedule(&test_fiber);
}
int main() {
    test1();
    return 0;
}