/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/filesystem/ftpdir_sequence.hpp
 *
 * Purpose:     Contains the basic_ftpdir_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     18th January 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file inetstl/filesystem/ftpdir_sequence.hpp
 *
 * \brief [C++ only] Definition of the inetstl::ftpdir_sequence
 *   class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE
#define INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE_MAJOR      2
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE_MINOR      1
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE_REVISION   3
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE_EDIT       27
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1100
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef INETSTL_INCL_INETSTL_H_INETSTL
# include <inetstl/inetstl.h>
#endif /* !INETSTL_INCL_INETSTL_H_INETSTL */
#ifndef INETSTL_OS_IS_WINDOWS
# error This file is currently compatible only with the Win32/Win64 API
#endif /* !INETSTL_OS_IS_WINDOWS */
#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <inetstl/filesystem/filesystem_traits.hpp>
#endif /* !INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS
#  include <inetstl/error/exceptions.hpp>           // for throw_internet_exception_policy
# endif /* !INETSTL_INCL_INETSTL_ERROR_HPP_EXCEPTIONS */
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
#  include <stlsoft/error/exceptions.hpp>           // for stlsoft::null_exception_policy
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE
# include <inetstl/filesystem/findfile_sequence.hpp>
#endif /* !INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */
#ifndef STLSOFT_INCL_ITERATOR
# define STLSOFT_INCL_ITERATOR
# include <iterator>
#endif /* !STLSOFT_INCL_ITERATOR */
#ifndef STLSOFT_INCL_VECTOR
# define STLSOFT_INCL_VECTOR
# include <vector>
#endif /* !STLSOFT_INCL_VECTOR */

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

/** STL collection of the files in an FTP directory matching a given pattern
 *
 * This class is described in detail in section 21.2 of
 * <a href="http://extendedstl.com/">Extended STL, volume 1</a>.
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   ss_typename_param_k X   =   throw_internet_exception_policy
# else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k X   =   stlsoft_ns_qual(null_exception_policy)
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k T = filesystem_traits<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k X /* = throw_internet_exception_policy */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_ftpdir_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef basic_findfile_sequence<C, T, X>                        sequence_type_;
public:
    typedef ss_typename_type_k sequence_type_::char_type            char_type;
    typedef ss_typename_type_k sequence_type_::value_type           value_type;
    typedef ss_typename_type_k sequence_type_::size_type            size_type;
    typedef ss_typename_type_k sequence_type_::bool_type            bool_type;
    typedef is_sint_t                                               flags_type;
private:
    typedef stlsoft_ns_qual_std(vector)<value_type>                 values_type_;
public:
    typedef ss_typename_type_k values_type_::const_reference        const_reference;
    typedef ss_typename_type_k values_type_::const_iterator         const_iterator;
    typedef ss_typename_type_k values_type_::const_reverse_iterator const_reverse_iterator;
    typedef basic_ftpdir_sequence<C, X, T>                          class_type;
/// @}

/// \name Member Constants
/// @{
public:
    enum search_flags
    {
            includeDots =   sequence_type_::includeDots
        ,   directories =   sequence_type_::directories
        ,   files       =   sequence_type_::files
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
        ,   noSort      =   sequence_type_::noSort
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
    };
/// @}

/// \name Construction
/// @{
public:
    basic_ftpdir_sequence(  HINTERNET           hconn
                        ,   char_type const*    pattern
                        ,   flags_type          flags = directories | files)
    {
        sequence_type_  ffs(hconn, pattern, flags);

        stlsoft_ns_qual_std(copy)(ffs.begin(), ffs.end(), stlsoft_ns_qual_std(back_inserter)(m_values));
    }
    /// Commence a search according to the given search pattern and flags, relative to \c directory
    basic_ftpdir_sequence(  HINTERNET           hconn
                        ,   char_type const*    directory
                        ,   char_type const*    pattern
                        ,   flags_type          flags = directories | files)
    {
        sequence_type_  ffs(hconn, directory, pattern, flags);

        stlsoft_ns_qual_std(copy)(ffs.begin(), ffs.end(), stlsoft_ns_qual_std(back_inserter)(m_values));
    }
    /// Commence a search according to the given search pattern and flags, relative to \c directory
    basic_ftpdir_sequence(  HINTERNET           hconn
                        ,   char_type const*    directory
                        ,   char_type const*    patterns
                        ,   char_type           delim
                        ,   flags_type          flags = directories | files)
    {
        sequence_type_  ffs(hconn, directory, patterns, delim, flags);

        stlsoft_ns_qual_std(copy)(ffs.begin(), ffs.end(), stlsoft_ns_qual_std(back_inserter)(m_values));
    }

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
    /// Constructs an instance of the sequence containing all the elements of the other
    basic_ftpdir_sequence(class_type const& rhs);
    /// Copies the contents of the given instance
    class_type& operator =(class_type const& rhs);
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Element access
/// @{
public:
    const_reference operator [](size_type index) const
    {
        INETSTL_MESSAGE_ASSERT("Invalid index", index < size());

        return m_values[index];
    }
/// @}

/// \name Iteration
/// @{
public:
    const_iterator          begin() const
    {
        return m_values.begin();
    }
    const_iterator          end() const
    {
        return m_values.end();
    }
    const_reverse_iterator  rbegin() const
    {
        return m_values.rbegin();
    }
    const_reverse_iterator  rend() const
    {
        return m_values.rend();
    }
/// @}

/// \name Size
/// @{
public:
    size_type   size() const
    {
        return m_values.size();
    }
    bool_type   empty() const
    {
        return m_values.empty();
    }
/// @}

/// \name Members
/// @{
private:
    values_type_    m_values;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_findfile_sequence template for the ANSI character type \c char
 *
 *
 */
typedef basic_ftpdir_sequence<is_char_a_t
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   filesystem_traits<is_char_a_t>
                            >                                                   ftpdir_sequence_a;
/** \brief Specialisation of the basic_ftpdir_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_ftpdir_sequence<is_char_w_t
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   filesystem_traits<is_char_w_t>
                            >                                                   ftpdir_sequence_w;
/** \brief Specialisation of the basic_ftpdir_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_ftpdir_sequence<TCHAR
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   filesystem_traits<TCHAR>
                            >                 ftpdir_sequence;

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/ftpdir_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION



#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FTPDIR_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
