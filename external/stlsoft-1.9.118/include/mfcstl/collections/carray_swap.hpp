/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/collections/carray_swap.hpp
 *
 * Purpose:     Contains the CArray_swap utility function.
 *
 * Created:     4th August 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file mfcstl/collections/carray_swap.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::CArray_swap utility function
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP
#define MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP_MAJOR     2
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP_MINOR     0
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP_REVISION  2
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP_EDIT      14
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

/* /////////////////////////////////////////////////////////////////////////
 * Version compatibility
 */

// All version of MFC up to and including 8.0 support swap-by-members
#if _MFC_VER <= 0x0800 && \
    !defined(MFCSTL_CARRAY_SWAP_MEMBERS_SUPPORT)
# define MFCSTL_CARRAY_SWAP_MEMBERS_SUPPORT
#endif /* _MFC_VER <= 0x0800 && !MFCSTL_CARRAY_SWAP_MEMBERS_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef MFCSTL_CARRAY_SWAP_MEMBERS_SUPPORT

#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
namespace array_impl
{
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

/** Helper class that facilitates safe swapping of the state of CArray<>
 * instances
 */
template <class A>
class CArray_swap_veneer
    : public A
{
/// \name Member Types
/// @{
public:
    typedef CArray_swap_veneer<A>   class_type;
/// @}

/// \name Operations
/// @{
public:
    static void swap(class_type& lhs, class_type& rhs)
    {
        std_swap(lhs.m_pData, rhs.m_pData);
        std_swap(lhs.m_nSize, rhs.m_nSize);
        std_swap(lhs.m_nMaxSize, rhs.m_nMaxSize);
        std_swap(lhs.m_nGrowBy, rhs.m_nGrowBy);
    }
/// @}
};

#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
} // namespace array_impl
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */


/** \brief Swaps the contents of two instances of CArray-family
 *    containers.
 *
 * \ingroup group__library__collections
 */
template <class A>
void CArray_swap(A& lhs, A& rhs)
{
#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
    using array_impl::CArray_swap_veneer;
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

    typedef CArray_swap_veneer<A>   swapper_t;

    swapper_t::swap(static_cast<swapper_t&>(lhs), static_cast<swapper_t&>(rhs));
}

#endif /* MFCSTL_CARRAY_SWAP_MEMBERS_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_ARRAY_SWAP */

/* ///////////////////////////// end of file //////////////////////////// */
