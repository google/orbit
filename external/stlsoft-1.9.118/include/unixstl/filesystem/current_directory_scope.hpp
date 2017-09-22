/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/filesystem/current_directory_scope.hpp
 *
 * Purpose:     Current working directory scoping class.
 *
 * Created:     12th November 1998
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file unixstl/filesystem/current_directory_scope.hpp
 *
 * \brief [C++ only] Definition of the unixstl::basic_current_directory_scope
 *  class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE
#define UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE_MAJOR       5
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE_MINOR       1
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE_REVISION    4
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE_EDIT        115
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <unixstl/filesystem/filesystem_traits.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
# include <unixstl/filesystem/file_path_buffer.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING
# include <unixstl/shims/access/string.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL
# include <stlsoft/util/operator_bool.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * basic_current_directory_scope
 *
 * This class pushes the given directory as the current directory upon
 * construction, and pops back to the original at destruction.
 */

/** \brief Current directory scoping class
 *
 * \ingroup group__library__filesystem
 *
 * This class scopes the process's current directory, by changing to the path
 * given in the constructor, and then, if that succeeded, changing back in the
 * destructor.
 *
 * \param C The character type (e.g. \c char, \c wchar_t).
 * \param T The file-system traits. In translators that support default template parameters that defaults to \c filesystem_traits<C>.
 */

