#include "util.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace awcotn {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return 0;
}

}