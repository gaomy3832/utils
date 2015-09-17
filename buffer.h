#ifndef UTILS_BUFFER_H_
#define UTILS_BUFFER_H_
/**
 * Generic buffer to store raw bytes.
 */
#include <algorithm>    // for std::copy

typedef uint8_t Byte;

class Buffer {
    public:
        Buffer() : buf(nullptr), sz(0), cap(0) {}

        Buffer(const Byte* data, size_t size) : Buffer() {
            reserve(size);
            std::copy(data, data + size, buf);
            sz = size;
        }

        ~Buffer() { delete[] buf; } // \c nullptr is safe

        /* Copy and move */

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& b) : buf(b.buf), sz(b.sz), cap(b.cap) {
            b.buf = nullptr;
            b.sz = 0;
            b.cap = 0;
        }

        Buffer& operator=(Buffer&& b) {
            // avoid self assign
            if (this == &b) return *this;
            // free current buf
            delete[] buf;
            // assign
            buf = b.buf;
            sz = b.sz;
            cap = b.cap;
            // release the source
            b.buf = nullptr;
            b.sz = 0;
            b.cap = 0;
            return *this;
        }

        /* Member access */

        Byte* data() const { return buf; }
        size_t size() const { return sz; }

        /* Modifiers */

        void reserve(size_t capacity) {
            // only grow buf
            if (capacity <= cap) return;
            // \c cap is always power of 2
            cap = 1;
            while (cap < capacity) cap <<= 1;
            // store old buf
            auto* old_buf = buf;
            // allocate buf
            buf = new Byte[cap];
            // copy buf
            std::copy(old_buf, old_buf + sz, buf);
            // delete old buf, \c nullptr is safe
            delete[] old_buf;
        }

        void resize(size_t size) {
            reserve(size);
            sz = size;
        }

        void append(const Byte* data, size_t size) {
            reserve(sz + size);
            std::copy(data, data + size, buf + sz);
            sz += size;
        }

    private:
        // data space
        Byte* buf;
        // used size
        size_t sz;
        // allocated capacity
        size_t cap;
};

#endif // UTILS_BUFFER_H_

