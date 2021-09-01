// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <string>

#include "compiler_check.hpp"

namespace krabs {

    /**
     * Wraps a range of a collection starting at the location
     * specified by the begin iterator and ending a the location
     * specified by the end iterator. The underlying items are
     * left in-place and should be considered const
     */
    template <typename T>
    struct collection_view
    {
    private:
        const T beg_;
        const T end_;

    public:
        /**
         * Construct a new view for the range specified by the
         * iterators 'begin' and 'end'
         */
        collection_view(const T begin, const T end)
            : beg_(begin)
            , end_(end)
        { }

        /**
         * Get the iterator for the beginning of the view range
         */
        const T begin() const
        {
            return beg_;
        }

        /**
         * Get the iterator for the end of the view range
         */
        const T end() const
        {
            return end_;
        }
    };

    /**
     * Create a view over the range specified by iterators 'begin' and 'end'
     */
    template <typename T>
    inline collection_view<T> view(const T& begin, const T& end)
    {
        return{ begin, end };
    }

    /**
     * Create a const_iterator view over the specified string
     */
    template <typename T>
    inline collection_view<typename std::basic_string<T>::const_iterator> view(const std::basic_string<T>& string)
    {
        return{ string.cbegin(), string.cend() };
    }

    /**
     * Create a const view over the range starting at 'begin' extending 'length' items
     */
    template <typename T>
    inline collection_view<const T*> view(const T* begin, size_t length)
    {
        return{ begin, begin + length };
    }

    /**
     * Create a const view over the specified array
     */
    template <typename T, size_t n>
    inline collection_view<const T*> view(const T(&arr)[n])
    {
        return{ arr, arr + n };
    }

} /* namespace krabs */