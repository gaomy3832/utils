/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_STREAM_H_
#define UTILS_STREAM_H_

#include <atomic>
#include <utility>  // for std::pair
#include "utils/chunk_list.h"

/**
 * @addtogroup containers
 *
 * @{
 */

/**
 * @brief
 * Lock-free stream of data between a producer-consumer pair.
 */
template<typename T, size_t C = 65536, class Allocator = std::allocator<T>>
class Stream {
private:
    // Implemented using ChunkList to avoid data reallocation.
    using StorageType = ChunkList<T, C, Allocator>;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename StorageType::iterator;
    using const_iterator = typename StorageType::const_iterator;

public:
    /**
     * @brief
     * Initialize a stream.
     */
    Stream()
        : data_(), getPos_(0), putPos_(0)
    {
        // A dummy element.
        data_.resize(1);
        getIter_ = data_.cbegin();
    }

    ~Stream() {}

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    Stream(Stream&& other) {
        data_.swap(other.data_);
        getPos_ = other.getPos_;
        putPos_.store(other.putPos_.load(std::memory_order_acquire), std::memory_order_release);
        getIter_ = other.getIter_;

        other.clear();
    }

    Stream& operator=(Stream&& other) {
        // Avoid self assign.
        if (this == &other) return *this;

        data_.swap(other.data_);
        getPos_ = other.getPos_;
        putPos_.store(other.putPos_.load(std::memory_order_acquire), std::memory_order_release);
        getIter_ = other.getIter_;

        other.clear();

        return *this;
    }

    /**
     * @name
     * Size access.
     */
    /**@{*/

    /**
     * @brief Check whether the stream is empty.
     */
    inline bool empty() const { return getPos_ == putPos_; }

    /**
     * @brief Return the number of elements.
     */
    inline size_type size() const { return putPos_ - getPos_; }

    /**
     * @brief Return the stream size in bytes.
     */
    inline size_type byte_size() const { return size() * sizeof(value_type); }

    /**@}*/

    /**
     * @name
     * Iterators.
     */
    /**@{*/

    /**
     * @brief Return a range of elements to get from the stream.
     */
    std::pair<const_iterator, const_iterator> get_range() {
        // Atomically acquire the current stream end.
        auto pos = putPos_.load(std::memory_order_acquire);
        assert(pos < data_.size());
        auto it = data_.cend();
        const auto* ptr = &data_.at(pos);
        while (&(*(--it)) != ptr) {}
        // Now the iterator points the to the stream end. Return the interval
        // and move the get position.
        auto range = std::make_pair(getIter_, it);
        getIter_ = it;
        getPos_ = pos;
        return range;
    }

    /**@}*/

    /**
     * @name
     * Modifiers.
     */
    /**@{*/

    /**
     * @brief Clear the contents.
     */
    void clear() {
        data_.resize(1);
        getPos_ = 0;
        putPos_ = 0;
        getIter_ = data_.cbegin();
    }

    /**
     * @brief Put an element to the stream.
     */
    void put(const value_type& value) {
        data_.back() = value;
        data_.resize(data_.size() + 1);
        auto pos = putPos_.fetch_add(1, std::memory_order_release);
        assert(pos + 1 == data_.size() - 1);
    }

    /**
     * @brief Put an element to the stream.
     */
    void put(value_type&& value) {
        data_.back() = std::move(std::forward<value_type>(value));
        data_.resize(data_.size() + 1);
        auto pos = putPos_.fetch_add(1, std::memory_order_release);
        assert(pos + 1 == data_.size() - 1);
    }

    /**@}*/

private:
    StorageType data_;

    // Points to the first element that has not been got.
    size_type getPos_;
    // Points to a dummy element after the last one that has been put.
    std::atomic<size_type> putPos_;

    // The iterator pointing to the get position.
    const_iterator getIter_;
};

/**@}*/

#endif  // UTILS_STREAM_H_

