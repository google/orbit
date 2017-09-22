/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/filesystem/findfile_sequence.hpp  (originally MInetEnm.h)
 *
 * Purpose:     Contains the basic_findfile_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     30th April 1999
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2009, Matthew Wilson and Synesis Software
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


/** \file inetstl/filesystem/findfile_sequence.hpp
 *
 * \brief [C++ only] Definition of the inetstl::findfile_sequence
 *   class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE
#define INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE_MAJOR    3
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE_MINOR    0
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE_REVISION 11
# define INETSTL_VER_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE_EDIT     139
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
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
# include <stlsoft/string/simple_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_TOKENISER_FUNCTIONS
# include <stlsoft/string/tokeniser_functions.hpp> // for find_next_token
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_TOKENISER_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifdef STLSOFT_UNITTEST
# include <stlsoft/conversion/integer_to_string.hpp>
# include <inetstl/network/connection.hpp>
# include <inetstl/network/session.hpp>
# include <stdio.h>
# include <string.h>
#endif /* STLSOFT_UNITTEST */

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
 * Forward declarations
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
class basic_findfile_sequence;

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
class basic_findfile_sequence_value_type;

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
class basic_findfile_sequence_const_input_iterator;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class basic_findfile_sequence
/** \brief Presents an STL-like sequence interface over the items on the file-system
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param T The traits type. On translators that support default template arguments this defaults to filesystem_traits<C>
 *
 * \note  This class was described in detail in the article
 * "Adapting Windows Enumeration Models to STL Iterator Concepts"
 * (http://www.windevnet.com/documents/win0303a/), in the March 2003 issue of
 * Windows Developer Network (http://windevnet.com).
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = filesystem_traits<C>
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   ss_typename_param_k X   =   throw_internet_exception_policy
# else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k X   =   stlsoft_ns_qual(null_exception_policy)
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
        ,   ss_typename_param_k X /* = throw_internet_exception_policy */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_findfile_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C                                                                   char_type;
    /// The exception policy type
    typedef X                                                                   exception_policy_type;
    /// The type thrown
    typedef ss_typename_param_k exception_policy_type::thrown_type              thrown_type;
    /// The traits type
    typedef T                                                                   traits_type;
    /// The current parameterisation of the type
    typedef basic_findfile_sequence<C, T, X>                                    class_type;
    /// The value type
    typedef basic_findfile_sequence_value_type<C, T, X>                         value_type;
    /// The non-mutating (const) iterator type supporting the Input Iterator concept
    typedef basic_findfile_sequence_const_input_iterator<C, T, X, value_type>   const_input_iterator;
    /// The non-mutating (const) iterator type
    typedef const_input_iterator                                                const_iterator;
    /// The reference type
    typedef value_type&                                                         reference;
    /// The non-mutable (const) reference type
    typedef value_type const&                                                   const_reference;
    /// The find-data type
    typedef ss_typename_type_k traits_type::find_data_type                      find_data_type;
    /// The difference type
    typedef is_ptrdiff_t                                                        difference_type;
    /// The size type
    typedef is_size_t                                                           size_type;
    /// The Boolean type
    typedef is_bool_t                                                           bool_type;
private:
    typedef is_sint_t                                                           flags_type;
    typedef stlsoft_ns_qual(basic_simple_string)<char_type>                     string_type;
/// @}

/// \name Member Constants
/// @{
public:
    enum search_flags
    {
            includeDots =   0x0008          //!< Causes the search to include the "." and ".." directories, which are elided by default
        ,   directories =   0x0010          //!< Causes the search to include directories
        ,   files       =   0x0020          //!< Causes the search to include files
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
        ,   noSort      =   0 /* 0x0100 */  //!<
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
    };
/// @}

/// \name Construction
/// @{
public:
    /// Commence a search according to the given search pattern and flags
    basic_findfile_sequence(HINTERNET           hconn
                        ,   char_type const*    pattern
                        ,   flags_type          flags = directories | files);
    /// Commence a search according to the given search pattern and flags, relative to \c directory
    basic_findfile_sequence(HINTERNET           hconn
                        ,   char_type const*    directory
                        ,   char_type const*    pattern
                        ,   flags_type          flags = directories | files);
    /// Commence a search according to the given search pattern and flags, relative to \c directory
    basic_findfile_sequence(HINTERNET           hconn
                        ,   char_type const*    directory
                        ,   char_type const*    patterns
                        ,   char_type           delim
                        ,   flags_type          flags = directories | files);
    /// Destructor
    ~basic_findfile_sequence() stlsoft_throw_0();
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator      begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator      end() const;
/// @}

