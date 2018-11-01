/*
 * Copyright 2018 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/stream.h"
#include <chrono>
#include <cstdlib>
#include <vector>
#include "utils/parallel.h"

class StreamTest : public ::testing::Test {
protected:
    struct Data {
        uint64_t a;
        uint8_t b;
    };
    typedef Stream<Data> stream_t;

    virtual void SetUp() {
        strm1 = new stream_t();
        strm2 = new stream_t();

        for (uint64_t idx = 0; idx < 16; idx++) {
            strm2->put({idx, static_cast<uint8_t>(idx % 128)});
        }
    }

    virtual void TearDown() {
        delete strm1;
        delete strm2;
    }

    stream_t* strm1;
    stream_t* strm2;
};

TEST_F(StreamTest, constructor) {
    ASSERT_EQ(0, strm1->size());
    ASSERT_EQ(16, strm2->size());
}

TEST_F(StreamTest, moveConstructor) {
    const auto size = strm2->size();

    stream_t strm3(std::move(*strm2));

    ASSERT_EQ(0, strm2->size());
    ASSERT_EQ(size, strm3.size());
}

TEST_F(StreamTest, moveAssignment) {
    const auto size = strm2->size();

    stream_t strm3 = stream_t();
    strm3 = std::move(*strm2);

    ASSERT_EQ(0, strm2->size());
    ASSERT_EQ(size, strm3.size());
}

TEST_F(StreamTest, byte_size) {
    ASSERT_EQ(0, strm1->byte_size());
    ASSERT_EQ(16 * sizeof(Data), strm2->byte_size());
}

TEST_F(StreamTest, clearAndEmpty) {
    ASSERT_TRUE(strm1->empty());
    ASSERT_EQ(0, strm1->size());
    strm1->put({1, 2});
    ASSERT_FALSE(strm1->empty());
    ASSERT_NE(0, strm1->size());
    strm1->clear();
    ASSERT_TRUE(strm1->empty());
    ASSERT_EQ(0, strm1->size());

    strm2->clear();
    ASSERT_TRUE(strm2->empty());
    ASSERT_EQ(0, strm2->size());
}

TEST_F(StreamTest, getRange) {
    ASSERT_TRUE(strm1->empty());
    auto range = strm1->get_range();
    ASSERT_EQ(range.first, range.second);

    range = strm2->get_range();
    size_t n = 0;
    for (auto it = range.first; it != range.second; ++it) {
        ASSERT_EQ(n, it->a);
        ASSERT_EQ(n % 128, it->b);
        n++;
    }
    ASSERT_EQ(16, n);

    ASSERT_TRUE(strm2->empty());
    range = strm2->get_range();
    ASSERT_EQ(range.first, range.second);

    strm2->put({64, 64});
    range = strm2->get_range();
    ASSERT_EQ(64, range.first->a);
    ASSERT_EQ(64, range.first->b);
    ASSERT_EQ(++range.first, range.second);
}

TEST_F(StreamTest, swap) {
    const auto size = strm2->size();

    stream_t strm3;

    std::swap(*strm2, strm3);

    ASSERT_EQ(0, strm2->size());
    ASSERT_EQ(size, strm3.size());
}

TEST_F(StreamTest, put) {
    const auto size = strm2->size();

    auto range = strm2->get_range();

    std::vector<uint64_t> avec;
    std::vector<uint8_t> bvec;

    for (size_t idx = size; idx < 65536; idx++) {
        strm2->put({idx, static_cast<uint8_t>(idx % 128)});
        avec.push_back(idx);
        bvec.push_back(static_cast<uint8_t>(idx % 128));
    }

    size_t n = 0;

    // Get range is still valid.
    for (auto it = range.first; it != range.second; ++it) {
        ASSERT_EQ(n, it->a);
        ASSERT_EQ(n % 128, it->b);
        n++;
    }
    ASSERT_EQ(size, n);

    // Get and check the new content.
    range = strm2->get_range();

    auto ita = avec.begin();
    auto itb = bvec.begin();
    for (auto it = range.first; it != range.second; ++it, ++ita, ++itb, ++n) {
        ASSERT_NE(avec.end(), ita);
        ASSERT_NE(bvec.end(), itb);
        ASSERT_EQ(*ita, (*it).a);
        ASSERT_EQ(*itb, (*it).b);
    }
    ASSERT_EQ(65536, n);

    // No more.
    ASSERT_TRUE(strm2->empty());
    range = strm2->get_range();
    ASSERT_EQ(range.first, range.second);
}

TEST_F(StreamTest, concurrency) {
    ASSERT_TRUE(strm1->empty());

    constexpr size_t size = 128 * 1024 - 3;

    auto producer = [&]() {
        for (size_t i = 0; i < size; i++) {
            strm1->put({i, static_cast<uint8_t>(i % 31)});
            std::this_thread::sleep_for(std::chrono::microseconds(std::rand() % 10));
        }
    };

    auto consumer = [&]() {
        size_t recv = 0;
        while (recv < size) {
            auto range = strm1->get_range();
            for (auto it = range.first; it != range.second; ++it) {
                ASSERT_EQ(recv, it->a);
                ASSERT_EQ(recv % 31, it->b);
                recv++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    thread_t prod(producer);
    thread_t cons(consumer);

    prod.join();
    cons.join();

    ASSERT_TRUE(strm1->empty());
    auto range = strm1->get_range();
    ASSERT_EQ(range.first, range.second);
}

