#include "awcotn/config.h"
#include "awcotn/log.h"

awcotn::ConfigVar<int>::ptr g_int_value_config = 
    awcotn::Config::Lookup("system.port", (int)8080, "system port");

int main(int argc, char** argv) {
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_int_value_config->getValue();
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_int_value_config->toString();
    return 0;
}