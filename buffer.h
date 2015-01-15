#ifndef UTILS_BUFFER_H
#define UTILS_BUFFER_H

#include <algorithm>    // for std::copy

typedef uint8_t Byte;

class Buffer {
    public:
        Buffer();
        Buffer(const Byte* data, size_t size);
        ~Buffer();

        // Move
        Buffer(Buffer&& b);
        Buffer& operator=(Buffer&& b);

        // No copy
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        // Member access
        Byte* data() const { return data_; }
        size_t size() const { return size_; }

        // Modifiers
        inline void reserve(size_t capacity);
        void append(const Byte* data, size_t size);

    private:
        Byte* data_;
        size_t size_;
        size_t capacity_;
};

/*
 * Implementation
 */
Buffer::Buffer() : data_{nullptr}, size_{0}, capacity_{0} {}

Buffer::Buffer(const Byte* data, size_t size) : Buffer() {
    reserve(size);
    std::copy(data, data + size, data_);
    size_ = size;
}

Buffer::~Buffer() {
    // delete nullptr is safe (3.7.4.2/3)
    delete[] data_;
}

Buffer::Buffer(Buffer&& b) : data_{b.data_}, size_{b.size_}, capacity_{b.capacity_} {
    b.data_ = nullptr;
    b.size_ = 0;
    b.capacity_ = 0;
}

Buffer& Buffer::operator=(Buffer&& b) {
    // avoid self assign
    if (this != &b) {
        // free current data
        delete[] data_;
        // assign
        data_ = b.data_;
        size_ = b.size_;
        capacity_ = b.capacity_;
        // release the source
        b.data_ = nullptr;
        b.size_ = 0;
        b.capacity_ = 0;
    }
    return *this;
}

void Buffer::reserve(size_t capacity) {
    if (capacity <= capacity_) return;
    // \c capacity_ is always power of 2
    capacity_ = 1;
    while (capacity_ < capacity) capacity_ <<= 1;
    // allocate space
    Byte* new_data = new Byte[capacity_];
    // copy data
    if (data_ != nullptr) {
        std::copy(data_, data_ + size_, new_data);
        delete[] data_;
    }
    data_ = new_data;
}

void Buffer::append(const Byte* data, size_t size) {
    reserve(size_ + size);
    std::copy(data, data + size, data_ + size_);
    size_ += size;
}

#endif // UTILS_BUFFER_H

