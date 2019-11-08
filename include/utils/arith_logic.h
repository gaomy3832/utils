/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_ARITH_LOGIC_H_
#define UTILS_ARITH_LOGIC_H_

#include <cstdint>

/**
 * @addtogroup bit_hack Bithack operations
 *
 * See the bithacks website
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 * @{
 */

/**
 * @brief
 * Test power of 2.
 *
 * @param val  Value to be tested.
 * @return     Whether \c val is a power of 2.
 */
template<typename T> inline bool isPow2(T val) {
    return val && !(val & (val - 1));
}

/**
 * @brief
 * Integer log2.
 *
 * Round down to the nearest integer.
 *
 * When \c val is 0, return 0.
 */
template<typename T> inline uint32_t ilog2(T val);

/**
 * @brief
 * Ceiling integer log2.
 *
 * Round up to the nearest integer.
 *
 * When \c val is 0, return 0.
 */
template<typename T> inline uint32_t cilog2(T val) {
    auto m = ilog2(val);
    return !val || isPow2(val) ? m : m + 1;
}

#if defined(__GNUC__) || defined(__clang__)
// Only specializations of unsigned types (no calling these with ints)
// __builtin_clz is undefined for 0 (internally, this uses bsr in x86-64)
template<> uint32_t ilog2<unsigned int>(unsigned int val) {
    return val? sizeof(unsigned int) * 8 - 1 - __builtin_clz(val) : 0;
}
template<> uint32_t ilog2<unsigned long>(unsigned long val) {
    return val? sizeof(unsigned long) * 8 - 1 - __builtin_clzl(val) : 0;
}
template<> uint32_t ilog2<unsigned long long>(unsigned long long val) {
    return val? sizeof(unsigned long long) * 8 - 1 - __builtin_clzll(val) : 0;
}
#endif

/**@}*/

#endif  // UTILS_ARITH_LOGIC_H_

