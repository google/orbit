/* /////////////////////////////////////////////////////////////////////////
 * File:        dotnetstl/shims/access/string/string.hpp
 *
 * Purpose:     String access shims for .net.
 *
 * Created:     24th June 2003
 * Updated:     10th August 2009
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


/** \file dotnetstl/shims/access/string/string.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>System::String</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING
#define DOTNETSTL_INCL_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_VER_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING_MAJOR       2
# define DOTNETSTL_VER_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING_MINOR       0
# define DOTNETSTL_VER_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING_REVISION    4
# define DOTNETSTL_VER_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING_EDIT        20
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
# include <dotnetstl/dotnetstl.hpp>
#endif /* !DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL */
#ifndef DOTNETSTL_INCL_DOTNETSTL_STRING_HPP_STRING_ACCESSOR
# include <dotnetstl/string/string_accessor.hpp>
#endif /* !DOTNETSTL_INCL_DOTNETSTL_STRING_HPP_STRING_ACCESSOR */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef _STLSOFT_NO_NAMESPACE
/* There is no stlsoft namespace, so must define ::dotnetstl */
namespace dotnetstl
{
#else
/* Define stlsoft::dotnet_project */

namespace stlsoft
{

namespace dotnetstl_project
{

#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

namespace impl
{
    typedef c_string_accessor<char>     accessor_a_t;
    typedef c_string_accessor<wchar_t>  accessor_w_t;
#ifdef UNICODE
    typedef accessor_w_t                accessor_t_t;
#else /* ? UNICODE */
    typedef accessor_a_t                accessor_t_t;
#endif /* UNICODE */

#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
    typedef System::String const        ^string_pointer_const_type;
    typedef System::String              ^string_pointer_type;
#else /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
    typedef System::String const*       string_pointer_const_type;
    typedef System::String*             string_pointer_type;
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */

} // namespace impl

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

// c_str_data

inline impl::accessor_a_t c_str_data_a(impl::string_pointer_const_type s)
{
    return impl::accessor_a_t(s);
}

inline impl::accessor_w_t c_str_data_w(impl::string_pointer_const_type s)
{
    return impl::accessor_w_t(s);
}

inline impl::accessor_t_t c_str_data(impl::string_pointer_const_type s)
{
    return impl::accessor_t_t(s);
}



inline ds_size_t c_str_len_a(impl::string_pointer_const_type s)
{
    return ::strlen(c_str_data_a(s));
}

inline ds_size_t c_str_len_w(impl::string_pointer_const_type s)
{
#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
    if(nullptr == s)
#else /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
    if(NULL == s)
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
    {
        return 0;
    }
    else
    {
        impl::string_pointer_type   s_ =   const_cast<impl::string_pointer_type>(s);

#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
        return static_cast<ds_size_t>(s_->Length);
#else /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
        return static_cast<ds_size_t>(s_->get_Length());
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
    }
}

inline ds_size_t c_str_len(impl::string_pointer_const_type s)
{
    return c_str_len_w(s);
}


inline impl::accessor_a_t c_str_ptr_a(impl::string_pointer_const_type s)
{
    return impl::accessor_a_t(s);
}

inline impl::accessor_w_t c_str_ptr_w(impl::string_pointer_const_type s)
{
    return impl::accessor_w_t(s);
}

inline impl::accessor_t_t c_str_ptr(impl::string_pointer_const_type s)
{
    return impl::accessor_t_t(s);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/string_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef _STLSOFT_NO_NAMESPACE
} // namespace dotnetstl
#else
} // namespace dotnetstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 */

#ifndef _DOTNETSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::dotnetstl::c_str_data;
using ::dotnetstl::c_str_data_a;
using ::dotnetstl::c_str_data_w;

using ::dotnetstl::c_str_len;
using ::dotnetstl::c_str_len_a;
using ::dotnetstl::c_str_len_w;

using ::dotnetstl::c_str_ptr;
using ::dotnetstl::c_str_ptr_a;
using ::dotnetstl::c_str_ptr_w;

#if 0
using ::dotnetstl::c_str_ptr_null;
using ::dotnetstl::c_str_ptr_null_a;
using ::dotnetstl::c_str_ptr_null_w;
#endif /* 0 */

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_DOTNETSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* DOTNETSTL_INCL_DOTNETSTL_SHIMS_ACCESS_STRING_HPP_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
