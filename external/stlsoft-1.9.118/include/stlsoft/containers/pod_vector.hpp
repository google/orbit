/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/containers/pod_vector.hpp
 *
 * Purpose:     Contains the pod_vector class.
 *
 * Created:     23rd December 2003
 * Updated:     10th August 2009
 *
 * Thanks to:   Chris Newcombe for requesting sufficient enhancements to
 *              auto_buffer such that pod_vector was born.
 *
 *              Christian Roessel, for spotting the bug in the copy ctor that
 *              fails an assert if the copied instance is empty
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/containers/pod_vector.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::pod_vector container class
 *   template
 *   (\ref group__library__containers "Containers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_POD_VECTOR
#define STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_POD_VECTOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_POD_VECTOR_MAJOR       4
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_POD_VECTOR_MINOR       2
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_POD_VECTOR_REVISION    2
# define STLSOFT_VER_STLSOFT_CONTAINERS_HPP_POD_VECTOR_EDIT        76
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/containers/pod_vector.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD
# include <stlsoft/algorithms/pod.hpp>          // for pod_copy_n(), etc.
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                            // for std::out_of_range
#endif /* !STLSOFT_INCL_STDEXCEPT */

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

/** \brief Efficient vector class for use with POD types only
 *
 * \ingroup group__library__containers
 */
template<   ss_typename_param_k T
#if defined(STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT) && \
    defined(STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT)
        ,   ss_typename_param_k A       =   ss_typename_type_def_k allocator_selector<T>::allocator_type
        ,   ss_size_t           SPACE   =   64
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT && STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A       /* =    ss_typename_type_def_k stlsoft_ns_qual(allocator_selector)<T>::allocator_type */
        ,   ss_size_t           SPACE   /* =    64 */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT && STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        >
class pod_vector
    : public stl_collection_tag
{
/// \name Typedefs
/// @{
private:
    typedef auto_buffer_old<T, A, SPACE>                            buffer_type_;
public:
    /// The value type
    typedef ss_typename_type_k buffer_type_::value_type             value_type;
    /// The allocator type
    typedef ss_typename_type_k buffer_type_::allocator_type         allocator_type;
    /// The type of the current parameterisation
    typedef pod_vector<T, A, SPACE>                                 class_type;
    /// The reference type
    typedef ss_typename_type_k buffer_type_::reference              reference;
    /// The non-mutable (const) reference type
    typedef ss_typename_type_k buffer_type_::const_reference        const_reference;
    /// The pointer type
    typedef ss_typename_type_k buffer_type_::pointer                pointer;
    /// The non-mutable (const) pointer type
    typedef ss_typename_type_k buffer_type_::const_pointer          const_pointer;
    /// The iterator type
    typedef ss_typename_type_k buffer_type_::iterator               iterator;
    /// The non-mutable (const) iterator type
    typedef ss_typename_type_k buffer_type_::const_iterator         const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The type of the non-const (mutating) reverse iterator
    typedef ss_typename_type_k buffer_type_::reverse_iterator       reverse_iterator;
    /// The type of the const (non-mutating) reverse iterator
    typedef ss_typename_type_k buffer_type_::const_reverse_iterator const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The size type
    typedef ss_typename_type_k buffer_type_::size_type              size_type;
    /// The difference type
    typedef ss_typename_type_k buffer_type_::difference_type        difference_type;
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k pod_vector(size_type cItems = 0);
    pod_vector(size_type cItems, value_type const& value);
    pod_vector(class_type const& rhs);
    pod_vector(const_iterator first, const_iterator last);

    pod_vector& operator =(class_type const& rhs);
/// @}

/// \name Iteration
/// @{
public:
    iterator                begin();
    const_iterator          begin() const;
    iterator                end();
    const_iterator          end() const;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    reverse_iterator        rbegin();
    const_reverse_iterator  rbegin() const;
    reverse_iterator        rend();
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Attributes
/// @{
public:
    size_type       size() const;
    size_type       capacity() const;
    size_type       max_size() const;
    ss_bool_t       empty() const;
    allocator_type  get_allocator() const;
/// @}

/// \name Accessors
/// @{
public:
    reference       at(size_type index);
    const_reference at(size_type index) const;
    reference       operator [](size_type index);
    const_reference operator [](size_type index) const;
    reference       front();
    const_reference front() const;
    reference       back();
    const_reference back() const;
/// @}

/// \name Operations
/// @{
public:
    void        clear();
    void        swap(class_type& rhs);
    void        reserve(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */;
// Note: resize() is split into two, so the one-param version can be very quick
    void        resize(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */;
    void        resize(size_type cItems, value_type const& value) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */;
    void        push_back(value_type const& value);
    void        pop_back();

    void        assign(const_iterator first, const_iterator last);
    void        assign(size_type cItems, value_type const& value = value_type());
    iterator    insert(iterator it, value_type const& value = value_type());
    void        insert(iterator it, size_type cItems, value_type const& value);
    void        insert(iterator it, const_iterator first, const_iterator last);
    iterator    erase(iterator it);
    iterator    erase(iterator first, iterator last);
/// @}

/// \name Implementation
/// @{
private:
    pointer         begin_();
    const_pointer   begin_() const;

    void        range_check_(size_type index) const /* stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) ) */;

    ss_bool_t   resize_(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */;

    ss_bool_t   is_valid_() const;
/// @}

/// \name Members
/// @{
private:
    size_type       m_cItems;   // A size member is used, rather than m_end (iterator), because some of the state
                                // is maintained in the parent class. Doing it this way makes swap() and other methods
                                // very simple.
    buffer_type_    m_buffer;   // The auto_buffer
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_bool_t operator ==(pod_vector<T, A, SPACE> const& lhs, pod_vector<T, A, SPACE> const& rhs)
{
    if(lhs.size() != rhs.size())
    {
        return false;
    }
    else
    {
#if 0
        for(ss_typename_type_k pod_vector<T, A, SPACE>::size_type i = 0, size = lhs.size(); i < size; ++i)
        {
            if(lhs[i] != rhs[i])
            {
                return false;
            }
        }

        return true;
#else /* ? 0 */
        return 0 == memcmp(&lhs[0], &rhs[0], sizeof(ss_typename_type_k pod_vector<T, A, SPACE>::size_type) * lhs.size());
#endif /* 0 */
    }
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_bool_t operator !=(pod_vector<T, A, SPACE> const& lhs, pod_vector<T, A, SPACE> const& rhs)
{
    return !operator ==(lhs, rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline void swap(pod_vector<T, A, SPACE>& lhs, pod_vector<T, A, SPACE>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/pod_vector_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER >= 1310
# define STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS
#endif /* compiler */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::pointer pod_vector<T, A, SPACE>::begin_()
{
    return m_buffer.data();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_pointer pod_vector<T, A, SPACE>::begin_() const
{
    return m_buffer.data();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline void pod_vector<T, A, SPACE>::range_check_(size_type index) const /* stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) ) */
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::range_check_(ss_typename_type_k pod_vector<T, A, SPACE>::size_type index) const /* stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) ) */
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::range_check_(size_type index) const /* stlsoft_throw_1(stlsoft_ns_qual_std(out_of_range) ) */
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(!(index < size()))
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("pod vector index out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    STLSOFT_MESSAGE_ASSERT("w index out of range", index < size());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_bool_t pod_vector<T, A, SPACE>::resize_(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_bool_t pod_vector<T, A, SPACE>::resize_(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_bool_t pod_vector<T, A, SPACE>::resize_(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    // This method is only called for insertions, so we can make some assumptions.

    size_type   curr_capacity   =   capacity();

    // We only resize the internal buffer if it is not large enough
    if(cItems > curr_capacity)
    {
        size_type   capacity = m_buffer.internal_size() + cItems;

        capacity -= capacity % m_buffer.internal_size();

        if(!m_buffer.resize(capacity))
        {
            return false;
        }
    }

    m_cItems = cItems;

    return true;
}


template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_bool_t pod_vector<T, A, SPACE>::is_valid_() const
{
    if(m_buffer.size() < m_cItems)
    {
        return false;
    }

    return true;
}


// Construction

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline /* ss_explicit_k */ pod_vector<T, A, SPACE>::pod_vector(size_type cItems /* = 0 */)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline /* ss_explicit_k */ pod_vector<T, A, SPACE>::pod_vector(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems /* = 0 */)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline /* ss_explicit_k */ pod_vector<T, A, SPACE>::pod_vector(size_type cItems /* = 0 */)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    : m_buffer(cItems)
{
    m_cItems = m_buffer.size(); // This is done here, since it comes before m_buffer in
                                // the object layout for efficiency (caching) reasons

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline pod_vector<T, A, SPACE>::pod_vector(size_type cItems, value_type const& value)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline pod_vector<T, A, SPACE>::pod_vector(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline pod_vector<T, A, SPACE>::pod_vector(size_type cItems, value_type const& value)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    : m_buffer(cItems)
{
    m_cItems = m_buffer.size(); // This is done here, since it comes before m_buffer in
                                // the object layout for efficiency (caching) reasons

    pod_fill_n(begin_(), size(), value);

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline pod_vector<T, A, SPACE>::pod_vector(class_type const& rhs)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline pod_vector<T, A, SPACE>::pod_vector(ss_typename_type_k pod_vector<T, A, SPACE>::class_type const& rhs)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline pod_vector<T, A, SPACE>::pod_vector(class_type const& rhs)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    : m_buffer(rhs.size())
{
    m_cItems = m_buffer.size(); // This is done here, since it comes before m_buffer in
                                // the object layout for efficiency (caching) reasons

    pod_copy_n(begin_(), rhs.begin_(), size());

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline pod_vector<T, A, SPACE>::pod_vector(const_iterator first, const_iterator last)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline pod_vector<T, A, SPACE>::pod_vector(ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator first, ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator last)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline pod_vector<T, A, SPACE>::pod_vector(const_iterator first, const_iterator last)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    : m_buffer(static_cast<ss_size_t>(last - first))
{
    m_cItems = m_buffer.size(); // This is done here, since it comes before m_buffer in
                                // the object layout for efficiency (caching) reasons

    if(0 != size()) // It will either be the full size requested, or 0
    {
        pod_copy(&*first, &*last, begin_());
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline pod_vector<T, A, SPACE> &pod_vector<T, A, SPACE>::operator =(class_type const& rhs)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline pod_vector<T, A, SPACE> &pod_vector<T, A, SPACE>::operator =(ss_typename_type_k pod_vector<T, A, SPACE>::class_type const& rhs)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline pod_vector<T, A, SPACE> &pod_vector<T, A, SPACE>::operator =(class_type const& rhs)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
// NOTE: This will be optimised by testing the contents of this and the rhs

    STLSOFT_ASSERT(is_valid_());

    class_type  temp(rhs);

    temp.swap(*this);

    STLSOFT_ASSERT(is_valid_());

    return *this;
}

// Iteration


template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::begin()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::begin()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return m_buffer.begin();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_iterator pod_vector<T, A, SPACE>::begin() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_iterator pod_vector<T, A, SPACE>::begin() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return m_buffer.begin();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::end()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::end()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return &begin_()[size()];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_iterator pod_vector<T, A, SPACE>::end() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_iterator pod_vector<T, A, SPACE>::end() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return &begin_()[size()];
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reverse_iterator pod_vector<T, A, SPACE>::rbegin()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reverse_iterator pod_vector<T, A, SPACE>::rbegin()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return reverse_iterator(end());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reverse_iterator pod_vector<T, A, SPACE>::rbegin() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reverse_iterator pod_vector<T, A, SPACE>::rbegin() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return const_reverse_iterator(end());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reverse_iterator pod_vector<T, A, SPACE>::rend()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reverse_iterator pod_vector<T, A, SPACE>::rend()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return reverse_iterator(begin());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reverse_iterator pod_vector<T, A, SPACE>::rend() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reverse_iterator pod_vector<T, A, SPACE>::rend() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Attributes

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::size_type pod_vector<T, A, SPACE>::size() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_size_t pod_vector<T, A, SPACE>::size() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return m_cItems;
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::size_type pod_vector<T, A, SPACE>::capacity() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_size_t pod_vector<T, A, SPACE>::capacity() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return m_buffer.size();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::size_type pod_vector<T, A, SPACE>::max_size() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_size_t pod_vector<T, A, SPACE>::max_size() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return static_cast<size_type>(-1) / sizeof(value_type);
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline ss_bool_t pod_vector<T, A, SPACE>::empty() const
{
    STLSOFT_ASSERT(is_valid_());

    return 0 == size();
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::allocator_type pod_vector<T, A, SPACE>::get_allocator() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::allocator_type pod_vector<T, A, SPACE>::get_allocator() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    return m_buffer.get_allocator();
}

// Accessors
template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::at(size_type index)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::at(ss_typename_type_k pod_vector<T, A, SPACE>::size_type index)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T &pod_vector<T, A, SPACE>::at(size_type index)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    range_check_(index);

    STLSOFT_ASSERT(is_valid_());

    return begin_()[index];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::at(size_type index) const
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::at(ss_typename_type_k pod_vector<T, A, SPACE>::size_type index) const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T const& pod_vector<T, A, SPACE>::at(size_type index) const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    range_check_(index);

    STLSOFT_ASSERT(is_valid_());

    return begin_()[index];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::operator [](size_type index)
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::operator [](ss_typename_type_k pod_vector<T, A, SPACE>::size_type index)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T &pod_vector<T, A, SPACE>::operator [](size_type index)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    // The index must be <= the size(). It is allowed to be equal to the size because
    // we must facilitate the taking of the end() element in order to specify ranges.
    STLSOFT_MESSAGE_ASSERT("Requested index is out of range", !(size() < index));

    return begin_()[index];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::operator [](size_type index) const
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::operator [](ss_typename_type_k pod_vector<T, A, SPACE>::size_type index) const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T const& pod_vector<T, A, SPACE>::operator [](size_type index) const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("Requested index is out of range", index < size());

    return begin_()[index];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::front()
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::front()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T &pod_vector<T, A, SPACE>::front()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("Range is empty!", 0 != size());

    return begin_()[0];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::front() const
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::front() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T const& pod_vector<T, A, SPACE>::front() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("Range is empty!", 0 != size());

    return begin_()[0];
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::back()
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::reference pod_vector<T, A, SPACE>::back()
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T &pod_vector<T, A, SPACE>::back()
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("Range is empty!", 0 != size());

    return begin_()[size() - 1];
}


template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED_EXCEPT_ARGS)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::back() const
#elif defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::const_reference pod_vector<T, A, SPACE>::back() const
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline T const& pod_vector<T, A, SPACE>::back() const
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("Range is empty!", 0 != size());

    return begin_()[size() - 1];
}

// Operations

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline void pod_vector<T, A, SPACE>::clear()
{
    STLSOFT_ASSERT(is_valid_());

    if(m_buffer.resize(0))
    {
        m_cItems = 0;
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline void pod_vector<T, A, SPACE>::swap(pod_vector<T, A, SPACE>& rhs)
{
    STLSOFT_ASSERT(is_valid_());

    m_buffer.swap(rhs.m_buffer);
    std_swap(m_cItems, rhs.m_cItems);

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::reserve(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::reserve(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    // We do not follow vector's much maligned example and refuse to truncate, although
    // we only do so if the requested size is 0.
    if( 0 == cItems ||
        cItems > size())
    {
        m_buffer.resize(cItems);
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::resize(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::resize(size_type cItems) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    resize(cItems, value_type());

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::resize(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::resize(size_type cItems, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value) /* stlsoft_throw_1(stlsoft_ns_qual_std(bad_alloc) ) */
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    if(m_buffer.resize(cItems))
    {
        if(m_cItems < cItems)
        {
            pod_fill_n(begin_() + m_cItems, cItems - m_cItems, value);
        }

        m_cItems = cItems;
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::push_back(ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::push_back(ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    insert(end(), value);

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
inline void pod_vector<T, A, SPACE>::pop_back()
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_MESSAGE_ASSERT("No elements to pop", size() > 0);

    if(0 == --m_cItems)
    {
        m_buffer.resize(0);
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::assign(ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator first, ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator last)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::assign(const_iterator first, const_iterator last)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

// NOTE: This will be optimised by testing the contents of this and the rhs

    class_type  temp(first, last);

    temp.swap(*this);

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::assign(ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value /* = value_type() */)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::assign(size_type cItems, value_type const& value /* = value_type() */)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

// NOTE: This will be optimised by testing the contents of this and the rhs

    class_type  temp(cItems, value);

    temp.swap(*this);

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::insert(ss_typename_type_k pod_vector<T, A, SPACE>::iterator it, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value /* = value_type() */)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::insert(iterator it, value_type const& value /* = value_type() */)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_ASSERT(!(end() < it));
    STLSOFT_ASSERT(!(it < begin()));

    size_type   index = static_cast<size_type>(it - begin());

    insert(it, 1, value);

    STLSOFT_ASSERT(is_valid_());

    return begin() + index;
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::insert(ss_typename_type_k pod_vector<T, A, SPACE>::iterator it, ss_typename_type_k pod_vector<T, A, SPACE>::size_type cItems, ss_typename_type_k pod_vector<T, A, SPACE>::value_type const& value)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::insert(iterator it, size_type cItems, value_type const& value)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_ASSERT(!(end() < it));
    STLSOFT_ASSERT(!(it < begin()));

    size_type   curr_size   =   size();
    size_type   index       =   static_cast<size_type>(it - begin());

    if(resize_(size() + cItems))
    {
        size_type cMove = curr_size - index;

        // The resize_ may have invalidated the iterator(s)!!
        it = begin() + index;

        // Move the existing ones up out of the way
        pod_move_n(&it[cItems], &it[0], cMove);

        // And insert the new ones
        pod_fill_n(begin_() + index, cItems, value);
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline void pod_vector<T, A, SPACE>::insert(ss_typename_type_k pod_vector<T, A, SPACE>::iterator it, ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator first, ss_typename_type_k pod_vector<T, A, SPACE>::const_iterator last)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline void pod_vector<T, A, SPACE>::insert(iterator it, const_iterator first, const_iterator last)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_ASSERT(!(end() < it));
    STLSOFT_ASSERT(!(it < begin()));

    size_type   curr_size   =   size();
    size_type   index       =   it - begin();
    size_type   cItems      =   last - first;

    if(resize_(size() + cItems))
    {
        size_type cMove = curr_size - index;

        // The resize_ may have invalidated the iterator(s)!!
        it = begin() + index;

        // Move the existing ones up out of the way
        pod_move_n(&it[cItems], &it[0], cMove);

        // And insert the new ones
        pod_copy_n(it, first, cItems);
    }

    STLSOFT_ASSERT(is_valid_());
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::erase(ss_typename_type_k pod_vector<T, A, SPACE>::iterator it)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::erase(iterator it)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_ASSERT(it < end());
    STLSOFT_ASSERT(!(it < begin()));

    size_type   index = it - begin();
    size_type   cMove = size() - (index + 1);

    pod_move_n(&it[0], &it[1], cMove);

    if(0 == --m_cItems)
    {
        m_buffer.resize(0);

        it = begin();
    }

    STLSOFT_ASSERT(is_valid_());

    return it;
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
        >
#if defined(STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED)
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::erase(ss_typename_type_k pod_vector<T, A, SPACE>::iterator first, ss_typename_type_k pod_vector<T, A, SPACE>::iterator last)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
inline ss_typename_type_ret_k pod_vector<T, A, SPACE>::iterator pod_vector<T, A, SPACE>::erase(iterator first, iterator last)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    STLSOFT_ASSERT(is_valid_());

    STLSOFT_ASSERT(first < end());
    STLSOFT_ASSERT(!(first < begin()));
    STLSOFT_ASSERT(!(end() < last));
    STLSOFT_ASSERT(!(last < begin()));

    size_type   curr_size   =   size();
    size_type   index_first =   first - begin();
    size_type   index_last  =   last - begin();
    size_type   cItems      =   last - first;
    size_type   cMove       =   curr_size - index_last;

    // Move the remaining ones down
    pod_move_n(&first[0], &last[0], cMove);

    resize(curr_size - cItems);

    STLSOFT_ASSERT(is_valid_());

    return begin() + index_first;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* In the special case of Intel behaving as VC++ 7.0 or earlier on Win32, we
 * illegally insert into the std namespace.
 */
#if defined(STLSOFT_CF_std_NAMESPACE)
# if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
         defined(_MSC_VER))) && \
     _MSC_VER < 1310
namespace std
{
    template<   ss_typename_param_k         T
            ,   ss_typename_param_k         A
            ,   stlsoft_ns_qual(ss_size_t)  SPACE
            >
    inline void swap(stlsoft_ns_qual(pod_vector)<T, A, SPACE>& lhs, stlsoft_ns_qual(pod_vector)<T, A, SPACE>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONTAINERS_HPP_POD_VECTOR */

/* ///////////////////////////// end of file //////////////////////////// */
