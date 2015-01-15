#ifndef UTILS_THREAD_H
#define UTILS_THREAD_H
/*
 *   Wrappers for pthreads.
 */
#include <cstddef>
#include <pthread.h>
#include "log.h"

typedef pthread_t thread_t;

inline thread_t create_thread(void* (*func)(void*), void* arg) {
    thread_t tid;
    int res = pthread_create(&tid, NULL, func, arg);
    if (res != 0) {
        panic("Failed call to pthread_create: %d.", res);
    }
    return tid;
}

inline void* join_thread(thread_t tid) {
    void* ret;
    int res = pthread_join(tid, &ret);
    if (res != 0) {
        panic("Failed call to pthread_join: %d.", res);
    }
    return ret;
}

#endif // UTILS_THREAD_H

