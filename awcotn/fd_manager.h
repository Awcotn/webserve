#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <memory>
#include "thread.h"
#include "iomanager.h"
#include "singleton.h"

namespace awcotn {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    typedef Mutex MutexType;
    FdCtx(int fd);
    ~FdCtx();

    bool init();
    bool isinit() const { return m_isInit; }
    bool isSocket() const { return m_isSocket; }
    bool isClosed() const { return m_isClosed; }

    void setUserNonblock(bool v) { m_sysNonblock = v; }
    bool getUserNonblock() const { return m_sysNonblock; }

    void setSysNonblock(bool v) { m_sysNonblock = v; }
    bool getSysNonblock() const { return m_sysNonblock; }

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);
private:
    // 位域标志，表示文件描述符是否已初始化
    bool m_isInit: 1;
    // 位域标志，表示文件描述符是否是socket
    bool m_isSocket: 1;
    // 位域标志，表示文件描述符是否被系统设置为非阻塞模式
    bool m_sysNonblock: 1;
    // 位域标志，表示文件描述符是否被用户设置为非阻塞模式
    bool m_userNonblock: 1;
    // 位域标志，表示文件描述符是否已关闭
    bool m_isClosed: 1;
    // 文件描述符
    int m_fd;

    // 接收操作的超时时间（毫秒）
    uint64_t m_recvTimeout;
    // 发送操作的超时时间（毫秒）
    uint64_t m_sendTimeout;

    // 关联的IO管理器，用于处理异步IO事件
    awcotn::IOManager* m_iomanager;
};

/**
 * @brief 文件描述符管理器类
 * @details 统一管理文件描述符上下文，提供访问和创建FdCtx的能力
 */
class FdManager {   
public:
    // 定义读写锁类型，用于保护内部数据结构的线程安全
    typedef RWMutex RWMutexType;

    FdManager();
    ~FdManager();

    /**
     * @brief 获取文件描述符对应的上下文
     * @param[in] fd 文件描述符
     * @param[in] auto_create 是否自动创建，如果不存在
     * @return 返回对应文件描述符的上下文，如果不存在且auto_create=false，则返回nullptr
     */
    FdCtx::ptr get(int fd, bool auto_create = false);
    
    /**
     * @brief 删除文件描述符上下文
     * @param[in] fd 文件描述符
     */
    void del(int fd);

private:
    // 读写锁，保护数据访问的线程安全
    RWMutexType m_mutex;
    // 存储所有文件描述符上下文的容器，索引就是文件描述符值
    std::vector<FdCtx::ptr> m_datas;
};

/**
 * @brief 文件描述符管理器单例
 * @details 提供全局访问FdManager的单例模式
 */
typedef awcotn::Singleton<FdManager> FdMgr;
}

#endif