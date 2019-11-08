/*
 * Copyright 2018 Mingyu Gao
 *
 */
#include "gtest/gtest.h"
#include "utils/nested_iterator.h"
#include <algorithm>  // for std::find_if_not
#include <array>
#include <deque>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct UnalignedType { int a; char b; };

class NestedIteratorTest : public ::testing::Test {
protected:
    typedef std::list<std::vector<int>> TypeA;
    typedef std::deque<std::list<UnalignedType>> TypeB;
    typedef std::vector<std::unordered_set<uint64_t>> TypeC;
    typedef std::unordered_map<uint32_t, std::list<char>> TypeD;

    virtual void SetUp() {
        a = new TypeA();
        b = new TypeB();
        c = new TypeC();
        d = new TypeD();

        a->push_back({0, 1, 2});
        a->push_back({3});
        a->push_back({4, 5});
        a->push_back({6, 7, 8, 9});
        a->push_back({});
        a->push_back({10});
        a->push_back({});

        b->push_back({});
        b->push_back({});
        b->push_back({});

        c->push_back({100, 101, 102, 103});
        c->push_back({});
        c->push_back({104, 104});  // duplicate.

        (*d)[1001] = {'a'};
        (*d)[1002] = {'b', 'c', 'd'};
        (*d)[1003] = {};
        (*d)[1004] = {'e'};
    }

    virtual void TearDown() {
        delete a;
        delete b;
        delete c;
        delete d;
    }

    TypeA* a;
    TypeB* b;
    TypeC* c;
    TypeD* d;

    typedef NestedIterator<TypeA, TypeA::value_type> IterTypeA;
    typedef NestedIterator<TypeA, TypeA::value_type, ConstIterTagType> ConstIterTypeA;
    typedef NestedIterator<TypeB, TypeB::value_type> IterTypeB;
    typedef NestedIterator<TypeC, TypeC::value_type> IterTypeC;
    typedef NestedIterator<TypeC, TypeC::value_type, ConstIterTagType> ConstIterTypeC;
    typedef NestedIterator<TypeD, TypeD::mapped_type> IterTypeD;
};


TEST_F(NestedIteratorTest, iterate) {
    auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
    auto itAEnd = IterTypeA(a);
    int i = 0;
    while (itA != itAEnd) {
        ASSERT_EQ(i, *itA);
        itA++;
        i++;
    }
    ASSERT_EQ(11, i);

    const TypeC* cc = c;
    auto itC = ConstIterTypeC(cc, cc->cbegin(), cc->cbegin()->cbegin());
    auto itCEnd = ConstIterTypeC(cc);
    uint64_t n = 0;
    while (itC != itCEnd) {
        itC++;
        n++;
    }
    ASSERT_EQ(5, n);
}

TEST_F(NestedIteratorTest, empty) {
    a->clear();
    auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
    auto itAEnd = IterTypeA(a);
    ASSERT_EQ(itA, itAEnd);

    c->clear();
    const TypeC* cc = c;
    auto itC = ConstIterTypeC(cc, cc->cbegin(), cc->cbegin()->cbegin());
    auto itCEnd = ConstIterTypeC(cc);
    ASSERT_EQ(itC, itCEnd);

    auto itB = IterTypeB(b, b->begin(), b->begin()->begin());
    auto itBEnd = IterTypeB(b);
    ASSERT_EQ(itB, itBEnd);

    b->push_back({UnalignedType{1, 2}});
    b->push_back({});
    auto itB2 = IterTypeB(b, b->begin(), b->begin()->begin());
    auto itB2End = IterTypeB(b);
    int i = 0;
    while (itB2 != itB2End) {
        itB2++;
        i++;
    }
    ASSERT_EQ(1, i);
}

TEST_F(NestedIteratorTest, reference) {
    a->push_front({-1});
    a->push_back({-1});

    auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
    auto itAEnd = IterTypeA(a);
    ASSERT_EQ(&a->front().front(), &(*itA));
    ASSERT_EQ(&a->back().back(), &(*(--itAEnd)));
    itAEnd++;

    for (auto tit = a->begin(); tit != a->end(); tit++) {
        auto bsz = tit->size();
        for (size_t i = 0; i < bsz; i++) {
            ASSERT_EQ(tit->data() + i, &(*itA));
            itA++;
        }
    }
    ASSERT_EQ(itAEnd, itA);
}

