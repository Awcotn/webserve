#ifndef __AWCOTN_UTIL_H__
#define __AWCOTN_UTIL_H__

#include <vector>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

namespace awcotn {

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

//时间ms
uint64_t GetCurrentMS();
//时间us
uint64_t GetCurrentUS();

}

#endif