#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include "byte_buf.h"
#include "stream.h"
#include "log.h"
#include "threads.h"
#include "thread_pool.h"

void buffer_test();
void stream_test();
void logger_test();
void threads_test();

int main() {
    std::cout << std::endl;
    buffer_test();
    std::cout << std::endl;
    stream_test();
    std::cout << std::endl;
    logger_test();
    std::cout << std::endl;
    threads_test();
    std::cout << std::endl;

    return 0;
}

void buffer_test() {
    auto get_capacity = [](const ByteBuf* b) {
        uintptr_t p = reinterpret_cast<uintptr_t>(b);
        p += sizeof(Byte*) + sizeof(size_t);
        const size_t* pcap = reinterpret_cast<const size_t*>(p);
        return *pcap;
    };

    ByteBuf* buffer;

    /*******************************************/
    buffer = new ByteBuf();
    assert(buffer->data() == nullptr && buffer->size() == 0);
    delete buffer;

    const unsigned char msg[] = "a very very long test message";
    buffer = new ByteBuf(msg, 5);
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
    ByteBuf buffer2(std::move(*buffer));
    assert(buffer->data() == nullptr && buffer->size() == 0 && get_capacity(buffer) == 0);

    *buffer = std::move(buffer2);
    assert(cap == get_capacity(buffer) && sz == buffer->size()
            && std::equal(ctx, ctx + sz, buffer->data()));

    // use move assignment
    buffer2 = ByteBuf(msg, 2);
    buffer2 = std::move(*buffer);
    *buffer = std::move(buffer2);
    assert(cap == get_capacity(buffer) && sz == buffer->size()
            && std::equal(ctx, ctx + sz, buffer->data()));
    delete[] ctx;

    /*******************************************/
    // copy, should be prohibited
    //ByteBuf buffer3 = *buffer;

    /*******************************************/
    delete buffer;
    std::cout << "Simple test on class ByteBuf succeeds!" << std::endl;
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
        auto* pstrm = reinterpret_cast<std::vector<Data>*>(p);
        return pstrm->capacity();
    };

    stream_t* stream;

    /*******************************************/
    stream = new stream_t(0);
    assert(stream->data() == nullptr && stream->size() == 0);
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
    std::copy(stream->data(), stream->data() + sz, ctx);

    // use move constructor
    stream_t stream2(std::move(*stream));
    assert(stream->data() == nullptr && stream->size() == 0 && get_capacity(stream) == 0);

    *stream = std::move(stream2);
    assert(cap == get_capacity(stream) && sz == stream->size()
            && std::equal(ctx, ctx + sz, stream->data()));

    // use move assignment
    stream2 = stream_t();
    stream2 = std::move(*stream);
    *stream = std::move(stream2);
    assert(cap == get_capacity(stream) && sz == stream->size()
            && std::equal(ctx, ctx + sz, stream->data()));

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
            && std::equal(ctx, ctx + sz, stream4.data()));
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
    // iterator and const iterator
    for (auto& d : stream4) {
        d.a = d.a + 1;
        d.b = d.b + 1;
    }
    auto dd = Data{0, 0};
    for (const auto& d : stream4) {
        assert(dd < d);
        dd = d;
    }

    /*******************************************/
    delete stream;
    std::cout << "Simple test on class Stream succeeds!" << std::endl;
}

void logger_test() {
    std::cout << "Message: \"This is a test, 12345.\"" << std::endl;

    std::cout << "Basic info and warn:" << std::endl;
    info("This is a %s, %d.", "test", 12345);
    warn("This is a %s, %d.", "test", 12345);

    std::cout << "To file, with header [TEST]:" << std::endl;
    const char* fname = "/tmp/logger_test.txt";
    Logger l("[TEST]", fname);
    l.log_info("This is a %s, %d.", "test", 12345);
    l.log_warn("This is a %s, %d.", "test", 12345);
    std::cout << "File content:" << std::endl;
    std::ifstream ifh(fname);
    // http://stackoverflow.com/questions/675953/how-to-print-an-entire-istream-to-standard-out-and-string
    // use i/ostreambuf_iterator to retain whitespaces
    std::copy(std::istreambuf_iterator<char>(ifh), std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(std::cout));
    ifh.close();
    int status = remove(fname);
    if (status) panic("Clean up test file fails!");

    std::cout << "Simple test on logger succeeds!" << std::endl;
}

void threads_test() {
    std::vector<thread_t> threads;
    constexpr uint32_t cnt = 8;
    bar_t bar(cnt);

    std::cout << "Test thread barrier:" << std::endl;
    auto threadFunc = [&bar](uint32_t idx){
        for (uint32_t iter = 0; iter < 4; iter++) {
            info("%u: In iteration %u", idx, iter);
            bar.wait();
        }
    };
    for (uint32_t idx = 0; idx < cnt; idx++) {
        threads.emplace_back(threadFunc, idx);
    }
    for (uint32_t idx = 0; idx < cnt; idx++) {
        threads[idx].join();
    }

    std::cout << "Test thread pool:" << std::endl;
    ThreadPool pool(cnt);
    for (uint32_t idx = 0; idx < cnt; idx++) {
        pool.add_task(std::bind(threadFunc, idx));
    }
    pool.wait_all();

    std::cout << "Test thread barrier callback:" << std::endl;
    auto threadFuncCallback = [&bar](uint32_t idx){
        for (uint32_t iter = 0; iter < 4; iter++) {
            bar.wait([iter](){ info("Serial point %u", iter); });
        }
    };
    for (uint32_t idx = 0; idx < cnt; idx++) {
        pool.add_task(std::bind(threadFuncCallback, idx));
    }
    pool.wait_all();

    std::cout << "Simple test on threads succeeds!" << std::endl;
}

