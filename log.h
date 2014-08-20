#ifndef UTILS_LOG_H
#define UTILS_LOG_H
/*
 *   General logging/info/warn/panic routines.
 */
#include <stdlib.h>
#include <stdio.h>

void initLog(const char* header = NULL, const char* file = NULL);
void __log_lock();
void __log_unlock();

#ifdef MT_SAFE_LOG
#define log_lock() __log_lock()
#define log_unlock() __log_unlock()
#else
#define log_lock()
#define log_unlock()
#endif

extern const char* logHeader;
extern FILE* logFdErr;
extern FILE* logFdOut;

#define PANIC_EXIT_CODE (112)

#define panic(...) \
{ \
    fprintf(logFdErr, "%sPanic on %s:%d: ", logHeader, __FILE__, __LINE__); \
    fprintf(logFdErr, __VA_ARGS__); \
    fprintf(logFdErr, "\n"); \
    fflush(logFdErr); \
    exit(PANIC_EXIT_CODE); \
}

#define warn(...) \
{ \
    log_lock(); \
    fprintf(logFdErr, "%sWARN: ", logHeader); \
    fprintf(logFdErr, __VA_ARGS__); \
    fprintf(logFdErr, "\n"); \
    fflush(logFdErr); \
    log_unlock(); \
}

#define info(...) \
{ \
    log_lock(); \
    fprintf(logFdOut, "%s", logHeader); \
    fprintf(logFdOut, __VA_ARGS__); \
    fprintf(logFdOut, "\n"); \
    fflush(logFdOut); \
    log_unlock(); \
}

#endif // UTILS_LOG_H
