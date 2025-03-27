#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

namespace awcotn {

static awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    : Scheduler(threads, use_caller, name) {
    m_epfd = epoll_create(5000);
    AWCOTN_ASSERT(m_epfd > 0);   

    int rt = pipe(m_tickleFds);
    AWCOTN_ASSERT(rt);

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    AWCOTN_ASSERT(rt == 0);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);

    AWCOTN_ASSERT(rt);

    m_fdContexts.resize(32);
        
    start();

}

IOManager::~IOManager() {
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[1]);

    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);
    for(size_t i = 0; i < m_fdContexts.size(); ++i) {
        if(!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

//1 success, 0 retry. -1 error
int IOManager::addEvent(int fd, Event event, std::function<void()> cb = nullptr) {
    FdContext* fd_ctx = nullptr;
    RWMutexType::ReadLock lock(m_mutex);
    if(fd < m_fdContexts.size()) {
        fd_ctx = m_fdContexts[fd];
        lock.unlock();
    } else {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(m_fdContexts.size() * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);

    if(fd_ctx->events & event) {
        AWCOTN_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
            << " event=" << event
            << " fd_ctx.event=" << fd_ctx->events;
        AWCOTN_ASSERT(!(fd_ctx->events & event));
    }

}
bool IOManager::delEvent(int fd, Event event) {

}
bool IOManager::cancelEvent(int fd, Event event) {

}

bool IOManager::cancelAll(int fd) {

}

IOManager* IOManager::GetThis() {

}
}