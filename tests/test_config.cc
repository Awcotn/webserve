#include "awcotn/config.h"
#include "awcotn/log.h"
#include <yaml-cpp/yaml.h>

awcotn::ConfigVar<int>::ptr g_int_value_config = 
    awcotn::Config::Lookup("system.port", (int)8080, "system port");
awcotn::ConfigVar<float>::ptr g_float_value_config = 
    awcotn::Config::Lookup("system.value", (float)10.2f, "system value");

awcotn::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    awcotn::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
} 
void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/awcotn/workspace/webserve/bin/conf/log.yml");
    //AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << root;

    print_yaml(root, 0);

}

void test_config() {
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "before: " << g_float_value_config->getValue();
    auto v = g_int_vec_value_config->getValue();
    for (auto& i : v) {
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "before int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("/home/awcotn/workspace/webserve/bin/conf/log.yml");
    awcotn::Config::LoadFromYaml(root);


    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after: " << g_float_value_config->getValue();

    v = g_int_vec_value_config->getValue();
    for (auto& i : v) {
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after int_vec: " << i;
    }

}

int main(int argc, char** argv) {
    // AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_int_value_config->getValue();
    // AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_float_value_config->toString();
    //test_yaml();
    test_config();
    return 0;
}