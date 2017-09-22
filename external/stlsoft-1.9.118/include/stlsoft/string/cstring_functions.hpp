/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/cstring_functions.hpp
 *
 * Purpose:     String duplication functions.
 *
 * Created:     26th May 2005
 * Updated:     26th September 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/string/cstring_functions.hpp
 *
 * \brief [C++ only] C-string functions
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS
#define STLSOFT_INCL_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS_MAJOR    2
# define STLSOFT_VER_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS_MINOR    2
# define STLSOFT_VER_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS_REVISION 2
# define STLSOFT_VER_STLSOFT_STRING_HPP_CSTRING_FUNCTIONS_EDIT     32
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
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>   // for STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */

#ifdef STLSOFT_UNITTEST
# include <stlsoft/memory/malloc_allocator.hpp>
# include <stlsoft/memory/new_allocator.hpp>
# include <string.h>
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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Allocates a copy of the string, using the given allocator
 *
 * \ingroup group__library__string
 *
 * \param str The string to copy
 * \param cch The number of elements in str to copy
 * \param ator The allocator to use to allocate the memory
 *
 * \note The caller is responsible for the allocated memory, and should free it
 * with ator (or a compatible allocator or memory function).
 *
 * \ingroup group__library__string
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C* string_dup_impl(C const* str, ss_size_t cch, A& ator)
{
    C* r = ator.allocate(1 + cch, NULL);

#ifndef STLSOFT_CF_EXCEPTION_OPERATOR_NEW_THROWS_BAD_ALLOC
    if(NULL != r)
#endif /* !STLSOFT_CF_EXCEPTION_OPERATOR_NEW_THROWS_BAD_ALLOC */
    {
        char_traits<C>::copy(r, str, cch);
        r[cch] = '\0';
    }

    return r;
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Allocates a copy of the string, using the given allocator
 *
 * \ingroup group__library__string
 *
 * \param str The string to copy
 * \param cch The number of elements in str to copy
 * \param ator The allocator to use to allocate the memory
 *
 * \note The caller is responsible for the allocated memory, and should free it
 * with ator (or a compatible allocator or memory function).
 *
 * \ingroup group__library__string
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C* string_dup(C const* str, ss_size_t cch, A& ator)
{
#ifdef STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
    typedef C                                                           char_t;
    typedef ss_typename_type_k A::ss_template_qual_k rebind<C>::other   ator_t;

    ator_t ator_;

    STLSOFT_SUPPRESS_UNUSED(ator);

    return string_dup_impl(str, cch, ator_);
#else /* ? STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */
    return string_dup_impl(str, cch, ator);
#endif /* STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */
}

/** \brief Allocates a copy of the string, using the given allocator
 *
 * \ingroup group__library__string
 *
 * \param str The string to copy
 * \param ator The allocator to use to allocate the memory
 * \param psize Pointer to receive the size of the allocated string. May be NULL.
 *
 * \note The caller is responsible for the allocated memory, and should free it
 * with ator (or a compatible allocator or memory function).
 *
 * \ingroup group__library__string
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C* string_dup(C const* str, A& ator, ss_size_t* psize = NULL)
{
    ss_size_t dummy;

    if(NULL == psize)
    {
        psize = &dummy;
    }

    return string_dup(str, *psize = c_str_len(str), ator);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstring_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_HPP_CSTRING_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
