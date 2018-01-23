#include "common.h"

#include <stdargs.h>
#include <stdlib.h>

int log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = printf(fmt"\n", args);
    return result;
}
