/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/string/string_traits.hpp
 *
 * Purpose:     Contains string_traits for ATL.
 *
 * Created:     28th April 2005
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


/** \file atlstl/string/string_traits.hpp
 *
 * \brief [C++] Specialisation of the stlsoft::string_traits traits class
 *   for ATL types
 *   (\ref group__library__string "String" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_STRING_HPP_STRING_TRAITS
#define ATLSTL_INCL_ATLSTL_STRING_HPP_STRING_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_STRING_HPP_STRING_TRAITS_MAJOR      3
# define ATLSTL_VER_ATLSTL_STRING_HPP_STRING_TRAITS_MINOR      0
# define ATLSTL_VER_ATLSTL_STRING_HPP_STRING_TRAITS_REVISION   2
# define ATLSTL_VER_ATLSTL_STRING_HPP_STRING_TRAITS_EDIT       20
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
# include <stlsoft/string/string_traits_fwd.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */

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

/* ATL's CComBSTR */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<CComBSTR>
{
    typedef CComBSTR                                    value_type;
    typedef WCHAR                                       char_type;
    typedef ss_size_t                                   size_type;
    typedef char_type const                             const_char_type;
    typedef value_type                                  string_type;
    typedef LPOLESTR                                    pointer;
    typedef LPCOLESTR                                   const_pointer;
    typedef pointer                                     iterator;
    typedef const_pointer                               const_iterator;
    enum
    {
            is_pointer          =   false
        ,   is_pointer_to_const =   false
        ,   char_type_size      =   sizeof(char_type)
    };

    static string_type empty_string()
    {
        return string_type();
    }

    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(static_cast<int>(len), static_cast<const_pointer>(src) + pos);
    }
    static string_type &assign_inplace(string_type &str, const_iterator first, const_iterator last)
    {
        return (str = string_type(static_cast<int>(last - first), first), str);
    }

    static iterator begin(string_type &str)
    {
        return &(str[0]);
    }
    static iterator end(string_type &str)
    {
        return begin(str) + str.Length();
    }
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/string_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ATLSTL_INCL_ATLSTL_STRING_HPP_STRING_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
