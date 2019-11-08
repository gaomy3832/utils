/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_CHUNK_LIST_H_
#define UTILS_CHUNK_LIST_H_

#include <array>
#include <list>
#include <stdexcept>
#include <type_traits>  // for std::is_standard_layout
#include <utility>  // for std::swap, std::pair
#include "utils/nested_iterator.h"

/**
 * @addtogroup containers
 *
 * @{
 */

/**
 * @brief
 * Chunk list, as linked list of chunks, each of which contiguously stores
 * multiple elements.
 */
template<typename T, size_t C = 65536, class Allocator = std::allocator<T>>
class ChunkList {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

public:
    static constexpr size_t CHUNK_CAPACITY = C / sizeof(T);

    /**
     * @brief Chunk with a fixed reserved space.
     */
    class Chunk {
    public:
        using value_type = ChunkList::value_type;
        using size_type = ChunkList::size_type;
        using difference_type = ChunkList::value_type;
        using pointer = ChunkList::pointer;
        using const_pointer = ChunkList::const_pointer;
        using reference = ChunkList::reference;
        using const_reference = ChunkList::const_reference;

    private:
        using ChunkStorageType = std::array<value_type, CHUNK_CAPACITY>;

    public:
        using iterator = typename ChunkStorageType::iterator;
        using const_iterator = typename ChunkStorageType::const_iterator;

    public:
        /**
         * @name
         * Construction.
         */
        /**@{*/

        Chunk()
            : size_(0)
        {
            // Nothing else to do.
        }

        ~Chunk() = default;
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;
        Chunk(Chunk&&) = default;
        Chunk& operator=(Chunk&&) = default;

        /**@}*/

        /**
         * @name
         * Size access.
         */
        /**@{*/

        inline size_type size() const { return size_; }

        inline bool empty() const { return size_ == 0; }

        inline size_type max_size() const { return storage_.size(); }

        /**@}*/

        /**
         * @name
         * Iterators.
         */
        /**@{*/

        inline iterator begin() { return storage_.begin(); }

        inline iterator end() { return storage_.begin() + size_; }

        inline const_iterator begin() const { return storage_.begin(); }

        inline const_iterator end() const { return storage_.begin() + size_; }

        inline const_iterator cbegin() const { return storage_.cbegin(); }

        inline const_iterator cend() const { return storage_.cbegin() + size_; }

        /**@}*/

        /**
         * @name
         * Member access.
         */
        /**@{*/

        inline reference operator[](size_type pos) { return storage_[pos]; }

        inline const_reference operator[](size_type pos) const { return storage_[pos]; }

        reference at(size_type pos) {
            if (pos >= size_)
                throw std::out_of_range("ChunkList::Chunk: out-of-range access.");
            return this->operator[](pos);
        }

        const_reference at(size_type pos) const {
            if (pos >= size_)
                throw std::out_of_range("ChunkList::Chunk: out-of-range access.");
            return this->operator[](pos);
        }

        reference front() { return storage_[0]; }

        const_reference front() const { return storage_[0]; }

        reference back() { return storage_[size_ ? size_ - 1 : 0]; }

        const_reference back() const { return storage_[size_ ? size_ - 1 : 0]; }

        pointer data() { return storage_.data(); }

        const_pointer data() const { return storage_.data(); }

        /**@}*/

        /**
         * @name
         * Modifiers.
         */
        /**@{*/

        void push_back(const value_type& value) {
            if (size_ < storage_.size())
                storage_[size_++] = value;
            else
                throw std::length_error("ChunkList::Chunk: chunk overflows.");
        }

        void push_back(value_type&& value) {
            if (size_ < storage_.size())
                std::swap(storage_[size_++], value);
            else
                throw std::length_error("ChunkList::Chunk: chunk overflows.");
        }

        void pop_back() {
            if (size_ == 0)
                throw std::length_error("ChunkList::Chunk: chunk underflows.");
            size_--;
        }

        void clear() {
            size_ = 0;
        }

        void swap(Chunk& other) {
            storage_.swap(other.storage_);
            std::swap(size_, other.size_);
        }

        void resize(size_type size, const value_type& value) {
            if (size > storage_.size())
                throw std::length_error("ChunkList::Chunk: chunk overflows.");
            for (size_type i = size_; i < size; i++) {
                storage_[i] = value;
            }
            size_ = size;
        }

        void resize(size_type size) {
            if (size > storage_.size())
                throw std::length_error("ChunkList::Chunk: chunk overflows.");
            size_ = size;
        }

