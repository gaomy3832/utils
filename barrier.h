#ifndef UTILS_BARRIER_H
#define UTILS_BARRIER_H
/*
 *   Wrappers for pthread barrier.
 */
#include <pthread.h>
#include "log.h"

typedef pthread_barrier_t bar_t;


/* Barriers */
inline void bar_init(bar_t& bar, unsigned count) {
    int res = pthread_barrier_init(&bar, NULL, count);
    if (res != 0) {
        panic("Failed call to pthread_barrier_init: %d.", res);
    }
}

inline void bar_destroy(bar_t& bar) {
    int res = pthread_barrier_destroy(&bar);
    if (res != 0) {
        panic("Failed call to pthread_barrier_destroy: %d.", res);
    }
}

inline int bar_wait(bar_t& bar) {
    int res = pthread_barrier_wait(&bar);
    if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD) {
        panic("Failed call to pthread_barrier_init: %d.", res);
    }
    return res == PTHREAD_BARRIER_SERIAL_THREAD ? 1 : 0;
}


#endif // UTILS_BARRIER_H
