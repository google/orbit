/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/functionals.hpp
 *
 * Purpose:     File-system related functions and predicates.
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


/** \file winstl/filesystem/functionals.hpp
 *
 * \brief [C++ only] Definition of the winstl::path_compare,
 *  winstl::path_compare_env, winstl::path_exists, winstl::path_exists_env
 *  and winstl::path_contains_file function classes
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FUNCTIONALS
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FUNCTIONALS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FUNCTIONALS_MAJOR     4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FUNCTIONALS_MINOR     1
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FUNCTIONALS_REVISION  4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FUNCTIONALS_EDIT      82
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <winstl/filesystem/filesystem_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
# include <winstl/filesystem/file_path_buffer.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef _WINSTL_FUNCTIONALS_NO_STD
# include <functional>
#else /* ? _WINSTL_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _WINSTL_FUNCTIONALS_NO_STD */

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
 * Utility functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline bool file_exists_envx_(C const* s, ws_bool_t bExpandEnvironmentStrings)
{
    typedef filesystem_traits<C>        traits_t;
    typedef basic_file_path_buffer<C>   buffer_t;

    buffer_t    nvx;
    buffer_t    full;

    if( bExpandEnvironmentStrings &&
        NULL == traits_t::str_chr(s, '%'))
    {
        bExpandEnvironmentStrings = false;
    }

    if(bExpandEnvironmentStrings)
    {
        if(!traits_t::expand_environment_strings(s, &nvx[0], nvx.size()))
        {
            return false;
        }
        else
        {
            s = nvx.c_str();
        }
    }

    if(!traits_t::get_full_path_name(s, full.size(), &full[0]))
    {
        return false;
    }
    else
    {
        return traits_t::file_exists(&full[0]);
    }
}

template <ss_typename_param_k C>
inline bool are_paths_equal_envx_(C const* s1, C const* s2, ws_bool_t bExpandEnvironmentStrings)
{
    typedef filesystem_traits<C>        traits_t;
    typedef basic_file_path_buffer<C>   buffer_t;

    buffer_t    full1;
    buffer_t    full2;
    buffer_t    nvx1;
    buffer_t    nvx2;
    C           *dummy;

    if( bExpandEnvironmentStrings &&
        NULL == traits_t::str_chr(s1, '%') &&
        NULL == traits_t::str_chr(s2, '%'))
    {
        bExpandEnvironmentStrings = false;
    }

    if(bExpandEnvironmentStrings)
    {
        if( !traits_t::expand_environment_strings(s1, &nvx1[0], nvx1.size()) ||
            !traits_t::expand_environment_strings(s2, &nvx2[0], nvx2.size()))
        {
            return false;
        }
        else
        {
            s1 = nvx1.c_str();
            s2 = nvx2.c_str();
        }
    }

    if( !traits_t::get_full_path_name(s1, full1.size(), &full1[0], &dummy) ||
        !traits_t::get_full_path_name(s2, full2.size(), &full2[0], &dummy))
    {
        return false;
    }
    else
    {
        traits_t::remove_dir_end(&full1[0]);
        traits_t::remove_dir_end(&full2[0]);

        s1 = full1.c_str();
        s2 = full2.c_str();
    }

    return 0 == traits_t::str_compare_no_case(s1, s2);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Binary predicate that compares two file-system paths.
 *
 * \ingroup group__library__filesystem
 *
 * \note Does not expand environment variables in the argument passed to
 * the function call operator
 *
 * \param C The character type
 * \param A1 The left-hand argument type; defaults to C const*
 * \param A2 The right-hand argument type; defaults to C const*
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A1 = C const*
        ,   ss_typename_param_k A2 = C const*
        >
// [[synesis:class:function-class:binary-predicate: path_compare<T<C>, T<A1>, T<A2>>]]
struct path_compare
    : public winstl_ns_qual_std(binary_function)<A1, A2, ws_bool_t>
{
/// \name Member Types
/// @{
private:
    typedef winstl_ns_qual_std(binary_function)<A1, A2, ws_bool_t>      parent_class_type;
public:
    /// The character type
    typedef C                                                           char_type;
    /// The first argument type
    typedef ss_typename_type_k parent_class_type::first_argument_type   first_argument_type;
    /// The second argument type
    typedef ss_typename_type_k parent_class_type::second_argument_type  second_argument_type;
    /// The result type
    typedef ss_typename_type_k parent_class_type::result_type           result_type;
    /// The current parameterisation of the type
    typedef path_compare<C, A1, A2>                                     class_type;
/// @}

/// \name Operations
/// @{
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
        return are_paths_equal_envx_(stlsoft_ns_qual(c_str_ptr)(s1), stlsoft_ns_qual(c_str_ptr)(s2), false);
    }
/// @}
};

