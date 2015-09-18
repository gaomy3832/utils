#include <cassert>
#include <iostream>
#include "buffer.h"
#include "stream.h"

void buffer_test();
void stream_test();

int main() {
    buffer_test();
    stream_test();

    return 0;
}

void buffer_test() {
    auto get_capacity = [](const Buffer* b) {
        uintptr_t p = reinterpret_cast<uintptr_t>(b);
        p += sizeof(Byte*) + sizeof(size_t);
        const size_t* pcap = reinterpret_cast<const size_t*>(p);
        return *pcap;
    };

    Buffer* buffer;

    /*******************************************/
    buffer = new Buffer();
    assert(buffer->data() == nullptr && buffer->size() == 0);
    delete buffer;

    const unsigned char msg[] = "a very very long test message";
    buffer = new Buffer(msg, 5);
    assert(buffer->size() == 5 && get_capacity(buffer) == 8);

    /*******************************************/
    // reserver
    buffer->reserve(6);
    assert(get_capacity(buffer) == 8);
    buffer->reserve(9);
    assert(get_capacity(buffer) == 16);

    /*******************************************/
    // append
    size_t cap = get_capacity(buffer);
    size_t sz = buffer->size();

    // append 0-length
    buffer->append(msg, 0);
    assert(sz == buffer->size());

    buffer->append(msg, cap - sz);
    // capacity does not change, and size become equal to capacity
    assert(cap == get_capacity(buffer));
    assert(cap == buffer->size());

    buffer->append(msg, 1);
    assert(cap * 2 == get_capacity(buffer));

    /*******************************************/
    // move
    cap = get_capacity(buffer);
    sz = buffer->size();
    Byte* ctx = new Byte[sz];
    std::copy(buffer->data(), buffer->data() + sz, ctx);

    // use move constructor
    Buffer buffer2(std::move(*buffer));
    assert(buffer->data() == nullptr && buffer->size() == 0 && get_capacity(buffer) == 0);

    *buffer = std::move(buffer2);
    assert(cap == get_capacity(buffer) && sz == buffer->size()
            && std::equal(ctx, ctx + sz, buffer->data()));

    // use move assignment
    buffer2 = Buffer(msg, 2);
    buffer2 = std::move(*buffer);
    *buffer = std::move(buffer2);
    assert(cap == get_capacity(buffer) && sz == buffer->size()
            && std::equal(ctx, ctx + sz, buffer->data()));
    delete[] ctx;

    /*******************************************/
    // copy, should be prohibited
    //Buffer buffer3 = *buffer;

    /*******************************************/
    delete buffer;
    std::cout << "Simple test on class Buffer succeeds!" << std::endl;
}

void stream_test() {
    struct Data {
        uint64_t a;
        uint8_t b;

        bool operator==(const Data& other) const {
            return a == other.a && b == other.b;
        }
        bool operator<(const Data& other) const {
            return a < other.a || (a == other.a && b < other.b);
        }
    };
    typedef Stream<Data> stream_t;

    auto get_capacity = [](const stream_t* s) {
        uintptr_t p = reinterpret_cast<uintptr_t>(s);
        p += sizeof(Data*) + sizeof(size_t);
        const size_t* pcap = reinterpret_cast<const size_t*>(p);
        return *pcap;
    };

    stream_t* stream;

    /*******************************************/
    stream = new stream_t(0);
    assert(stream->stream() == nullptr && stream->size() == 0);
    delete stream;

    stream = new stream_t(1024);
    assert(stream->size() == 0 && get_capacity(stream) == 1024);

    /*******************************************/
    // reset
    stream->reset(256);
    assert(get_capacity(stream) == 256);

    /*******************************************/
    // put
    size_t cap = get_capacity(stream);
    size_t sz = stream->size();

    for (size_t i = 0; i < cap - sz; i++) {
        stream->put({i, static_cast<uint8_t>(i % 128)});
    }
    // capacity does not change, and size become equal to capacity
    assert(cap == get_capacity(stream));
    assert(cap == stream->size());
    // byte size
    assert(stream->byte_size() == sizeof(Data) * stream->size());

    stream->put({1024, 130});
    assert(cap * 2 == get_capacity(stream));

    /*******************************************/
    // move
    cap = get_capacity(stream);
    sz = stream->size();
    Data* ctx = new Data[sz];
    std::copy(stream->stream(), stream->stream() + sz, ctx);

    // use move constructor
    stream_t stream2(std::move(*stream));
    assert(stream->stream() == nullptr && stream->size() == 0 && get_capacity(stream) == 0);

    *stream = std::move(stream2);
    assert(cap == get_capacity(stream) && sz == stream->size()
            && std::equal(ctx, ctx + sz, stream->stream()));

    // use move assignment
    stream2 = stream_t();
    stream2 = std::move(*stream);
    *stream = std::move(stream2);
    assert(cap == get_capacity(stream) && sz == stream->size()
            && std::equal(ctx, ctx + sz, stream->stream()));

    /*******************************************/
    // copy, should be prohibited
    //stream_t stream3 = *stream;

    /*******************************************/
    // swap
    cap = get_capacity(stream);
    sz = stream->size();

    stream_t stream4;
    stream->swap(stream4);
    assert(stream4.size() == sz && get_capacity(&stream4) == cap
            && std::equal(ctx, ctx + sz, stream4.stream()));
    delete[] ctx;

    /*******************************************/
    // sort, and []
    stream4.sort();
    Data d = stream4[0];
    for (size_t i = 1; i < stream4.size(); i++) {
        assert(d < stream4[i]);
        d = stream4[i];
    }

    /*******************************************/
    delete stream;
    std::cout << "Simple test on class Stream succeeds!" << std::endl;
}



