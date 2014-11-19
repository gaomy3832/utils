#ifndef UTILS_STREAM_H
#define UTILS_STREAM_H

#include <algorithm>    // for std::sort
#include <cstring>      // for memcpy
#include <utility>      // for std::swap

template <typename Data>
class Stream
{
    public:
        Stream(size_t cp = 256);
        ~Stream() { delete[] stream; }

        Stream(const Stream<Data>& other);
        Stream(Stream<Data>&& other);
        Stream<Data>& operator=(Stream<Data> other);

        void reset(size_t cp = 256);
        void sort();

        size_t getSize() const { return size; }
        Data* getStream() const { return stream; }
        size_t getByteSize() const { return size * sizeof(Data); }

        Data& operator[](size_t idx) { return stream[idx]; }
        const Data& operator[](size_t idx) const { return stream[idx]; }

        void put(const Data& d);

        void swap(Stream<Data>& other);

    private:
        inline void extend(size_t factor = 2) {
            capacity *= factor;
            Data* newStream = new Data[capacity];
            memcpy(newStream, stream, this->getByteSize());
            delete[] stream;
            stream = newStream;
        }

    private:
        // Implemented in sequential storage
        size_t capacity;
        Data* stream;
        size_t size;
};

/*
 * Implementation
 */
template <typename Data>
Stream<Data>::Stream(size_t cp)
    : capacity(cp), stream(new Data[capacity]), size(0) {}

template <typename Data>
Stream<Data>::Stream(const Stream<Data>& other)
    : capacity(other.capacity), stream(new Data[capacity]), size(other.size)
{
    memcpy(stream, other.stream, size * sizeof(Data));
}

template <typename Data>
Stream<Data>::Stream(Stream<Data>&& other)
    : capacity(other.capacity), stream(other.stream), size(other.size)
{
    other.capacity = 256;
    other.stream = new Data[other.capacity];
    other.size = 0;
}

template <typename Data>
Stream<Data>& Stream<Data>::operator=(Stream<Data> other) {
    swap(other);
    return *this;
}

template <typename Data>
void Stream<Data>::reset(size_t cp) {
    delete[] stream;
    capacity = cp;
    stream = new Data[capacity];
    size = 0;
}

template <typename Data>
void Stream<Data>::sort() {
    std::sort(stream, stream + size);
}

template <typename Data>
void Stream<Data>::put(const Data& d) {
    if (size == capacity) {
        extend();
    }
    stream[size++] = d;
}

template <typename Data>
void Stream<Data>::swap(Stream<Data>& other) {
    std::swap(stream, other.stream);
    std::swap(size, other.size);
    std::swap(capacity, other.capacity);
}

#endif // UTILS_STREAM_H

