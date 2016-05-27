/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_BYTE_BUF_H_
#define UTILS_BYTE_BUF_H_
/**
 * Generic buffer to store raw bytes.
 */
#include <algorithm>    // for std::copy, std::fill

class ByteBuf {
public:
    /**
     * Byte type.
     */
    typedef uint8_t Byte;

public:
    /**
     * Initialize empty buffer.
     */
    ByteBuf()
            : buffer_(nullptr), size_(0), capacity_(0) {
        // Nothing else to do.
    }

    /**
     * Initialize buffer with data.
     */
    template<typename T>
    ByteBuf(const T* data, size_t sz)
            : ByteBuf() {
        reserve(sz);
        const Byte* d = reinterpret_cast<const Byte*>(data);
        std::copy(d, d + sz, buffer_);
        size_ = sz;
    }

    ~ByteBuf() {
        delete[] buffer_;  // nullptr is safe.
    }

    /* Copy and move. */

    ByteBuf(const ByteBuf&) = delete;

    ByteBuf& operator=(const ByteBuf&) = delete;

    /**
     * Move constructor.
     */
    ByteBuf(ByteBuf&& b)
            : buffer_(b.buffer_), size_(b.size_), capacity_(b.capacity_) {
        b.buffer_ = nullptr;
        b.size_ = 0;
        b.capacity_ = 0;
    }

    /**
     * Move assignment.
     */
    ByteBuf& operator=(ByteBuf&& b) {
        // Avoid self assign.
        if (this == &b) return *this;
        // Free current buffer.
        delete[] buffer_;
        // Assign.
        buffer_ = b.buffer_;
        size_ = b.size_;
        capacity_ = b.capacity_;
        // Release the source.
        b.buffer_ = nullptr;
        b.size_ = 0;
        b.capacity_ = 0;
        return *this;
    }

    /* Member access */

    /**
     * Get raw pointer to the data.
     */
    Byte* data() { return buffer_; }

    /**
     * Get const raw pointer to the data.
     */
    const Byte* data() const { return buffer_; }

    /**
     * Get buffer size in bytes.
     */
    size_t size() const { return size_; }

    /**
     * Get buffer capacity in bytes.
     */
    size_t capacity() const { return capacity_; }

    /* Modifiers */

    /**
     * Reserve at least \c cap bytes for the buffer. Keep original data.
     */
    void reserve(size_t cap) {
        // Only grow buffer.
        if (cap <= capacity_) return;
        // Capacity is always power of 2.
        capacity_ = 1;
        while (capacity_ < cap) capacity_ <<= 1;
        // Store old buffer.
        auto* obuf = buffer_;
        // Allocate buffer.
        buffer_ = new Byte[capacity_];
        // Copy buffer.
        std::copy(obuf, obuf + size_, buffer_);
        // Delete old buffer, nullptr is safe.
        delete[] obuf;
    }

    /**
     * Resize buffer. If size becomes larger, new space is all zero.
     */
    void resize(size_t sz) {
        reserve(sz);
        if (sz > size_) {
            std::fill(buffer_ + size_, buffer_ + sz, 0);
        }
        size_ = sz;
    }

    /**
     * Append more data to the buffer.
     *
     * @param data  data pointer to be appended.
     * @param sz    size of data to be appended.
     */
    template<typename T>
    void append(const T* data, size_t sz) {
        reserve(size_ + sz);
        const Byte* d = reinterpret_cast<const Byte*>(data);
        std::copy(d, d + sz, buffer_ + size_);
        size_ += sz;
    }

private:
    /**
     * Buffer space.
     */
    Byte* buffer_;

    /**
     * Used size of buffer.
     */
    size_t size_;

    /**
     * Allocated capacity of buffer.
     */
    size_t capacity_;
};

#endif  // UTILS_BYTE_BUF_H_

