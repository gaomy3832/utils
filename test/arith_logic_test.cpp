/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/arith_logic.h"

TEST(ArithLogic, ilog2) {
    ASSERT_EQ(0, ilog2(1u));
    ASSERT_EQ(1, ilog2(2u));
    ASSERT_EQ(2, ilog2(4u));
    ASSERT_EQ(30, ilog2(1024u*1024*1024));
    ASSERT_EQ(50, ilog2(1024uL*1024*1024*1024*1024));

    ASSERT_EQ(3, ilog2(8u));
    ASSERT_EQ(3, ilog2(10u));
    ASSERT_EQ(3, ilog2(15u));
    ASSERT_EQ(7, ilog2(255u));
    ASSERT_EQ(31, ilog2(-1u));
    ASSERT_EQ(63, ilog2(-1uL));

    ASSERT_EQ(0, ilog2(0u));

    ASSERT_EQ(30, ilog2((uint32_t)(1024u*1024*1024)));
    ASSERT_EQ(50, ilog2((uint64_t)(1024uL*1024*1024*1024*1024)));
}

TEST(ArithLogic, cilog2) {
    ASSERT_EQ(0, cilog2(1u));
    ASSERT_EQ(1, cilog2(2u));
    ASSERT_EQ(2, cilog2(4u));
    ASSERT_EQ(30, cilog2(1024u*1024*1024));
    ASSERT_EQ(50, cilog2(1024uL*1024*1024*1024*1024));

    ASSERT_EQ(3, cilog2(8u));
    ASSERT_EQ(4, cilog2(10u));
    ASSERT_EQ(4, cilog2(15u));
    ASSERT_EQ(8, cilog2(255u));
    ASSERT_EQ(32, cilog2(-1u));
    ASSERT_EQ(64, cilog2(-1uL));

    ASSERT_EQ(0, cilog2(0u));

    ASSERT_EQ(30, ilog2((uint32_t)(1024u*1024*1024)));
    ASSERT_EQ(50, ilog2((uint64_t)(1024uL*1024*1024*1024*1024)));
}

TEST(ArithLogic, isPow2) {
    ASSERT_TRUE(isPow2(1));
    ASSERT_TRUE(isPow2(2));
    ASSERT_TRUE(isPow2(4));
    ASSERT_TRUE(isPow2(65536));
    ASSERT_TRUE(isPow2(1uL << 32));
    ASSERT_TRUE(isPow2(1uL << 63));

    ASSERT_FALSE(isPow2(0));
    ASSERT_FALSE(isPow2(3));
    ASSERT_FALSE(isPow2(127));
    ASSERT_FALSE(isPow2(-1));
    ASSERT_FALSE(isPow2(-4));
}

