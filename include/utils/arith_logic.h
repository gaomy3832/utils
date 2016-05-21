/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_ARITH_LOGIC_H_
#define UTILS_ARITH_LOGIC_H_
/**
 * Basic arithmetic and logic operations.
 */
#include <cstdint>

/**
 * Bithack operations.
 *
 * See the bithacks website
 * http://graphics.stanford.edu/~seander/bithacks.html
 */
// Integer log2.
template<typename T> static inline uint32_t ilog2(T val);
// Only specializations of unsigned types (no calling these with ints)
// __builtin_clz is undefined for 0 (internally, this uses bsr in x86-64)
template<> uint32_t ilog2<uint32_t>(uint32_t val) {
    return val? 31 - __builtin_clz(val) : 0;
}
template<> uint32_t ilog2<uint64_t>(uint64_t val) {
    return val? 63 - __builtin_clzl(val) : 0;
}

// Test power of 2.
template<typename T>
static inline bool isPow2(T val) {
    return val && !(val & (val - 1));
}

#endif  // UTILS_ARITH_LOGIC_H_