/// \name Attributes
/// @{
public:
    /// Returns the directory of the search
    ///
    /// \note Will be the empty string for instances created with the first constructor
    char_type const*    get_directory(size_type* pn = NULL) const;
/// @}

/// \name State
/// @{
public:
#ifdef STLSOFT_OBSOLETE
    /// Returns the number of items in the sequence
    ///
    /// \note This is a potentially very expensive operation
    /// \deprecated
    size_type           size() const;
#endif /* STLSOFT_OBSOLETE */
    /// Indicates whether the sequence is empty
    bool_type           empty() const;
    /// Returns the maximum number of items in the sequence
    static size_type    max_size();
/// @}

/// \name Members
/// @{
private:
    friend class basic_findfile_sequence_value_type<C, T, X>;
    friend class basic_findfile_sequence_const_input_iterator<C, T, X, value_type>;

    const HINTERNET     m_hconn;
    const char_type     m_delim;
    const flags_type    m_flags;
    const string_type   m_rootDir;
    const string_type   m_patterns;
/// @}

/// \name Invariant
/// @{
private:
    bool_type is_valid() const;
/// @}

/// \name Implementation
/// @{
private:
    static flags_type   validate_flags_(flags_type flags);
    static void         extract_subpath_(HINTERNET hconn, char_type *dest, char_type const* pattern);

    static  HINTERNET   find_first_file_(HINTERNET hconn, char_type const* spec, flags_type flags, find_data_type *findData);
/// @}

/// \name Not to be implemented
/// @{
private:
    basic_findfile_sequence(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_findfile_sequence template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findfile_sequence<is_char_a_t
                            ,   filesystem_traits<is_char_a_t>
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            >                                                   findfile_sequence_a;
/** \brief Specialisation of the basic_findfile_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findfile_sequence<is_char_w_t
                            ,   filesystem_traits<is_char_w_t>
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            >                                                   findfile_sequence_w;
/** \brief Specialisation of the basic_findfile_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findfile_sequence<TCHAR
                            ,   filesystem_traits<TCHAR>
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                            ,   throw_internet_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                            ,   stlsoft_ns_qual(null_exception_policy)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            >                 findfile_sequence;

/* ////////////////////////////////////////////////////////////////////// */

// class basic_findfile_sequence_value_type
/** \brief Value type for the basic_findfile_sequence
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
class basic_findfile_sequence_value_type
{
/// \name Member Types
/// @{
private:
    typedef basic_findfile_sequence<C, T, X>                        sequence_type;
public:
    /// The character type
    typedef C                                                       char_type;
    /// The exception policy type
    typedef X                                                       exception_policy_type;
    /// The type thrown
    typedef ss_typename_param_k exception_policy_type::thrown_type  thrown_type;
    /// The traits type
    typedef T                                                       traits_type;
    /// The current parameterisation of the type
    typedef basic_findfile_sequence_value_type<C, T, X>             class_type;
    /// The find-data type
    typedef ss_typename_type_k traits_type::find_data_type          find_data_type;
    /// The size type
    typedef ss_typename_type_k sequence_type::size_type             size_type;
private:
    typedef ss_typename_type_k sequence_type::bool_type             bool_type;
    typedef ss_typename_type_k sequence_type::flags_type            flags_type;
    typedef stlsoft_ns_qual(basic_simple_string)<char_type>         string_type;
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor
    basic_findfile_sequence_value_type();
private:
    basic_findfile_sequence_value_type(find_data_type const& data, char_type const* path, size_type cchPath)
        : m_data(data)
    {
        INETSTL_ASSERT(NULL != path || 0 == cchPath);

        INETSTL_ASSERT(cchPath < STLSOFT_NUM_ELEMENTS(m_path));
        STLSOFT_SUPPRESS_UNUSED(cchPath);

        size_type cchFile = traits_type::str_len(data.cFileName);

        if('/' != data.cFileName[0])
        {
            traits_type::char_copy(m_path, path, cchPath + 1);
            if(!traits_type::has_dir_end(m_path))
            {
                traits_type::ensure_dir_end(m_path);
                ++cchPath;
            }
        }
        else
        {
            m_path[0] = '\0';
            cchPath = 0u;
        }
        traits_type::char_copy(m_path + cchPath, data.cFileName, cchFile + 1);
    }
// @}

/** \brief Accessors
 *
 * \ingroup group__library__filesystem
 */
// @{
public:
    /// Returns a non-mutating reference to find-data
    find_data_type const&   get_find_data() const;
#ifdef STLSOFT_OBSOLETE
    /// Returns a non-mutating reference to find-data
    ///
    /// \deprecated This method may be removed in a future release. get_find_data() should be used instead
    find_data_type const&   GetFindData() const;   // Deprecated
#endif /* STLSOFT_OBSOLETE */

    /// Returns the filename part of the item
    char_type const*        get_filename() const;
    /// Returns the short form of the filename part of the item
    char_type const*        get_short_filename() const;
    /// Returns the full path of the item
    char_type const*        get_path() const;
    /// Returns the full path of the item
    char_type const*        c_str() const;

    /// Implicit conversion to a pointer-to-const of the full path
    operator char_type const* () const;

    /// Indicates whether the entry is a directory
    is_bool_t               is_directory() const;
    /// Indicates whether the entry is a file
    is_bool_t               is_file() const;
    /// Indicates whether the entry is read-only
    is_bool_t               is_read_only() const;

    is_bool_t               equal(char_type const* rhs) const;
    is_bool_t               equal(class_type const& rhs) const;
// @}

/// \name Members
/// @{
private:
    friend class basic_findfile_sequence_const_input_iterator<C, T, X, class_type>;

    find_data_type  m_data;
    char_type       m_path[1 + _MAX_PATH];
/// @}
};

