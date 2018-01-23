#include "common.h"

#include <stdarg.h>
#include <cstdio>

int log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = printf(fmt, args);
    printf("\n");
    return result;
}
