/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_CHUNK_LIST_H_
#define UTILS_CHUNK_LIST_H_

#include <array>
#include <list>
#include <stdexcept>
#include <type_traits>  // for std::is_standard_layout, std::enable_if, std::conditional, std::is_same
#include <utility>  // for std::swap

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
    private:
        using ChunkStorageType = std::array<value_type, CHUNK_CAPACITY>;
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
    using ChunkListStorageType = std::list<Chunk, Allocator>;

private:
    ChunkListStorageType list_;
    size_type size_;

private:
    struct ConstIterTagType {};
    struct IterTagType {};

    template<typename TagType>
    class BaseIterator {
    public:
        using value_type = ChunkList::value_type;
        using difference_type = ChunkList::difference_type;
        using pointer = ChunkList::pointer;
        using reference = ChunkList::reference;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        using LSType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
              const ChunkListStorageType, ChunkListStorageType>::type;

        using LSIterType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
              typename ChunkListStorageType::const_iterator,
              typename ChunkListStorageType::iterator>::type;

        using CSIterType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
              typename Chunk::ChunkStorageType::const_iterator,
              typename Chunk::ChunkStorageType::iterator>::type;

    public:
        BaseIterator(LSType* list, const LSIterType& listIter, const CSIterType& chIter)
            : list_(list), listIter_(listIter), chIter_(chIter)
        {
            // Nothing else to do.
        }

        BaseIterator(LSType* list, const LSIterType& listIter)
            : list_(list), listIter_(listIter), chIter_()
        {
            // Nothing else to do.
        }

        BaseIterator(LSType* list = nullptr)
            : list_(list), listIter_(), chIter_()
        {
            // Nothing else to do.
        }

        virtual ~BaseIterator() = default;
        BaseIterator(const BaseIterator&) = default;
        BaseIterator& operator=(const BaseIterator&) = default;
        BaseIterator(BaseIterator&&) = default;
        BaseIterator& operator=(BaseIterator&&) = default;

        const_reference operator*() const { return *chIter_; }

        const_pointer operator->() const { return &(this->operator*()); }

        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, reference>::type
        operator*() { return *(this->chIter_); }

        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, pointer>::type
        operator->() { return &(this->operator*()); }

        bool operator==(const BaseIterator& other) const {
            return listIter_ == other.listIter_ && chIter_ == other.chIter_;
        }

        bool operator!=(const BaseIterator& other) const { return !(this->operator==(other)); }

        BaseIterator& operator++() {
            // Undefined for ++ on before-begin or past-the-end iterators which
            // are not dereferenceable.
            if (chIter_ != NULL_CHUNK_ITER) {
                chIter_++;
                if (chIter_ == ch_iter_end()) {
                    if (++listIter_ == list_iter_end()) {
                        // Past-the-end: (list_iter_end(), ()).
                        chIter_ = NULL_CHUNK_ITER;
                        return *this;
                    }
                    chIter_ = ch_iter_begin();
                }
            }
            return *this;
        }

        BaseIterator operator++(int) { auto it = *this; ++(*this); return it; }

        BaseIterator& operator--() {
            // Need to support -- on all iterators which can be incremented to,
            // including all dereferenceable and past-the-end iterators.
            if (chIter_ != NULL_CHUNK_ITER) {
                if (chIter_ == ch_iter_begin()) {
                    if (listIter_-- == list_iter_begin()) {
                        // Before-begin: (list_iter_begin() - 1, ()).
                        chIter_ = NULL_CHUNK_ITER;
                        return *this;
                    }
                    chIter_ = ch_iter_end();
                }
                chIter_--;
            } else if (listIter_ == list_iter_end()) {
                // Past-the-end iterator.
                listIter_--;
                chIter_ = ch_iter_end();
                chIter_--;
            }
            return *this;
        }

        BaseIterator operator--(int) { auto it = *this; --(*this); return it; }

    private:
        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, LSIterType>::type
        list_iter_begin() { return list_->begin(); }

        template<typename U = TagType>
        typename std::enable_if<std::is_same<U, ConstIterTagType>::value, LSIterType>::type
        list_iter_begin() { return list_->cbegin(); }

        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, LSIterType>::type
        list_iter_end() { return list_->end(); }

        template<typename U = TagType>
        typename std::enable_if<std::is_same<U, ConstIterTagType>::value, LSIterType>::type
        list_iter_end() { return list_->cend(); }

        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, CSIterType>::type
        ch_iter_begin() { return listIter_->begin(); }

        template<typename U = TagType>
        typename std::enable_if<std::is_same<U, ConstIterTagType>::value, CSIterType>::type
        ch_iter_begin() { return listIter_->cbegin(); }

        template<typename U = TagType>
        typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, CSIterType>::type
        ch_iter_end() { return listIter_->end(); }

        template<typename U = TagType>
        typename std::enable_if<std::is_same<U, ConstIterTagType>::value, CSIterType>::type
        ch_iter_end() { return listIter_->cend(); }

    private:
        LSType* const list_;
        LSIterType listIter_;
        CSIterType chIter_;

        static const CSIterType NULL_CHUNK_ITER;
    };

public:
    using iterator = BaseIterator<IterTagType>;
    using const_iterator = BaseIterator<ConstIterTagType>;

    /**
     * @name
     * Iterators.
     */
    /**@{*/

    /**
     * @brief Return an iterator to the beginning.
     */
    inline iterator begin() {
        return empty() ? end() : iterator(&list_, list_.begin(), list_.front().begin());
    }

    /**
     * @brief Return an iterator to the end.
     */
    inline iterator end() { return iterator(&list_, list_.end()); }

    /**
     * @brief Return a constant iterator to the beginning.
     */
    inline const_iterator cbegin() const {
        return empty() ? cend() : const_iterator(&list_, list_.cbegin(), list_.front().cbegin());
    }

    /**
     * @brief Return a constant iterator to the end.
     */
    inline const_iterator cend() const { return const_iterator(&list_, list_.cend()); }

    /**@}*/
};

template<typename T, size_t C, class Allocator>
constexpr size_t ChunkList<T, C, Allocator>::CHUNK_CAPACITY;

template<typename T, size_t C, class Allocator>
template<typename TagType>
const typename ChunkList<T, C, Allocator>::template BaseIterator<TagType>::CSIterType
ChunkList<T, C, Allocator>::BaseIterator<TagType>::NULL_CHUNK_ITER = CSIterType();

/**@}*/

#endif  // UTILS_CHUNK_LIST_H_

