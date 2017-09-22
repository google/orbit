/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/logical/is_empty/std/container.hpp
 *
 * Purpose:     Generic adapted case of is_empty shim for standard(-like)
 *              collections.
 *
 * Created:     18th December 2006
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


/** \file stlsoft/shims/logical/is_empty/std/container.hpp
 *
 * \brief [C++] Primary include file for is_empty collection attribute shim
 *   for standard collection types
 *   (\ref group__concept__shim__collection_logical__is_empty "is_empty Collection Logical Shim").
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER
#define STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER_MAJOR     2
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER_MINOR     0
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER_REVISION  3
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER_EDIT      21
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES
# include <stlsoft/shims/logical/is_empty/util/features.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES */

#ifdef STLSOFT_UNITTEST
# include <deque>
# include <list>
# include <vector>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* !_STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifdef STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP

STLSOFT_OPEN_WORKER_NS_(is_empty_ns)

    // This overload of is_empty_helper_assistant() denotes the
    // assumption that, by default, a type is able to indicate its
    // emptiness by a non-mutating method empty().
    yes_type is_empty_helper_assistant(...)
    {
        return yes_type();
    }

    template <ss_typename_param_k C>
    inline ss_bool_t is_empty_helper(C const& c, yes_type)
    {
        return c.empty();
    }

STLSOFT_CLOSE_WORKER_NS_(is_empty_ns)

#endif /* STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP */

/** Indicates whether the collection is empty
 *
 * \ingroup group__concept__shim__collection_logical__is_empty
 *
 */
template <ss_typename_param_k C>
inline ss_bool_t is_empty(C const& c)
{
#ifdef STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP

    return STLSOFT_WORKER_NS_QUAL_(is_empty_ns, is_empty_helper)(c, STLSOFT_WORKER_NS_QUAL_(is_empty_ns, is_empty_helper_assistant)(&c, &c));

#else /* ? STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP */

    return c.empty();

#endif /* STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP */
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/container_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* !_STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_STD_HPP_CONTAINER */

/* ///////////////////////////// end of file //////////////////////////// */
