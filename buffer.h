#ifndef UTILS_BUFFER_H
#define UTILS_BUFFER_H

#include <cstdint>
#include <cstring>
typedef uint8_t Byte;

class Buffer
{
    public:
        Buffer();
        Buffer(const Byte* data, size_t size);
        ~Buffer();

        Byte* data() const { return m_data; }
        size_t size() const { return m_size; }

        inline void reserve(size_t capacity);
        void append(const Byte* data, size_t size);

    private:
        Byte* m_data;
        size_t m_size;
        size_t m_capacity;
};

Buffer::Buffer() {
    m_size = 0;
    m_capacity = 0;
    m_data = NULL;
}

Buffer::Buffer(const Byte* data, size_t size) {
    reserve(size);
    memcpy((void*)m_data, (void*)data, size);
    m_size = size;
}

Buffer::~Buffer() {
    if (m_data != NULL) {
        delete[] m_data;
    }
}

void Buffer::reserve(size_t capacity) {
    if (capacity <= m_capacity) return;
    m_capacity = 1;
    while (m_capacity < capacity) m_capacity <<= 1;
    Byte* new_data = new Byte[m_capacity];
    if (m_data != NULL) {
        memcpy((void*)new_data, (void*)m_data, m_size);
        delete[] m_data;
    }
    m_data = new_data;
}

void Buffer::append(const Byte* data, size_t size) {
    reserve(m_size + size);
    memcpy((void*)(m_data + m_size), (void*)data, size);
    m_size += size;
}

#endif // UTILS_BUFFER_H

