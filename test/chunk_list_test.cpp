/*
 * Copyright 2018 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/chunk_list.h"
#include <algorithm>  // for std::copy, std::equal, std::find_if_not
#include <array>

struct UnalignedType { int a; char b; };

class ChunkTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        chk1 = new ChunkList<int>::Chunk();
        chk2 = new ChunkList<UnalignedType>::Chunk();

        chk1->push_back(0);
        chk1->push_back(1);
        chk1->push_back(2);
    }

    virtual void TearDown() {
        delete chk1;
        delete chk2;
    }

    ChunkList<int>::Chunk* chk1;
    ChunkList<UnalignedType>::Chunk* chk2;
};

class ChunkListTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        cl1 = new ChunkList<int>();
        cl2 = new ChunkList<UnalignedType>();

        for (size_t i = 0; i < cl1->CHUNK_CAPACITY; i++) cl1->push_back(7);
        cl1->push_back(0);
        cl1->push_back(1);
        cl1->push_back(2);
    }

    virtual void TearDown() {
        delete cl1;
        delete cl2;
    }

    ChunkList<int>* cl1;
    ChunkList<UnalignedType>* cl2;
};

TEST_F(ChunkTest, pushBackAndSize) {
    ASSERT_EQ(chk1->size(), 3);
    chk1->push_back(1);
    ASSERT_EQ(chk1->size(), 4);
    chk1->push_back(2);
    ASSERT_EQ(chk1->size(), 5);
    for (uint32_t i = 0; i < ChunkList<int>::CHUNK_CAPACITY - 5; i++) chk1->push_back(i);
    ASSERT_EQ(chk1->size(), ChunkList<int>::CHUNK_CAPACITY);

    ASSERT_EQ(chk2->size(), 0);
    for (uint32_t i = 0; i < 10; i++) chk2->push_back({1, 1});
    ASSERT_EQ(chk2->size(), 10);
}

TEST_F(ChunkTest, popBackAndSize) {
    auto size = chk1->size();
    chk1->pop_back();
    ASSERT_EQ(chk1->size(), size - 1);
    ASSERT_EQ(chk1->back(), chk1->at(size - 2));
    chk1->pop_back();
    ASSERT_EQ(chk1->size(), size - 2);
    ASSERT_EQ(chk1->back(), chk1->at(size - 3));
}

TEST_F(ChunkTest, resizeAndSize) {
    ASSERT_EQ(chk1->size(), 3);
    int contents[3];
    std::copy(chk1->data(), chk1->data() + 3, contents);

    chk1->resize(10);
    ASSERT_EQ(chk1->size(), 10);
    ASSERT_TRUE(std::equal(chk1->data(), chk1->data() + 3, contents));

    chk1->resize(20, -1);
    ASSERT_EQ(chk1->size(), 20);
    ASSERT_TRUE(std::equal(chk1->data(), chk1->data() + 3, contents));
    for (uint32_t i = 10; i < 20; i++) ASSERT_EQ(chk1->at(i), -1);

    chk1->resize(5);
    ASSERT_EQ(chk1->size(), 5);
    ASSERT_TRUE(std::equal(chk1->data(), chk1->data() + 3, contents));

    ASSERT_EQ(chk2->size(), 0);

    chk2->resize(5, UnalignedType{2, 2});
    ASSERT_EQ(chk2->size(), 5);
    for (uint32_t i = 0; i < 5; i++) {
        ASSERT_EQ(chk2->at(i).a, 2);
        ASSERT_EQ(chk2->at(i).b, 2);
    }

    chk2->resize(0);
    ASSERT_EQ(chk2->size(), 0);
}

TEST_F(ChunkTest, clearAndEmpty) {
    ASSERT_FALSE(chk1->empty());
    chk1->push_back(2);
    ASSERT_FALSE(chk1->empty());
    chk1->clear();
    ASSERT_TRUE(chk1->empty());
    chk1->push_back(2);
    ASSERT_FALSE(chk1->empty());

    ASSERT_TRUE(chk2->empty());
    chk2->push_back({1, 1});
    ASSERT_FALSE(chk2->empty());
    chk2->clear();
    ASSERT_TRUE(chk2->empty());
}

TEST_F(ChunkTest, maxSize) {
    ASSERT_EQ(chk1->max_size(), 65536 / sizeof(int));
    ASSERT_EQ(chk2->max_size(), 65536 / sizeof(UnalignedType));

    auto chk = new ChunkList<UnalignedType, 5 * sizeof(UnalignedType)>::Chunk();
    ASSERT_EQ(chk->max_size(), 5);
}

TEST_F(ChunkTest, iterators) {
    ASSERT_EQ(chk1->front(), *(chk1->begin()));
    ASSERT_EQ(chk1->back(), *(chk1->end() - 1));
    ASSERT_GE(chk1->end(), chk1->begin());
    size_t cnt = 0;
    for (auto it = chk1->begin(); it != chk1->end(); it++) cnt++;
    ASSERT_EQ(chk1->size(), cnt);

    chk1->clear();
    chk1->resize(10, -2);
    for (auto it = chk1->begin(); it != chk1->end(); it++) {
        ASSERT_EQ(-2, *it);
        *it = 2;
    }
    ASSERT_EQ(chk1->end(), std::find_if_not(chk1->begin(), chk1->end(), [](int n){ return n == 2; }));

    const auto* chk = chk1;
    ASSERT_EQ(chk->front(), *(chk->cbegin()));
    ASSERT_EQ(chk->back(), *(chk->cend() - 1));
    ASSERT_GE(chk->cend(), chk->cbegin());
    cnt = 0;
    for (auto it = chk->cbegin(); it != chk->cend(); it++) cnt++;
    ASSERT_EQ(chk->size(), cnt);

    auto it1 = chk->cbegin();
    decltype(it1) it2 = chk->begin();
    for (; it1 != chk->cend() && it2 != chk->end(); it1++, it2++) {
        ASSERT_EQ(2, *it1);
        ASSERT_EQ(it1, it2);
    }
}

TEST_F(ChunkTest, iteratorUpdate) {
    auto beg = chk1->begin();
    auto end = chk1->end();
    chk1->push_back(1);
    ASSERT_EQ(beg, chk1->begin());
    ASSERT_EQ(end + 1, chk1->end());
    chk1->push_back(1);
    ASSERT_EQ(beg, chk1->begin());
    ASSERT_EQ(end + 2, chk1->end());
}

TEST_F(ChunkTest, operatorSqBrckt) {
    for (int i = 0; i < (int)chk1->size(); i++) {
        ASSERT_EQ((*chk1)[i], i);
        (*chk1)[i] = -i;
    }
    const auto* chk = chk1;
    for (int i = 0; i < (int)chk->size(); i++) {
        ASSERT_EQ((*chk)[i], -i);
    }
}

TEST_F(ChunkTest, at) {
    for (int i = 0; i < (int)chk1->size(); i++) {
        ASSERT_EQ((*chk1)[i], chk1->at(i));
        chk1->at(i) = -i;
    }
    const auto* chk = chk1;
    for (int i = 0; i < (int)chk->size(); i++) {
        ASSERT_EQ(chk->at(i), -i);
    }
}

TEST_F(ChunkTest, atOutOfRange) {
    ASSERT_THROW(chk1->at(chk1->size()) = 1, std::out_of_range);
    ASSERT_THROW(chk1->at(-1) = 1, std::out_of_range);

    for (int i = 0; i < (int)chk1->size(); i++)
        ASSERT_EQ(chk1->at(i), i);

    const auto* chk = chk1;
    ASSERT_THROW(chk->at(chk->size()), std::out_of_range);
    ASSERT_THROW(chk->at(-1), std::out_of_range);

    ASSERT_NO_THROW(chk1->at(chk1->size() - 1) = 1);
    ASSERT_NO_THROW(chk->at(chk->size() - 1));
}

TEST_F(ChunkTest, frontAndBack) {
    const auto* chk = chk1;
    ASSERT_EQ(chk->front(), 0);
    ASSERT_EQ(chk->back(), 2);

    chk1->push_back(3);
    ASSERT_EQ(chk1->front(), 0);
    ASSERT_EQ(chk1->back(), 3);

    chk1->front() = 10;
    chk1->back() = 20;
    ASSERT_EQ(chk1->front(), 10);
    ASSERT_EQ(chk1->back(), 20);
}

TEST_F(ChunkTest, data) {
    auto data1 = chk1->data();
    ASSERT_EQ(reinterpret_cast<uintptr_t>(data1), reinterpret_cast<uintptr_t>(chk1));

    auto data2 = chk2->data();
    ASSERT_EQ(reinterpret_cast<uintptr_t>(data2), reinterpret_cast<uintptr_t>(chk2));

    const auto* chk = chk1;
    const auto* data = chk->data();
    ASSERT_EQ(reinterpret_cast<uintptr_t>(data), reinterpret_cast<uintptr_t>(chk));
}

TEST_F(ChunkTest, swap) {
    auto chk = new ChunkList<int>::Chunk();
    ASSERT_EQ(chk1->size(), 3);
    ASSERT_TRUE(chk->empty());

    chk1->swap(*chk);

    ASSERT_EQ(chk->size(), 3);
    ASSERT_TRUE(chk1->empty());
}

TEST_F(ChunkTest, pushBackException) {
    chk1->resize(chk1->max_size() - 1);
    ASSERT_NO_THROW(chk1->push_back(0));
    ASSERT_ANY_THROW(chk1->push_back(0));

    chk2->resize(chk2->max_size() - 1);
    ASSERT_NO_THROW(chk2->push_back({0, 0}));
    ASSERT_ANY_THROW(chk2->push_back({0, 0}));
}

TEST_F(ChunkTest, popBackException) {
    while (!chk1->empty()) chk1->pop_back();
    ASSERT_ANY_THROW(chk1->pop_back());

    ASSERT_ANY_THROW(chk2->pop_back());
}

TEST_F(ChunkTest, resizeException) {
    ASSERT_ANY_THROW(chk1->resize(chk1->max_size() + 1, 0));
    ASSERT_ANY_THROW(chk2->resize(chk2->max_size() + 1));

    ASSERT_NO_THROW(chk1->resize(chk1->max_size(), 0));
    ASSERT_NO_THROW(chk2->resize(chk2->max_size()));
}

TEST_F(ChunkListTest, pushBackAndSizeAndChunkCount) {
    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY + 3);
    ASSERT_EQ(cl1->chunk_count(), 2);
    auto size = cl1->size();
    cl1->push_back(1);
    ASSERT_EQ(cl1->size(), size + 1);
    cl1->push_back(2);
    ASSERT_EQ(cl1->size(), size + 2);
    ASSERT_EQ(cl1->chunk_count(), 2);
    for (uint32_t i = 0; i < cl1->CHUNK_CAPACITY - 5; i++) cl1->push_back(i);
    ASSERT_EQ(cl1->size(), 2 * cl1->CHUNK_CAPACITY);
    ASSERT_EQ(cl1->chunk_count(), 2);
    cl1->push_back(2);
    ASSERT_EQ(cl1->size(), 2 * cl1->CHUNK_CAPACITY + 1);
    ASSERT_EQ(cl1->chunk_count(), 3);

    ASSERT_EQ(cl2->size(), 0);
    ASSERT_EQ(cl2->chunk_count(), 0);
    for (uint32_t i = 0; i < 10; i++) cl2->push_back({1, 1});
    ASSERT_EQ(cl2->size(), 10);
    ASSERT_EQ(cl2->chunk_count(), 1);
}

TEST_F(ChunkListTest, popBackAndSizeAndChunkCount) {
    auto size = cl1->size();
    auto chcnt = cl1->chunk_count();
    cl1->pop_back();
    ASSERT_EQ(cl1->size(), size - 1);
    ASSERT_EQ(cl1->chunk_count(), chcnt);
    ASSERT_EQ(cl1->back(), cl1->at(size - 2));
    cl1->pop_back();
    ASSERT_EQ(cl1->size(), size - 2);
    ASSERT_EQ(cl1->chunk_count(), chcnt);
    ASSERT_EQ(cl1->back(), cl1->at(size - 3));

    cl1->pop_back();
    ASSERT_EQ(cl1->size(), size - 3);
    ASSERT_EQ(cl1->chunk_count(), chcnt - 1);
    ASSERT_EQ(cl1->back(), cl1->at(size - 4));
}

TEST_F(ChunkListTest, resizeAndSizeAndChunkCount) {
    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY + 3);
    ASSERT_EQ(cl1->chunk_count(), 2);
    std::array<int,
        std::remove_reference<decltype(*cl1)>::type::CHUNK_CAPACITY + 3> contents;
    std::copy(cl1->begin(), cl1->end(), contents.begin());

    cl1->resize(10);
    ASSERT_EQ(cl1->size(), 10);
    ASSERT_EQ(cl1->chunk_count(), 1);
    ASSERT_TRUE(std::equal(contents.begin(), contents.begin() + 10, cl1->begin()));

    cl1->resize(20, -1);
    ASSERT_EQ(cl1->size(), 20);
    ASSERT_TRUE(std::equal(contents.begin(), contents.begin() + 10, cl1->begin()));
    for (uint32_t i = 10; i < 20; i++) ASSERT_EQ(cl1->at(i), -1);

    cl1->resize(5);
    ASSERT_EQ(cl1->size(), 5);
    ASSERT_TRUE(std::equal(contents.begin(), contents.begin() + 5, cl1->begin()));

    ASSERT_EQ(cl2->size(), 0);

    cl2->resize(cl2->CHUNK_CAPACITY + 5, UnalignedType{2, 2});
    ASSERT_EQ(cl2->size(), cl2->CHUNK_CAPACITY + 5);
    for (uint32_t i = 0; i < cl2->size(); i++) {
        ASSERT_EQ(cl2->at(i).a, 2);
        ASSERT_EQ(cl2->at(i).b, 2);
    }

    cl2->resize(0);
    ASSERT_EQ(cl2->size(), 0);
}

TEST_F(ChunkListTest, resizeBoundary) {
    cl1->resize(cl1->CHUNK_CAPACITY - 1);
    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY - 1);
    cl1->at(cl1->CHUNK_CAPACITY - 2) = 100;
    cl1->resize(cl1->CHUNK_CAPACITY, 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->back(), 101);
    cl1->resize(cl1->CHUNK_CAPACITY + 1, 102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY), 102);
    ASSERT_EQ(cl1->back(), 102);
    cl1->resize(cl1->CHUNK_CAPACITY + 2, 103);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY + 1), 103);
    ASSERT_EQ(cl1->back(), 103);

    cl1->resize(cl1->CHUNK_CAPACITY + 1, 1001);
    ASSERT_EQ(cl1->back(), 102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY), 102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
    cl1->resize(cl1->CHUNK_CAPACITY, 1001);
    ASSERT_EQ(cl1->back(), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
    cl1->resize(cl1->CHUNK_CAPACITY - 1, 1001);
    ASSERT_EQ(cl1->back(), 100);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
}

TEST_F(ChunkListTest, pushBackPopBackBoundary) {
    cl1->resize(cl1->CHUNK_CAPACITY - 1);
    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY - 1);
    cl1->at(cl1->CHUNK_CAPACITY - 2) = 100;
    cl1->push_back(101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->back(), 101);
    cl1->push_back(102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY), 102);
    ASSERT_EQ(cl1->back(), 102);
    cl1->push_back(103);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY + 1), 103);
    ASSERT_EQ(cl1->back(), 103);

    cl1->pop_back();
    ASSERT_EQ(cl1->back(), 102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY), 102);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
    cl1->pop_back();
    ASSERT_EQ(cl1->back(), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 1), 101);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
    cl1->pop_back();
    ASSERT_EQ(cl1->back(), 100);
    ASSERT_EQ(cl1->at(cl1->CHUNK_CAPACITY - 2), 100);
}

TEST_F(ChunkListTest, clearAndEmptyAndChunkCount) {
    ASSERT_FALSE(cl1->empty());
    cl1->push_back(2);
    ASSERT_FALSE(cl1->empty());
    ASSERT_EQ(cl1->chunk_count(), 2);
    cl1->clear();
    ASSERT_TRUE(cl1->empty());
    ASSERT_EQ(cl1->chunk_count(), 0);
    cl1->push_back(2);
    ASSERT_FALSE(cl1->empty());
    ASSERT_EQ(cl1->chunk_count(), 1);

    ASSERT_TRUE(cl2->empty());
    ASSERT_EQ(cl2->chunk_count(), 0);
    cl2->push_back({1, 1});
    ASSERT_FALSE(cl2->empty());
    ASSERT_EQ(cl2->chunk_count(), 1);
    cl2->clear();
    ASSERT_TRUE(cl2->empty());
    ASSERT_EQ(cl2->chunk_count(), 0);
}

TEST_F(ChunkListTest, iterators) {
    ASSERT_EQ(cl1->front(), *(cl1->begin()));
    ASSERT_EQ(cl1->back(), *(--cl1->end()));
    size_t cnt = 0;
    for (auto it = cl1->begin(); it != cl1->end(); it++) cnt++;
    ASSERT_EQ(cl1->size(), cnt);

    cl1->clear();
    cl1->resize(cl1->CHUNK_CAPACITY + 10, -2);
    for (auto it = cl1->begin(); it != cl1->end(); ++it) {
        ASSERT_EQ(-2, *it);
        *it = 2;
    }
    ASSERT_EQ(cl1->end(), std::find_if_not(cl1->begin(), cl1->end(), [](int n){ return n == 2; }));

    const auto* cl = cl1;
    ASSERT_EQ(cl->front(), *(cl->cbegin()));
    ASSERT_GE(cl->back(), *(--cl->cend()));
    cnt = 0;
    for (auto it = cl->cbegin(); it != cl->cend(); it++) cnt++;
    ASSERT_EQ(cl->size(), cnt);

    auto it1 = cl->cbegin();
    decltype(it1) it2 = cl->begin();
    for (; it1 != cl->cend() && it2 != cl->end(); ++it1, ++it2) {
        ASSERT_EQ(2, *it1);
        ASSERT_EQ(it1, it2);
    }

    cl2->clear();
    cl2->resize(cl2->CHUNK_CAPACITY * 2 + 10, UnalignedType{1, 2});
    for (auto it = cl2->cbegin(); it != cl2->cend(); it++) {
        ASSERT_EQ(1, it->a);
        ASSERT_EQ(2, it->b);
    }
    for (auto it = cl2->begin(); it != cl2->end(); ++it) {
        it->b = 1;
    }
    ASSERT_EQ(cl2->cend(), std::find_if_not(cl2->cbegin(), cl2->cend(),
                [](const UnalignedType& x){ return x.a == 1 && x.b == 1; }));
}

TEST_F(ChunkListTest, iteratorIncDec) {
    auto pre = cl1->begin();
    auto post = cl1->begin();
    while (pre != cl1->end() && post != cl1->end()) {
        ASSERT_EQ(pre, post);
        ASSERT_EQ(*pre, *post);
        auto postExp = post;
        auto preRet = ++pre;
        auto postRet = post++;
        auto preExp = pre;
        ASSERT_EQ(preExp, preRet);
        ASSERT_EQ(postExp, postRet);
    }

    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY + 3);
    std::array<int,
        std::remove_reference<decltype(*cl1)>::type::CHUNK_CAPACITY + 3> contents;
    std::copy(cl1->begin(), cl1->end(), contents.begin());

    ASSERT_EQ(cl1->end(), pre);
    ASSERT_EQ(cl1->end(), post);
    auto it = contents.rbegin();
    do {
        auto postExp = post;
        auto preRet = --pre;
        auto postRet = post--;
        auto preExp = pre;
        ASSERT_EQ(preExp, preRet);
        ASSERT_EQ(postExp, postRet);
        ASSERT_EQ(pre, post);
        ASSERT_EQ(*pre, *it);
        ASSERT_EQ(*post, *it);
        it++;
    } while (pre != cl1->begin() && post != cl1->begin());
}

TEST_F(ChunkListTest, iteratorUpdate) {
    // Existing iterators are still valid after push_back.

    for (size_t i = 0; i < 10; i++) cl1->push_back(100 + i);

    auto beg = cl1->begin();
    auto last1 = --cl1->end();
    const auto& lastVal1 = *last1;

    cl1->push_back(200);
    ASSERT_EQ(beg, cl1->begin());
    ASSERT_EQ(&lastVal1, &(*last1));
    ASSERT_EQ(lastVal1, *last1);
    auto last2 = --cl1->end();
    const auto& lastVal2 = *last2;

    cl1->push_back(300);
    ASSERT_EQ(beg, cl1->begin());
    ASSERT_EQ(&lastVal1, &(*last1));
    ASSERT_EQ(&lastVal2, &(*last2));
    ASSERT_EQ(lastVal1, *last1);
    ASSERT_EQ(lastVal2, *last2);

    // Even across chunk boundary.
    auto chcnt = cl1->chunk_count();
    while (cl1->size() < chcnt * cl1->CHUNK_CAPACITY) cl1->push_back(cl1->size());
    ASSERT_EQ(chcnt, cl1->chunk_count());
    auto last3 = --cl1->end();
    const auto& lastVal3 = *last3;
    cl1->push_back(0);
    ASSERT_EQ(chcnt + 1, cl1->chunk_count());
    ASSERT_EQ(&lastVal3, &(*last3));
    ASSERT_EQ(lastVal3, *last3);
    ASSERT_EQ(last3, --(--cl1->end()));
}

TEST_F(ChunkListTest, at) {
    for (int i = 0; i < (int)cl1->size() - 3; i++) {
        ASSERT_EQ(7, cl1->at(i));
        cl1->at(i) = -i;
    }
    ASSERT_EQ(0, cl1->at(cl1->size() - 3));
    cl1->at(cl1->size() - 3) = -3;
    ASSERT_EQ(1, cl1->at(cl1->size() - 2));
    cl1->at(cl1->size() - 2) = -2;
    ASSERT_EQ(2, cl1->at(cl1->size() - 1));
    cl1->at(cl1->size() - 1) = -1;
    const auto* cl = cl1;
    for (int i = 0; i < (int)cl->size() - 3; i++) {
        ASSERT_EQ(-i, cl->at(i));
    }
    ASSERT_EQ(-3, cl->at(cl->size() - 3));
    ASSERT_EQ(-2, cl->at(cl->size() - 2));
    ASSERT_EQ(-1, cl->at(cl->size() - 1));
}

TEST_F(ChunkListTest, atOutOfRange) {
    ASSERT_THROW(cl1->at(cl1->size()) = 1, std::out_of_range);
    ASSERT_THROW(cl1->at(-1) = 1, std::out_of_range);

    const auto* cl = cl1;
    ASSERT_THROW(cl->at(cl->size()), std::out_of_range);
    ASSERT_THROW(cl->at(-1), std::out_of_range);

    ASSERT_NO_THROW(cl1->at(cl1->size() - 1) = 1);
    ASSERT_NO_THROW(cl->at(cl->size() - 1));
}

TEST_F(ChunkListTest, frontAndBack) {
    const auto* cl = cl1;
    ASSERT_EQ(cl->front(), 7);
    ASSERT_EQ(cl->back(), 2);

    cl1->push_back(3);
    ASSERT_EQ(cl1->front(), 7);
    ASSERT_EQ(cl1->back(), 3);

    cl1->front() = 10;
    cl1->back() = 20;
    ASSERT_EQ(cl1->front(), 10);
    ASSERT_EQ(cl1->back(), 20);
}

TEST_F(ChunkListTest, swap) {
    auto cl = new ChunkList<int>();
    ASSERT_EQ(cl1->size(), cl1->CHUNK_CAPACITY + 3);
    ASSERT_TRUE(cl->empty());

    cl1->swap(*cl);

    ASSERT_EQ(cl->size(), cl->CHUNK_CAPACITY + 3);
    ASSERT_TRUE(cl1->empty());
}

TEST_F(ChunkListTest, popBackException) {
    while (!cl1->empty()) cl1->pop_back();
    ASSERT_ANY_THROW(cl1->pop_back());

    ASSERT_ANY_THROW(cl2->pop_back());
}

TEST_F(ChunkListTest, chunkRange) {
    constexpr size_t chunk_count = 10;

    cl1->clear();
    for (size_t i = 0; i < cl1->CHUNK_CAPACITY * chunk_count - 20; i++) {
        cl1->push_back(i);
    }
    ASSERT_EQ(chunk_count, cl1->chunk_count());

    for (size_t c = 0; c < chunk_count; c++) {
        auto rng = cl1->chunk_range(c);
        size_t cnt = 0;
        for (auto it = rng.first; it != rng.second; ++it) {
            ASSERT_EQ(c * cl1->CHUNK_CAPACITY + cnt, *it);
            cnt++;
        }
        if (c == chunk_count - 1) cnt += 20;
        ASSERT_EQ(cl1->CHUNK_CAPACITY, cnt);
    }

    for (size_t c = chunk_count; c < 2 * chunk_count; c++) {
        auto rng = cl1->chunk_range(c);
        ASSERT_EQ(rng.first, rng.second);
    }
}

