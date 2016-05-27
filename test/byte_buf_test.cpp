/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/byte_buf.h"
#include <algorithm>        // for std::equal

class ByteBufTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        buf1 = new ByteBuf();
        buf2 = new ByteBuf(msg, 6);
    }

    virtual void TearDown() {
        delete buf1;
        delete buf2;
    }

    const char* msg = "a very very long test message";

    ByteBuf* buf1;
    ByteBuf* buf2;
};

TEST_F(ByteBufTest, defaultConstructor) {
    ASSERT_EQ(nullptr, buf1->data());
    ASSERT_EQ(0, buf1->size());
    ASSERT_EQ(0, buf1->capacity());
}

TEST_F(ByteBufTest, initializeConstructor) {
    ASSERT_NE(nullptr, buf2->data());
    ASSERT_EQ(6, buf2->size());
    // Initialization will round up capacity to power of 2.
    ASSERT_EQ(8, buf2->capacity());

    std::vector<uint32_t> data(4, 0);
    data[0] = (3u << 24) + (2u << 16) + (1u << 8) + 0u;
    data[1] = (7u << 24) + (6u << 16) + (5u << 8) + 4u;
    data[2] = (11u << 24) + (10u << 16) + (9u << 8) + 8u;
    data[3] = (15u << 24) + (14u << 16) + (13u << 8) + 12u;

    ByteBuf buf3(data.data(), data.size() * sizeof(uint32_t));
    ASSERT_EQ(data.size() * sizeof(uint32_t), buf3.size());
    const auto* d = buf3.data();
    for (Byte idx = 0; idx < buf3.size(); idx++) {
        ASSERT_EQ(idx, d[idx]);
    }
}

TEST_F(ByteBufTest, moveConstructor) {
    const auto size = buf2->size();
    const auto capacity = buf2->capacity();

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + size, check);

    ByteBuf buf3(std::move(*buf2));

    ASSERT_EQ(nullptr, buf2->data());
    ASSERT_EQ(0, buf2->size());
    ASSERT_EQ(0, buf2->capacity());
    ASSERT_NE(nullptr, buf3.data());
    ASSERT_EQ(size, buf3.size());
    ASSERT_EQ(capacity, buf3.capacity());
    ASSERT_TRUE(std::equal(buf3.data(), buf3.data() + size, check));
}

TEST_F(ByteBufTest, moveAssignment) {
    const auto size = buf2->size();
    const auto capacity = buf2->capacity();

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + size, check);

    ByteBuf buf3 = ByteBuf(msg, 2);

    buf3 = std::move(*buf2);

    ASSERT_EQ(nullptr, buf2->data());
    ASSERT_EQ(0, buf2->size());
    ASSERT_EQ(0, buf2->capacity());
    ASSERT_NE(nullptr, buf3.data());
    ASSERT_EQ(size, buf3.size());
    ASSERT_EQ(capacity, buf3.capacity());
    ASSERT_TRUE(std::equal(buf3.data(), buf3.data() + size, check));
}

TEST_F(ByteBufTest, reserve) {
    buf1->reserve(10);
    ASSERT_EQ(16, buf1->capacity());
    ASSERT_EQ(0, buf1->size());

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + buf2->size(), check);
    const auto size = buf2->size();

    // Reserve smaller-then-current capacity.
    buf2->reserve(7);
    ASSERT_EQ(8, buf2->capacity());
    ASSERT_EQ(size, buf2->size());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));

    buf2->reserve(40);
    ASSERT_EQ(64, buf2->capacity());
    ASSERT_EQ(size, buf2->size());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
}

TEST_F(ByteBufTest, append) {
    const auto size = buf2->size();
    const auto capacity = buf2->capacity();

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + size, check);

    // Append 0-length: size and capacity do not change.
    buf2->append(msg, 0);
    ASSERT_EQ(size, buf2->size());
    ASSERT_EQ(capacity, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));

    // Append up to capacity: capacity does not change.
    buf2->append(msg, capacity - size);
    ASSERT_EQ(capacity, buf2->capacity());
    ASSERT_EQ(capacity, buf2->size());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
    ASSERT_TRUE(std::equal(buf2->data() + size, buf2->data() + capacity, msg));

    // Append to more than capacity: capacity should double.
    buf2->append(msg, 1);
    ASSERT_EQ(capacity * 2, buf2->capacity());
    ASSERT_EQ(capacity + 1, buf2->size());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
    ASSERT_TRUE(std::equal(buf2->data() + size, buf2->data() + capacity, msg));
    ASSERT_TRUE(std::equal(
                buf2->data() + capacity, buf2->data() + capacity + 1, msg));
}

TEST_F(ByteBufTest, resizeSmaller) {
    const auto size = buf2->size();
    const auto capacity = buf2->capacity();

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + size, check);

    // Resizing to smaller size doesn't affect capacity.
    buf2->resize(5);
    ASSERT_EQ(5, buf2->size());
    ASSERT_EQ(capacity, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + buf2->size(), check));

    buf2->resize(3);
    ASSERT_EQ(3, buf2->size());
    ASSERT_EQ(capacity, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + buf2->size(), check));

    buf2->resize(0);
    ASSERT_EQ(0, buf2->size());
    ASSERT_EQ(capacity, buf2->capacity());
}

TEST_F(ByteBufTest, resizeLarger) {
    const auto size = buf2->size();
    const auto capacity = buf2->capacity();

    Byte check[100];
    std::copy(buf2->data(), buf2->data() + size, check);
    Byte zero[100];
    std::fill(zero, zero + 100, 0);

    // Resizing to larger size may require larger capacity.
    buf2->resize(7);
    ASSERT_EQ(7, buf2->size());
    ASSERT_EQ(capacity, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
    ASSERT_TRUE(std::equal(
                buf2->data() + size, buf2->data() + buf2->size(), zero));

    buf2->resize(30);
    ASSERT_EQ(30, buf2->size());
    ASSERT_EQ(32, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
    ASSERT_TRUE(std::equal(
                buf2->data() + size, buf2->data() + buf2->size(), zero));

    buf2->resize(40);
    ASSERT_EQ(40, buf2->size());
    ASSERT_EQ(64, buf2->capacity());
    ASSERT_TRUE(std::equal(buf2->data(), buf2->data() + size, check));
    ASSERT_TRUE(std::equal(
                buf2->data() + size, buf2->data() + buf2->size(), zero));
}