        /**@}*/

    private:
        ChunkStorageType storage_;
        size_type size_;
    };

    static_assert(CHUNK_CAPACITY > 0, "Chunk list needs larger chunks to store element.");

    static_assert(std::is_standard_layout<Chunk>::value, "Chunk list must have standard-layout chunk.");

public:
    /**
     * @brief
     * Initialize an empty chunk list.
     */
    ChunkList()
        : list_(), size_(0)
    {
        // Nothing else to do.
    }

    ~ChunkList() = default;
    ChunkList(const ChunkList&) = default;
    ChunkList& operator=(const ChunkList&) = default;
    ChunkList(ChunkList&&) = default;
    ChunkList& operator=(ChunkList&&) = default;

    /**
     * @name
     * Size access.
     */
    /**@{*/

    /**
     * @brief Return the number of elements.
     */
    inline size_type size() const { return size_; }

    /**
     * @brief Check whether the container is empty.
     */
    inline bool empty() const { return size_ == 0; }

    /**
     * @brief Return the number of chunks.
     */
    inline size_type chunk_count() const { return list_.size(); }

    /**@}*/

    /**
     * @name
     * Member access.
     */
    /**@{*/

    /**
     * @brief Access specific element with bounds checking.
     */
    reference at(size_type pos) {
        if (pos >= size_)
            throw std::out_of_range("ChunkList: out-of-range access.");

        // Chunk index and offset.
        size_type chunkIndex = pos / CHUNK_CAPACITY;
        size_type chunkOffset = pos % CHUNK_CAPACITY;

        // Get the chunk in the list.
        size_type i = 0;
        auto listIter = list_.begin();
        while (i < chunkIndex) {
            i++;
            listIter++;
            assert(listIter != list_.end());
        }
        auto& chunk = *listIter;

        // Get the element in the chunk.
        return chunk[chunkOffset];
    }

    /**
     * @brief Access specific element with bounds checking.
     */
    const_reference at(size_type pos) const {
        if (pos >= size_)
            throw std::out_of_range("ChunkList: out-of-range access.");

        // Chunk index and offset.
        size_type chunkIndex = pos / CHUNK_CAPACITY;
        size_type chunkOffset = pos % CHUNK_CAPACITY;

        // Get the chunk in the list.
        size_type i = 0;
        auto listIter = list_.cbegin();
        while (i < chunkIndex) {
            i++;
            listIter++;
            assert(listIter != list_.cend());
        }
        auto& chunk = *listIter;

        // Get the element in the chunk.
        return chunk[chunkOffset];
    }

    /**
     * @brief Access the first element. Undefined behavior if empty.
     */
    reference front() { return list_.front().front(); }

    /**
     * @brief Access the first element. Undefined behavior if empty.
     */
    const_reference front() const { return list_.front().front(); }

    /**
     * @brief Access the last element. Undefined behavior if empty.
     */
    reference back() { return list_.back().back(); }

    /**
     * @brief Access the last element. Undefined behavior if empty.
     */
    const_reference back() const { return list_.back().back(); }

    /**@}*/

    /**
     * @name
     * Modifiers.
     */
    /**@{*/

    /**
     * @brief And an element to the end.
     */
    void push_back(const value_type& value) {
        if (list_.empty()) list_.emplace_back();
        try {
            list_.back().push_back(value);
        } catch (std::length_error) {
            assert(size_ % CHUNK_CAPACITY == 0);
            list_.emplace_back();
            list_.back().push_back(value);
        }
        size_++;
        assert(list_.empty() || !list_.back().empty());
    }

    /**
     * @brief And an element to the end.
     */
    void push_back(value_type&& value) {
        if (list_.empty()) list_.emplace_back();
        try {
            list_.back().push_back(std::move(value));
        } catch (std::length_error) {
            assert(size_ % CHUNK_CAPACITY == 0);
            list_.emplace_back();
            list_.back().push_back(std::move(value));
        }
        size_++;
        assert(list_.empty() || !list_.back().empty());
    }

    /**
     * @brief Remove the last element.
     */
    void pop_back() {
        if (size_ == 0)
            throw std::length_error("ChunkList: chunk underflows.");
        list_.back().pop_back();
        size_--;
        if (list_.back().empty()) {
            assert(size_ % CHUNK_CAPACITY == 0);
            list_.pop_back();
        }
        assert(list_.empty() || !list_.back().empty());
    }

