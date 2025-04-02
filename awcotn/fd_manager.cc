#include "fd_manager.h"
#include <sys/stat.h>
#include "hook.h"


namespace awcotn {

FdCtx::FdCtx(int fd) 
    : m_isInit(false)
    , m_isSocket(false)
    , m_sysNonblock(false)
    , m_userNonblock(false)
    , m_isClosed(false)
    , m_fd(fd)
    , m_recvTimeout(-1)
    , m_sendTimeout(-1) {
    init();
}

FdCtx::~FdCtx() {
}

bool FdCtx::init() {
    if(m_isInit) {
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;
    struct stat fd_stat;
    if(fstat(m_fd, &fd_stat) == -1) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        m_isInit = true;
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    if(m_isSocket) {
        int flags = fcntl(m_fd, F_GETFL, 0);
        if(!(flags & O_NONBLOCK)) {
           fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
        } 
        m_sysNonblock = true;
    } else {
        m_sysNonblock = false;
    }

    m_userNonblock = false;
    m_isClosed = false;
    return m_isInit;
}

void FdCtx::setTimeout(int type, uint64_t v) {
    if(type == SO_RCVTIMEO) {
        m_recvTimeout = v;
    } else {
        m_sendTimeout = v;
    }
}

uint64_t FdCtx::getTimeout(int type) {
    if(type == SO_RCVTIMEO) {
        return m_recvTimeout;
    } else {
        return m_sendTimeout;
    }
}

FdManager::FdManager() {    
    m_datas.resize(64);
}

FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    RWMutexType::ReadLock lock(m_mutex);
    if(fd < (int)m_datas.size()) {
        if(m_datas[fd]) {
            return m_datas[fd];
        }
    }
    lock.unlock();

    if(auto_create) {
        FdCtx::ptr ctx(new FdCtx(fd));
        RWMutexType::WriteLock lock(m_mutex);
        if(fd >= (int)m_datas.size()) {
            m_datas.resize(fd * 1.5);
        }
        m_datas[fd] = ctx;
        return ctx;
    }
    return nullptr;    
}

void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if(fd < (int)m_datas.size()) {
        m_datas[fd].reset();
    }
}

}