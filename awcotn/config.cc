#include "config.h"

namespace awcotn
{

// 存储配置变量的静态映射表

// 根据配置名称查找配置项
// @param name 配置项名称
// @return 返回配置项的基类指针，如果未找到则返回nullptr
ConfigVarBase::ptr Config::LookupBase(const std::string &name)
{
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

// 递归遍历YAML节点，收集所有配置项
// 例如处理 "A.B", 10 这样的配置项，将转换为树形结构：
// A:
//   B: 10
// @param prefix 当前节点的前缀路径
// @param node 当前处理的YAML节点
// @param output 收集的结果列表，包含完整路径和节点值
static void ListAllMember(const std::string &prefix,
                            const YAML::Node &node,
                            std::list<std::pair<std::string, const YAML::Node>> &output)
{ 
    // 检查配置项名称是否合法（只允许小写字母、数字、点和下划线）
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
    {
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "Config invalid name: " << prefix << ":" << node;
        return;
    }
    // 将当前节点添加到结果列表
    output.push_back(std::make_pair(prefix, node));

    // 如果当前节点是Map类型，递归处理其子节点
    if (node.IsMap())
    {
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            // 构建子节点的完整路径，如果prefix为空，直接使用当前键名，否则用点号连接
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
        }
    }
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb)
{
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap &datas = GetDatas();
    for (auto it = datas.begin(); it != datas.end(); ++it)
    {
        cb(it->second);
    }
}  

// 从YAML配置文件中加载配置
// @param root YAML根节点
void Config::LoadFromYaml(const YAML::Node &root)
{
    // 存储所有配置节点的列表
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    // 收集所有配置项
    ListAllMember("", root, all_nodes);

    // 遍历所有配置节点
    for(auto& i : all_nodes) {
        std::string key = i.first;
        if(key.empty()) {
            continue;
        }
        // 将配置项名称转换为小写
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        // 查找配置项
        ConfigVarBase::ptr var = Config::LookupBase(key);
        if(var) {
            // 如果是标量类型（如字符串、数字等），直接转换
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
            } else {
                // 如果是复杂类型，先转换为字符串
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
            }
        }
    }
}

}