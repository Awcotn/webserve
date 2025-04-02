#include "hook.h"
#include <dlfcn.h>
#include <cstdarg>
#include <sys/ioctl.h>
#include "iomanager.h"
#include "fd_manager.h"

awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

namespace awcotn {

static thread_local bool t_hook_enable = true;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt) \

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hook_init();
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

}

struct timer_info {
    int cancelled = 0;
};

/**
 * @brief 通用IO操作封装，处理非阻塞IO的调度
 * 
 * @param fd 文件描述符
 * @param fun 原始IO函数指针(如read/write/send/recv等)
 * @param func_name 函数名称(用于日志)
 * @param event 事件类型(读/写)
 * @param timeout_so 超时选项(SO_RCVTIMEO或SO_SNDTIMEO)
 * @param args 转发给原始IO函数的其他参数
 * @return 成功返回操作的字节数，失败返回-1并设置errno
 */
template<typename OrigFunc, typename... Args>
static size_t do_io(int fd, OrigFunc fun, const char* func_name, 
                    uint32_t event, int timeout_so, Args&&... args) {
    // 如果钩子未启用，直接调用原始函数
    if(!awcotn::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取文件描述符上下文，如不存在则直接调用原始函数
    awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(fd, false);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 检查文件描述符是否已关闭
    if(ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }

    // 如果是非socket且用户设置了非阻塞，直接调用原始函数
    if(!(ctx->isSocket()) && ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取超时时间
    uint64_t to = ctx->getTimeout(timeout_so);
    // 创建定时器信息对象，用于取消和超时处理
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    // 尝试执行IO操作
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    // 处理被信号中断的情况
    while(n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    
    // 处理IO操作会阻塞的情况
    if(n == -1 && errno == EAGAIN) {
        // 获取当前所在的IO管理器
        awcotn::IOManager* iom = awcotn::IOManager::GetThis();
        awcotn::Timer::ptr timer; 
        std::weak_ptr<timer_info> winfo(tinfo);
        
        // 如果有超时设置，创建条件定时器
        if(to != (uint64_t)-1) {
            timer = iom->addConditionTimer(to, [winfo, fd, event, iom]() {
                // 定时器回调：超时时取消事件并设置超时标志
                auto t = winfo.lock();
                if(!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (awcotn::IOManager::Event)event); 
            }, winfo);
        }

        // 添加IO事件到事件循环
        int rt = iom->addEvent(fd, (awcotn::IOManager::Event)event);
        if(rt) {
            // 添加事件失败，取消定时器并返回错误
            AWCOTN_LOG_ERROR(g_logger) << func_name << " addEvent( fd=" << fd << ", " << event << ") failed";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            // 让出当前协程执行权，等待IO事件或超时发生
            awcotn::Fiber::YieldToHold();
            // 恢复执行后取消定时器(如果有)
            if(timer) {
                timer->cancel();
            }
            // 检查是否因超时而恢复
            if(tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }

            // 事件触发，重新尝试IO操作
            goto retry;
        }
    }
    // 返回IO操作结果
    return n;
}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!awcotn::t_hook_enable) {
        return sleep_f(seconds);
    }
    awcotn::Fiber::ptr fiber = awcotn::Fiber::GetThis();
    awcotn::IOManager* iom = awcotn::IOManager::GetThis();
    
    iom->addTimer(seconds * 1000, std::bind((void(awcotn::Scheduler::*)
    (awcotn::Fiber::ptr, int thread))&awcotn::IOManager::schedule
    ,iom, fiber, -1));
    awcotn::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!awcotn::t_hook_enable) {
        return usleep_f(usec);
    }
    awcotn::Fiber::ptr fiber = awcotn::Fiber::GetThis();
    awcotn::IOManager* iom = awcotn::IOManager::GetThis();
    
    iom->addTimer(usec / 1000, std::bind((void(awcotn::Scheduler::*)
    (awcotn::Fiber::ptr, int thread))&awcotn::IOManager::schedule
    ,iom, fiber, -1));
    awcotn::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!awcotn::t_hook_enable) {
        return nanosleep_f(req, rem);
    }
    awcotn::Fiber::ptr fiber = awcotn::Fiber::GetThis();
    awcotn::IOManager* iom = awcotn::IOManager::GetThis();
    
    iom->addTimer(req->tv_sec * 1000 + req->tv_nsec / 1000000, std::bind((void(awcotn::Scheduler::*)
    (awcotn::Fiber::ptr, int thread))&awcotn::IOManager::schedule
    ,iom, fiber, -1));
    awcotn::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!awcotn::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        AWCOTN_LOG_ERROR(g_logger) << "socket(" << domain << ", " << type << ", " << protocol << ") error";
        return fd;
    }
    awcotn::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms) {
    
}

int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen){
    return connect_f(sockfd, addr, addrlen);
}

 int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen){
    int fd = do_io(sockfd, accept_f, "accept", awcotn::IOManager::READ, SO_RCVTIMEO,
                   addr, addrlen); 
    if(fd >= 0) {
        awcotn::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
 }

ssize_t read(int fd, void* buf, size_t count) {
    return do_io(fd, read_f, "read", awcotn::IOManager::READ, SO_RCVTIMEO,
                 buf, count);
}

ssize_t readv(int fd, const struct iovec* iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", awcotn::IOManager::READ, SO_RCVTIMEO,
                 iov, iovcnt);
}
ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", awcotn::IOManager::READ, SO_RCVTIMEO,
                 buf, len, flags);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                  struct sockaddr* src_addr, socklen_t* addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", awcotn::IOManager::READ, SO_RCVTIMEO,
                 buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr* msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", awcotn::IOManager::READ, SO_RCVTIMEO,
                 msg, flags);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return do_io(fd, write_f, "write", awcotn::IOManager::WRITE, SO_SNDTIMEO,
                 buf, count);
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", awcotn::IOManager::WRITE, SO_SNDTIMEO,
                 iov, iovcnt);
}

ssize_t send(int sockfd, const void* buf, size_t len, int flags) {
    return do_io(sockfd, send_f, "send", awcotn::IOManager::WRITE, SO_SNDTIMEO,
                 buf, len, flags);
}

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,const struct sockaddr* dest_addr, socklen_t addrlen) {
    return do_io(sockfd, sendto_f, "sendto", awcotn::IOManager::WRITE, SO_SNDTIMEO,
                 buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr* msg, int flags) {
    return do_io(sockfd, sendmsg_f, "sendmsg", awcotn::IOManager::WRITE, SO_SNDTIMEO,
                 msg, flags);
}

int close(int fd) {
    if(!awcotn::t_hook_enable) {
        return close_f(fd);
    }
    awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = awcotn::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        awcotn::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg); 
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void* arg = va_arg(va, void*);
    va_end(va);

    if(FIONBIO == request) {
        bool user_nonblock = !!*(int*)arg;
        awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(d);
        if(!ctx || ctx->isClosed() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if(!awcotn::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            awcotn::FdCtx::ptr ctx = awcotn::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}


}