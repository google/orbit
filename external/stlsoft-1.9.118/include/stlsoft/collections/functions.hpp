/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/collections/functions.hpp
 *
 * Purpose:     Collection manipulation functions.
 *
 * Created:     11th November 2004
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


/** \file stlsoft/collections/functions.hpp
 *
 * \brief [C++ only] Definition of collection manipulation functions
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_FUNCTIONS
#define STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_FUNCTIONS_MAJOR    2
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_FUNCTIONS_MINOR    0
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_FUNCTIONS_REVISION 3
# define STLSOFT_VER_STLSOFT_COLLECTIONS_HPP_FUNCTIONS_EDIT     34
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifdef STLSOFT_UNITTEST
# include <map>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler / library compatibility
 */

#ifdef STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE
# undef STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE
#endif /* STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC)
# if STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION <= STLSOFT_CF_DINKUMWARE_VC_VERSION_6_0
#  define   STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE
# endif /* STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION */
#endif /* STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief Looks up and returns the matching referent for a given key, or throws
 * std::out_of_range if the key is not found in the map
 *
 * \ingroup group__library__collections
 *
 * \param m The map to be searched
 * \param key The key to be searched for
 *
 * \return The matching referent
 *
 * \note If \c key does not refer to an entry in the map \c m, then
 * std::out_of_range is thrown
 */
#if (   !defined(STLSOFT_COMPILER_IS_INTEL) && \
        !defined(STLSOFT_COMPILER_IS_MSVC)) || \
    _MSC_VER >= 1310
template<   ss_typename_param_k M
        ,   ss_typename_param_k K
        >
# ifdef STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE
inline ss_typename_type_ret_k M::referent_type &lookup(M &m, K const& key)
# else /* ? STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE */
inline ss_typename_type_ret_k M::mapped_type &lookup(M &m, K const& key)
# endif /* STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE */
{
    ss_typename_type_k M::iterator it = m.find(key);

    if(m.end() == it)
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("invalid key"));
    }

    return (*it).second;
}
#endif /* compiler */

/** \brief Looks up and returns the matching referent for a given key, or throws
 * std::out_of_range if the key is not found in the map
 *
 * \ingroup group__library__collections
 *
 * \param m The map to be searched
 * \param key The key to be searched for
 *
 * \return The matching referent
 *
 * \note If \c key does not refer to an entry in the map \c m, then
 * std::out_of_range is thrown
 */
template<   ss_typename_param_k M
        ,   ss_typename_param_k K
        >
#ifdef STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE
inline ss_typename_type_ret_k M::referent_type const& lookup(M const& m, K const& key)
#else /* ? STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE */
inline ss_typename_type_ret_k M::mapped_type const& lookup(M const& m, K const& key)
#endif /* STLSOFT_CONTAINER_ACCESS_MAPPED_TYPE_IS_REFERENT_TYPE */
{
    ss_typename_type_k M::const_iterator it = m.find(key);

    if(m.end() == it)
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("invalid key"));
    }

    return (*it).second;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_HPP_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