    /**
     * @brief Clear the contents.
     */
    void clear() {
        size_ = 0;
        list_.clear();
    }

    /**
     * @brief Swap the contents.
     */
    void swap(ChunkList& other) {
        list_.swap(other.list_);
        std::swap(size_, other.size_);
    }

    /**
     * @brief Change the number of elements stored.
     */
    template<typename... Args>
    void resize(size_type size, Args&&... args) {
        if (size > size_) {
            // Grow.
            auto diff = size - size_;
            while (diff) {
                if (list_.empty() || list_.back().size() == CHUNK_CAPACITY) list_.emplace_back();
                auto occup = list_.back().size();  // occupied size in the last chunk.
                auto incr = std::min(diff, CHUNK_CAPACITY - occup);
                list_.back().resize(occup + incr, std::forward<Args>(args)...);
                diff -= incr;
            }
        } else {
            // Shrink.
            auto diff = size_ - size;
            while (diff) {
                assert(!list_.empty());
                auto occup = list_.back().size();  // occupied size in the last chunk.
                if (diff >= occup) {
                    list_.pop_back();
                    diff -= occup;
                } else {
                    list_.back().resize(occup - diff);
                    diff = 0;
                }
            }
        }
        size_ = size;
        assert(list_.empty() || !list_.back().empty());
    }

    /**@}*/

private:
    using ChunkListStorageType = std::list<Chunk, typename Allocator::template rebind<Chunk>::other>;

private:
    ChunkListStorageType list_;
    size_type size_;

public:
    using iterator = NestedIterator<ChunkListStorageType, Chunk, IterTagType>;
    using const_iterator = NestedIterator<ChunkListStorageType, Chunk, ConstIterTagType>;

    /**
     * @name
     * Iterators.
     */
    /**@{*/

    /**
     * @brief Return an iterator to the beginning.
     */
    inline iterator begin() { return iterator(&list_, list_.begin(), list_.begin()->begin()); }

    /**
     * @brief Return an iterator to the end.
     */
    inline iterator end() { return iterator(&list_); }

    /**
     * @brief Return a constant iterator to the beginning.
     */
    inline const_iterator begin() const { return const_iterator(&list_, list_.cbegin(), list_.cbegin()->cbegin()); }

    /**
     * @brief Return a constant iterator to the end.
     */
    inline const_iterator end() const { return const_iterator(&list_); }

    /**
     * @brief Return a constant iterator to the beginning.
     */
    inline const_iterator cbegin() const { return const_iterator(&list_, list_.cbegin(), list_.cbegin()->cbegin()); }

    /**
     * @brief Return a constant iterator to the end.
     */
    inline const_iterator cend() const { return const_iterator(&list_); }

    /**
     * @brief Return range of a chunk as a pair of iterators.
     */
    inline std::pair<iterator, iterator> chunk_range(size_t idx) {
        auto lastChunkIdx = chunk_count() - 1;
        if (empty() || idx > lastChunkIdx) {
            return std::make_pair(end(), end());
        } else if (idx == lastChunkIdx) {
            auto listIter = list_.end();
            listIter--;
            return std::make_pair(iterator(&list_, listIter, listIter->begin()), end());
        }
        auto listIter = list_.begin();
        while (idx--) ++listIter;
        auto first = iterator(&list_, listIter, listIter->begin());
        ++listIter;
        auto second = iterator(&list_, listIter, listIter->begin());
        return std::make_pair(first, second);
    }

    /**
     * @brief Return range of a chunk as a pair of const iterators.
     */
    inline std::pair<const_iterator, const_iterator> chunk_range(size_t idx) const {
        auto lastChunkIdx = chunk_count() - 1;
        if (empty() || idx > lastChunkIdx) {
            return std::make_pair(cend(), cend());
        } else if (idx == lastChunkIdx) {
            auto listIter = list_.cend();
            listIter--;
            return std::make_pair(iterator(&list_, listIter, listIter->cbegin()), cend());
        }
        auto listIter = list_.cbegin();
        while (idx--) ++listIter;
        auto first = iterator(&list_, listIter, listIter->cbegin());
        ++listIter;
        auto second = iterator(&list_, listIter, listIter->cbegin());
        return std::make_pair(first, second);
    }

    /**@}*/
};

template<typename T, size_t C, class Allocator>
constexpr size_t ChunkList<T, C, Allocator>::CHUNK_CAPACITY;

/**@}*/

#endif  // UTILS_CHUNK_LIST_H_

