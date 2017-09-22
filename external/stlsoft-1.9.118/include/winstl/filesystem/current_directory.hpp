/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/current_directory.hpp
 *
 * Purpose:     Simple class that gets, and makes accessible, the current
 *              directory.
 *
 * Created:     10th December 2002
 * Updated:     12th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/filesystem/current_directory.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_current_directory class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_MAJOR       4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_MINOR       2
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_REVISION    1
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_EDIT        68
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

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SPECIAL_STRING_INSTANCE
# include <stlsoft/string/special_string_instance.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SPECIAL_STRING_INSTANCE */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <winstl/filesystem/filesystem_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
  // Borland is a bit of a thicky, and requires a (valid) spin_mutex_type
# include <winstl/synch/spin_mutex.hpp>
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief \ref group__pattern__special_string_instance "Special String Instance"
 *   policy template for eliciting the <b>current</b> directory.
 *
 * \ingroup group__library__system
 */
template <ss_typename_param_k C>
struct cwd_policy
{
    typedef C                           char_type;
    typedef processheap_allocator<C>    allocator_type;
    typedef ss_size_t                   size_type;
    typedef size_type                   (*pfn_type)(char_type *, size_type);
#if defined(STLSOFT_COMPILER_IS_BORLAND)
    // Borland is a bit of a thicky, and requires a (valid) spin_mutex_type
    typedef winstl::spin_mutex          spin_mutex_type;
    typedef winstl::atomic_int_t        atomic_int_type;
#endif /* compiler */

    enum { internalBufferSize       =   128 };

    enum { allowImplicitConversion  =   1   };

    enum { sharedState              =   0   };

    static pfn_type     get_fn()
    {
        return winstl::filesystem_traits<char_type>::get_current_directory;
    }
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>current</b> directory; ANSI specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< cwd_policy<ws_char_a_t> >   current_directory_a;
/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>current</b> directory; 'Unicode' specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< cwd_policy<ws_char_w_t> >   current_directory_w;
/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>current</b> directory; TCHAR specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< cwd_policy<TCHAR> >         current_directory;

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/current_directory_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY */

/* ///////////////////////////// end of file //////////////////////////// */
