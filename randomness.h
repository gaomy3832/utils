#ifndef UTILS_RANDOMNESS_H
#define UTILS_RANDOMNESS_H

#include <random>   // for std::mt19937, std::uniform_real_distribution

template<typename Type>
class UniformRand {
    public:
        UniformRand(Type min, Type max, unsigned seed) : gen_(seed), dst_(min, max) {}

        Type get() { return dst_(gen_); }

        template<typename Iter>
        void assign(Iter begin, Iter end) {
            for (Iter it = begin; it != end; it++) {
                *it = get();
            }
        }

    protected:
        std::mt19937 gen_;
        std::uniform_real_distribution<Type> dst_;
};

#endif // UTILS_RANDOMNESS_H