template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = filesystem_traits<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_current_directory_scope
{
public:
    typedef C                                       char_type;  /*!< The character type */
private:
    typedef T                                       traits_type;
    typedef basic_current_directory_scope<C, T>     class_type;

// Construction
public:
    /// \brief Constructs a scope instance and changes to the given directory
    ///
    /// \param dir The name of the directory to change the current directory to
    ss_explicit_k basic_current_directory_scope(char_type const* dir);
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    /// \brief Constructs a scope instance and changes to the given directory
    ///
    /// \param dir The name of the directory to change the current directory to
    template <ss_typename_param_k S>
    ss_explicit_k basic_current_directory_scope(S const& dir)
    {
        init_(stlsoft_ns_qual(c_str_ptr)(dir));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    /// \brief Returns the current directory to its original location
    ~basic_current_directory_scope() stlsoft_throw_0();

// Attributes
public:
    /// \brief Returns a C-string pointer to the original directory
    char_type const* get_previous() const;

// Conversions
public:
    /// \brief Returns a C-string pointer to the original directory
    operator char_type const* () const;

/// \name State
/// @{
private:
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(class_type, operator_bool_generator_type, operator_bool_type);
public:
    /// \brief Indicates whether the construction was successful
    ///
    /// \retval true The scope instance was successfully constructed and the current directory changed as per the constructor argument
    /// \retval false The scope instance was not successfully constructed, and the current directory was unchanged.
    operator operator_bool_type() const
    {
        return operator_bool_generator_type::translate('\0' != m_previous[0]);
    }

/// @}

// Implementation
private:
    void init_(char_type const* dir);

// Members
private:
    basic_file_path_buffer<char_type>   m_previous;

// Not to be implemented
private:
    basic_current_directory_scope();
    basic_current_directory_scope(class_type const&);
    class_type const& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_current_directory_scope template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_current_directory_scope<us_char_a_t, filesystem_traits<us_char_a_t> >     current_directory_scope_a;
/** \brief Specialisation of the basic_current_directory_scope template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_current_directory_scope<us_char_w_t, filesystem_traits<us_char_w_t> >     current_directory_scope_w;
/** \brief Specialisation of the basic_current_directory_scope template for the ambient UNIX character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_current_directory_scope<char, filesystem_traits<char> >                   current_directory_scope;

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline us_char_a_t const* c_str_ptr_null_a(unixstl_ns_qual(basic_current_directory_scope)<us_char_a_t, C> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_a)(b.c_str());
}
template <ss_typename_param_k C>
inline us_char_w_t const* c_str_ptr_null_w(unixstl_ns_qual(basic_current_directory_scope)<us_char_w_t, C> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for unixstl::basic_current_directory_scope
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline C const* c_str_ptr_null(basic_current_directory_scope<C, T> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.get_previous());
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline us_char_a_t const* c_str_ptr_a(unixstl_ns_qual(basic_current_directory_scope)<us_char_a_t, C> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k C>
inline us_char_w_t const* c_str_ptr_w(unixstl_ns_qual(basic_current_directory_scope)<us_char_w_t, C> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for unixstl::basic_current_directory_scope
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline C const* c_str_ptr(basic_current_directory_scope<C, T> const& b)
{
    return b.get_previous();
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline us_char_a_t const* c_str_data_a(unixstl_ns_qual(basic_current_directory_scope)<us_char_a_t, C> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k C>
inline us_char_w_t const* c_str_data_w(unixstl_ns_qual(basic_current_directory_scope)<us_char_w_t, C> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for unixstl::basic_current_directory_scope
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline C const* c_str_data(basic_current_directory_scope<C, T> const& b)
{
    return b.get_previous();
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline us_size_t c_str_len_a(unixstl_ns_qual(basic_current_directory_scope)<us_char_a_t, C> const& b)
{
    return stlsoft_ns_qual(c_str_len_a)(b.c_str());
}
template <ss_typename_param_k C>
inline us_size_t c_str_len_w(unixstl_ns_qual(basic_current_directory_scope)<us_char_w_t, C> const& b)
{
    return stlsoft_ns_qual(c_str_len_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for unixstl::basic_current_directory_scope
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline us_size_t c_str_len(basic_current_directory_scope<C, T> const& b)
{
    return stlsoft_ns_qual(c_str_len)(b.get_previous());
}




template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline S& operator <<(S& s, basic_current_directory_scope<C, T> const& b)
{
    s << b.get_previous();

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/current_directory_scope_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline void basic_current_directory_scope<C, T>::init_(ss_typename_type_k basic_current_directory_scope<C, T>::char_type const* dir)
{
    if( 0 == traits_type::get_current_directory(m_previous.size(), &m_previous[0]) ||
        !traits_type::set_current_directory(dir))
    {
        m_previous[0] = '\0';
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline basic_current_directory_scope<C, T>::basic_current_directory_scope(ss_typename_type_k basic_current_directory_scope<C, T>::char_type const* dir)
{
    init_(dir);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline basic_current_directory_scope<C, T>::~basic_current_directory_scope() stlsoft_throw_0()
{
    if('\0' != m_previous[0])
    {
        traits_type::set_current_directory(m_previous.data());
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_current_directory_scope<C, T>::char_type const* basic_current_directory_scope<C, T>::get_previous() const
{
#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 3
    return m_previous.c_str();
#else /* ? __GNUC__ < 3 */
    return stlsoft_ns_qual(c_str_ptr)(m_previous);
#endif /* ? __GNUC__ < 3 */
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
#if defined(STLSOFT_COMPILER_IS_GCC)
inline basic_current_directory_scope<C, T>::operator C const* () const
#else /* ? compiler */
inline basic_current_directory_scope<C, T>::operator ss_typename_type_k basic_current_directory_scope<C, T>::char_type const* () const
#endif /* compiler */
{
    return get_previous();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace stlsoft::unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::unixstl::c_str_data;
using ::unixstl::c_str_data_a;
using ::unixstl::c_str_data_w;

using ::unixstl::c_str_len;
using ::unixstl::c_str_len_a;
using ::unixstl::c_str_len_w;

using ::unixstl::c_str_ptr;
using ::unixstl::c_str_ptr_a;
using ::unixstl::c_str_ptr_w;

using ::unixstl::c_str_ptr_null;
using ::unixstl::c_str_ptr_null_a;
using ::unixstl::c_str_ptr_null_w;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
