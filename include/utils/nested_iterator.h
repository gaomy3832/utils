/*
 * Copyright 2018 Mingyu Gao
 *
 */
#ifndef UTILS_NESTED_ITERATOR_H_
#define UTILS_NESTED_ITERATOR_H_

#include <iterator>
#include <type_traits>  // for std::enable_if, std::conditional, std::is_same

/**
 * @addtogroup iterator Iterators
 *
 * @{
 */

/**
 * @brief Type tag for iterator.
 */
struct IterTagType final {};

/**
 * @brief Type tag for constant iterator.
 */
struct ConstIterTagType final {};

/**
 * @brief
 * Iterator for nested containers.
 */
template<typename T, typename B, typename TagType = IterTagType>
class NestedIterator {
public:
    /**
     * @name
     * Iterator type traits.
     */
    /**@{*/

    using value_type = typename B::value_type;
    using difference_type = typename B::difference_type;
    using pointer = typename B::pointer;
    using reference = typename B::reference;
    using iterator_category = std::bidirectional_iterator_tag;

    /**@}*/

public:
    /**
     * @name
     * Other type defines.
     */
    /**@{*/

    using const_pointer = typename B::const_pointer;
    using const_reference = typename B::const_reference;

    using TopType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
          const T, T>::type;

    using BotType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
          const B, B>::type;

    using TopIterType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
          typename T::const_iterator, typename T::iterator>::type;

    using BotIterType = typename std::conditional<std::is_same<TagType, ConstIterTagType>::value,
          typename B::const_iterator, typename B::iterator>::type;

    /**@}*/

public:
    /**
     * @name
     * Construction.
     */
    /**@{*/

    NestedIterator(TopType* top, const TopIterType& topIter, const BotIterType& botIter)
        : top_(top), topIter_(topIter), botIter_(botIter)
    {
        if (topIter_ == top_iter_end()) botIter_ = NULL_BOT_ITER;
        while (botIter_ == bot_iter_end()) {
            bool stop = (topIter_ == top_iter_end());
            if (stop) break;
            topIter_++;
            botIter_ = bot_iter_begin();
        }
    }

    /**
     * @brief Construct a past-the-end iterator.
     */
    NestedIterator(TopType* top)
        : top_(top), topIter_(), botIter_(NULL_BOT_ITER)
    {
        topIter_ = top_iter_end();
    }

    /**
     * @brief Default null iterator.
     */
    NestedIterator()
        : top_(nullptr), topIter_(), botIter_(NULL_BOT_ITER)
    {
        // Nothing else to do.
    }

    virtual ~NestedIterator() = default;
    NestedIterator(const NestedIterator&) = default;
    NestedIterator& operator=(const NestedIterator&) = default;
    NestedIterator(NestedIterator&&) = default;
    NestedIterator& operator=(NestedIterator&&) = default;

    /**@}*/

    /**
     * @name
     * Dereference.
     */
    /**@{*/

    const_reference operator*() const { return *botIter_; }

    const_pointer operator->() const { return &(this->operator*()); }

    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, reference>::type
    operator*() { return *botIter_; }

    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, pointer>::type
    operator->() { return &(this->operator*()); }

    /**@}*/

    /**
     * @name
     * Comparison.
     */
    /**@{*/

    bool operator==(const NestedIterator& other) const {
        return topIter_ == other.topIter_ && botIter_ == other.botIter_;
    }

    bool operator!=(const NestedIterator& other) const { return !(this->operator==(other)); }

    /**@}*/

    /**
     * @name
     * Advance.
     */
    /**@{*/

    NestedIterator& operator++() {
        // Undefined on before-begin or past-the-end iterators, which are not
        // dereferenceable.
        // For a before-begin iterator, ++ moves to the begin iterator.
        // For a past-the-end iterator, ++ keeps.
        // For a last iterator, ++ moves to the past-the-end iterator.
        if (botIter_ != NULL_BOT_ITER) botIter_++;
        // When reaching the end of the current bottom container, move to the
        // beginning of the next non-empty bottom container, until reaching the
        // end of the top container.
        while (botIter_ == bot_iter_end()) {
            bool stop = (topIter_ == top_iter_end());
            if (stop) break;
            topIter_++;
            botIter_ = bot_iter_begin();
        }
        return *this;
    }

    NestedIterator operator++(int) { auto it = *this; ++(*this); return it; }

