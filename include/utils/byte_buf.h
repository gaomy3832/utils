/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_BYTE_BUF_H_
#define UTILS_BYTE_BUF_H_

#include <algorithm>    // for std::copy, std::fill

/**
 * @brief
 * Generic buffer to store raw bytes.
 */
class ByteBuf {
public:
    /**
     * Byte type.
     */
    typedef uint8_t Byte;

public:
    /**
     * @brief
     * Initialize empty buffer.
     */
    ByteBuf()
            : buffer_(nullptr), size_(0), capacity_(0) {
        // Nothing else to do.
    }

    /**
     * @brief
     * Initialize buffer with data.
     *
     * @param data  pointer to the initial data in the buffer.
     * @param sz    byte size of the initial data.
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

    /**
     * @name
     * Copy and move.
     */
    /**@{*/

    ByteBuf(const ByteBuf&) = delete;

    ByteBuf& operator=(const ByteBuf&) = delete;

    ByteBuf(ByteBuf&& b)
            : buffer_(b.buffer_), size_(b.size_), capacity_(b.capacity_) {
        b.buffer_ = nullptr;
        b.size_ = 0;
        b.capacity_ = 0;
    }

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

    /**@}*/

    /**
     * @name
     * Member access.
     */
    /**@{*/

    /**
     * @return  raw pointer to the data.
     */
    Byte* data() { return buffer_; }

    /**
     * @return  const raw pointer to the data.
     */
    const Byte* data() const { return buffer_; }

    /**
     * @return  buffer size in bytes.
     */
    size_t size() const { return size_; }

    /**
     * @return  buffer capacity in bytes.
     */
    size_t capacity() const { return capacity_; }

    /**@}*/

    /**
     * @name
     * Modifiers.
     */
    /**@{*/

    /**
     * @brief
     * Reserve at least \c cap bytes for the buffer.
     *
     * Keep original data. If requested \c cap is not larger than the current
     * capacity, no changes will happen.
     *
     * @param cap  requested capacity of the buffer.
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
     * @brief
     * Resize buffer to \c sz bytes.
     *
     * If size becomes larger, new data are all initialized to zero.
     *
     * @param sz  requested size of the buffer in bytes.
     */
    void resize(size_t sz) {
        reserve(sz);
        if (sz > size_) {
            std::fill(buffer_ + size_, buffer_ + sz, 0);
        }
        size_ = sz;
    }

    /**
     * @brief
     * Append more data to the buffer.
     *
     * @param data  point to the data to be appended.
     * @param sz    byte size of the data to be appended.
     */
    template<typename T>
    void append(const T* data, size_t sz) {
        reserve(size_ + sz);
        const Byte* d = reinterpret_cast<const Byte*>(data);
        std::copy(d, d + sz, buffer_ + size_);
        size_ += sz;
    }

    /**@}*/

private:
    // Buffer space.
    Byte* buffer_;

    // Used buffer size in bytes.
    size_t size_;

    // Allocated buffer capacity in bytes.
    size_t capacity_;
};

#endif  // UTILS_BYTE_BUF_H_

