/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/collections/clist_traits.hpp
 *
 * Purpose:     Definition of the CList_traits traits class.
 *
 * Created:     1st December 2002
 * Updated:     3rd February 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
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


/** \file mfcstl/collections/clist_traits.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::CList_traits traits class
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS
#define MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS_MAJOR       3
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS_MINOR       0
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS_REVISION    1
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS_EDIT        58
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(MFCSTL_NO_INCLUDE_AFXTEMPL_BY_DEFAULT)
# include <afxtempl.h>
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT && !MFCSTL_NO_INCLUDE_AFXTEMPL_BY_DEFAULT */

#ifdef STLSOFT_UNITTEST
# include <stlsoft/meta/is_same_type.hpp>
#endif /* STLSOFT_UNITTEST */

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

/* /////////////////////////////////////////////////////////////////////////
 * Pre-processor options
 *
 * Because the CObList, CPtrList, CStringList and CList<,> implementations all
 * internally represent their logical position indicators (of type POSTION) as
 * pointers to the nodes within the lists, it is workable to be able to copy
 * these position variables.
 *
 * However, nothing in the MFC documentation stipulates this to be a reliable
 * and documented part of the classes' interfaces, so this is a potentially
 * unsafe assumption.
 *
 * Therefore, the iterator model for the CList class is Input Iterator.
 * If you wish to use forward iterators, you may specify the preprocessor
 * symbol _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR, in which case the iterator
 * classes will implement copy semantics, rather than the default move
 * semantics.
 */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Traits for CList_adaptor_base
 *
 * \ingroup group__library__collections
 *
 * Regrettably, since MFC's template classes do not define any member types,
 * it is not possible to generalise the traits, so we must just use
 * specialisations. Sigh!
 */
template <ss_typename_param_k C>
struct CList_traits
{
    /// \brief Typedef that defines the type of the elements in the list
    typedef ????            value_type;
    /// \brief Typedef that defines the type of the arguments to the methods of the list
    typedef ????            arg_type;
    /// \brief Typedef that identifies the actual class type used to parameterise the traits
    typedef ????            list_type;
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k C>
struct CList_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct CList_traits<CObList>
{
    typedef CObject*        value_type;
    typedef CObject*        arg_type;
    typedef CObList         list_type;
};

// For CPtrList

STLSOFT_TEMPLATE_SPECIALISATION
struct CList_traits<CPtrList>
{
    typedef void*           value_type;
    typedef void*           arg_type;
    typedef CPtrList        list_type;
};

// For CStringList

STLSOFT_TEMPLATE_SPECIALISATION
struct CList_traits<CStringList>
{
    typedef CString         value_type;
# if 0
    typedef CString const&  arg_type;
# else /* ? 0 */
    typedef LPCTSTR         arg_type;
# endif /* 0 */
    typedef CStringList     list_type;
};

// For CList<, >

# if defined(__AFXTEMPL_H__) && \
     defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT)
/* If your translator supports partial template specialisation, then you
 * should be fine with the following specialisation, otherwise you will need
 * to provide your own traits class, e.g
 *
 *  struct my_traits_type
 *  {
 *    typedef MyValType       value_type;
 *    typedef MyValType const& arg_type;
 *  };
 */

template <class V, class A>
struct CList_traits<CList<V, A> >
{
    typedef V               value_type;
    typedef A               arg_type;
    typedef CList<V, A>     list_type;
};

# endif /* __AFXTEMPL_H__ && STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Traits type for specific setting of value_type and arg_type
 *
 * \ingroup group__library__collections
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k A
        ,   ss_typename_param_k C
        >
struct CList_specific_traits
{
    typedef V               value_type;
    typedef A               arg_type;
    typedef C               list_type;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/clist_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace mfcstl */
# else
} /* namespace mfcstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
