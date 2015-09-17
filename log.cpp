#include "log.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "threads.h"

FILE* logFdErr = stderr;
FILE* logFdOut = stdout;
const char* logHeader = "";

static lock_t log_printLock;


void initLog(const char* header, const char* file) {
    char* logHdr = (char*)calloc(strlen(header) + 1, sizeof(char));
    strcpy(logHdr, header);
    logHeader = logHdr;

    if (file) {
        FILE* fd = fopen(file, "a");
        if (fd == NULL) {
            perror("fopen() failed");
            // We can panic in InitLog (will dump to stderr)
            panic("Could not open logfile %s", file);
        }
        logFdOut = fd;
        logFdErr = fd;
        //NOTE: We technically never close this fd, but always flush it
    }
}

void __log_lock() {
    log_printLock.lock();
}

void __log_unlock() {
    log_printLock.unlock();
}

