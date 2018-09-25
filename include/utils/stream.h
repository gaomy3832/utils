/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_STREAM_H_
#define UTILS_STREAM_H_

#include <algorithm>    // for std::sort
#include <utility>      // for std::swap
#include <vector>

/**
 * @addtogroup containers
 *
 * @{
 */

/**
 * @brief
 * Generic stream of data.
 */
template<typename Data>
class Stream {
public:
    typedef typename std::vector<Data>::iterator Iter;
    typedef typename std::vector<Data>::const_iterator ConstIter;

public:
    /**
     * @brief
     * Initialize stream.
     *
     * @param num    initial capacity in number of elements.
     */
    explicit Stream(size_t num = 16) {
        stream_.reserve(num);
    }

    ~Stream() {}

    /**
     * @name
     * Copy and move.
     */
    /**@{*/

    Stream(const Stream<Data>&) = delete;

    Stream<Data>& operator=(const Stream<Data>&) = delete;

    Stream(Stream<Data>&& s) {
        stream_.swap(s.stream_);
    }

    Stream<Data>& operator=(Stream<Data>&& s) {
        // Avoid self assign.
        if (this == &s) return *this;
        stream_.swap(s.stream_);
        return *this;
    }

    /**@}*/

    /**
     * @name
     * Member access.
     */
    /**@{*/

    /**
     * @return  raw pointer to the data.
     */
    Data* data() { return stream_.data(); }

    /**
     * @return  const raw pointer to the data.
     */
    const Data* data() const { return stream_.data(); }

    /**
     * @return  stream size in number of elements.
     */
    size_t size() const { return stream_.size(); }

    /**
     * @return  stream size in bytes.
     */
    size_t byte_size() const { return size() * sizeof(Data); }

    /**
     * @return  stream capacity in number of elements.
     */
    size_t capacity() const { return stream_.capacity(); }

    /**@}*/

    /**
     * @name
     * Iterator.
     */
    /**@{*/

    /**
     * @return  iterator to the elements in the stream.
     */
    Iter begin() { return stream_.begin(); }

    /**
     * @return  end of the iterator to the elements in the stream.
     */
    Iter end() { return stream_.end(); }

    /**
     * @return  const iterator to the elements in the stream.
     */
    ConstIter begin() const { return stream_.begin(); }

    /**
     * @return  end of the const iterator to the elements in the stream.
     */
    ConstIter end() const { return stream_.end(); }

    /**
     * @return  const iterator to the elements in the stream.
     */
    ConstIter cbegin() const { return stream_.cbegin(); }

    /**
     * @return  end of the const iterator to the elements in the stream.
     */
    ConstIter cend() const { return stream_.cend(); }

    /**@}*/

    /**
     * @name
     * Modifiers.
     */
    /**@{*/

    /**
     * @brief
     * Reset stream by clearing all elements.
     *
     * Initialize capacity to \c num elements.
     *
     * Warning: non-binding.
     *
     * @param num    reserved capacity in number of elements.
     */
    void reset(size_t num = 16) {
        if (num != stream_.capacity()) {
            // Non-binding request, as shrink_to_fit() is non-binding.
            stream_.resize(num);
            stream_.shrink_to_fit();
        }
        stream_.clear();
    }

    /**
     * @brief
     * Swap this stream with another stream \c s.
     *
     * @param s  the other stream to be swapped.
     */
    void swap(Stream<Data>& s) {
        stream_.swap(s.stream_);
    }

    /**
     * @brief
     * Append an element \c d to the stream.
     *
     * @param d  the element to be appended.
     */
    void put(const Data& d) {
        // The growth of the STL vector is implementation dependent, but it
        // usually grows exponentially as a nearly-optimal solution.
        stream_.push_back(d);
    }

    /**
     * @brief
     * Append an element \c d to the stream.
     *
     * @param d  the element to be appended.
     */
    void put(Data&& d) {
        stream_.push_back(std::forward<Data>(d));
    }

    /**
     * @brief
     * Sort the elements in the stream.
     */
    void sort() {
        std::sort(stream_.begin(), stream_.end());
    }

    /**@}*/

private:
    // Implemented using std::vector.
    std::vector<Data> stream_;
};

/**@}*/

#endif  // UTILS_STREAM_H_