/** \brief Binary predicate object that compares two file-system paths,
 * after expanding environment variables in the compared path strings.
 *
 * \ingroup group__library__filesystem
 *
 * \note Does not expand environment variables in the argument passed to
 * the function call operator
 *
 * \param C The character type
 * \param A1 The left-hand argument type; defaults to C const*
 * \param A2 The right-hand argument type; defaults to C const*
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A1 = C const*
        ,   ss_typename_param_k A2 = C const*
        >
// [[synesis:class:function-class:binary-predicate: path_compare_env<T<C>, T<A1>, T<A2>>]]
struct path_compare_env
    : public winstl_ns_qual_std(binary_function)<A1, A2, ws_bool_t>
{
/// \name Member Types
/// @{
private:
    typedef winstl_ns_qual_std(binary_function)<A1, A2, ws_bool_t>      parent_class_type;
public:
    /// The character type
    typedef C                                                           char_type;
    /// The first argument type
    typedef ss_typename_type_k parent_class_type::first_argument_type   first_argument_type;
    /// The second argument type
    typedef ss_typename_type_k parent_class_type::second_argument_type  second_argument_type;
    /// The result type
    typedef ss_typename_type_k parent_class_type::result_type           result_type;
    /// The current parameterisation of the type
    typedef path_compare_env<C, A1, A2>                                 class_type;
/// @}

/// \name Operations
/// @{
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
        return are_paths_equal_envx_(stlsoft_ns_qual(c_str_ptr)(s1), stlsoft_ns_qual(c_str_ptr)(s2), true);
    }
/// @}
};

/** \brief Unary predicate that indicates whether a given path exists.
 *
 * \ingroup group__library__filesystem
 *
 * \note Does not expand environment variables in the argument passed to
 * the function call operator
 *
 * \param C The character type
 * \param A The argument type; defaults to C const*
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A = C const*
        >
// [[synesis:class:function-class:unary-predicate: path_exists<T<C>, T<A>>]]
struct path_exists
    : public winstl_ns_qual_std(unary_function)<A, ws_bool_t>
{
/// \name Member Types
/// @{
private:
    typedef winstl_ns_qual_std(unary_function)<A, ws_bool_t>    parent_class_type;
public:
    /// The character type
    typedef C                                                   char_type;
    /// The argument type
    typedef A                                                   argument_type;
    /// The current parameterisation of the type
    typedef path_exists<C, A>                                   class_type;
/// @}

/// \name Operations
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k S>
    ws_bool_t operator ()(S const& s) const
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    ws_bool_t operator ()(argument_type s) const
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        return file_exists_envx_(stlsoft_ns_qual(c_str_ptr)(s), false);
    }
/// @}
};

/** \brief Unary predicate that indicates whether a given path exists, after
 * expanding environment variables in the path string.
 *
 * \ingroup group__library__filesystem
 *
 * \note Expands environment variables in the argument passed to
 * the function call operator
 *
 * \param C The character type
 * \param A The argument type; defaults to C const*
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A = C const*
        >
// [[synesis:class:function-class:unary-predicate: path_exists_env<T<C>, T<A>>]]
struct path_exists_env
    : public winstl_ns_qual_std(unary_function)<A, ws_bool_t>
{
/// \name Member Types
/// @{
private:
    typedef winstl_ns_qual_std(unary_function)<A, ws_bool_t>    parent_class_type;
public:
    /// The character type
    typedef C                                                   char_type;
    /// The argument type
    typedef A                                                   argument_type;
    /// The current parameterisation of the type
    typedef path_exists_env<C, A>                               class_type;
/// @}

/// \name Operations
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k S>
    ws_bool_t operator ()(S const& s) const
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    ws_bool_t operator ()(argument_type s) const
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        return file_exists_envx_(stlsoft_ns_qual(c_str_ptr)(s), true);
    }
/// @}
};


/** \brief Unary predicate that searches for the existance of a given file
 *  in the directory presented in its function call argument.
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param A The argument type; defaults to C const*
 */

/** \brief \note The file-name passed to the constructor is retained as a
 *    pointer, rather than an instance of a string class. Consequently, the
 *    behaviour is undefined if the memory pointed to by the constructor
 *    argument does not persist for the lifetime of the function object.
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A = C const*
        >
// [[synesis:class:function-class:unary-predicate: path_contains_file<T<C>, T<A>>]]
struct path_contains_file
    : public winstl_ns_qual_std(unary_function)<A, ws_bool_t>
{
/// \name Member Types
/// @{
private:
    typedef winstl_ns_qual_std(unary_function)<A, ws_bool_t>    parent_class_type;
public:
    /// The character type
    typedef C                                                   char_type;
    /// The argument type
    typedef A                                                   argument_type;
    /// The current parameterisation of the type
    typedef path_contains_file<C, A>                            class_type;
/// @}

/// \name Construction
/// @{
public:
    path_contains_file(char_type const* file)
        : m_file(file)
    {}
/// @}

/// \name Operations
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k S>
    ws_bool_t operator ()(S const& s) const
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    ws_bool_t operator ()(argument_type s) const
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        return path_contains_file_(stlsoft_ns_qual(c_str_ptr)(s), m_file);
    }
/// @}

/// \name Implementation
/// @{
private:
    static ws_bool_t path_contains_file_(char_type const* directory, char_type const* file)
    {
        typedef filesystem_traits<char_type>            traits_t;
        typedef stlsoft_ns_qual(auto_buffer)<char_type> buffer_t;

        const ws_size_t cchDirectory    =   traits_t::str_len(directory);
        const ws_size_t cchFile         =   traits_t::str_len(file);
        buffer_t        path(1 + cchDirectory + 1 + cchFile + 1);

        traits_t::char_copy(&path[0], directory, cchDirectory);
        path[cchDirectory] = '\0';
        traits_t::ensure_dir_end(&path[0]);
        traits_t::str_cat(&path[0], file);

        return traits_t::file_exists(path.data());
    }
/// @}

/// \name Members
/// @{
private:
    char_type const* m_file;
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/functionals_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FUNCTIONALS */

/* ///////////////////////////// end of file //////////////////////////// */
