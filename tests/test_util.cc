#include "awcotn/awcotn.h"
#include <assert.h>


awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();

void test_assert() {
    AWCOTN_LOG_INFO(g_logger) << awcotn::BacktraceToString(10);
    AWCOTN_ASSERT2(0, "test assert");

}
int main(int argc, char** argv) {
    test_assert();
    return 0;   
}