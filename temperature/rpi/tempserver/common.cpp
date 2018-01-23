#include "common.h"

#include <stdarg.h>
#include <cstdio>


int log(const char *fmt, ...) {
    FILE *logFile = nullptr;

    logFile = fopen("tempserver.log", "a+");
    
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(logFile, fmt, args);
    fprintf(logFile, "\n");

    fclose(logFile);
    
    return result;
}
