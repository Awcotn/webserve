#include "awcotn/config.h"
#include "awcotn/log.h"

awcotn::ConfigVar<int>::ptr g_int_value_config = 
    awcotn::Config::Lookup("system.port", (int)8080, "system port");
awcotn::ConfigVar<float>::ptr g_float_value_config = 
    awcotn::Config::Lookup("system.value", (float)10.2f, "system value");

int main(int argc, char** argv) {
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_int_value_config->getValue();
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_float_value_config->toString();
    return 0;
}