/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/containers/unsorted_map.hpp
 *
 * Purpose:     An associative container that maintains the order of element insertion.
 *
 * Created:     12th February 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/containers/unsorted_map.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::unsorted_map container
 *   class template
 *   (\ref group__library__containers "Containers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP
#define STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP_MAJOR      1
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP_MINOR      3
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP_REVISION   2
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP_EDIT       20
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */
#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR
# include <stlsoft/iterators/member_selector_iterator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR */

#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */
#ifndef STLSOFT_INCL_FUNCTIONAL
# define STLSOFT_INCL_FUNCTIONAL
# include <functional>
#endif /* !STLSOFT_INCL_FUNCTIONAL */
#ifndef STLSOFT_INCL_UTILITY
# define STLSOFT_INCL_UTILITY
# include <utility>
#endif /* !STLSOFT_INCL_UTILITY */
#ifndef STLSOFT_INCL_VECTOR
# define STLSOFT_INCL_VECTOR
# include <vector>
#endif /* !STLSOFT_INCL_VECTOR */

#ifdef STLSOFT_UNITTEST
# include <string>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief An associative container that maintains the order of element
 *   insertion.
 *
 * \ingroup group__library__containers
 *
 * \param K The container key type
 * \param T The container mapped type
 * \param P The comparison predicate type
 * \param A The allocator type
 *
\code
  stlsoft::unsorted_map
\endcode
 */
template<   ss_typename_param_k K
        ,   ss_typename_param_k T
        ,   ss_typename_param_k P   =   stlsoft_ns_qual_std(less)<K>
        ,   ss_typename_param_k A   =   ss_typename_type_def_k allocator_selector<T>::allocator_type
        >
class unsorted_map
    : public stl_collection_tag
{
public:
    /// \brief The key type
    typedef K                                                               key_type;
    /// \brief The mapped type
    typedef T                                                               mapped_type;
    /// \brief The predicate type
    typedef P                                                               key_compare;
    /// \brief The value type
    typedef stlsoft_ns_qual_std(pair)<const K, T>                           value_type;
    /// \brief The allocator type
    typedef A                                                               allocator_type;
    /// \brief This type
    typedef unsorted_map<K, T, P, A>                                        class_type;
private:
    typedef stlsoft_ns_qual_std(pair)<K, T>                                 internal_value_type_;
    typedef stlsoft_ns_qual_std(vector)<internal_value_type_
                                    ,   ss_typename_type_k allocator_selector<internal_value_type_>::allocator_type
                                    >                                       container_type_;
public:
    typedef ss_typename_type_k container_type_::size_type                   size_type;
    typedef ss_typename_type_k container_type_::difference_type             difference_type;
    typedef ss_bool_t                                                       bool_type;
    typedef value_type*                                                     pointer;
    typedef value_type const*                                               const_pointer;
    typedef value_type&                                                     reference;
    typedef value_type const&                                               const_reference;
private:
    typedef pointer_iterator<value_type, pointer, reference>                iterator_gen_;
    typedef pointer_iterator<value_type, const_pointer, const_reference>    const_iterator_gen_;
public:
    typedef ss_typename_type_k iterator_gen_::type                          iterator;
    typedef ss_typename_type_k const_iterator_gen_::type                    const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   iterator
                                                        ,   value_type
                                                        ,   reference
                                                        ,   pointer
                                                        ,   difference_type
                                                        >                   reverse_iterator;
    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   const_iterator
                                                        ,   value_type
                                                        ,   const_reference
                                                        ,   const_pointer
                                                        ,   difference_type
                                                        >                   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

public:
    /// \brief The number of elements in the map
    size_type   size() const
    {
        return m_elements.size();
    }

    /// Indicates whether the map is empty
    bool_type   empty() const
    {
        return m_elements.empty();
    }

#if 1
    reference   operator [](size_type index)
    {
        return *sap_cast<value_type*>(&m_elements[index]);
    }
    const_reference operator [](size_type index) const
    {
        return *sap_cast<value_type*>(&m_elements[index]);
    }
#else /* ? 0 */
    reference   operator [](mapped_type const& key)
    {
        return m_elements[key]);
    }
    const_reference operator [](mapped_type const& key) const
    {
        return m_elements[key]);
    }
#endif /* 0 */

    reference   front()
    {
        return *sap_cast<value_type*>(&m_elements.front());
    }
    reference   back()
    {
        return *sap_cast<value_type*>(&m_elements.back());
    }
    const_reference front() const
    {
        return *sap_cast<value_type const*>(&m_elements.front());
    }
    const_reference back() const
    {
        return *sap_cast<value_type const*>(&m_elements.back());
    }

public:
    iterator    begin()
    {
        return sap_cast<value_type*>(&*m_elements.begin());
    }
    iterator    end()
    {
        return sap_cast<value_type*>(&*m_elements.end());
    }
    const_iterator  begin() const
    {
        return sap_cast<value_type const*>(&*m_elements.begin());
    }
    const_iterator  end() const
    {
        return sap_cast<value_type const*>(&*m_elements.end());
    }
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator    rbegin()
    {
        return const_reverse_iterator(end());
    }
    reverse_iterator    rend()
    {
        return const_reverse_iterator(begin());
    }
    const_reverse_iterator  rbegin() const
    {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator  rend() const
    {
        return const_reverse_iterator(begin());
    }
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

public:
    size_type erase(key_type const& key)
    {
        ss_typename_type_k container_type_::iterator    it;

        it = stlsoft_ns_qual_std(find)( member_selector(m_elements.begin(), &internal_value_type_::first)
                                    ,   member_selector(m_elements.end(), &internal_value_type_::first)
                                    ,   key).base();

        if(m_elements.end() != it)
        {
            m_elements.erase(it);

            return 1;
        }

        return 0;
    }

public:
    void push_back(value_type const& value)
    {
        push_back(value.first, value.second);
    }
    void push_back(key_type const& key, mapped_type const& value)
    {
        ss_typename_type_k container_type_::iterator    it;

        it = stlsoft_ns_qual_std(find)( member_selector(m_elements.begin(), &internal_value_type_::first)
                                    ,   member_selector(m_elements.end(), &internal_value_type_::first)
                                    ,   key).base();

        if(m_elements.end() == it)
        {
            m_elements.push_back(value_type(key, value));
        }
        else
        {
            (*it).second = value;
        }
    }

private:
    container_type_ m_elements;
};

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/unsorted_map_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_UNSORTED_MAP */

/* ///////////////////////////// end of file //////////////////////////// */
