/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_RANDOM_H_
#define UTILS_RANDOM_H_
/**
 * Random number generator.
 */
#include <limits>
#include <random>
#include "utils/log.h"

class Random {
public:
    typedef uint64_t IntType;
    typedef double RealType;

public:
    explicit Random(uint64_t seed)
            : prng_(seed) {
        // Nothing else to do.
    }

    Random()
            : Random(std::random_device()()) {
        // Nothing else to do.
    }

    IntType get_integer() {
        return intDist_(prng_);
    }

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

    RealType get_real() {
        return realDist_(prng_);
    }

    RealType get_real(RealType min, RealType max) {
        assert(max >= min);
        auto r = get_real();
        auto span = max - min;
        return min + r * span;
    }

private:
    std::mt19937_64 prng_;

    std::uniform_int_distribution<uint64_t> intDist_;
    std::uniform_real_distribution<double> realDist_;
};

#endif  // UTILS_RANDOM_H_

