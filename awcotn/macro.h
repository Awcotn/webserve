#ifndef __AWCOTN_MACRO_H__
#define __AWCOTN_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"

#define AWCOTN_ASSERT(x) \
    if(!(x)) { \
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << awcotn::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }
#define AWCOTN_ASSERT2(x, w) \
    if(!(x)) { \
        AWCOTN_LOG_ERROR(AWCOTN_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << awcotn::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }


#endif