TEST_F(NestedIteratorTest, modify) {
    size_t cnt = 0;
    for (auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
            itA != IterTypeA(a); itA++) {
        *itA = -2;
        cnt++;
    }
    ASSERT_EQ(IterTypeA(a), std::find_if_not(IterTypeA(a, a->begin(), a->begin()->begin()), IterTypeA(a), [](int n){ return n == -2; }));
    for (auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
            itA != IterTypeA(a); itA++) cnt--;
    ASSERT_EQ(0, cnt);
}

TEST_F(NestedIteratorTest, insert) {
    size_t cntA = 0;
    for (auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
            itA != IterTypeA(a); itA++) cntA++;
    auto titA = a->begin();
    titA++;
    titA++;
    titA->push_back(-1);
    titA++;
    a->insert(titA, {-2});
    size_t cntA2 = 0;
    for (auto itA = IterTypeA(a, a->begin(), a->begin()->begin());
            itA != IterTypeA(a); itA++) cntA2++;
    ASSERT_EQ(cntA + 2, cntA2);

    auto titB = b->begin();
    titB++;
    titB->push_back({1, 2});
    titB++;
    b->insert(titB, TypeB::value_type({UnalignedType{2, 1}}));
    size_t cntB2 = 0;
    for (auto itB = IterTypeB(b, b->begin(), b->begin()->begin());
            itB != IterTypeB(b); itB++) cntB2++;
    ASSERT_EQ(2, cntB2);
}

TEST_F(NestedIteratorTest, incDec) {
    auto pre = IterTypeA(a, a->begin(), a->begin()->begin());
    auto post = IterTypeA(a, a->begin(), a->begin()->begin());
    while (pre != IterTypeA(a) && post != IterTypeA(a)) {
        ASSERT_EQ(pre, post);
        ASSERT_EQ(*pre, *post);
        auto postExp = post;
        auto preRet = ++pre;
        auto postRet = post++;
        auto preExp = pre;
        ASSERT_EQ(preExp, preRet);
        ASSERT_EQ(postExp, postRet);
    }

    std::array<int, 11> contents;
    std::copy(ConstIterTypeA(a, a->cbegin(), a->cbegin()->cbegin()), ConstIterTypeA(a), contents.begin());

    ASSERT_EQ(IterTypeA(a), pre);
    ASSERT_EQ(IterTypeA(a), post);
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
    } while (pre != IterTypeA(a, a->begin(), a->begin()->begin()) && post != IterTypeA(a, a->begin(), a->begin()->begin()));
}

TEST_F(NestedIteratorTest, validation) {
    // Existing iterators are kept valid under certain cases.

    auto beg = IterTypeA(a, a->begin(), a->begin()->begin());
    auto last1 = --IterTypeA(a);
    const auto& lastVal1 = *last1;

    a->push_back({});
    ASSERT_EQ(beg, IterTypeA(a, a->begin(), a->begin()->begin()));
    ASSERT_EQ(&lastVal1, &(*last1));
    auto last2 = --IterTypeA(a);
    const auto& lastVal2 = *last2;

    a->back().push_back(300);
    ASSERT_EQ(beg, IterTypeA(a, a->begin(), a->begin()->begin()));
    ASSERT_EQ(&lastVal1, &(*last1));
    ASSERT_EQ(&lastVal2, &(*last2));
    ASSERT_EQ(lastVal1, *last1);
    ASSERT_EQ(lastVal2, *last2);

    // Even in the middle for std::list.
    auto tit = a->begin();
    for (int i = 0; i < 3; i++) tit++;
    ASSERT_EQ(6, tit->front());
    ASSERT_EQ(4, tit->size());
    tit->insert(tit->begin() + 2, -1);
    ASSERT_EQ(5, tit->size());
    a->insert(tit, {});
    ASSERT_EQ(beg, IterTypeA(a, a->begin(), a->begin()->begin()));
    ASSERT_EQ(&lastVal1, &(*last1));
    ASSERT_EQ(&lastVal2, &(*last2));
}

TEST_F(NestedIteratorTest, mappedType) {
    auto itD = IterTypeD(d, d->begin(), d->begin()->second.begin());
    auto itDEnd = IterTypeD(d);
    int i = 0;
    while (itD != itDEnd) {
        ASSERT_LE('a', *itD);
        ASSERT_LE(*itD, 'e');
        itD++;
        i++;
    }
    ASSERT_EQ(5, i);
}

