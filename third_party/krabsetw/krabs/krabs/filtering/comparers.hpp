// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <algorithm>

#include "../compiler_check.hpp"

namespace krabs { namespace predicates {

    namespace comparers {

        // Algorithms
        // --------------------------------------------------------------------

        /**
         * Iterator based equals
         */
        template <typename Comparer>
        struct equals
        {
            template <typename Iter1, typename Iter2>
            bool operator()(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) const
            {
                return std::equal(first1, last1, first2, last2, Comparer());
            }
        };

        /**
         * Iterator based search
         */
        template <typename Comparer>
        struct contains
        {
            template <typename Iter1, typename Iter2>
            bool operator()(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) const
            {
                // empty test range always contained, even when input range empty
                return first2 == last2
                    || std::search(first1, last1, first2, last2, Comparer()) != last1;
            }
        };

        /**
         * Iterator based starts_with
         */
        template <typename Comparer>
        struct starts_with
        {
            template <typename Iter1, typename Iter2>
            bool operator()(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) const
            {
                const auto first_nonequal = std::mismatch(first1, last1, first2, last2, Comparer());
                return first_nonequal.second == last2;
            }
        };

        /**
        * Iterator based ends_with
        */
        template <typename Comparer>
        struct ends_with
        {
            template <typename Iter1, typename Iter2>
            bool operator()(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2) const
            {
                const auto dist1 = std::distance(first1, last1);
                const auto dist2 = std::distance(first2, last2);

                if (dist2 > dist1)
                    return false;

                const auto suffix_begin = std::next(first1, dist1 - dist2);
                return std::equal(suffix_begin, last1, first2, last2, Comparer());
            }
        };

        // Custom Comparison
        // --------------------------------------------------------------------

        template <typename T>
        struct iequal_to
        {
            bool operator()(const T& a, const T& b) const
            {
                static_assert(sizeof(T) == 0,
                    "iequal_to needs a specialized overload for type");
            }
        };

        /**
        * <summary>
        *   Binary predicate for comparing two wide characters case insensitively
        *   Does not handle all locales
        * </summary>
        */
        template <>
        struct iequal_to<wchar_t>
        {
            bool operator()(const wchar_t& a, const wchar_t& b) const
            {
                return towupper(a) == towupper(b);
            }
        };

        /**
        * <summary>
        *   Binary predicate for comparing two characters case insensitively
        *   Does not handle all locales
        * </summary>
        */
        template <>
        struct iequal_to<char>
        {
            bool operator()(const char& a, const char& b) const
            {
                return toupper(a) == toupper(b);
            }
        };

    } /* namespace comparers */

} /* namespace predicates */ } /* namespace krabs */