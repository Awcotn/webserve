#include "awcotn/config.h"
#include "awcotn/log.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

#if 1
awcotn::ConfigVar<int>::ptr g_int_value_config = 
    awcotn::Config::Lookup("system.port", (int)8080, "system port");
awcotn::ConfigVar<float>::ptr g_int_valuex_config = 
    awcotn::Config::Lookup("system.port", (float)8080, "system port");

awcotn::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config = 
    awcotn::Config::Lookup("system.int_vec", std::vector<int>{1,2}, "system int vec");

awcotn::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
    awcotn::Config::Lookup("system.int_list", std::list<int>{1,2}, "system int list");

awcotn::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
    awcotn::Config::Lookup("system.int_set", std::set<int>{1,2}, "system int set");

awcotn::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
    awcotn::Config::Lookup("system.int_uset", std::unordered_set<int>{1,2}, "system int uset");

awcotn::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
    awcotn::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k",2}}, "system str int map");

awcotn::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
    awcotn::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k",2}}, "system str int map");

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
    //AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "before: " << g_float_value_config->getValue();
#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }


    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/awcotn/workspace/webserve/bin/conf/test.yml");
    awcotn::Config::LoadFromYaml(root);

    AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    //AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after: " << g_float_value_config->toString();

    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

#endif

class Person {
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;
    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age << "]";
        return ss.str();
    }
};

namespace awcotn {

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


}

// awcotn::ConfigVar<Person>::ptr g_person = 
//     awcotn::Config::Lookup("class.person", Person(), "system person");

void test_class() {
   // AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " <<g_person->toString();
    
    YAML::Node root = YAML::LoadFile("/home/awcotn/workspace/webserve/bin/conf/test.yml");
    awcotn::Config::LoadFromYaml(root);

    //AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " <<g_person->toString();
}

void test_log() {
    static awcotn::Logger::ptr system_log = AWCOTN_LOG_NAME("system");
    AWCOTN_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << awcotn::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/awcotn/workspace/webserve/bin/conf/log.yml");
    awcotn::Config::LoadFromYaml(root);
    std::cout << "==================" << std::endl;
    std::cout << awcotn::LoggerMgr::GetInstance()->toYamlString() << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << root << std::endl;
    AWCOTN_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    AWCOTN_LOG_INFO(system_log) << "hello system" << std::endl;
}

int main(int argc, char** argv) {
    // AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_int_value_config->getValue();
    // AWCOTN_LOG_INFO(AWCOTN_LOG_ROOT()) << g_float_value_config->toString();
    //test_yaml();
    //test_config();
    //test_class();
    test_log();
    return 0;
}