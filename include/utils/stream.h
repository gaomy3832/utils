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
    typedef typename std::vector<Data>::iterator StreamIter;
    typedef typename std::vector<Data>::const_iterator StreamConstIter;

public:
    /**
     * Initialize stream.
     *
     * @param num    initial number of elements.
     */
    explicit Stream(size_t num = 16) {
        stream.reserve(num);
    }

    ~Stream() {}

    /* Copy and move */

    Stream(const Stream<Data>&) = delete;

    Stream<Data>& operator=(const Stream<Data>&) = delete;

    /**
     * Move constructor.
     */
    Stream(Stream<Data>&& s) {
        stream.swap(s.stream);
    }

    /**
     * Move assignment.
     */
    Stream<Data>& operator=(Stream<Data>&& s) {
        // Avoid self assign.
        if (this == &s) return *this;
        stream.swap(s.stream);
        return *this;
    }

    /* Member access */

    /**
     * Get raw pointer to the data.
     */
    Data* data() { return stream.data(); }

    /**
     * Get const raw pointer to the data.
     */
    const Data* data() const { return stream.data(); }

    /**
     * Get stream size in number of elements.
     */
    size_t size() const { return stream.size(); }

    /**
     * Get stream size in bytes.
     */
    size_t byte_size() const { return size() * sizeof(Data); }

    /**
     * Get element with index \c idx.
     */
    Data& operator[](size_t idx) { return stream[idx]; }

    /**
     * Get const element with index \c idx.
     */
    const Data& operator[](size_t idx) const { return stream[idx]; }

    /**
     * Get iterator to the elements in the stream.
     */
    StreamIter begin() { return stream.begin(); }

    /**
     * Get end of the iterator to the elements in the stream.
     */
    StreamIter end() { return stream.end(); }

    /**
     * Get const iterator to the elements in the stream.
     */
    StreamConstIter begin() const { return stream.begin(); }

    /**
     * Get end of the const iterator to the elements in the stream.
     */
    StreamConstIter end() const { return stream.end(); }

    /**
     * Get const iterator to the elements in the stream.
     */
    StreamConstIter cbegin() const { return stream.cbegin(); }

    /**
     * Get end of the const iterator to the elements in the stream.
     */
    StreamConstIter cend() const { return stream.cend(); }

    /* Modifiers */

    /**
     * Reset stream. Clear all elements and initialize to \c num elements.
     * Warning: non-binding.
     *
     * @param num    reserved number of elements.
     */
    void reset(size_t num = 16) {
        if (num != stream.capacity()) {
            // Non-binding request, as shrink_to_fit() is non-binding.
            stream.resize(num);
            stream.shrink_to_fit();
        }
        stream.clear();
    }

    /**
     * Swap this stream with another stream \c s.
     */
    void swap(Stream<Data>& s) {
        stream.swap(s.stream);
    }

    /**
     * Append a element to the stream.
     */
    void put(const Data& d) {
        // The growth of the STL vector is implementation dependent, but it
        // usually grows exponentially as a nearly-optimal solution.
        stream.push_back(d);
    }

    /**
     * Append a element to the stream.
     */
    void put(Data&& d) {
        stream.push_back(std::forward<Data>(d));
    }

    /**
     * Sort the elements in the stream.
     */
    void sort() {
        std::sort(stream.begin(), stream.end());
    }

private:
    std::vector<Data> stream;
};

#endif  // UTILS_STREAM_H_

