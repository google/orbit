/* /////////////////////////////////////////////////////////////////////////
 * File:        dotnetstl/string/string_accessor.hpp
 *
 * Purpose:     A useful tool for accessing a String object's content as a c-string.
 *
 * Created:     24th June 2003
 * Updated:     30th July 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2013, Matthew Wilson and Synesis Software
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


/** \file dotnetstl/string/string_accessor.hpp
 *
 * \brief [C++ only; requires .NET] Definition of the
 *   dotnetstl::c_string_accessor class template
 *   (\ref group__library__string "String" Library).
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_STRING_HPP_STRING_ACCESSOR
#define DOTNETSTL_INCL_DOTNETSTL_STRING_HPP_STRING_ACCESSOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
#define DOTNETSTL_VER_DOTNETSTL_STRING_HPP_STRING_ACCESSOR_MAJOR    4
#define DOTNETSTL_VER_DOTNETSTL_STRING_HPP_STRING_ACCESSOR_MINOR    0
#define DOTNETSTL_VER_DOTNETSTL_STRING_HPP_STRING_ACCESSOR_REVISION 3
#define DOTNETSTL_VER_DOTNETSTL_STRING_HPP_STRING_ACCESSOR_EDIT     48
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
# include <dotnetstl/dotnetstl.hpp>
#endif /* !DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */
#ifdef STLSOFT_UNITTEST
# include <string.h>
# include <wchar.h>
#endif /* STLSOFT_UNITTEST */

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
 * Classes
 */

/** \brief Template providing scoped C-string access to a String
 *
 * \ingroup group__library__string
 */
template <ss_typename_param_k C>
class c_string_accessor
{
public:
    typedef C                       char_type;
    typedef C*                      pointer;
    typedef C const*                const_pointer;
    typedef c_string_accessor<C>    class_type;
#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
    typedef System::String const^   string_pointer_const_type_;
    typedef System::String^         string_pointer_type_;
#else /* ? DOTNETSTL_HAT_SYNTAX_SUPPORT */
    typedef System::String const*   string_pointer_const_type_;
    typedef System::String*         string_pointer_type_;
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */

// Construction
public:
    /// Construct from a System::String
    ///
    /// \param s The String for which C-string access is to be provided. Must not be NULL
    ss_explicit_k c_string_accessor(string_pointer_const_type_ s)
        : m_h(get_cstring_(s))
    {}
    /// Release resources
    ~c_string_accessor() stlsoft_throw_0()
    {
        System::Runtime::InteropServices::Marshal::FreeHGlobal(m_h);
    }

    c_string_accessor(c_string_accessor& rhs)
        : m_h(rhs.m_h)
    {
        rhs.m_h = 0;
    }

// Accessors
public:
    /// Implicit conversion operator to a C-string
    operator const_pointer() const
    {
#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
        System::IntPtr h = m_h;

        return static_cast<const_pointer>(h.ToPointer());
#else /* ? DOTNETSTL_HAT_SYNTAX_SUPPORT */
        return m_h;
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */
    }

// Implementation
private:
    pointer get_cstring_(string_pointer_const_type_ s);

// Members
private:
#if defined(DOTNETSTL_HAT_SYNTAX_SUPPORT)
    System::IntPtr  m_h;
#else /* ? DOTNETSTL_HAT_SYNTAX_SUPPORT */
    pointer         m_h;
#endif /* DOTNETSTL_HAT_SYNTAX_SUPPORT */

// Not to be implemented
private:
    c_string_accessor& operator =(class_type const&);
};

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
inline c_string_accessor<ds_char_a_t>::pointer c_string_accessor<ds_char_a_t>::get_cstring_(string_pointer_const_type_ s)
{
    return sap_cast<ds_char_a_t*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(const_cast<string_pointer_type_>(s)).ToPointer());
}

STLSOFT_TEMPLATE_SPECIALISATION
inline c_string_accessor<ds_char_w_t>::pointer c_string_accessor<ds_char_w_t>::get_cstring_(string_pointer_const_type_ s)
{
    return sap_cast<ds_char_w_t*>(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(const_cast<string_pointer_type_>(s)).ToPointer());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/string_accessor_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef _STLSOFT_NO_NAMESPACE
} // namespace dotnetstl
#else
} // namespace dotnetstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* DOTNETSTL_INCL_DOTNETSTL_STRING_HPP_STRING_ACCESSOR */

/* ///////////////////////////// end of file //////////////////////////// */
