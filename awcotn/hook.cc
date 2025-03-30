#include "hook.h"
#include <dlfcn.h>
#include "iomanager.h"

namespace awcotn {

static thread_local bool t_hook_enable = true;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \

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
    if(!iom || iom->isStopping()) {
        return sleep_f(seconds);
    }

    awcotn::IOManager::Timer::ptr timer(new awcotn::IOManager::Timer(seconds * 1000, nullptr, true));
    iom->addTimer(timer);
    fiber->yield();
    return 0;
}

typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;