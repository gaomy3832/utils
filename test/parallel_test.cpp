/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/parallel.h"
#include <algorithm>        // for std::fill

constexpr uint32_t thCnt = 8;
constexpr uint32_t itCnt = 4;

TEST(Parallel, thread) {
    std::vector<thread_t> threads;
    const uint32_t val = 123;

    std::array<uint32_t, thCnt> spawned;
    std::fill(spawned.begin(), spawned.end(), 0);

    auto threadFunc = [&val, &spawned](uint32_t idx){
        ASSERT_EQ(123, val);
        spawned[idx] = 1;
    };

    for (uint32_t idx = 0; idx < thCnt; idx++) {
        threads.emplace_back(threadFunc, idx);
    }
    for (uint32_t idx = 0; idx < thCnt; idx++) {
        threads[idx].join();
    }

    for (const auto s : spawned) {
        ASSERT_EQ(1, s);
    }
}

TEST(Parallel, threadPool) {
    const uint32_t val = 123;

    std::array<uint32_t, thCnt> spawned;
    std::fill(spawned.begin(), spawned.end(), 0);

    auto threadFunc = [&val, &spawned](uint32_t idx){
        ASSERT_EQ(123, val);
        spawned[idx] = 1;
    };

    thread_pool_t pool(thCnt);
    for (uint32_t idx = 0; idx < thCnt; idx++) {
        pool.add_task(std::bind(threadFunc, idx));
    }
    pool.wait_all();

    for (const auto s : spawned) {
        ASSERT_EQ(1, s);
    }
}

TEST(Parallel, mutex) {
    uint32_t var = 0;
    lock_t mutex;

    auto threadFunc = [&var, &mutex](uint32_t idx){
        for (uint32_t i = 0; i < itCnt; i++) {
            mutex.lock();
            var += idx;
            mutex.unlock();
        }
    };

    thread_pool_t pool(thCnt);
    for (uint32_t idx = 0; idx < thCnt; idx++) {
        pool.add_task(std::bind(threadFunc, idx));
    }
    pool.wait_all();

    ASSERT_EQ((0+7)*8/2 * itCnt, var);
}

TEST(Parallel, mutexRegion) {
    uint32_t var = 0;
    lock_t mutex;

    auto threadFunc = [&var, &mutex](uint32_t idx){
        for (uint32_t i = 0; i < itCnt; i++) {
            mutex_begin(uqlk, mutex);
            var += idx;
            mutex_end();
        }
    };

    thread_pool_t pool(thCnt);
    for (uint32_t idx = 0; idx < thCnt; idx++) {
        pool.add_task(std::bind(threadFunc, idx));
    }
    pool.wait_all();

    ASSERT_EQ((0+7)*8/2 * itCnt, var);
}

TEST(Parallel, barrier) {
    uint32_t var = 0;
    bar_t bar(thCnt);

    auto threadFunc = [&var, &bar](uint32_t){
        for (uint32_t i = 0; i < itCnt; i++) {
            bar.wait([&var](){ var++; });
        }
    };

    thread_pool_t pool(thCnt);
    for (uint32_t idx = 0; idx < thCnt; idx++) {
        pool.add_task(std::bind(threadFunc, idx));
    }
    pool.wait_all();

    ASSERT_EQ(itCnt, var);
}

