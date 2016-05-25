/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/random.h"
#include <algorithm>        // for std::fill, std::equal
#include <cmath>            // for sqrt

TEST(Random, get_integer) {
    std::vector<uint32_t> count(10, 0);
    const uint32_t totalCount = 1000000;

    Random rnd;

    for (uint32_t i = 0; i < totalCount; i++) {
        auto v = rnd.get_integer(10, 19);
        uint32_t index = v - 10;
        ASSERT_LT(index, count.size());
        count[index]++;
    }

    // Total counts.
    auto sx  = std::accumulate(count.begin(), count.end(), 0);
    ASSERT_EQ(totalCount, sx);

    // Inclusive.
    ASSERT_NE(0, count.front());
    ASSERT_NE(0, count.back());

    // Variance.
    double sx2 = std::accumulate(count.begin(), count.end(), 0.,
            [](const double& a, const double& b) { return a + b*b; });
    double n = static_cast<double>(count.size());
    auto xbar = sx / n;
    auto s2 = (sx2 - n*xbar*xbar) / (n - 1);
    auto s = sqrt(s2);
    ASSERT_LE(s, xbar * 0.02);
}

TEST(Random, get_real) {
    std::vector<uint32_t> count(50, 0);
    const uint32_t totalCount = 1000000;

    Random rnd;

    for (uint32_t i = 0; i < totalCount; i++) {
        auto v = rnd.get_real(1.0, 1.5);
        uint32_t index = static_cast<uint32_t>((v - 1.0) / 0.5 * 50);
        ASSERT_LT(index, count.size());
        count[index]++;
    }

    // Total counts.
    auto sx  = std::accumulate(count.begin(), count.end(), 0);
    ASSERT_EQ(totalCount, sx);

    // Inclusive.
    ASSERT_NE(0, count.front());
    ASSERT_NE(0, count.back());

    // Variance.
    double sx2 = std::accumulate(count.begin(), count.end(), 0.,
            [](const double& a, const double& b) { return a + b*b; });
    double n = static_cast<double>(count.size());
    auto xbar = sx / n;
    auto s2 = (sx2 - n*xbar*xbar) / (n - 1);
    auto s = sqrt(s2);
    ASSERT_LE(s, xbar * 0.02);
}

TEST(Random, seed) {
    uint64_t seed = 1234567;
    const uint32_t totalCount = 1000;

    Random rnd1(seed);
    std::vector<uint64_t> vec1;
    for (uint32_t idx = 0; idx < totalCount; idx++) {
        vec1.push_back(rnd1.get_integer());
    }

    Random rnd2(seed);
    std::vector<uint64_t> vec2;
    for (uint32_t idx = 0; idx < totalCount; idx++) {
        vec2.push_back(rnd2.get_integer());
    }

    ASSERT_TRUE(std::equal(vec1.begin(), vec1.end(), vec2.begin()));
}

