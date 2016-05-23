/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_THREAD_H_
#define UTILS_THREAD_H_
/**
 * Wrappers for thread support routines,
 * including thread manipulation and synchronization primitives.
 *
 * Use c++11 primitives and routines.
 */
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "utils/exception.h"
#include "utils/log.h"

class _Barrier;
class _ThreadPool;

using thread_t = std::thread;
using lock_t = std::mutex;
using cond_t = std::condition_variable;
using bar_t = _Barrier;

using task_t = std::function<void()>;
using thread_pool_t = _ThreadPool;

/**
 * Threads.
 */
// Use constructor to create a thread and execute the function:
// template<class Function, class... Args>
// explicit thread(Function&& f, Args&&... args);

// Use member function join() to join.

/**
 * Mutexes.
 */
// Use member functions lock(), try_lock(), and unlock().

// General-purpose mutex ownership wrapper.
#define mutex_begin(uniq_lk, lk) \
    { std::unique_lock<lock_t> uniq_lk(lk);
#define mutex_end() \
    }

/**
 * Condition variables.
 */
// Use member functions to wait:
// void wait(std::unique_lock<std::mutex>& lock);
// template<class Predicate>
// void wait(std::unique_lock<std::mutex>& lock, Predicate pred);

// Use member functions to notify:
// void notify_one();
// void notify_all();

/**
 * Barrier.
 */
class _Barrier {
public:
    static constexpr int SERIAL_LAST_THREAD = 1;

    /**
     * Construct the barrier.
     *
     * @param threadCount   The number of threads involved in the barrier.
     */
    explicit _Barrier(const std::size_t threadCount)
            : threadCount_(threadCount), remain_(threadCount), barCount_(0) {
        // Nothing else to do.
    }

    /**
     * Wait on the barrier.
     *
     * @param onSerialPoint   A callback to be called at the barrier serial
     * point.
     * @return  SERIAL_LAST_THREAD if the thread is the last one arriving
     * at the barrier, or 0 otherwise.
     */
    int wait(const std::function<void(void)>& onSerialPoint = [](){}) {
        mutex_begin(lock_, mutex_);
        auto curBarCount = barCount_;
        remain_--;
        if (remain_) {
            // Not all threads have arrived, wait.
            // Wait predicate should not depend on \c remain_, since it must be
            // reset at the time all threads arriving at the barrier.
            cv_.wait(lock_, [this, &curBarCount]{
                    return barCount_ != curBarCount; });
            return 0;
        }
        mutex_end();
        // Only executed by the very last thread arriving at the barrier.
        // Reset \c remain_.
        remain_ = threadCount_;
        // Increase \c barCount_, which is used as the wait predicate.
        barCount_++;
        // Call callback function at serial point.
        if (onSerialPoint) onSerialPoint();
        // Notify all after updating wait predicate, should not hold the mutex.
        cv_.notify_all();
        return SERIAL_LAST_THREAD;
    }

private:
    const std::size_t threadCount_;
    std::mutex mutex_;
    std::condition_variable cv_;

    std::size_t remain_;
    std::size_t barCount_;
};

/**
 * Task queue for thread pool.
 */
class _TaskQueue {
public:
    _TaskQueue() : stop_(false) {}

    ~_TaskQueue() {}

    _TaskQueue(_TaskQueue&&) = default;

    _TaskQueue& operator=(_TaskQueue&&) = default;

    _TaskQueue(const _TaskQueue&) = delete;

    _TaskQueue& operator=(const _TaskQueue&) = delete;

    /**
     * Enqueue a task.
     */
    void enqueue(const task_t& task) {
        mutex_begin(uqlk, lock_);
        if (stop_) {
            throw PermissionException(
                    "TaskQueue: enqueue on stopped task queue!");
        }
        if (!task) {
            throw InvalidArgumentException(
                    "TaskQueue: enqueue empty task!");
        }
        queue_.push(task);
        mutex_end();
        enqueue_.notify_one();
    }

    /**
     * Dequeue a task. Blocked.
     */
    task_t dequeue() {
        mutex_begin(uqlk, lock_);
        enqueue_.wait(uqlk, [this]{ return !queue_.empty() || stop_; });
        if (stop_ && queue_.empty()) {
            // empty task
            return std::function<void()>();
        }
        auto task = queue_.front();
        queue_.pop();
        return task;
        mutex_end();
    }

    /**
     * Tear down the task queue.
     */
    void close() {
        mutex_begin(uqlk, lock_);
        stop_ = true;
        mutex_end();
        enqueue_.notify_all();
    }

private:
    std::queue<task_t> queue_;
    lock_t lock_;
    cond_t enqueue_;
    bool stop_;
};

/**
 * Thread pool.
 */
class _ThreadPool {
public:
    static const uint32_t INV_TID = -1u;

    explicit _ThreadPool(uint32_t threadCount)
            : threadCount_(threadCount), queues_(threadCount),
              taskCount_(0), curThreadIdx_(0) {
        for (uint32_t tid = 0; tid < threadCount_; tid++) {
            threads_.emplace_back(&_ThreadPool::thread_func, this, tid);
        }
    }

    ~_ThreadPool() {
        for (auto& q : queues_) {
            q.close();
        }
        for (auto& t : threads_) {
            t.join();
        }
    }

    _ThreadPool(_ThreadPool&&) = default;

    _ThreadPool& operator=(_ThreadPool&&) = default;

    _ThreadPool(const _ThreadPool&) = delete;

    _ThreadPool& operator=(const _ThreadPool&) = delete;

    void add_task(const task_t& task, uint32_t tid = INV_TID) {
        if (tid == INV_TID) {
            tid = next_tid();
        }

        mutex_begin(uqlk, taskLock_);
        taskCount_++;
        mutex_end();

        queues_[tid].enqueue(task);
    }

    void wait_all() {
        mutex_begin(uqlk, taskLock_);
        taskDone_.wait(uqlk, [this]{ return taskCount_ == 0; });
        mutex_end();
    }

private:
    const uint32_t threadCount_;
    std::vector<thread_t> threads_;
    std::vector<_TaskQueue> queues_;

    // Track number of pending tasks
    lock_t taskLock_;
    cond_t taskDone_;
    uint32_t taskCount_;

    // For task assignment
    uint32_t curThreadIdx_;

private:
    uint32_t next_tid() {
        return curThreadIdx_ = (curThreadIdx_ + 1) % threadCount_;
    }

    void thread_func(uint32_t tid) {
        while (true) {
            auto task = queues_[tid].dequeue();
            if (!task) return;

            task();

            mutex_begin(uqlk, taskLock_);
            assert(taskCount_ > 0);
            taskCount_--;
            mutex_end();
            taskDone_.notify_all();
        }
    }
};

#endif  // UTILS_THREAD_H_

