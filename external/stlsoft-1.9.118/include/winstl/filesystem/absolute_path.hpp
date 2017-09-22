/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/absolute_path.hpp
 *
 * Purpose:     Simple class that converts a relative path to an absolute one.
 *
 * Created:     20th December 2002
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


/** \file winstl/filesystem/absolute_path.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_absolute_path class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH_MAJOR       4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH_MINOR       3
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH_REVISION    1
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH_EDIT        68
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
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <winstl/filesystem/filesystem_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */

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
 *   policy template for eliciting the <b>absolute form</b> of a given path.
 *
 * \ingroup group__library__system
 */
template <ss_typename_param_k C>
struct abspath_policy
{
/// \name Member Types
/// @{
public:
    typedef C                           char_type;
    typedef char_type const*            argument_0_type;
    typedef processheap_allocator<C>    allocator_type;
    typedef ws_size_t                   size_type;
    typedef size_type                   (*pfn_type)(argument_0_type, char_type *, size_type);
#if defined(STLSOFT_COMPILER_IS_BORLAND)
    // Borland is a bit of a thicky, and requires a (valid) spin_mutex_type
    typedef winstl::spin_mutex          spin_mutex_type;
    typedef winstl::atomic_int_t        atomic_int_type;
#endif /* compiler */
/// @}

/// \name Member Constants
/// @{
public:
    enum { internalBufferSize       =   128 };

    enum { allowImplicitConversion  =   1   };

    enum { sharedState              =   0   };
/// @}

/// \name Operations
/// @{
public:
    static pfn_type     get_fn()
    {
        return winstl::filesystem_traits<char_type>::get_full_path_name;
    }
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * basic_absolute_path
 *
 * This class converts a relative path to an absolute one, and effectively acts
 * as a C-string of its value.
 */

/** \brief Converts a relative path to an absolute path
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param T The traits type. On translators that support default template
 *  arguments, this defaults to filesystem_traits<C>
 */
template <ss_typename_param_k C>
class basic_absolute_path
    : public stlsoft_ns_qual(special_string_instance_1)<abspath_policy<C> >
{
/// \name Member Types
/// @{
private:
    typedef stlsoft_ns_qual(special_string_instance_1)<abspath_policy<C> >  parent_class_type;
public:
    /// \brief The char type
    typedef ss_typename_type_k parent_class_type::char_type     char_type;
    /// \brief The current parameterisation of the type
    typedef basic_absolute_path<C>                              class_type;
    /// \brief The size type
    typedef ss_typename_type_k parent_class_type::size_type     size_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an absolute path from \c path
    ss_explicit_k basic_absolute_path(char_type const* path)
        : parent_class_type(path)
    {}
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// \brief Constructs an absolute path from \c path
    template<ss_typename_param_k S>
    ss_explicit_k basic_absolute_path(S const& path)
        : parent_class_type(stlsoft_ns_qual(c_str_ptr)(path))
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
/// @}

/** \brief @{
 *
 * \ingroup group__library__filesystem
 */
private:
    basic_absolute_path(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_absolute_path template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_absolute_path<ws_char_a_t>    absolute_path_a;
/** \brief Specialisation of the basic_absolute_path template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_absolute_path<ws_char_w_t>    absolute_path_w;
/** \brief Specialisation of the basic_absolute_path template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_absolute_path<TCHAR>          absolute_path;

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER >= 1100

/** \brief This \ref group__pattern__creator_function "creator function"
 *   makes an absolute path variable without needing to qualify the template
 *   parameter.
 */
template<ss_typename_param_k C>
inline basic_absolute_path<C> make_absolute_path(C const* path)
{
    return basic_absolute_path<C>(path);
}

#endif /* compiler */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/absolute_path_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_ABSOLUTE_PATH */

/* ///////////////////////////// end of file //////////////////////////// */
