/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_PADDED_H_
#define UTILS_PADDED_H_

/**
 * @addtogroup containers
 *
 * @{
 */

/**
 * @brief
 * Padded data, aligned to \c ALIGNED_SIZE.
 */
template<unsigned ALIGNED_SIZE, typename T>
class alignas(ALIGNED_SIZE) Padded {
public:
    /**
     * @brief The data type being padded.
     */
    using value_type = T;

    /**
     * @brief The alignment requirement of the padded data.
     */
    static constexpr unsigned ALIGN = ALIGNED_SIZE;

public:
    /**
     * @name
     * Pad.
     */
    /**@{*/

    Padded(const value_type& value) : value_(value) {}

    Padded(value_type&& value) : value_(std::forward<value_type>(value)) {}

    Padded& operator=(const value_type& value) {
        value_ = value;
        return *this;
    }

    Padded& operator=(value_type&& value) {
        value_ = std::forward<value_type>(value);
        return *this;
    }

    /**@}*/

    /**
     * @name
     * Unpack.
     */
    /**@{*/

    value_type value() const { return value_; }

    value_type& value() { return value_; }

    operator value_type() const { return value_; }

    operator value_type&() { return value_; }

    /**@}*/

    /**
     * @name
     * Comparison.
     */
    /**@{*/

    template<typename U, typename SFINAEType = value_type>
    inline auto operator==(U other) const
        -> decltype(std::declval<SFINAEType>() == other)
    {
        return value_ == other;
    }

    template<typename U, typename SFINAEType = value_type>
    inline auto operator!=(U other) const
        -> decltype(std::declval<SFINAEType>() == other)
    {
        return !operator==(other);
    }

    template<typename U, typename SFINAEType = value_type>
    inline auto operator<(U other) const
        -> decltype(std::declval<SFINAEType>() < other)
    {
        return value_ < other;
    }

    template<typename U, typename SFINAEType = value_type>
    inline auto operator>(U other) const
        -> decltype(other < std::declval<SFINAEType>())
    {
        return other < value_;
    }

    template<typename U, typename SFINAEType = value_type>
    inline auto operator<=(U other) const
        -> decltype(other < std::declval<SFINAEType>())
    {
        return !operator>(other);
    }

    template<typename U, typename SFINAEType = value_type>
    inline auto operator>=(U other) const
        -> decltype(std::declval<SFINAEType>() < other)
    {
        return !operator<(other);
    }

    /**@}*/

private:
    value_type value_;
};

/**@}*/

#endif  // UTILS_PADDED_H_

