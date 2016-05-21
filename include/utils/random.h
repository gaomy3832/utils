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
    typedef IntType uint64_t;
    typedef RealType double;

public:
    explicit Random(uint64_t seed)
            : prng(seed) {
        // Nothing else to do.
    }

    Random()
            : Random(std::random_device()()) {
        // Nothing else to do.
    }

    IntType getInteger() {
        return intDist(prng);
    }

    IntType getInteger(IntType min, IntType max) {
        assert(max >= min);
        if (max == min) return min;
        auto r = getInteger();
        auto span = max - min;
        if (span == std::numerical_limits<IntType>::max())
            return r;
        else
            return min + r % (span + 1);
    }

    RealType getReal() {
        return realDist(prng);
    }

    RealType getReal(RealType min, RealType max) {
        assert(max >= min);
        auto r = getReal();
        auto span = max - min;
        return min + r * span;
    }

private:
    std::mt19937_64 prng;

    std::uniform_int_distribution<uint64_t> intDist;
    std::uniform_real_distribution<double> realDist;
};

#endif  // UTILS_RANDOM_H_

