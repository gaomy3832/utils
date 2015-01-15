#ifndef UTILS_SYNC_H
#define UTILS_SYNC_H
/*
 *   Wrappers for pthread mutexes and condition variables.
 */
#include <pthread.h>
#include "log.h"

typedef pthread_mutex_t lock_t;
typedef pthread_cond_t cond_t;
typedef pthread_barrier_t bar_t;

/* Mutexes */
static inline void lock_init(lock_t* lock) {
    int res = pthread_mutex_init(lock, NULL);
    if (res != 0) {
        panic("Failed call to pthread_mutex_init: %d.", res);
    }
}

static inline void lock_destroy(lock_t* lock) {
    int res = pthread_mutex_destroy(lock);
    if (res != 0) {
        panic("Failed call to pthread_mutex_destroy: %d.", res);
    }
}

static inline void lock_acquire(lock_t* lock) {
    int res = pthread_mutex_lock(lock);
    if (res != 0) {
        panic("Failed call to pthread_mutex_lock: %d.", res);
    }
}

static inline void lock_release(lock_t* lock) {
    int res = pthread_mutex_unlock(lock);
    if (res != 0) {
        panic("Failed call to pthread_mutex_unlock: %d.", res);
    }
}


/* Condition variables */
static inline void cond_init(cond_t* cond) {
    int res = pthread_cond_init(cond, NULL);
    if (res != 0) {
        panic("Failed call to pthread_cond_init: %d.", res);
    }
}

static inline void cond_destroy(cond_t* cond) {
    int res = pthread_cond_destroy(cond);
    if (res != 0) {
        panic("Failed call to pthread_cond_destroy: %d.", res);
    }
}

static inline void cond_wait(cond_t* cond, lock_t* lock) {
    int res = pthread_cond_wait(cond, lock);
    if (res != 0) {
        panic("Failed call to pthread_cond_wait: %d.", res);
    }
}

static inline void cond_signal(cond_t* cond) {
    int res = pthread_cond_signal(cond);
    if (res != 0) {
        panic("Failed call to pthread_cond_signal: %d.", res);
    }
}

static inline void cond_broadcast(cond_t* cond) {
    int res = pthread_cond_broadcast(cond);
    if (res != 0) {
        panic("Failed call to pthread_cond_broadcast: %d.", res);
    }
}


/* Barriers */
static inline void bar_init(bar_t* bar, unsigned count) {
    int res = pthread_barrier_init(bar, NULL, count);
    if (res != 0) {
        panic("Failed call to pthread_barrier_init: %d.", res);
    }
}

static inline void bar_destroy(bar_t* bar) {
    int res = pthread_barrier_destroy(bar);
    if (res != 0) {
        panic("Failed call to pthread_barrier_destroy: %d.", res);
    }
}

static inline int bar_wait(bar_t* bar) {
    int res = pthread_barrier_wait(bar);
    if (res != 0 && res != PTHREAD_BARRIER_SERIAL_THREAD) {
        panic("Failed call to pthread_barrier_init: %d.", res);
    }
    return res == PTHREAD_BARRIER_SERIAL_THREAD ? 1 : 0;
}


#endif // UTILS_SYNC_H

