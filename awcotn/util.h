#ifndef __AWCOTN_UTIL_H__
#define __AWCOTN_UTIL_H__

#include <sys/types.h>
#include <cstdint>

namespace awcotn {

pid_t GetThreadId();
uint32_t GetFiberId();

}

#endif