    template<typename TI = TopIterType, typename BI = BotIterType>
    typename std::enable_if<
        std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<TI>::iterator_category>::value &&
        std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<BI>::iterator_category>::value,
        NestedIterator&>::type
    operator--() {
        // Undefined on before-begin or begin iterators, which cannot be
        // incremented to from dereferenceable iterators.
        // For a before-begin iterator, -- keeps.
        // For a begin iterator, -- moves to the before-begin iterator (or segfault).
        // For a past-the-end iterator, -- moves to the last iterator.
        // When reaching the beginning of the current bottom container, move to
        // the end of the next non-empty bottom container, until reaching
        // beyond the beginning of the top container.
        while (botIter_ == bot_iter_begin()) {
            bool stop = (topIter_ == top_iter_begin());
            topIter_--;
            botIter_ = bot_iter_end();
            if (stop) break;
        }
        if (botIter_ != NULL_BOT_ITER) botIter_--;
        return *this;
    }

    template<typename TI = TopIterType, typename BI = BotIterType>
    typename std::enable_if<
        std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<TI>::iterator_category>::value &&
        std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<BI>::iterator_category>::value,
        NestedIterator>::type
    operator--(int) { auto it = *this; --(*this); return it; }

    /**@{*/

private:

    /* Direct dereference, top type as vector, array, list, etc.. */

    template<typename TI = TopIterType>
    typename std::enable_if<
        std::is_same<B, typename std::iterator_traits<TI>::value_type>::value,
        typename std::iterator_traits<TI>::value_type&>::type
    top_iter_deref() { return *topIter_; }

    template<typename TI = TopIterType>
    typename std::enable_if<
        std::is_same<B, typename std::iterator_traits<TI>::value_type>::value,
        const typename std::iterator_traits<TI>::value_type&>::type
    top_iter_deref() const { return *topIter_; }

    /* Dereference mapped value, top type as unordered_map, map, etc.. */

    template<typename TI = TopIterType>
    typename std::enable_if<
        std::is_same<B, typename std::iterator_traits<TI>::value_type::second_type>::value,
        typename std::iterator_traits<TI>::value_type::second_type&>::type
    top_iter_deref() { return topIter_->second; }

    template<typename TI = TopIterType>
    typename std::enable_if<
        std::is_same<B, typename std::iterator_traits<TI>::value_type::second_type>::value,
        const typename std::iterator_traits<TI>::value_type::second_type&>::type
    top_iter_deref() const { return topIter_->second; }

private:
    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, TopIterType>::type
    top_iter_begin() { return top_->begin(); }

    template<typename U = TagType>
    typename std::enable_if<std::is_same<U, ConstIterTagType>::value, TopIterType>::type
    top_iter_begin() const { return top_->cbegin(); }

    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, TopIterType>::type
    top_iter_end() { return top_->end(); }

    template<typename U = TagType>
    typename std::enable_if<std::is_same<U, ConstIterTagType>::value, TopIterType>::type
    top_iter_end() const { return top_->cend(); }

    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, BotIterType>::type
    bot_iter_begin() {
        auto curTopIter = topIter_;
        if (curTopIter == top_->end() || ++curTopIter == top_->begin()) return NULL_BOT_ITER;
        return top_iter_deref().begin();
    }

    template<typename U = TagType>
    typename std::enable_if<std::is_same<U, ConstIterTagType>::value, BotIterType>::type
    bot_iter_begin() const {
        auto curTopIter = topIter_;
        if (curTopIter == top_->cend() || ++curTopIter == top_->cbegin()) return NULL_BOT_ITER;
        return top_iter_deref().cbegin();
    }

    template<typename U = TagType>
    typename std::enable_if<!std::is_same<U, ConstIterTagType>::value, BotIterType>::type
    bot_iter_end() {
        auto curTopIter = topIter_;
        if (curTopIter == top_->end() || ++curTopIter == top_->begin()) return NULL_BOT_ITER;
        return top_iter_deref().end();
    }

    template<typename U = TagType>
    typename std::enable_if<std::is_same<U, ConstIterTagType>::value, BotIterType>::type
    bot_iter_end() const {
        auto curTopIter = topIter_;
        if (curTopIter == top_->cend() || ++curTopIter == top_->cbegin()) return NULL_BOT_ITER;
        return top_iter_deref().cend();
    }

private:
    TopType* top_;
    TopIterType topIter_;
    BotIterType botIter_;

    static const BotIterType NULL_BOT_ITER;

    // Before-begin iterator: (top_begin_iter - 1, NULL_BOT_ITER)
    // Past-the-end iterator: (top_end_iter, NULL_BOT_ITER)
};

template<typename T, typename B, typename TagType>
const typename NestedIterator<T, B, TagType>::BotIterType
NestedIterator<T, B, TagType>::NULL_BOT_ITER = BotIterType();

/**@}*/

#endif  // UTILS_NESTED_ITERATOR_H_

