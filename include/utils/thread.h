/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_THREAD_H_
#define UTILS_THREAD_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "utils/exception.h"
#include "utils/log.h"

/**
 * @addtogroup parallel Parallel processing
 *
 * Include thread manipulation and synchronization primitives.
 *
 * Use C++11 primitives and routines.
 *
 * @{
 */

/**
 * @brief
 * Threads.
 *
 * Use constructor to create a thread and execute the function:
 * @code
 * template<class Function, class... Args>
 * explicit thread(Function&& f, Args&&... args);
 * @endcode
 *
 * To join, use
 * @code
 * void join()
 * @endcode
 */
using thread_t = std::thread;

/**
 * @brief
 * Mutexes.
 *
 * Use member functions:
 * @code
 * void lock()
 * @endcode
 * @code
 * bool try_lock()
 * @endcode
 * @code
 * void unlock()
 * @endcode
 */
using lock_t = std::mutex;

/**
 * @name
 * Mutex ownership region.
 */
/**@{*/
#define mutex_begin(uniq_lk, lk) \
    { std::unique_lock<lock_t> uniq_lk(lk);
#define mutex_end() \
    }
/**@}*/

/**
 * @brief
 * Condition variables.
 *
 * Use member functions to wait:
 * @code
 * void wait(std::unique_lock<std::mutex>& lock);
 * @endcode
 * @code
 * template<class Predicate>
 * void wait(std::unique_lock<std::mutex>& lock, Predicate pred);
 * @endcode
 *
 * Use member functions to notify:
 * @code
 * void notify_one();
 * @endcode
 * @code
 * void notify_all();
 * @endcode
 */
using cond_t = std::condition_variable;

/**
 * @brief
 * Barrier.
 */
class bar_t {
public:
    /**
     * Return value from the thread with the barrier serial point.
     */
    static constexpr int SERIAL_LAST_THREAD = 1;

public:
    /**
     * @brief
     * Initialize the barrier.
     *
     * @param count  the number of threads involved in the barrier.
     */
    explicit bar_t(const size_t count)
            : threadCount_(count), remain_(count), barCount_(0) {
        // Nothing else to do.
    }

    /**
     * @brief
     * Wait on the barrier.
     *
     * @param onSerialPoint  callback function at the barrier serial point.
     * Default to be empty function.
     * @return  \c SERIAL_LAST_THREAD if the thread is the last one arriving at
     * the barrier, or 0 otherwise.
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
 * @brief
 * Thread pool.
 */
class thread_pool_t {
private:
    /**
     * @brief
     * Task queue for thread pool.
     */
    class TaskQueue {
    public:
        using Task = std::function<void()>;

    public:
        TaskQueue() : stop_(false) {}

        ~TaskQueue() {}

        TaskQueue(TaskQueue&&) = default;

        TaskQueue& operator=(TaskQueue&&) = default;

        TaskQueue(const TaskQueue&) = delete;

        TaskQueue& operator=(const TaskQueue&) = delete;

        /**
         * @brief
         * Enqueue a task.
         */
        void enqueue(const Task& task) {
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
         * @brief
         * Dequeue a task. Blocked.
         */
        Task dequeue() {
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
         * @brief
         * Tear down the task queue.
         */
        void close() {
            mutex_begin(uqlk, lock_);
            stop_ = true;
            mutex_end();
            enqueue_.notify_all();
        }

    private:
        std::queue<Task> queue_;
        lock_t lock_;
        cond_t enqueue_;
        bool stop_;
    };

public:
    static const uint32_t INV_TID = -1u;

public:
    /**
     * @brief
     * Initialize the thread pool.
     *
     * @param count  the number of threads.
     */
    explicit thread_pool_t(uint32_t count)
            : threadCount_(count), queues_(count),
              taskCount_(0), curThreadIdx_(0) {
        for (uint32_t tid = 0; tid < threadCount_; tid++) {
            threads_.emplace_back(&thread_pool_t::thread_func, this, tid);
        }
    }

    ~thread_pool_t() {
        for (auto& q : queues_) {
            q.close();
        }
        for (auto& t : threads_) {
            t.join();
        }
    }

    /**
     * @name
     * Copy and move.
     */
    /**@{*/

    thread_pool_t(thread_pool_t&&) = default;

    thread_pool_t& operator=(thread_pool_t&&) = default;

    thread_pool_t(const thread_pool_t&) = delete;

    thread_pool_t& operator=(const thread_pool_t&) = delete;

    /**@}*/

    /**
     * @brief
     * Add task to the thread pool to be executed.
     *
     * @param task  task function.
     * @param tid   optional thread ID to which the task should be assigned to.
     * If \c INV_TID, then assign to the next thread.
     */
    void add_task(const std::function<void()>& task, uint32_t tid = INV_TID) {
        if (tid == INV_TID) {
            tid = next_tid();
        }

        mutex_begin(uqlk, taskLock_);
        taskCount_++;
        mutex_end();

        queues_[tid].enqueue(task);
    }

    /**
     * @brief
     * Wait until all tasks finish.
     */
    void wait_all() {
        mutex_begin(uqlk, taskLock_);
        taskDone_.wait(uqlk, [this]{ return taskCount_ == 0; });
        mutex_end();
    }

private:
    const uint32_t threadCount_;
    std::vector<thread_t> threads_;
    std::vector<TaskQueue> queues_;

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

/**@}*/

#endif  // UTILS_THREAD_H_

