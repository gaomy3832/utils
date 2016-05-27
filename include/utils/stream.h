/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_STREAM_H_
#define UTILS_STREAM_H_
/**
 * Generic FIFO stream.
 *
 * Implemented by std::vector.
 */
#include <algorithm>    // for std::sort
#include <utility>      // for std::swap
#include <vector>

template<typename Data>
class Stream {
public:
    typedef typename std::vector<Data>::iterator Iter;
    typedef typename std::vector<Data>::const_iterator ConstIter;

public:
    /**
     * Initialize stream.
     *
     * @param num    initial number of elements.
     */
    explicit Stream(size_t num = 16) {
        stream_.reserve(num);
    }

    ~Stream() {}

    /* Copy and move */

    Stream(const Stream<Data>&) = delete;

    Stream<Data>& operator=(const Stream<Data>&) = delete;

    /**
     * Move constructor.
     */
    Stream(Stream<Data>&& s) {
        stream_.swap(s.stream_);
    }

    /**
     * Move assignment.
     */
    Stream<Data>& operator=(Stream<Data>&& s) {
        // Avoid self assign.
        if (this == &s) return *this;
        stream_.swap(s.stream_);
        return *this;
    }

    /* Member access */

    /**
     * Get raw pointer to the data.
     */
    Data* data() { return stream_.data(); }

    /**
     * Get const raw pointer to the data.
     */
    const Data* data() const { return stream_.data(); }

    /**
     * Get stream size in number of elements.
     */
    size_t size() const { return stream_.size(); }

    /**
     * Get stream size in bytes.
     */
    size_t byte_size() const { return size() * sizeof(Data); }

    /**
     * Get stream capacity in number of elements.
     */
    size_t capacity() const { return stream_.capacity(); }

    /**
     * Get iterator to the elements in the stream.
     */
    Iter begin() { return stream_.begin(); }

    /**
     * Get end of the iterator to the elements in the stream.
     */
    Iter end() { return stream_.end(); }

    /**
     * Get const iterator to the elements in the stream.
     */
    ConstIter begin() const { return stream_.begin(); }

    /**
     * Get end of the const iterator to the elements in the stream.
     */
    ConstIter end() const { return stream_.end(); }

    /**
     * Get const iterator to the elements in the stream.
     */
    ConstIter cbegin() const { return stream_.cbegin(); }

    /**
     * Get end of the const iterator to the elements in the stream.
     */
    ConstIter cend() const { return stream_.cend(); }

    /* Modifiers */

    /**
     * Reset stream. Clear all elements and initialize to \c num elements.
     * Warning: non-binding.
     *
     * @param num    reserved number of elements.
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
     * Swap this stream with another stream \c s.
     */
    void swap(Stream<Data>& s) {
        stream_.swap(s.stream_);
    }

    /**
     * Append a element to the stream.
     */
    void put(const Data& d) {
        // The growth of the STL vector is implementation dependent, but it
        // usually grows exponentially as a nearly-optimal solution.
        stream_.push_back(d);
    }

    /**
     * Append a element to the stream.
     */
    void put(Data&& d) {
        stream_.push_back(std::forward<Data>(d));
    }

    /**
     * Sort the elements in the stream.
     */
    void sort() {
        std::sort(stream_.begin(), stream_.end());
    }

private:
    std::vector<Data> stream_;
};

#endif  // UTILS_STREAM_H_

