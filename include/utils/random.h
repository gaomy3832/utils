/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_

#include <limits>
#include <random>
#include "utils/log.h"

/**
 * @brief
 * Random number generator.
 */
class Random {
public:
    typedef uint64_t IntType;
    typedef double RealType;

public:
    /**
     * @brief
     * Initialize the random number generator.
     *
     * @param seed  the seed for the generator.
     */
    explicit Random(uint64_t seed)
            : prng_(seed) {
        // Nothing else to do.
    }

    /**
     * @brief
     * Initialize the random number generator with random seed.
     */
    Random()
            : Random(std::random_device()()) {
        // Nothing else to do.
    }

    /**
     * @brief
     * Get a random integer.
     *
     * @return  a random integer.
     */
    IntType get_integer() {
        return intDist_(prng_);
    }

    /**
     * @brief
     * Get a random integer in a range.
     *
     * @param min, max  inclusive range of the random number.
     * @return  a random integer.
     */
    IntType get_integer(IntType min, IntType max) {
        assert(max >= min);
        if (max == min) return min;
        auto r = get_integer();
        auto span = max - min;
        if (span == std::numeric_limits<IntType>::max())
            return r;
        else
            return min + r % (span + 1);
    }

    /**
     * @brief
     * Get a random floating-point number.
     *
     * @return  a random floating-point number.
     */
    RealType get_real() {
        return realDist_(prng_);
    }

    /**
     * @brief
     * Get a random floating-point number in a range.
     *
     * @param min, max  range of the random number.
     * @return  a random floating-point number.
     */
    RealType get_real(RealType min, RealType max) {
        assert(max >= min);
        auto r = get_real();
        auto span = max - min;
        return min + r * span;
    }

private:
    // The pseudo random number generator.
    std::mt19937_64 prng_;

    // Integer distribution.
    std::uniform_int_distribution<uint64_t> intDist_;
    // Floating-point distribution.
    std::uniform_real_distribution<double> realDist_;
};

#endif  // UTILS_RANDOM_H_

