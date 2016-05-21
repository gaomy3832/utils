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
    explicit _Barrier(const std::size_t _threadCount)
            : threadCount(_threadCount), remain(_threadCount), barCount(0) {
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
        mutex_begin(lock, mutex);
        auto curBarCount = barCount;
        remain--;
        if (remain) {
            // Not all threads have arrived, wait.
            // Wait predicate should not depend on \c remain, since it must be
            // reset at the time all threads arriving at the barrier.
            cv.wait(lock, [this, &curBarCount]{
                    return barCount != curBarCount; });
            return 0;
        }
        mutex_end();
        // Only executed by the very last thread arriving at the barrier.
        // Reset \c remain.
        remain = threadCount;
        // Increase \c barCount, which is used as the wait predicate.
        barCount++;
        // Call callback function at serial point.
        if (onSerialPoint) onSerialPoint();
        // Notify all after updating wait predicate, should not hold the mutex.
        cv.notify_all();
        return SERIAL_LAST_THREAD;
    }

private:
    const std::size_t threadCount;
    std::mutex mutex;
    std::condition_variable cv;

    std::size_t remain;
    std::size_t barCount;
};

/**
 * Task queue for thread pool.
 */
class _TaskQueue {
public:
    _TaskQueue() : stop(false) {}

    ~_TaskQueue() {}

    _TaskQueue(_TaskQueue&&) = default;

    _TaskQueue& operator=(_TaskQueue&&) = default;

    _TaskQueue(const _TaskQueue&) = delete;

    _TaskQueue& operator=(const _TaskQueue&) = delete;

    /**
     * Enqueue a task.
     */
    void enqueue(const task_t& task) {
        mutex_begin(uqlk, lock);
        if (stop) {
            throw std::PermissionException(
                    "TaskQueue: enqueue on stopped task queue!");
        }
        if (!task) {
            throw std::InvalidArgumentException(
                    "TaskQueue: enqueue empty task!");
        }
        queue.push(task);
        mutex_end();
        enqueue.notify_one();
    }

    /**
     * Dequeue a task. Blocked.
     */
    task_t dequeue() {
        mutex_begin(uqlk, lock);
        enqueue.wait(uqlk, [this]{ return !queue.empty() || stop; });
        if (stop && queue.empty()) {
            // empty task
            return std::function<void()>();
        }
        auto task = queue.front();
        queue.pop();
        return task;
        mutex_end();
    }

    /**
     * Tear down the task queue.
     */
    void close() {
        mutex_begin(uqlk, lock);
        stop = true;
        mutex_end();
        enqueue.notify_all();
    }

private:
    std::queue<task_t> queue;
    lock_t lock;
    cond_t enqueue;
    bool stop;
};

/**
 * Thread pool.
 */
class _ThreadPool {
public:
    static const uint32_t INV_TID = -1u;

    explicit _ThreadPool(uint32_t _threadCount)
            : threadCount(_threadCount), queues(_threadCount),
              taskCount(0), curThreadIdx(0) {
        for (uint32_t tid = 0; tid < threadCount; tid++) {
            threads.emplace_back(&_ThreadPool::thread_func, this, tid);
        }
    }

    ~_ThreadPool() {
        for (auto& q : queues) {
            q.close();
        }
        for (auto& t : threads) {
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

        mutex_begin(uqlk, taskLock);
        taskCount++;
        mutex_end();

        queues[tid].enqueue(task);
    }

    void wait_all() {
        mutex_begin(uqlk, taskLock);
        taskDone.wait(uqlk, [this]{ return taskCount == 0; });
        mutex_end();
    }

private:
    const uint32_t threadCount;
    std::vector<thread_t> threads;
    std::vector<_TaskQueue> queues;

    // Track number of pending tasks
    lock_t taskLock;
    cond_t taskDone;
    uint32_t taskCount;

    // For task assignment
    uint32_t curThreadIdx;

private:
    uint32_t next_tid() {
        return curThreadIdx = (curThreadIdx + 1) % threadCount;
    }

    void thread_func(uint32_t tid) {
        while (true) {
            auto task = queues[tid].dequeue();
            if (!task) return;

            task();

            mutex_begin(uqlk, taskLock);
            assert(taskCount > 0);
            taskCount--;
            mutex_end();
            taskDone.notify_all();
        }
    }
};

#endif  // UTILS_THREAD_H_

