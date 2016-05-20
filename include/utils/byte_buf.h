#ifndef UTILS_BYTE_BUF_H_
#define UTILS_BYTE_BUF_H_
/**
 * Generic buffer to store raw bytes.
 */
#include <algorithm>    // for std::copy

typedef uint8_t Byte;

class ByteBuf {
public:
    /**
     * Initialize empty buffer.
     */
    ByteBuf()
            : buffer(nullptr), size(0), capacity(0) {
        // Nothing else to do.
    }

    /**
     * Initialize buffer with data.
     */
    ByteBuf(const Byte* data, size_t sz)
            : ByteBuf() {
        reserve(sz);
        std::copy(data, data + sz, buffer);
        size = sz;
    }

    ~ByteBuf() {
        delete[] buffer;  // nullptr is safe.
    }

    /* Copy and move. */

    ByteBuf(const ByteBuf&) = delete;

    ByteBuf& operator=(const ByteBuf&) = delete;

    /**
     * Move constructor.
     */
    ByteBuf(ByteBuf&& b)
            : buffer(b.buffer), size(b.size), capacity(b.capacity) {
        b.buffer = nullptr;
        b.size = 0;
        b.capacity = 0;
    }

    /**
     * Move assignment.
     */
    ByteBuf& operator=(ByteBuf&& b) {
        // Avoid self assign.
        if (this == &b) return *this;
        // Free current buffer.
        delete[] buffer;
        // Assign.
        buffer = b.buffer;
        size = b.size;
        capacity = b.capacity;
        // Release the source.
        b.buffer = nullptr;
        b.size = 0;
        b.capacity = 0;
        return *this;
    }

    /* Member access */

    /**
     * Get raw pointer to the data.
     */
    Byte* data() { return buffer; }

    /**
     * Get const raw pointer to the data.
     */
    const Byte* data() const { return buffer; }

    /**
     * Get buffer size in bytes.
     */
    size_t size() const { return size; }

    /* Modifiers */

    /**
     * Reserve at least \c cap bytes for the buffer. Keep original data.
     */
    void reserve(size_t cap) {
        // Only grow buffer.
        if (cap <= capacity) return;
        // Capacity is always power of 2.
        capacity = 1;
        while (capacity < cap) capacity <<= 1;
        // Store old buffer.
        auto* obuf = buffer;
        // Allocate buffer.
        buffer = new Byte[capacity];
        // Copy buffer.
        std::copy(obuf, obuf + size, buffer);
        // Delete old buffer, nullptr is safe.
        delete[] obuf;
    }

    /**
     * Resize buffer.
     */
    void resize(size_t sz) {
        reserve(sz);
        size = sz;
    }

    /**
     * Append more data to the buffer.
     *
     * @param data  data pointer to be appended.
     * @param sz    size of data to be appended.
     */
    void append(const Byte* data, size_t sz) {
        reserve(size + sz);
        std::copy(data, data + sz, buffer + size);
        size += sz;
    }

private:
    /**
     * Buffer space.
     */
    Byte* buffer;

    /**
     * Used size of buffer.
     */
    size_t size;

    /**
     * Allocated capacity of buffer.
     */
    size_t capacity;
};

#endif  // UTILS_BYTE_BUF_H_

