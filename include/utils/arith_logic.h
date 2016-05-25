/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_ARITH_LOGIC_H_
#define UTILS_ARITH_LOGIC_H_
/**
 * @file
 *
 * @brief
 * Basic arithmetic and logic operations.
 */

#include <cstdint>

/**
 * @name Bithack operations.
 *
 * See the bithacks website
 * http://graphics.stanford.edu/~seander/bithacks.html
 */
/**@{*/

/**
 * Test power of 2.
 *
 * @param val  Value to be tested.
 * @return     Whether \c val is a power of 2.
 */
template<typename T> inline bool isPow2(T val) {
    return val && !(val & (val - 1));
}

/**
 * Integer log2. Round down to the nearest integer.
 *
 * When \c val is 0, return 0.
 */
template<typename T> inline uint32_t ilog2(T val);

/**
 * Ceiling integer log2. Round up to the nearest integer.
 *
 * When \c val is 0, return 0.
 */
template<typename T> inline uint32_t cilog2(T val) {
    auto m = ilog2(val);
    return !val || isPow2(val) ? m : m + 1;
}

#ifdef __GNUC__
// Only specializations of unsigned types (no calling these with ints)
// __builtin_clz is undefined for 0 (internally, this uses bsr in x86-64)
template<> uint32_t ilog2<uint32_t>(uint32_t val) {
    return val? 31 - __builtin_clz(val) : 0;
}
template<> uint32_t ilog2<uint64_t>(uint64_t val) {
    return val? 63 - __builtin_clzl(val) : 0;
}
#endif

/**@}*/

#endif  // UTILS_ARITH_LOGIC_H_

