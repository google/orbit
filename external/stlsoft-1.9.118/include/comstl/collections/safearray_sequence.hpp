/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/collections/safearray_sequence.hpp
 *
 * Purpose:     STL sequence for COM collection interfaces.
 *
 * Created:     17th April 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file comstl/collections/safearray_sequence.hpp
 *
 * \brief [C++ only] Definition of the comstl::safearray_sequence
 *   collection class template
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef COMSTL_INCL_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE
#define COMSTL_INCL_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE_MAJOR     4
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE_MINOR     2
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE_REVISION  1
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE_EDIT      61
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error comstl/collections/safearray_sequence.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS
# include <comstl/error/exceptions.hpp>
#endif /* !COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Provides an STL-iterable view over a COM SAFEARRAY
 *
 * \ingroup group__library__collections
 */
template <ss_typename_param_k T>
class safearray_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef SAFEARRAY const*                                LPCSAFEARRAY;
public:
    /// \brief The value type
    typedef T                                               value_type;
    /// \brief The current parameterisation of the type
    typedef safearray_sequence<T>                           class_type;
    /// \brief The size type
    typedef cs_size_t                                       size_type;
    /// \brief The difference type
    typedef cs_ptrdiff_t                                    difference_type;
    /// \brief The reference type
    typedef value_type&                                     reference;
    /// \brief The non-mutable (const) reference type
    typedef value_type const&                               const_reference;
    /// \brief The pointer type
    typedef value_type*                                     pointer;
    /// \brief The non-mutable (const) pointer type
    typedef value_type const*                               const_pointer;
    /// \brief The iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator<value_type
                                    ,   pointer
                                    ,   reference
                                    >::type                 iterator;
    /// \brief The non-mutating (const) iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator<value_type const
                                    ,   const_pointer
                                    ,   const_reference
                                    >::type                 const_iterator;

    /// \brief The non-mutating (const) reverse iterator type
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef stlsoft_ns_qual(reverse_iterator_base)< iterator
                                                ,   value_type
                                                ,   reference
                                                ,   pointer
                                                ,   difference_type
                                                >           reverse_iterator;

    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   const_iterator
                                                    ,   value_type const
                                                    ,   const_reference
                                                    ,   const_pointer
                                                    ,   difference_type
                                                    >       const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k safearray_sequence(LPCSAFEARRAY array); // throw variant_type_exception
/// @}

/// \name Iteration
/// @{
public:
    /// \brief Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator          begin() const;
    /// \brief Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator          end() const;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// \brief Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// \brief Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

    /// \brief Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator                begin();
    /// \brief Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator                end();
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// \brief Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator        rbegin();
    /// \brief Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator        rend();
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Attributes
/// @{
public:
    /// \brief The number of elements in the array
    size_type       size() const;
    /// \brief Indicates whether the array is empty
    bool            empty() const;
/// @}

/// \name Direct memory access
/// @{
public:
//  void            **access_data();    // Should RAII this
//  void            unaccess_data();
/// @}

/// \name Implementation
/// @{
private:
    static bool     type_is_compatible_(LPCSAFEARRAY array);
    static DWORD    calc_size_(LPCSAFEARRAY array);
/// @}

/// \name Members
/// @{
private:
    LPCSAFEARRAY    m_sa;
    DWORD const     m_cItems;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/safearray_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline /* static */ bool safearray_sequence<T>::type_is_compatible_(LPCSAFEARRAY array)
{
    return sizeof(value_type) == array->cbElements;
}

template <ss_typename_param_k T>
inline /* static */ DWORD safearray_sequence<T>::calc_size_(LPCSAFEARRAY array)
{
    DWORD   cItems  =   1;

    for(USHORT dim = 0; dim < array->cDims; ++dim)
    {
        cItems *= array->rgsabound[dim].cElements;
    }

    return cItems;
}

template <ss_typename_param_k T>
inline safearray_sequence<T>::safearray_sequence(LPCSAFEARRAY array) // throw variant_type_exception
    : m_sa(array)
    , m_cItems(calc_size_(array))
{
    if(!type_is_compatible_(array))
    {
        STLSOFT_THROW_X(variant_type_exception("initialising safearray_sequence from safe array to incompatible type", DISP_E_BADVARTYPE));
    }
}

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::const_iterator safearray_sequence<T>::begin() const
{
    return static_cast<pointer>(m_sa->pvData);
}

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::const_iterator safearray_sequence<T>::end() const
{
    return static_cast<pointer>(m_sa->pvData) + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::const_reverse_iterator safearray_sequence<T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::const_reverse_iterator safearray_sequence<T>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::iterator safearray_sequence<T>::begin()
{
    return static_cast<pointer>(m_sa->pvData);
}

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::iterator safearray_sequence<T>::end()
{
    return static_cast<pointer>(m_sa->pvData) + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::reverse_iterator safearray_sequence<T>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::reverse_iterator safearray_sequence<T>::rend()
{
    return reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k T>
inline ss_typename_type_ret_k safearray_sequence<T>::size_type safearray_sequence<T>::size() const
{
    return m_cItems;
}

template <ss_typename_param_k T>
inline bool safearray_sequence<T>::empty() const
{
    return 0 != size();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace stlsoft::comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_COLLECTIONS_HPP_SAFEARRAY_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
