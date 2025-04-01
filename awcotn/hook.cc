#include "hook.h"
#include <dlfcn.h>
#include "iomanager.h"



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