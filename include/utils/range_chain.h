/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_RANGE_CHAIN_H_
#define UTILS_RANGE_CHAIN_H_

#include <initializer_list>
#include <iterator>  // for std::iterator_traits
#include <type_traits>
#include <utility>  // for std::pair
#include <vector>

/**
 * @addtogroup iterator
 *
 * @{
 */

/**
 * @brief
 * A chain of multiple ranges that can be iterated continuously. Each range is
 * given as a pair of iterators.
 */
template<typename IterType>
class RangeChain {
public:
    using RangeType = std::pair<IterType, IterType>;

public:
    class iterator {
    public:
        /**
         * @name
         * Iterator type traits.
         */
        /**@{*/

        using value_type = typename std::iterator_traits<IterType>::value_type;
        using difference_type = typename std::iterator_traits<IterType>::difference_type;
        using pointer = typename std::iterator_traits<IterType>::pointer;
        using reference = typename std::iterator_traits<IterType>::reference;
        using iterator_category = typename std::conditional<
            std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<IterType>::iterator_category>::value,
            std::bidirectional_iterator_tag,
            typename std::iterator_traits<IterType>::iterator_category>::type;

        /**@}*/

    public:
        /**
         * @name
         * Construction.
         */
        /**@{*/

        iterator(const RangeChain* rc, size_t pos, IterType it)
            : rc_(rc), pos_(pos), it_(it)
        {
            if (pos_ >= rc->ranges_.size()) it_ = NULL_ITER;
            while (it_ == curr_range_end()) {
                // Reach the end of the current range. Jump to the next range.
                bool stop = (pos_ >= rc_->ranges_.size());
                if (stop) break;
                pos_++;
                it_ = curr_range_begin();
            }
        }

        /**
         * @brief Construct an iterator that starts at one of the ranges.
         */
        iterator(const RangeChain* rc, size_t pos)
            : iterator(rc, pos, pos >= rc->ranges_.size() ? IterType() : rc->ranges_.at(pos).first)
        {
            // Nothing else to do.
        }

        /**
         * @brief Construct a past-the-end iterator.
         */
        explicit iterator(const RangeChain* rc)
            : iterator(rc, rc->ranges_.size())
        {
            // Nothing else to do.
        }

        ~iterator() = default;
        iterator(const iterator&) = default;
        iterator& operator=(const iterator&) = default;
        iterator(iterator&&) = default;
        iterator& operator=(iterator&&) = default;

        /**@}*/

        /**
         * @name
         * Dereference.
         */
        /**@{*/

        reference operator*() const { return *it_; }

        pointer operator->() const { return &(this->operator*()); }

        /**@}*/

        /**
         * @name
         * Comparison.
         */
        /**@{*/

        bool operator==(const iterator& other) const {
            return rc_ == other.rc_ && pos_ == other.pos_ && it_ == other.it_;
        }

        bool operator!=(const iterator& other) const { return !(this->operator==(other)); }

        /**@}*/

        /**
         * @name
         * Advance.
         */
        /**@{*/

        iterator& operator++() {
            if (it_ != IterType()) it_++;
            while (it_ == curr_range_end()) {
                // Reach the end of the current range. Jump to the next range.
                bool stop = (pos_ >= rc_->ranges_.size());
                if (stop) break;
                pos_++;
                it_ = curr_range_begin();
            }
            return *this;
        }

        iterator operator++(int) { auto it = *this; ++(*this); return it; }

        template<typename T = IterType>
        typename std::enable_if<
            std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<T>::iterator_category>::value,
            iterator&>::type
        operator--() {
            while (it_ == curr_range_begin()) {
                // Reach the beginning of the current range. Jump to the previous range.
                bool stop = (pos_ == 0);
                pos_--;
                it_ = curr_range_end();
                if (stop) break;
            }
            if (it_ != IterType()) it_--;
            return *this;
        }

        template<typename T = IterType>
        typename std::enable_if<
            std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<T>::iterator_category>::value,
            iterator>::type
        operator--(int) { auto it = *this; --(*this); return it; }

        /**@{*/

    private:
        inline IterType curr_range_begin() const {
            try {
                return rc_->ranges_.at(pos_).first;
            } catch (...) {
                return IterType();
            }
        }

        inline IterType curr_range_end() const {
            try {
                return rc_->ranges_.at(pos_).second;
            } catch (...) {
                return IterType();
            }
        }

    private:
        const RangeChain* rc_;
        size_t pos_;
        IterType it_;

        static const IterType NULL_ITER;

        // Before-begin iterator: (-1uL, NULL_ITER)
        // Past-the-end iterator: (rc_->ranges_.size(), NULL_ITER)
    };

public:
    /**
     * @name
     * Construction.
     */
    /**@{*/

    RangeChain()
        : ranges_()
    {
        // Nothing else to do.
    }

    RangeChain(std::initializer_list<RangeType> init)
        : ranges_(init)
    {
        // Nothing else to do.
    }

    ~RangeChain() = default;
    RangeChain(const RangeChain&) = default;
    RangeChain& operator=(const RangeChain&) = default;
    RangeChain(RangeChain&&) = default;
    RangeChain& operator=(RangeChain&&) = default;

    /**@}*/

    /**
     * @name
     * Add a range.
     */
    /**@{*/

    inline void add(const RangeType& range) { ranges_.push_back(range); }

    inline void add(RangeType&& range) { ranges_.push_back(std::move(range)); }

    inline void add(const IterType& first, const IterType& second) { add(std::make_pair(first, second)); }

    /**@}*/

    /**
     * @name
     * Iterators.
     */
    /**@{*/

    /**
     * @brief Return an iterator to the beginning.
     */
    inline iterator begin() { return iterator(this, 0); }

    /**
     * @brief Return an iterator to the end.
     */
    inline iterator end() { return iterator(this); }

    /**@}*/

    /**
     * @brief Whether the range chain is empty.
     */
    bool empty() const { return iterator(this, 0) == iterator(this); }

    /**
     * @brief Clear the contents.
     */
    void clear() { ranges_.clear(); }

private:
    std::vector<RangeType> ranges_;
};

template<typename IterType>
const IterType RangeChain<IterType>::iterator::NULL_ITER = IterType();

/**@}*/

#endif  // UTILS_RANGE_CHAIN_H_