// class basic_findfile_sequence_const_input_iterator
/** \brief Iterator type for the basic_findfile_sequence supporting the Input Iterator concept
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        ,   ss_typename_param_k V
        >
class basic_findfile_sequence_const_input_iterator
#ifndef STLSOFT_COMPILER_IS_WATCOM
    : public stlsoft_ns_qual(iterator_base)<inetstl_ns_qual_std(input_iterator_tag)
                                        ,   V
                                        ,   is_ptrdiff_t
                                        ,   void    // By-Value Temporary reference
                                        ,   V       // By-Value Temporary reference
                                        >
#endif /* compiler */
{
/// \name Member Types
/// @{
private:
    typedef basic_findfile_sequence<C, T, X>                            sequence_type;
public:
    /// The character type
    typedef C                                                           char_type;
    /// The exception policy type
    typedef X                                                           exception_policy_type;
    /// The type thrown
    typedef ss_typename_param_k exception_policy_type::thrown_type      thrown_type;
    /// The traits type
    typedef T                                                           traits_type;
    /// The value type
    typedef V                                                           value_type;
    /// The current parameterisation of the type
    typedef basic_findfile_sequence_const_input_iterator<C, T, X, V>    class_type;
    /// The find-data type
    typedef ss_typename_type_k traits_type::find_data_type              find_data_type;
    /// The size type
    typedef ss_typename_type_k sequence_type::size_type                 size_type;
private:
    typedef ss_typename_type_k sequence_type::bool_type                 bool_type;
    typedef ss_typename_type_k sequence_type::flags_type                flags_type;
    typedef ss_typename_type_k sequence_type::string_type               string_type;
/// @}

/// \name Utility classes
/// @{
private:
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    struct shared_handle
    {
    /// \name Member Types
    /// @{
    public:
        typedef shared_handle       class_type;
        typedef HINTERNET           handle_type;
    /// @}

    /// \name Members
    /// @{
    public:
        handle_type     hSrch;
    private:
        ss_sint32_t     m_refCount;
    /// @}

    /// \name Construction
    /// @{
    public:
        ss_explicit_k shared_handle(handle_type h)
            : hSrch(h)
            , m_refCount(1)
        {}
# if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
    protected:
# else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
    private:
# endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
        ~shared_handle() stlsoft_throw_0()
        {
            INETSTL_MESSAGE_ASSERT("Shared search handle being destroyed with outstanding references!", 0 == m_refCount);

            if(NULL != hSrch)
            {
                traits_type::find_close(hSrch);
            }
        }
    /// @}

    /// \name Operations
    /// @{
    public:
        ss_sint32_t AddRef()
        {
            return ++m_refCount;
        }
        ss_sint32_t Release()
        {
            ss_sint32_t rc = --m_refCount;

            if(0 == rc)
            {
                delete this;
            }

            return rc;
        }
    /// @}

    /// \name Not to be implemented
    /// @{
    private:
        shared_handle(class_type const&);
        class_type& operator =(class_type const&);
    /// @}
    };
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Construction
/// @{
private:
    basic_findfile_sequence_const_input_iterator(
        sequence_type const&    l
    ,   char_type const*        rootDir
    ,   char_type const*        patterns
    ,   char_type               delim
    ,   flags_type              flags
    );
    basic_findfile_sequence_const_input_iterator(   sequence_type const& l);
public:
    /// Default constructor
    basic_findfile_sequence_const_input_iterator();
    /// <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">Move constructor</a>
    basic_findfile_sequence_const_input_iterator(class_type const& rhs);
    /// Destructor
    ~basic_findfile_sequence_const_input_iterator() stlsoft_throw_0();

    // Copy assignment operator
    basic_findfile_sequence_const_input_iterator& operator =(class_type const& rhs);
/// @}

/// \name Input Iterator methods
/// @{
public:
    /// Pre-increment operator
    class_type& operator ++();
    /// Post-increment operator
    class_type operator ++(int);
    /// Dereference to return the value at the current position
    const value_type operator *() const;
    /// Evaluates whether \c this and \c rhs are equivalent
    is_bool_t equal(class_type const& rhs) const;
/// @}

/// \name Implementation
/// @{
private:
    static int          find_next_pattern_(char_type const*& p0, char_type const*& p1, char_type delim);
    static HINTERNET    find_first_file_(HINTERNET hconn, char_type const* spec, flags_type flags, find_data_type *findData);
/// @}

/// \name Members
/// @{
private:
    friend class basic_findfile_sequence<C, T, X>;

    sequence_type const* const                      m_list;
    shared_handle*                                  m_handle;
    ss_typename_type_k traits_type::find_data_type  m_data;
    char_type const*                                m_rootDir;
    char_type const*                                m_pattern0;
    char_type const*                                m_pattern1;
    char_type                                       m_delim;
    flags_type                                      m_flags;
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Operators

// basic_findfile_sequence_const_input_iterator

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        ,   ss_typename_param_k V
        >
inline is_bool_t operator ==(   basic_findfile_sequence_const_input_iterator<C, T, X, V> const& lhs
                            ,   basic_findfile_sequence_const_input_iterator<C, T, X, V> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        ,   ss_typename_param_k V
        >
inline is_bool_t operator !=(   basic_findfile_sequence_const_input_iterator<C, T, X, V> const& lhs
                            ,   basic_findfile_sequence_const_input_iterator<C, T, X, V> const& rhs)
{
    return !lhs.equal(rhs);
}

// basic_findfile_sequence_value_type

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator == (  basic_findfile_sequence_value_type<C, T, X> const&  lhs
                            ,   basic_findfile_sequence_value_type<C, T, X> const&  rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator == (  basic_findfile_sequence_value_type<C, T, X> const& lhs
                            ,   C const* rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator == (C const* lhs, basic_findfile_sequence_value_type<C, T, X> const& rhs)
{
    return rhs.equal(lhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator != (  basic_findfile_sequence_value_type<C, T, X> const& lhs
                            ,   basic_findfile_sequence_value_type<C, T, X> const& rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator != (  basic_findfile_sequence_value_type<C, T, X> const&  lhs
                            ,   C const* rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k X
        >
inline is_bool_t operator != (  C const* lhs, basic_findfile_sequence_value_type<C, T, X> const& rhs)
{
    return !rhs.equal(lhs);
}

////////////////////////////////////////////////////////////////////////////
// Shims

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_a_t const* c_str_data_a(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_a_t, T, X> const& v)
{
    return v.get_path();
}
template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_w_t const* c_str_data_w(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_w_t, T, X> const& v)
{
    return v.get_path();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for inetstl::basic_findfile_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 *
 */
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline C const* c_str_data(inetstl_ns_qual(basic_findfile_sequence_value_type)<C, T, X> const& v)
{
    return v.get_path();
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k X>
inline is_size_t c_str_len_a(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_a_t, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_len_a(v.get_path()));
}
template <ss_typename_param_k T, ss_typename_param_k X>
inline is_size_t c_str_len_w(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_w_t, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_len_w(v.get_path()));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for inetstl::basic_findfile_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_size_t c_str_len(inetstl_ns_qual(basic_findfile_sequence_value_type)<C, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_len(v.get_path()));
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_a_t const* c_str_ptr_a(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_a_t, T, X> const& v)
{
    return v.get_path();
}
template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_w_t const* c_str_ptr_w(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_w_t, T, X> const& v)
{
    return v.get_path();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for inetstl::basic_findfile_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline C const* c_str_ptr(inetstl_ns_qual(basic_findfile_sequence_value_type)<C, T, X> const& v)
{
    return v.get_path();
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_a_t const* c_str_ptr_null_a(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_a_t, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null(v.get_path()));
}
template <ss_typename_param_k T, ss_typename_param_k X>
inline is_char_w_t const* c_str_ptr_null_w(inetstl_ns_qual(basic_findfile_sequence_value_type)<is_char_w_t, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null(v.get_path()));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for inetstl::basic_findfile_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline C const* c_str_ptr_null(inetstl_ns_qual(basic_findfile_sequence_value_type)<C, T, X> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null(v.get_path()));
}



////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/findfile_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// basic_findfile_sequence

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline /* static */ HINTERNET basic_findfile_sequence<C, T, X>::find_first_file_(
    HINTERNET                                                               hconn
,   ss_typename_type_k basic_findfile_sequence<C, T, X>::char_type const*   spec
,   ss_typename_type_k basic_findfile_sequence<C, T, X>::flags_type         /* flags */
,   ss_typename_type_k basic_findfile_sequence<C, T, X>::find_data_type*    findData
)
{
    HINTERNET   hSrch   =   traits_type::find_first_file(hconn, spec, findData);

    if(NULL == hSrch)
    {
        DWORD       dwErr   =   ::GetLastError();

        if(ERROR_FTP_TRANSFER_IN_PROGRESS == dwErr)
        {
            exception_policy_type()("Already enumerating using current connection", dwErr);
        }
        else
        {
            exception_policy_type()("Search failed", dwErr);
        }
    }

    return hSrch;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline /* static */ ss_typename_type_ret_k basic_findfile_sequence<C, T, X>::flags_type basic_findfile_sequence<C, T, X>::validate_flags_(ss_typename_type_k basic_findfile_sequence<C, T, X>::flags_type flags)
{
    const flags_type    validFlags  =   0
                                    |   includeDots
                                    |   directories
                                    |   files
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
                                    |   noSort
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
                                    |   0;

    INETSTL_MESSAGE_ASSERT("Specification of unrecognised/unsupported flags", flags == (flags & validFlags));
    STLSOFT_SUPPRESS_UNUSED(validFlags);

    if(0 == (flags & (directories | files)))
    {
        flags |= (directories | files);
    }

    return flags;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline /* static */ is_bool_t basic_findfile_sequence<C, T, X>::is_valid() const
{
    return true;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline /* static */ void basic_findfile_sequence<C, T, X>::extract_subpath_(HINTERNET hconn, char_type *dest, char_type const* pattern)
{
    char_type* pFile;

    traits_type::get_full_path_name(hconn, pattern, _MAX_PATH, dest, &pFile);

    if(NULL != pFile)
    {
        *pFile = '\0';
    }
}

// Construction
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline basic_findfile_sequence<C, T, X>::basic_findfile_sequence(HINTERNET hconn, char_type const* pattern, ss_typename_type_k basic_findfile_sequence<C, T, X>::flags_type flags /* = directories | files */)
    : m_hconn(hconn)
    , m_delim('\0')
    , m_flags(validate_flags_(flags))
    , m_rootDir()
    , m_patterns(pattern)
{
    INETSTL_ASSERT(is_valid());
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline basic_findfile_sequence<C, T, X>::basic_findfile_sequence(HINTERNET hconn, char_type const* directory, char_type const* pattern, ss_typename_type_k basic_findfile_sequence<C, T, X>::flags_type flags /* = directories | files */)
    : m_hconn(hconn)
    , m_delim('\0')
    , m_flags(validate_flags_(flags))
    , m_rootDir(directory)
    , m_patterns(pattern)
{
    INETSTL_ASSERT(is_valid());
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline basic_findfile_sequence<C, T, X>::basic_findfile_sequence(
    HINTERNET                                                       hconn
,   char_type const*                                                directory
,   char_type const*                                                patterns
,   char_type                                                       delim
,   ss_typename_type_k basic_findfile_sequence<C, T, X>::flags_type flags /* = directories | files */
)
    : m_hconn(hconn)
    , m_delim(delim)
    , m_flags(validate_flags_(flags))
    , m_rootDir(directory)
    , m_patterns(patterns)
{
    INETSTL_ASSERT(is_valid());
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline basic_findfile_sequence<C, T, X>::~basic_findfile_sequence() stlsoft_throw_0()
{
    INETSTL_ASSERT(is_valid());
}

// Iteration
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence<C, T, X>::const_iterator basic_findfile_sequence<C, T, X>::begin() const
{
    INETSTL_ASSERT(is_valid());

    return const_input_iterator(*this, stlsoft_ns_qual(c_str_ptr)(m_rootDir), stlsoft_ns_qual(c_str_ptr)(m_patterns), m_delim, m_flags);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence<C, T, X>::const_iterator basic_findfile_sequence<C, T, X>::end() const
{
    INETSTL_ASSERT(is_valid());

    return const_input_iterator(*this);
}

// Attributes
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
ss_typename_type_k basic_findfile_sequence<C, T, X>::char_type const* basic_findfile_sequence<C, T, X>::get_directory(ss_typename_type_k basic_findfile_sequence<C, T, X>::size_type* pn) const
{
    INETSTL_ASSERT(is_valid());

    size_type n_;

    if(NULL == pn)
    {
        pn = &n_;
    }

    *pn = m_rootDir.size();

    return m_rootDir.c_str();
}

// State
#ifdef STLSOFT_OBSOLETE
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence<C, T, X>::size_type basic_findfile_sequence<C, T, X>::size() const
{
    INETSTL_ASSERT(is_valid());

    return stlsoft_ns_qual_std(distance)(begin(), end());
}
#endif /* STLSOFT_OBSOLETE */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence<C, T, X>::empty() const
{
    INETSTL_ASSERT(is_valid());

    return begin() == end();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline /* static */ ss_typename_type_ret_k basic_findfile_sequence<C, T, X>::size_type basic_findfile_sequence<C, T, X>::max_size()
{
    return static_cast<size_type>(-1);
}

// basic_findfile_sequence_value_type

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline basic_findfile_sequence_value_type<C, T, X>::basic_findfile_sequence_value_type()
{
    m_data.dwFileAttributes         =   0xFFFFFFFF;
    m_data.cFileName[0]             =   '\0';
    m_data.cAlternateFileName[0]    =   '\0';
    m_path[0]                       =   '\0';
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::find_data_type const& basic_findfile_sequence_value_type<C, T, X>::get_find_data() const
{
    return m_data;
}

#ifdef STLSOFT_OBSOLETE
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::find_data_type const& basic_findfile_sequence_value_type<C, T, X>::GetFindData() const
{
    return get_find_data();
}
#endif /* STLSOFT_OBSOLETE */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::char_type const* basic_findfile_sequence_value_type<C, T, X>::get_filename() const
{
    return m_data.cFileName;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::char_type const* basic_findfile_sequence_value_type<C, T, X>::get_short_filename() const
{
    return m_data.cAlternateFileName[0] != '\0' ? m_data.cAlternateFileName : m_data.cFileName;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::char_type const* basic_findfile_sequence_value_type<C, T, X>::get_path() const
{
    return &m_path[0];
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline ss_typename_type_ret_k basic_findfile_sequence_value_type<C, T, X>::char_type const* basic_findfile_sequence_value_type<C, T, X>::c_str() const
{
    return get_path();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
#if defined(STLSOFT_COMPILER_IS_GCC) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1100)
inline basic_findfile_sequence_value_type<C, T, X>::operator C const* () const
#else /* ? compiler */
inline basic_findfile_sequence_value_type<C, T, X>::operator ss_typename_type_k basic_findfile_sequence_value_type<C, T, X>::char_type const* () const
#endif /* !compiler */
{
    return get_path();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence_value_type<C, T, X>::is_directory() const
{
    return traits_type::is_directory(&m_data);
}
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence_value_type<C, T, X>::is_file() const
{
    return traits_type::is_file(&m_data);
}
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence_value_type<C, T, X>::is_read_only() const
{
    return 0 != (m_data.dwFileAttributes & FILE_ATTRIBUTE_READONLY);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence_value_type<C, T, X>::equal(char_type const* rhs) const
{
    INETSTL_ASSERT(NULL != rhs);

    return 0 == traits_type::str_compare(this->get_path(), rhs);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X>
inline is_bool_t basic_findfile_sequence_value_type<C, T, X>::equal(basic_findfile_sequence_value_type<C, T, X> const& rhs) const
{
    return equal(rhs.get_path());
}

// basic_findfile_sequence_const_input_iterator

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline /* static */ int basic_findfile_sequence_const_input_iterator<C, T, X, V>::find_next_pattern_(  ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type const*& p0
                                                                                                ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type const*& p1
                                                                                                ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type delim)
{
    INETSTL_ASSERT(NULL != p0);
    INETSTL_ASSERT(NULL != p1);
    INETSTL_ASSERT(p0 <= p1);

    return stlsoft_ns_qual(find_next_token)(p0, p1, delim);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline /* static */ HINTERNET basic_findfile_sequence_const_input_iterator<C, T, X, V>::find_first_file_(  HINTERNET                                                                                   hconn
#ifdef STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type const*  pattern
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::flags_type        flags
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::find_data_type*   findData)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    ,   char_type const*    pattern
    ,   flags_type          flags
    ,   find_data_type*     findData)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
{
    HINTERNET   hSrch = traits_type::find_first_file(hconn, pattern, findData);

    if(hSrch != NULL)
    {
        // Now need to validate against the flags
        for(; hSrch != NULL; )
        {
            if(0 == (findData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // A file, and files requested, so break
                if(flags & sequence_type::files)
                {
                    break;
                }
            }
            else
            {
                if(traits_type::is_dots(findData->cFileName))
                {
                    if(flags & sequence_type::includeDots)
                    {
                        // A dots file, and dots are requested
                        break;
                    }
                }
                else if(flags & sequence_type::directories)
                {
                    // A directory, and directories requested
                    break;
                }
            }

            if(!traits_type::find_next_file(hSrch, findData))
            {
                traits_type::find_close(hSrch);

                hSrch = NULL;

                break;
            }
        }
    }

    return hSrch;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline basic_findfile_sequence_const_input_iterator<C, T, X, V>::basic_findfile_sequence_const_input_iterator()
    : m_list(NULL)
    , m_handle(NULL)
    , m_rootDir(NULL)
    , m_pattern0(NULL)
    , m_pattern1(NULL)
    , m_delim('\0')
    , m_flags(0)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline basic_findfile_sequence_const_input_iterator<C, T, X, V>::basic_findfile_sequence_const_input_iterator( sequence_type const&                                                                         l
#ifdef STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type const*   rootDir
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type const*   patterns
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::char_type          delim
    ,   ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::flags_type         flags)
#else /* ? STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    ,   char_type const*    rootDir
    ,   char_type const*    patterns
    ,   char_type           delim
    ,   flags_type          flags)
#endif /* STLSOFT_CF_FUNCTION_SIGNATURE_FULL_ARG_QUALIFICATION_REQUIRED */
    : m_list(&l)
    , m_handle(NULL)
    , m_rootDir(rootDir)
    , m_pattern0(patterns)
    , m_pattern1(patterns)
    , m_delim(delim)
    , m_flags(flags)
{
    operator ++();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline basic_findfile_sequence_const_input_iterator<C, T, X, V>::basic_findfile_sequence_const_input_iterator(sequence_type const& l)
    : m_list(&l)
    , m_handle(NULL)
    , m_rootDir(NULL)
    , m_pattern0(NULL)
    , m_pattern1(NULL)
    , m_delim('\0')
    , m_flags(0)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline basic_findfile_sequence_const_input_iterator<C, T, X, V>::basic_findfile_sequence_const_input_iterator(class_type const& rhs)
    : m_list(rhs.m_list)
    , m_handle(rhs.m_handle)
    , m_data(rhs.m_data)
    , m_rootDir(rhs.m_rootDir)
    , m_pattern0(rhs.m_pattern0)
    , m_pattern1(rhs.m_pattern1)
    , m_delim(rhs.m_delim)
    , m_flags(rhs.m_flags)
{
    if(NULL != m_handle)
    {
        m_handle->AddRef();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
#ifndef STLSOFT_COMPILER_IS_WATCOM
inline ss_typename_type_ret_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::class_type& basic_findfile_sequence_const_input_iterator<C, T, X, V>::operator =(ss_typename_param_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::class_type const& rhs)
#else /* ? compiler */
inline basic_findfile_sequence_const_input_iterator<C, T, X, V> &basic_findfile_sequence_const_input_iterator<C, T, X, V>::operator =(basic_findfile_sequence_const_input_iterator<C, T, X, V> const& rhs)
#endif /* compiler */
{
    INETSTL_MESSAGE_ASSERT("Assigning iterators from separate sequences", (NULL == m_list || NULL == rhs.m_list || m_list == rhs.m_list));    // Should only be comparing iterators from same container

    shared_handle* this_handle = m_handle;

    m_handle    =   rhs.m_handle;
    m_data      =   rhs.m_data;
    m_rootDir   =   rhs.m_rootDir;
    m_pattern0  =   rhs.m_pattern0;
    m_pattern1  =   rhs.m_pattern1;
    m_delim     =   rhs.m_delim;
    m_flags     =   rhs.m_flags;

    if(NULL != m_handle)
    {
        m_handle->AddRef();
    }

    if(NULL != this_handle)
    {
        this_handle->Release();
    }

    return *this;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline basic_findfile_sequence_const_input_iterator<C, T, X, V>::~basic_findfile_sequence_const_input_iterator() stlsoft_throw_0()
{
    if(NULL != m_handle)
    {
        m_handle->Release();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline ss_typename_type_ret_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::class_type& basic_findfile_sequence_const_input_iterator<C, T, X, V>::operator ++()
{
    INETSTL_ASSERT(NULL != m_pattern0);
    INETSTL_ASSERT(NULL != m_pattern1);

    INETSTL_MESSAGE_ASSERT("Attempting to increment an invalid iterator!", '\0' != *m_pattern0);

    // Possible call states:
    //
    //  1. starting out
    //  2.

    for(; '\0' != *m_pattern0 || '\0' != *m_pattern1;)
    {
        if(NULL == m_handle)
        {
            // Need to work through the

            while(find_next_pattern_(m_pattern0, m_pattern1, m_delim))
            {
                if(m_pattern1 != m_pattern0)    // Will return m_pattern0 == m_pattern1 for empty tokens
                {
                    string_type     pattern(m_pattern0, m_pattern1);
                    string_type     search  =   m_rootDir;

                    if(search.back() != '/')
                    {
                        static const char_type  slash[] = { '/', '\0' };

                        search += slash;
                    }
                    search += pattern;

//printf("[%s]\n", search.c_str());
                    HINTERNET       hSrch   =   find_first_file_(m_list->m_hconn, stlsoft_ns_qual(c_str_ptr)(search), m_flags, &m_data);

                    if(NULL != hSrch)
                    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                        try
                        {
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                            m_handle = new shared_handle(hSrch);

                            if(NULL == m_handle)
                            {
                                ::FindClose(hSrch);
                            }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                        }
                        catch(...)
                        {
                            ::FindClose(hSrch);

                            throw;
                        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

                        return *this;
                    }
                }
            }
        }

        if(NULL != m_handle)
        {
            for(; m_handle->hSrch != NULL; )
            {
                if(!traits_type::find_next_file(m_handle->hSrch, &m_data))
                {
                    m_handle->Release();

                    m_handle = NULL;

                    break;
                }
                else
                {
                    if((m_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        // A file, and files requested, so break
                        if(m_flags & sequence_type::files)
                        {
                            return *this;
                        }
                    }
                    else
                    {
                        if(traits_type::is_dots(m_data.cFileName))
                        {
                            if(m_flags & sequence_type::includeDots)
                            {
                                // A dots file, and dots are requested
                                return *this;
                            }
                        }
                        else if(m_flags & sequence_type::directories)
                        {
                            // A directory, and directories requested
                            return *this;
                        }
                    }
                }
            }
        }
    }

    return *this;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline ss_typename_type_ret_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::class_type basic_findfile_sequence_const_input_iterator<C, T, X, V>::operator ++(int)
{
    class_type  ret(*this);

    operator ++();

    return ret;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline const ss_typename_type_k basic_findfile_sequence_const_input_iterator<C, T, X, V>::value_type basic_findfile_sequence_const_input_iterator<C, T, X, V>::operator *() const
{
    if(NULL != m_handle)
    {
        size_type           dirLen  =   0;
        char_type const*    dir     =   m_list->get_directory(&dirLen);

        return value_type(m_data, dir, dirLen);
    }
    else
    {
        INETSTL_MESSAGE_ASSERT("Dereferencing end()-valued iterator", 0);

        return value_type();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k X, ss_typename_param_k V>
inline is_bool_t basic_findfile_sequence_const_input_iterator<C, T, X, V>::equal(class_type const& rhs) const
{
    // Should only be comparing iterators from same container
    INETSTL_MESSAGE_ASSERT("Comparing iterators from separate sequences", (m_list == rhs.m_list || NULL == m_list || NULL == rhs.m_list));

    return m_handle == rhs.m_handle;
}

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

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _INETSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::inetstl::c_str_data;
using ::inetstl::c_str_data_a;
using ::inetstl::c_str_data_w;

using ::inetstl::c_str_len;
using ::inetstl::c_str_len_a;
using ::inetstl::c_str_len_w;

using ::inetstl::c_str_ptr;
using ::inetstl::c_str_ptr_a;
using ::inetstl::c_str_ptr_w;

using ::inetstl::c_str_ptr_null;
using ::inetstl::c_str_ptr_null_a;
using ::inetstl::c_str_ptr_null_w;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_FILESYSTEM_HPP_FINDFILE_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
