#include "common.h"

#include <stdarg.h>
#include <cstdio>
#include <ctime>


int log(const char *fmt, ...) {
    FILE *logFile = nullptr;

    logFile = fopen("tempserver.log", "a+");

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80] = {0};
    
    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S - ", timeinfo);
    fprintf(logFile, "%s", buffer);
    
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(logFile, fmt, args);
    fprintf(logFile, "\n");

    fclose(logFile);
    
    return result;
}
