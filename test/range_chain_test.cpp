/*
 * Copyright 2018 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/range_chain.h"
#include <algorithm>  // for std::copy
#include <array>
#include <list>
#include <unordered_map>

class RangeChainTest : public ::testing::Test {
protected:
    typedef std::list<int> TypeA;
    typedef std::unordered_map<uint32_t, char> TypeB;

    typedef typename TypeA::iterator IterTypeA;
    typedef typename TypeA::const_iterator ConstIterTypeA;
    typedef typename TypeB::iterator IterTypeB;
    typedef typename TypeB::const_iterator ConstIterTypeB;

    typedef RangeChain<IterTypeA> RangeChainA;
    typedef RangeChain<ConstIterTypeA> RangeChainConstA;
    typedef RangeChain<IterTypeB> RangeChainB;
    typedef RangeChain<ConstIterTypeB> RangeChainConstB;

    virtual void SetUp() {
        a1 = TypeA({0, 1, 2});
        a2 = TypeA({3, 4});
        b1 = TypeB({{0, 'a'}, {1, 'b'}, {2, 'c'}});
        b2 = TypeB({{3, 'd'}});

        rca = new RangeChainA();
        rcb = new RangeChainB();
        rcca = new RangeChainConstA();
        rccb = new RangeChainConstB();

        rca->add(a1.begin(), a1.end());
        rca->add(a1.end(), a1.end());
        rca->add(a2.begin(), a2.end());

        rccb->add(b1.cbegin(), b1.cend());
        rccb->add(b2.cbegin(), b2.cend());
    }

    virtual void TearDown() {
        delete rca;
        delete rcb;
        delete rcca;
        delete rccb;
    }

    RangeChainA* rca;
    RangeChainB* rcb;
    RangeChainConstA* rcca;
    RangeChainConstB* rccb;

    TypeA a1;
    TypeA a2;
    TypeB b1;
    TypeB b2;
};


TEST_F(RangeChainTest, iterate) {
    int i = 0;
    for (auto it = rca->begin(); it != rca->end(); ++it) {
        ASSERT_EQ(i, *it);
        i++;
    }
    ASSERT_EQ(5, i);

    i = 0;
    for (auto it = rccb->begin(); it != rccb->end(); ++it) {
        ASSERT_EQ('a' + it->first, it->second);
        i++;
    }
    ASSERT_EQ(4, i);
}

TEST_F(RangeChainTest, clearAndEmpty) {
    ASSERT_FALSE(rca->empty());
    ASSERT_TRUE(rcb->empty());
    ASSERT_TRUE(rcca->empty());
    ASSERT_FALSE(rccb->empty());

    ASSERT_EQ(rcb->begin(), rcb->end());
    ASSERT_EQ(rcca->begin(), rcca->end());

    rca->clear();
    ASSERT_TRUE(rca->empty());
}

TEST_F(RangeChainTest, emptyRanges) {
    ASSERT_TRUE(rcb->empty());
    rcb->add(b1.begin(), b1.begin());
    rcb->add(b2.end(), b2.end());
    ASSERT_TRUE(rcb->empty());

    ASSERT_TRUE(rcca->empty());
    rcca->add(a1.begin(), a1.begin());
    rcca->add(a2.end(), a2.end());
    ASSERT_TRUE(rcca->empty());

    rca->add(a1.end(), a1.end());
    rca->add(a1.begin(), a1.end());
    size_t cnt = 0;
    for (auto it = rca->begin(); it != rca->end(); it++) {
        cnt++;
    }
    ASSERT_EQ(a1.size() * 2 + a2.size(), cnt);
}

TEST_F(RangeChainTest, iteratorCategory) {
    static_assert(std::is_same<
            std::random_access_iterator_tag,
            typename std::iterator_traits<typename std::vector<int>::iterator>::iterator_category>::value,
            "");
    typedef RangeChain<typename std::vector<int>::iterator> RC;
    static_assert(std::is_same<
            std::bidirectional_iterator_tag,
            typename std::iterator_traits<typename RC::iterator>::iterator_category>::value,
            "RangeChain iterator can at most be bidirectional");
}

TEST_F(RangeChainTest, iteratorDereference) {
    ASSERT_EQ(0, *rca->begin());
    auto it = rca->begin();
    *it = -1;
    ASSERT_EQ(-1, *rca->begin());

    ASSERT_EQ(rccb->begin()->first + 'a', rccb->begin()->second);
}

TEST_F(RangeChainTest, iteratorIncDec) {
    auto pre = rca->begin();
    auto post = rca->begin();
    while (pre != rca->end() && post != rca->end()) {
        ASSERT_EQ(pre, post);
        ASSERT_EQ(*pre, *post);
        auto postExp = post;
        auto preRet = ++pre;
        auto postRet = post++;
        auto preExp = pre;
        ASSERT_EQ(preExp, preRet);
        ASSERT_EQ(postExp, postRet);
    }

    std::array<int, 5> contents;
    std::copy(rca->begin(), rca->end(), contents.begin());

    ASSERT_EQ(rca->end(), pre);
    ASSERT_EQ(rca->end(), post);
    auto it = contents.rbegin();
    do {
        ASSERT_EQ(pre, post);
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
    } while (pre != rca->begin() && post != rca->begin());
}

