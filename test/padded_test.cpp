/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/padded.h"
#include <utility>
#include <vector>

class PaddedTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        for (uint32_t i = 0; i < 10; i++) padvec1.push_back(Pad1(3 + i));
        padvec2.push_back(Pad2({1, 3}));
        padvec2.push_back(Pad2({2, 2}));
        padvec2.push_back(Pad2({3, 1}));
    }

    typedef Padded<64, uint64_t> Pad1;
    struct Data2 {
        uint32_t x;
        uint32_t y;
        Data2(uint32_t _x, uint64_t _y) : x(_x), y(_y) {}
        bool operator==(const Data2& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Data2& other) const { return !(*this == other); }
        void inc() { x++; }
    };
    typedef Padded<128, Data2> Pad2;

    std::vector<Pad1> padvec1;
    std::vector<Pad2> padvec2;
};

TEST_F(PaddedTest, constructor) {
    std::vector<Pad1> vec;
    for (uint32_t i = 0; i < 1024; i++) vec.emplace_back(i);
    for (uint32_t i = 0; i < 1024; i++) {
        ASSERT_EQ(i, vec[i].value());
        if (i > 0) {
            ASSERT_EQ(0,
                    (reinterpret_cast<uintptr_t>(&vec[i])
                     - reinterpret_cast<uintptr_t>(&vec[i - 1]))
                    % Pad1::ALIGN);
        }
    }
}

TEST_F(PaddedTest, constructorMove) {
    std::vector<Pad2> vec;
    for (uint32_t i = 0; i < 1024; i++) vec.push_back(Data2(i, i));
    for (uint32_t i = 0; i < 1024; i++) {
        ASSERT_EQ(i, vec[i].value().x);
        ASSERT_EQ(i, vec[i].value().y);
        if (i > 0) {
            ASSERT_EQ(0,
                    (reinterpret_cast<uintptr_t>(&vec[i])
                     - reinterpret_cast<uintptr_t>(&vec[i - 1]))
                    % Pad2::ALIGN);
        }
    }
}

TEST_F(PaddedTest, compare) {
    for (uint32_t i = 0; i < 10; i++) ASSERT_EQ(Pad1(3 + i), padvec1[i]);
    ASSERT_EQ(Pad2({1, 3}), padvec2[0]);
    ASSERT_EQ(Pad2({2, 2}), padvec2[1]);
    ASSERT_EQ(Pad2({3, 1}), padvec2[2]);
}

TEST_F(PaddedTest, compareImplicit) {
    for (uint32_t i = 0; i < 10; i++) {
        ASSERT_EQ(3 + i, padvec1[i]);
        ASSERT_NE(i, padvec1[i]);
        ASSERT_LT(i, padvec1[i]);
        ASSERT_LE(i, padvec1[i]);
        ASSERT_GT(padvec1[i], i);
        ASSERT_GE(padvec1[i], i);
    }

    ASSERT_EQ(Data2(1, 3), padvec2[0]);
    ASSERT_EQ(padvec2[0], Data2(1, 3));
    ASSERT_NE(Data2(0, 0), padvec2[0]);
    ASSERT_NE(padvec2[0], Data2(0, 0));
}

TEST_F(PaddedTest, assignment) {
    for (uint32_t i = 0; i < 10; i++) {
        ASSERT_EQ(4 + i, padvec1[i] = padvec1[i] + 1);
    }

    padvec2[1] = padvec2[0];
    ASSERT_EQ(padvec2[0], padvec2[1]);
}

TEST_F(PaddedTest, conversion) {
    for (uint32_t i = 0; i < 10; i++) {
        ASSERT_EQ(4 + i, ++padvec1[i]);
    }

    padvec2[0].value().inc();
    ASSERT_EQ(2, padvec2[0].value().x);
    padvec2[1].value().inc();
    ASSERT_EQ(3, padvec2[1].value().x);
    padvec2[2].value().inc();
    ASSERT_EQ(4, padvec2[2].value().x);
}

