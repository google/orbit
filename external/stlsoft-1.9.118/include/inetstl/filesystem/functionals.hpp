/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/filesystem/functionals.hpp
 *
 * Purpose:     File-system functionals.
 *
 * Created:     19th January 2002
 * Updated:     29th November 2010
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


/** \file inetstl/filesystem/functionals.hpp
 *
 * \brief [C++ only] File-system functionals
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FUNCTIONALS
#define INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FUNCTIONALS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FUNCTIONALS_MAJOR       4
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FUNCTIONALS_MINOR       0
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FUNCTIONALS_REVISION    4
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FUNCTIONALS_EDIT        34
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef INETSTL_INCL_INETSTL_H_INETSTL
# include <inetstl/inetstl.h>
#endif /* !INETSTL_INCL_INETSTL_H_INETSTL */
#ifndef INETSTL_OS_IS_WINDOWS
# error This file is currently compatible only with the Win32/Win64 API
#endif /* !INETSTL_OS_IS_WINDOWS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <inetstl/filesystem/filesystem_traits.hpp>
#endif /* !INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_MINMAX
# include <stlsoft/util/minmax.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_MINMAX */

#ifndef _INETSTL_FUNCTIONALS_NO_STD
# include <functional>
#else /* ? _INETSTL_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _INETSTL_FUNCTIONALS_NO_STD */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::inetstl */
namespace inetstl
{
# else
/* Define stlsoft::inetstl_project */

namespace stlsoft
{

namespace inetstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief A function class that compares two file-system paths
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param A1 The left-hand argument type; defaults to C const*
 * \param A2 The right-hand argument type; defaults to C const*
 */
// [[synesis:class:function-class:binary-predicate: path_compare<T<C>, T<A1>, T<A2>>]]
template<   ss_typename_param_k C
        ,   ss_typename_param_k A1 = C const*
        ,   ss_typename_param_k A2 = C const*
        >
struct path_compare
    : public inetstl_ns_qual_std(binary_function)<A1, A2, is_bool_t>
{
public:
    /// The character type
    typedef C                                                           char_type;
private:
    typedef inetstl_ns_qual_std(binary_function)<A1, A2, is_bool_t>     parent_class_type;
public:
    /// The first argument type
    typedef ss_typename_type_k parent_class_type::first_argument_type   first_argument_type;
    /// The second argument type
    typedef ss_typename_type_k parent_class_type::second_argument_type  second_argument_type;
    /// The result type
    typedef ss_typename_type_k parent_class_type::result_type           result_type;
    /// The traits type
    typedef filesystem_traits<C>                                        traits_type;
    /// The current parameterisation of the type
    typedef path_compare<C, A1, A2>                                     class_type;

public:
    /// Function call, compares \c s1 with \c s2
    ///
    /// \note The comparison is determined by evaluation the full-paths of both \c s1 and \c s2
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template<ss_typename_param_k T1, ss_typename_param_k T2>
    result_type operator ()(T1 const& s1, T2 const& s2) const
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    result_type operator ()(first_argument_type s1, second_argument_type s2) const
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        return compare_(stlsoft_ns_qual(c_str_ptr)(s1), stlsoft_ns_qual(c_str_ptr)(s2));
    }

// Implementation
private:
    static result_type compare_(char_type const* s1, char_type const* s2)
    {
        char_type       path1[_MAX_PATH + 1];
        char_type       path2[_MAX_PATH + 1];
        is_size_t const len1    =   traits_type::str_len(s1);
        is_size_t const len2    =   traits_type::str_len(s2);

        traits_type::char_copy(&path1[0], s1, stlsoft_ns_qual(minimum)(STLSOFT_NUM_ELEMENTS(path1), len1));
        path1[len1] = '\0';
        traits_type::char_copy(&path2[0], s2, stlsoft_ns_qual(minimum)(STLSOFT_NUM_ELEMENTS(path2), len2));
        path2[len2] = '\0';

        traits_type::remove_dir_end(&path1[0]);
        traits_type::remove_dir_end(&path2[0]);

        s1 = &path1[0];
        s2 = &path2[0];

        return 0 == traits_type::str_compare(s1, s2);
    }
};

/** \brief Predicate that indicates whether a given path exists
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param A The argument type; defaults to C const*
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A = C const*
        >
// [[synesis:class:function-class:unary-predicate: path_exists<T<C>, T<A>>]]
struct path_exists
    : public inetstl_ns_qual_std(unary_function)<A, is_bool_t>
{
public:
    /// The character type
    typedef C                       char_type;
    /// The traits type
    typedef filesystem_traits<C>    traits_type;
    /// The current parameterisation of the type
    typedef path_exists<C>          class_type;

public:
    ss_explicit_k path_exists(HINTERNET hConnection)
        : m_hConnection(hConnection)
    {}

public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k S>
    is_bool_t operator ()(S const& s) const
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    is_bool_t operator ()(argument_type s) const
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        return exists_(stlsoft_ns_qual(c_str_ptr)(s));
    }

private:
    is_bool_t exists_(char_type const* s) const
    {
        char_type   sz0[1 + _MAX_PATH];
        is_dword_t  dw;

        if(!traits_type::get_full_path_name(m_hConnection, s, STLSOFT_NUM_ELEMENTS(sz0), sz0))
        {
            dw = 0xFFFFFFFF;
        }
        else
        {
            dw = ::GetFileAttributes(sz0);
        }

        return 0xFFFFFFFF != dw;
    }

private:
    HINTERNET   m_hConnection;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/functionals_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace inetstl
# else
} // namespace inetstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FUNCTIONALS */

/* ///////////////////////////// end of file //////////////////////////// */
