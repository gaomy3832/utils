/*
 * Copyright 2016 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/stream.h"
#include <vector>

class StreamTest : public ::testing::Test {
protected:
    struct Data {
        uint64_t a;
        uint8_t b;

        bool operator==(const Data& other) const {
            return a == other.a && b == other.b;
        }
        bool operator<(const Data& other) const {
            return a < other.a || (a == other.a && b < other.b);
        }
    };
    typedef Stream<Data> stream_t;

    virtual void SetUp() {
        strm1 = new stream_t();
        strm2 = new stream_t(256);
        strm3 = new stream_t(0);

        for (uint64_t idx = 0; idx < 16; idx++) {
            strm2->put({idx, static_cast<uint8_t>(idx % 128)});
        }
    }

    virtual void TearDown() {
        delete strm1;
        delete strm2;
        delete strm3;
    }

    stream_t* strm1;
    stream_t* strm2;
    stream_t* strm3;
};

TEST_F(StreamTest, constructor) {
    ASSERT_NE(nullptr, strm1->data());
    ASSERT_EQ(0, strm1->size());
    ASSERT_LE(16, strm1->capacity());

    ASSERT_NE(nullptr, strm2->data());
    ASSERT_EQ(16, strm2->size());
    ASSERT_LE(256, strm2->capacity());

    ASSERT_EQ(nullptr, strm3->data());
    ASSERT_EQ(0, strm3->size());
    ASSERT_EQ(0, strm3->capacity());
}

TEST_F(StreamTest, moveConstructor) {
    const auto size = strm2->size();
    const auto capacity = strm2->capacity();

    Data check[100];
    std::copy(strm2->data(), strm2->data() + size, check);

    stream_t strm3(std::move(*strm2));

    ASSERT_EQ(nullptr, strm2->data());
    ASSERT_EQ(0, strm2->size());
    ASSERT_EQ(0, strm2->capacity());
    ASSERT_NE(nullptr, strm3.data());
    ASSERT_EQ(size, strm3.size());
    ASSERT_EQ(capacity, strm3.capacity());
    ASSERT_TRUE(std::equal(strm3.data(), strm3.data() + size, check));
}

TEST_F(StreamTest, moveAssignment) {
    const auto size = strm2->size();
    const auto capacity = strm2->capacity();

    Data check[100];
    std::copy(strm2->data(), strm2->data() + size, check);

    stream_t strm3 = stream_t();

    strm3 = std::move(*strm2);

    // strm2 becomes the default status.
    ASSERT_NE(nullptr, strm2->data());
    ASSERT_EQ(strm1->size(), strm2->size());
    ASSERT_EQ(strm1->capacity(), strm2->capacity());
    ASSERT_NE(nullptr, strm3.data());
    ASSERT_EQ(size, strm3.size());
    ASSERT_EQ(capacity, strm3.capacity());
    ASSERT_TRUE(std::equal(strm3.data(), strm3.data() + size, check));
}

TEST_F(StreamTest, byte_size) {
    ASSERT_EQ(0, strm1->byte_size());
    ASSERT_EQ(16 * sizeof(Data), strm2->byte_size());
    ASSERT_EQ(0, strm3->size());
}

TEST_F(StreamTest, reset) {
    strm2->reset(32);
    ASSERT_NE(nullptr, strm2->data());
    ASSERT_EQ(0, strm2->size());
    ASSERT_LE(32, strm2->capacity());

    strm3->reset(512);
    ASSERT_NE(nullptr, strm3->data());
    ASSERT_EQ(0, strm3->size());
    ASSERT_LE(512, strm3->capacity());
}

TEST_F(StreamTest, iterator) {
    // Iterator.
    for (auto& d : *strm2) {
        d.a = d.a + 1;
        d.b = d.b + 1;
    }

    // Const iterator.
    auto dd = Data{0, 0};
    for (const auto& d : *strm2) {
        assert(dd < d);
        dd = d;
    }
}

TEST_F(StreamTest, swap) {
    const Data d{12, 2};
    const auto size = strm2->size();
    Data check[100];
    std::copy(strm2->data(), strm2->data() + size, check);

    strm1->put(d);

    strm2->swap(*strm1);
    strm3->swap(*strm1);

    // 1 has orig 3, 2 has orig 1, 3 has orig 2.
    ASSERT_TRUE(std::equal(strm3->begin(), strm3->end(), check));
    ASSERT_EQ(1, strm2->size());
    ASSERT_EQ(d, *strm2->begin());
    ASSERT_EQ(nullptr, strm1->data());
}

TEST_F(StreamTest, put) {
    const auto size = strm2->size();
    const auto capacity = strm2->capacity();

    std::vector<uint64_t> avec;
    std::vector<uint8_t> bvec;

    for (size_t idx = 0; idx < capacity - size; idx++) {
        strm2->put({idx, static_cast<uint8_t>(idx % 128)});
        avec.push_back(idx);
        bvec.push_back(static_cast<uint8_t>(idx % 128));
    }
    // Capacity does not change, and size becomes equal to capacity.
    ASSERT_EQ(capacity, strm2->capacity());
    ASSERT_EQ(capacity, strm2->size());

    strm2->put({1024, 130});
    avec.push_back(1024);
    bvec.push_back(130);
    // Capacity grows.
    ASSERT_LE(capacity, strm2->capacity());

    // Check numbers.
    auto it = strm2->begin() + size;
    auto ita = avec.begin();
    auto itb = bvec.begin();
    for (; it != strm2->end(); it++, ita++, itb++) {
        ASSERT_NE(avec.end(), ita);
        ASSERT_NE(bvec.end(), itb);
        ASSERT_EQ(*ita, (*it).a);
        ASSERT_EQ(*itb, (*it).b);
    }
}

TEST_F(StreamTest, sort) {
    for (size_t idx = 0; idx < 128; idx++) {
        strm3->put({idx % 32, static_cast<uint8_t>(idx % 127)});
    }

    strm3->sort();

    auto it = strm3->begin();
    Data d = *it;
    it++;
    for (; it != strm3->end(); it++) {
        ASSERT_LT(d, *it);
        d = *it;
    }
}

