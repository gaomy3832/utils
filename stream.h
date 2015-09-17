#ifndef UTILS_STREAM_H_
#define UTILS_STREAM_H_
/**
 * Generic FIFO stream.
 */
#include <algorithm>    // for std::copy, std::sort
#include <utility>      // for std::swap

template <typename Data>
class Stream {
    public:
        Stream(size_t cp = 256);
        ~Stream();

        // Move
        Stream(Stream<Data>&& s);
        Stream<Data>& operator=(Stream<Data>&& s);

        // No copy
        Stream(const Stream<Data>&) = delete;
        Stream<Data>& operator=(const Stream<Data>&) = delete;

        // Member access
        Data* stream() const { return stream_; }
        size_t size() const { return size_; }
        size_t byte_size() const { return size_ * sizeof(Data); }

        Data& operator[](size_t idx) { return stream_[idx]; }
        const Data& operator[](size_t idx) const { return stream_[idx]; }

        // Modifiers
        void reset(size_t cp = 256);
        void swap(Stream<Data>& s);
        void put(const Data& d);
        void sort();

    private:
        inline void extend(size_t factor = 2) {
            // \c capacity_ starts from 4
            if (capacity_ < 4) capacity_ = 4;
            // extend \c capacity_ by \c factor
            capacity_ *= factor;
            // allocate space
            Data* new_stream = new Data[capacity_];
            // copy stream
            if (stream_ != nullptr) {
                std::copy(stream_, stream_ + size_, new_stream);
                delete[] stream_;
            }
            stream_ = new_stream;
        }

    private:
        // Implemented in sequential storage
        Data* stream_;
        size_t size_;
        size_t capacity_;
};

/*
 * Implementation
 */
template <typename Data>
Stream<Data>::Stream(size_t cp) {
    if (cp == 0) {
        stream_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    } else {
        capacity_ = cp;
        stream_ = new Data[capacity_];
        size_ = 0;
    }
}

template <typename Data>
Stream<Data>::~Stream() {
    // delete nullptr is safe (3.7.4.2/3)
    delete[] stream_;
}

template <typename Data>
Stream<Data>::Stream(Stream<Data>&& s)
    : stream_{s.stream_}, size_{s.size_}, capacity_{s.capacity_}
{
    s.stream_ = nullptr;
    s.size_ = 0;
    s.capacity_ = 0;
}

template <typename Data>
Stream<Data>& Stream<Data>::operator=(Stream<Data>&& s) {
    // avoid self assign
    if (this != &s) {
        // free current stream
        delete[] stream_;
        // assign
        stream_ = s.stream_;
        size_ = s.size_;
        capacity_ = s.capacity_;
        // release the source
        s.stream_ = nullptr;
        s.size_ = 0;
        s.capacity_ = 0;
    }
    return *this;
}

template <typename Data>
void Stream<Data>::reset(size_t cp) {
    delete[] stream_;
    capacity_ = cp;
    stream_ = new Data[capacity_];
    size_ = 0;
}

template <typename Data>
void Stream<Data>::swap(Stream<Data>& s) {
    std::swap(stream_, s.stream_);
    std::swap(size_, s.size_);
    std::swap(capacity_, s.capacity_);
}

template <typename Data>
void Stream<Data>::put(const Data& d) {
    if (size_ == capacity_) {
        extend();
    }
    stream_[size_++] = d;
}

template <typename Data>
void Stream<Data>::sort() {
    std::sort(stream_, stream_ + size_);
}

#endif // UTILS_STREAM_H_

