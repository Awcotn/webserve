#include "hook.h"
#include <dlfcn.h>
#include "iomanager.h"
#include "macro.h"
#include "fd_manager.h"


namespace awcotn {

awcotn::Logger::ptr g_logger = AWCOTN_LOG_NAME("system");

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
        awcotn::Timmer::ptr timer; 
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



typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;
}