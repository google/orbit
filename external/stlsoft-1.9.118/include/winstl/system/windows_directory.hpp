/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/windows_directory.hpp
 *
 * Purpose:     Simple class that gets, and makes accessible, the windows
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


/** \file winstl/system/windows_directory.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_windows_directory class
 *  template
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY_MAJOR       4
# define WINSTL_VER_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY_MINOR       2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY_REVISION    1
# define WINSTL_VER_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY_EDIT        65
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SPECIAL_STRING_INSTANCE
# include <stlsoft/string/special_string_instance.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SPECIAL_STRING_INSTANCE */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX
# include <winstl/synch/spin_mutex.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS
# include <winstl/system/system_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS */

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
 *   policy template for eliciting the <b>Windows</b> directory.
 *
 * \ingroup group__library__system
 */
template <ss_typename_param_k C>
struct windir_policy
{
/// \name Member Types
/// @{
public:
    typedef C                           char_type;
    typedef processheap_allocator<C>    allocator_type;
    typedef ws_size_t                   size_type;
    typedef size_type                   (*pfn_type)(char_type *, size_type);
    typedef winstl::spin_mutex          spin_mutex_type;
    typedef winstl::atomic_int_t        atomic_int_type;
/// @}

/// \name Member Constants
/// @{
public:
    enum { internalBufferSize       =   32  };

    enum { allowImplicitConversion  =   1   };

    enum { sharedState              =   1   };
/// @}

/// \name Operations
/// @{
public:
    static pfn_type     get_fn()
    {
        return winstl::system_traits<char_type>::get_windows_directory;
    }
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>Windows</b> directory; ANSI specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< windir_policy<ws_char_a_t> >    windows_directory_a;
/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>Windows</b> directory; 'Unicode' specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< windir_policy<ws_char_w_t> >    windows_directory_w;
/** \brief A \ref group__pattern__special_string_instance "Special String Instance"
 *   that represents the <b>Windows</b> directory; TCHAR specialisation.
 *
 * \ingroup group__library__system
 */
typedef stlsoft_ns_qual(special_string_instance_0)< windir_policy<TCHAR> >          windows_directory;

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

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_WINDOWS_DIRECTORY */

/* ///////////////////////////// end of file //////////////////////////// */
