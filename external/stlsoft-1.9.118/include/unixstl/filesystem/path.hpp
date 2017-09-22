/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/filesystem/path.hpp
 *
 * Purpose:     Simple class that represents a path.
 *
 * Created:     1st May 1993
 * Updated:     29th November 2010
 *
 * Thanks to:   Pablo Aguilar for reporting defect in push_ext() (which
 *              doesn't work for wide-string builds).
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1993-2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/filesystem/path.hpp
 *
 * \brief [C++ only] Definition of the unixstl::basic_path class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH
#define UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_MAJOR      6
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_MINOR      6
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_REVISION   4
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_EDIT       236
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
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef UNIXSTL_INCL_UNIXSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
#  include <unixstl/error/exceptions.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>       // for STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING
# include <unixstl/shims/access/string.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS
# include <stlsoft/string/copy_functions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                            // for std::logic_error
#endif /* !STLSOFT_INCL_STDEXCEPT */
#ifdef _WIN32
# include <ctype.h>
#endif /* _WIN32 */

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
 * basic_path
 *
 * This class represents a path, and effectively acts as a C-string of its value.
 */

/** Represents a path
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param T The traits type. Defaults to filesystem_traits<C>. On translators that do not support default template arguments, it must be explicitly stipulated
 * \param A The allocator class. Defaults to stlsoft::allocator_selector<C>::allocator_type. On translators that do not support default template arguments, it must be explicitly stipulated
 *
 * \note This class derives from the Synesis Software class Path, but has been influenced
 * by other, later, ideas. The idea of using the / operator for path concatenation was
 * sparked by the Boost implementation (although the details were not investigated prior
 * to this implementation, so the two may have significant semantic differences). This
 * has been added without requiring any major fundamental changes to the original
 * <code>push/pop</code>-based interface
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = filesystem_traits<C>
        ,   ss_typename_param_k A = ss_typename_type_def_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
        ,   ss_typename_param_k A /* = ss_typename_type_def_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_path
{
/// \name Types
/// @{
public:
    /// The char type
    typedef C                           char_type;
    /// The traits type
    typedef T                           traits_type;
    /// The allocator type
    typedef A                           allocator_type;
    /// The current parameterisation of the type
    typedef basic_path<C, T, A>         class_type;
    /// The size type
    typedef us_size_t                   size_type;
    /// The Boolean type
    typedef us_bool_t                   bool_type;

// TODO: Use the slice string, and provide iterators over the directory parts

/// @}

/// \name Construction
/// @{
public:
    /// Constructs an empty path
    basic_path();
    /// Constructs a path from \c path
    ss_explicit_k basic_path(char_type const* path);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// Constructs a path from \c path
    template<ss_typename_param_k S>
    ss_explicit_k basic_path(S const& s)
    {
        m_len = stlsoft_ns_qual(c_str_len)(s);

        traits_type::char_copy(&m_buffer[0], stlsoft_ns_qual(c_str_data)(s), m_len);
        m_buffer[m_len] = '\0';
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    /// Constructs a path from \c cch characters in \c path
    basic_path(char_type const* path, size_type cch);

#ifndef STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD
    /// Copies the contents of \c rhs
    basic_path(class_type const& rhs);
#endif /* !STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD */

#ifndef STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD
    /// Copies the contents of \c rhs
    class_type& operator =(class_type const& rhs);
#endif /* !STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD */
    /// Copies the contents of \c rhs
    class_type& operator =(char_type const* rhs);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /// Copies the contents of \c s
    template<ss_typename_param_k S>
    class_type& operator =(S const& s)
    {
        return operator_equal_(stlsoft_ns_qual(c_str_ptr)(s));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

    // Creates a root path
    static class_type root(char_type const* s);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    // Creates a root path
    template<ss_typename_param_k S>
    static class_type root(S const& s)
    {
        return root(stlsoft_ns_qual(c_str_ptr)(s));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
/// @}

/// \name Operations
/// @{
public:
    /// Appends the contents of \c rhs to the path
    class_type& push(class_type const& rhs, bool_type bAddPathNameSeparator = false);
    /// Appends the contents of \c rhs to the path
    class_type& push(char_type const* rhs, bool_type bAddPathNameSeparator = false);
    /// Appends the contents of \c rhs to the path as an extension
    class_type& push_ext(class_type const& rhs, bool_type bAddPathNameSeparator = false);
    /// Appends the contents of \c rhs to the path as an extension
    class_type& push_ext(char_type const* rhs, bool_type bAddPathNameSeparator = false);
    /// Ensures that the path has a trailing path name separator
    class_type& push_sep();
    /// Pops the last path element from the path
    ///
    /// \note In previous versions, this operation did not remove the
    ///   left-most path component. That behaviour is no longer supported,
    ///   and the method will now leave the path instance empty in that
    ///   case.
    class_type& pop(bool_type bRemoveTrailingPathNameSeparator = true);
    /// Ensures that the path does not have a trailing path name separator
    ///
    /// \note Does not trim the separator character from the root designator
    class_type& pop_sep();
    /// Removes the extension, if any, from the file component of the path
    class_type& pop_ext();

    /// Equivalent to push()
    class_type& operator /=(char_type const* rhs);

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED)
    /// Equivalent to push()
    class_type& operator /=(class_type const& rhs);
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED */

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT)
    /// Equivalent to push()
    template <ss_typename_param_k S>
    class_type& operator /=(S const& rhs)
    {
        return push(stlsoft_ns_qual(c_str_ptr)(rhs));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

    /// Removes all content
    void        clear();

    /// Converts the path to absolute form
    class_type& make_absolute(bool_type bRemoveTrailingPathNameSeparator = true);
    /// Canonicalises the path, removing all "./" parts and evaluating all "../" parts
    class_type& canonicalise(bool_type bRemoveTrailingPathNameSeparator = true);
/// @}

/// \name Attributes
/// @{
public:
    /// Returns a pointer to the part of the path after the last path name separator
    ///
    /// \note If the path represents a directory, the name of the directory will be returned, except
    /// if the path is terminated by the path name separator
    ///
    /// \note If the path contains no path name separator, the full path will be returned
    char_type const* get_file() const;
    /// Returns a pointer to the extension, or to the empty string if there is no extension
    char_type const* get_ext() const;
    /// Returns the length of the converted path
    size_type       length() const;
    /// Returns the length of the converted path
    ///
    /// \remarks Equivalent to length()
    size_type       size() const;
    /// The maximum possible length of a path
    static size_type  max_size();
    /// Determines whether the path is empty
    bool_type       empty() const;
    /// Conversion to a non-mutable (const) pointer to the path
    char_type const* c_str() const;
    /// Returns a non-mutable (const) reference to the character at
    ///  the given index
    ///
    /// \note The behaviour is undefined if <code>index >= size()</code>.
    char_type const& operator [](size_type index) const;
    /// Indicates whether the path represents an existing file system entry
    bool_type       exists() const;
    /// Indicates whether the path is rooted
    bool_type       is_rooted() const;
    /// Indicates whether the path is absolute
    bool_type       is_absolute() const;
    /// Indicates whether the path has a trailing separator
    bool_type       has_sep() const;

    /// Copies the contents into a caller supplied buffer
    ///
    /// \param buffer Pointer to character buffer to receive the contents.
    ///  May be NULL, in which case the method returns size().
    /// \param cchBuffer Number of characters of available space in \c buffer.
    size_type       copy(char_type *buffer, size_type cchBuffer) const;
/// @}

/// \name Comparison
/// @{
public:
    bool_type equivalent(char_type const* rhs) const;
    bool_type equivalent(class_type const& rhs) const;

    bool_type equal(char_type const* rhs) const;
    bool_type equal(class_type const& rhs) const;
/// @}

/// \name Iteration
/// @{
public:
#if 0
    directory_iterator  dir_begin() const;
    directory_iterator  dir_end() const;
#endif /* 0 */
/// @}

// Implementation
private:
    class_type&             operator_equal_(char_type const* path);

    void                    swap(class_type& rhs);
    class_type&             concat_(char_type const* rhs, size_type cch);

    static char_type const  *next_slash_or_end(char_type const* p);
    static char_type        path_name_separator_alt();

// Members
private:
    typedef basic_file_path_buffer<
        char_type
    ,   allocator_type
    >                       buffer_type_;

    struct part
    {
        enum Type
        {
                normal
            ,   dot
            ,   dotdot
        };

        size_type           len;
        char_type const*    p;
        Type                type;
    };

    buffer_type_    m_buffer;
    size_type       m_len;
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** Specialisation of the basic_path template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_path<us_char_a_t, filesystem_traits<us_char_a_t> >       path_a;
/** Specialisation of the basic_path template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_path<us_char_w_t, filesystem_traits<us_char_w_t> >       path_w;
/** Specialisation of the basic_path template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_path<us_char_a_t, filesystem_traits<us_char_a_t> >       path;

/* /////////////////////////////////////////////////////////////////////////
 * Support for PlatformSTL redefinition by inheritance+namespace, for confused
 * compilers (e.g. VC++ 6)
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

    template<   ss_typename_param_k C
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
            ,   ss_typename_param_k T = filesystem_traits<C>
            ,   ss_typename_param_k A = ss_typename_type_def_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            ,   ss_typename_param_k T /* = filesystem_traits<C> */
            ,   ss_typename_param_k A /* = ss_typename_type_def_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type */
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            >
    class basic_path__
        : public unixstl_ns_qual(basic_path)<C, T, A>
    {
    private:
        typedef unixstl_ns_qual(basic_path)<C, T, A>                    parent_class_type;
        typedef unixstl_ns_qual(basic_path__)<C, T, A>                  class_type;
    public:
        typedef ss_typename_type_k parent_class_type::char_type         char_type;
        typedef ss_typename_type_k parent_class_type::traits_type       traits_type;
        typedef ss_typename_type_k parent_class_type::allocator_type    allocator_type;
        typedef ss_typename_type_k parent_class_type::size_type         size_type;

    public:
        basic_path__()
            : parent_class_type()
        {}
        ss_explicit_k basic_path__(char_type const* path)
            : parent_class_type(path)
        {}
# ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
        /// Constructs a path from \c path
        template<ss_typename_param_k S>
        ss_explicit_k basic_path__(S const& s)
            : parent_class_type(s)
        {}
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
        basic_path__(char_type const* path, size_type cch)
            : parent_class_type(path, cch)
        {}
        basic_path__(class_type const& rhs)
            : parent_class_type(rhs)
        {}

        class_type& operator =(class_type const& rhs)
        {
            parent_class_type::operator =(rhs);

            return *this;
        }
        class_type& operator =(char_type const* rhs)
        {
            parent_class_type::operator =(rhs);

            return *this;
        }
# ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
        template<ss_typename_param_k S>
        class_type& operator =(S const& s)
        {
            parent_class_type::operator =(s);

            return *this;
        }
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    };

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator ==(basic_path<C, T, A> const& lhs, ss_typename_type_k basic_path<C, T, A>::char_type const* rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator !=(basic_path<C, T, A> const& lhs, ss_typename_type_k basic_path<C, T, A>::char_type const* rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator ==(ss_typename_type_k basic_path<C, T, A>::char_type const* lhs, basic_path<C, T, A> const& rhs)
{
    return rhs.equal(lhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator !=(ss_typename_type_k basic_path<C, T, A>::char_type const* lhs, basic_path<C, T, A> const& rhs)
{
    return !rhs.equal(lhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator ==(basic_path<C, T, A> const& lhs, basic_path<C, T, A> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t operator !=(basic_path<C, T, A> const& lhs, basic_path<C, T, A> const& rhs)
{
    return !lhs.equal(rhs);
}

// operator /

/** Concatenates \c rhs to the path \c lhs
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A> operator /(basic_path<C, T, A> const& lhs, ss_typename_type_k basic_path<C, T, A>::char_type const* rhs)
{
    return basic_path<C, T, A>(lhs) /= rhs;
}

/** Concatenates \c rhs to the path \c lhs
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A> operator /(ss_typename_type_k basic_path<C, T, A>::char_type const* lhs, basic_path<C, T, A> const& rhs)
{
    return basic_path<C, T, A>(lhs) /= rhs;
}

/** Concatenates \c rhs to the path \c lhs
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A> operator /(basic_path<C, T, A> const& lhs, basic_path<C, T, A> const& rhs)
{
    return basic_path<C, T, A>(lhs) /= rhs;
}

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#if !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if !defined(STLSOFT_COMPILER_IS_MSVC) || \
     _MSC_VER >= 1100

/** This helper function makes a path variable without needing to
 * qualify the template parameter.
 *
 * \ingroup group__library__filesystem
 */
template<ss_typename_param_k C>
inline basic_path<C> make_path(C const* path)
{
    return basic_path<C>(path);
}

# endif /* compiler */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline void swap(basic_path<C, T, A>& lhs, basic_path<C, T, A>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \ref group__concept__shim__string_access__c_str_data for unixstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_data(unixstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_a_t const* c_str_data_a(unixstl_ns_qual(basic_path)<us_char_a_t, T, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_w_t const* c_str_data_w(unixstl_ns_qual(basic_path)<us_char_w_t, T, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_len for unixstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_size_t c_str_len(unixstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline us_size_t c_str_len_a(unixstl_ns_qual(basic_path)<us_char_a_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_a)(b.c_str());
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline us_size_t c_str_len_w(unixstl_ns_qual(basic_path)<us_char_w_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \ref group__concept__shim__string_access__c_str_ptr for unixstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr(unixstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_a_t const* c_str_ptr_a(unixstl_ns_qual(basic_path)<us_char_a_t, T, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_w_t const* c_str_ptr_w(unixstl_ns_qual(basic_path)<us_char_w_t, T, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \ref group__concept__shim__string_access__c_str_ptr_null for unixstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr_null(unixstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_a_t const* c_str_ptr_null_a(unixstl_ns_qual(basic_path)<us_char_a_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_a)(b.c_str());
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline us_char_w_t const* c_str_ptr_null_w(unixstl_ns_qual(basic_path)<us_char_w_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */





/** \ref group__concept__shim__stream_insertion "stream insertion shim" for unixstl::basic_path
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline S& operator <<(S& s, unixstl_ns_qual(basic_path)<C, T, A> const& b)
{
    s << b.c_str();

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/path_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER >= 1300
#  pragma warning(push)
#  pragma warning(disable : 4702)
# endif /* compiler*/

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::next_slash_or_end(ss_typename_param_k basic_path<C, T, A>::char_type const* p)
{
    for(; ; ++p)
    {
        switch(*p)
        {
            case    '/':
#ifdef _WIN32
            case    '\\':
#endif /* _WIN32 */
                ++p;
            case    '\0':
                return p;
            default:
                break;
        }
    }

    return NULL;
}

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER >= 1300
#  pragma warning(pop)
# endif /* compiler*/


template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type basic_path<C, T, A>::path_name_separator_alt()
{
    return '\\';
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline void basic_path<C, T, A>::swap(basic_path<C, T, A>& rhs)
{
    m_buffer.swap(rhs.m_buffer);
    std_swap(m_len, rhs.m_len);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline
ss_typename_param_k basic_path<C, T, A>::class_type&
basic_path<C, T, A>::concat_(
    ss_typename_param_k basic_path<C, T, A>::char_type const*   rhs
,   ss_typename_param_k basic_path<C, T, A>::size_type          cch
)
{
    traits_type::char_copy(&m_buffer[0] + m_len, rhs, cch);
    m_len += cch;
    m_buffer[m_len] = '\0';

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>::basic_path()
    : m_len(0)
{
    m_buffer[0] = '\0';
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* ss_explicit_k */ basic_path<C, T, A>::basic_path(ss_typename_type_k basic_path<C, T, A>::char_type const* path)
    : m_len(0)
{
    if(NULL != path)
    {
        size_type cch = traits_type::str_len(path);

        UNIXSTL_MESSAGE_ASSERT("path too long", cch < m_buffer.size());

        traits_type::char_copy(&m_buffer[0], path, cch);

        m_len = cch;
    }

    m_buffer[m_len] = '\0';
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>::basic_path( ss_typename_type_k basic_path<C, T, A>::char_type const* path
                                    ,   ss_typename_type_k basic_path<C, T, A>::size_type       cch)
    : m_len(cch)
{
    UNIXSTL_ASSERT((NULL != path) || (0 == cch));

    if(0 != cch)
    {
        UNIXSTL_MESSAGE_ASSERT("path too long", cch < m_buffer.size());

        traits_type::char_copy(&m_buffer[0], path, cch);
    }
    m_buffer[cch] = '\0';
}

#ifndef STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>::basic_path(basic_path<C, T, A> const& rhs)
    : m_len(rhs.m_len)
{
    traits_type::char_copy(&m_buffer[0], rhs.m_buffer.c_str(), rhs.m_len + 1); // +1 to get the NUL terminator
}
#endif /* !STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD */

#ifndef STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::operator =(basic_path<C, T, A> const& path)
{
    class_type  newPath(path);

    swap(newPath);

    return *this;
}
#endif /* !STLSOFT_CF_NO_COPY_CTOR_AND_COPY_CTOR_TEMPLATE_OVERLOAD */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::operator =(ss_typename_type_k basic_path<C, T, A>::char_type const* path)
{
    return operator_equal_(path);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::operator_equal_(ss_typename_type_k basic_path<C, T, A>::char_type const* path)
{
    class_type  newPath(path);

    swap(newPath);

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_type_ret_k basic_path<C, T, A>::class_type basic_path<C, T, A>::root(ss_typename_type_k basic_path<C, T, A>::char_type const* s)
{
    return class_type(s);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push(class_type const& rhs, us_bool_t bAddPathNameSeparator /* = false */)
{
    return push(rhs.c_str(), bAddPathNameSeparator);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push(char_type const* rhs, us_bool_t bAddPathNameSeparator /* = false */)
{
    UNIXSTL_ASSERT(NULL != rhs);

    if('\0' != *rhs)
    {
        if(traits_type::is_path_rooted(rhs))
        {
            class_type  newPath(rhs);

            swap(newPath);
        }
        else
        {
            class_type  newPath(*this);

            newPath.push_sep();
            newPath.concat_(rhs, traits_type::str_len(rhs));
            if(bAddPathNameSeparator)
            {
                newPath.push_sep();
            }

            swap(newPath);
        }
    }

    return *this;
}

#if 0
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push_ext(class_type const& rhs, us_bool_t bAddPathNameSeparator /* = false */)
{
}
#endif /* 0 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push_ext(char_type const* rhs, us_bool_t bAddPathNameSeparator /* = false */)
{
    UNIXSTL_ASSERT(NULL != rhs);

    class_type  newPath(*this);

    newPath.pop_sep();
    if('.' != *rhs)
    {
        static char_type const s_dot[] = { '.', '\0' };

        newPath.concat_(&s_dot[0], 1u);
    }
    newPath.concat_(rhs, traits_type::str_len(rhs));
    if(bAddPathNameSeparator)
    {
        newPath.push_sep();
    }

    swap(newPath);

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push_sep()
{
    if(0 != m_len)
    {
        if(traits_type::path_name_separator() != m_buffer[m_len - 1])
        {
#ifdef _WIN32
            if(path_name_separator_alt() != m_buffer[m_len - 1])
#endif /* _WIN32 */
            {
                UNIXSTL_ASSERT(m_len + 1 < m_buffer.size());

                m_buffer[m_len]     =   traits_type::path_name_separator();
                m_buffer[m_len + 1] =   '\0';
                ++m_len;
            }
        }
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop(us_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    char_type   *slash      =   traits_type::str_rchr(m_buffer.c_str(), traits_type::path_name_separator());
#ifdef _WIN32
    char_type   *slash_a    =   traits_type::str_rchr(m_buffer.c_str(), path_name_separator_alt());

    if(slash_a > slash)
    {
        slash = slash_a;
    }
#endif /* _WIN32 */

    if(NULL != slash)
    {
        *(slash + 1) = '\0';
        m_len = static_cast<size_type>((slash + 1) - m_buffer.c_str());
    }
    else
    {
        clear();
    }

    if(bRemoveTrailingPathNameSeparator)
    {
        this->pop_sep();
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop_sep()
{
    if(0 != m_len)
    {
        if( 1 == m_len &&
            traits_type::is_path_name_separator(m_buffer[0]))
        {
            // It's / or \ - ignore
        }
#ifdef _WIN32
        else if(3 == m_len &&
                ':' == m_buffer[1] &&
                traits_type::is_path_name_separator(m_buffer[2]))
        {
            // It's drive rooted - ignore
        }
#endif /* _WIN32 */
        else
        {
            char_type* last = &m_buffer[m_len - 1];

            if(*last == traits_type::path_name_separator())
            {
                m_buffer[m_len-- - 1] = '\0';
            }
#ifdef _WIN32
            else if(*last == path_name_separator_alt())
            {
                m_buffer[m_len-- - 1] = '\0';
            }
#endif /* _WIN32 */
        }
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop_ext()
{
    { for(us_size_t len = m_len; 0 != len; --len)
    {
        char_type* last = &m_buffer[len - 1];

        if(traits_type::is_path_name_separator(*last))
        {
            break;
        }
        else if('.' == *last)
        {
            m_len = len - 1;

            m_buffer[m_len] = '\0';

            break;
        }
    }}

    return *this;
}


#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED)

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::operator /=(basic_path<C, T, A> const& path)
{
    return push(path);
}

#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_OVERLOAD_DISCRIMINATED */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::operator /=(ss_typename_type_k basic_path<C, T, A>::char_type const* path)
{
    return push(path);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline void basic_path<C, T, A>::clear()
{
    m_buffer[0] =   '\0';
    m_len       =   0;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::make_absolute(us_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    if(0 != size())
    {
        buffer_type_    buffer;
        size_type       cch = traits_type::get_full_path_name(c_str(), buffer.size(), &buffer[0]);

        if(0 == cch)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(unix_exception("could not determine the absolute path", errno));
#else /* ?STLSOFT_CF_EXCEPTION_SUPPORT */
            return *this;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        class_type      newPath(buffer.c_str(), cch);

        if(bRemoveTrailingPathNameSeparator)
        {
            newPath.pop_sep();
        }

        swap(newPath);
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::canonicalise(us_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    class_type  newPath(*this);

#ifdef _DEBUG
    memset(&newPath.m_buffer[0], '~', newPath.m_buffer.size());
#endif /* _DEBUG */

    // Basically we scan through the path looking for ./ .\ ..\ and ../

#ifdef STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
    typedef ss_typename_type_k A::ss_template_qual_k rebind<part>::other    part_ator_type;
#else /* ? STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */
    typedef ss_typename_type_k allocator_selector<part>::allocator_type     part_ator_type;
#endif /* STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */

    typedef stlsoft_ns_qual(auto_buffer_old)<   part
                                            ,   part_ator_type
                                            >                               part_buffer_t;

    part_buffer_t       parts(this->length() / 2);  // Uncanonicalised directory parts
    char_type*          dest   =   &newPath.m_buffer[0];
    char_type const*    p1     =   this->c_str();
    char_type const*    p2;

    if(this->is_absolute())
    {
#ifdef _WIN32
        if(traits_type::is_path_UNC(this->c_str()))
        {
            UNIXSTL_ASSERT('\\' == m_buffer[0]);
            UNIXSTL_ASSERT('\\' == m_buffer[1]);
            UNIXSTL_ASSERT('\\' != m_buffer[2]);

            char_type const* slash0 = next_slash_or_end(&m_buffer[3]);
            char_type const* slash1 = next_slash_or_end(slash0);

            for(us_size_t i = 0, n = slash1 - &m_buffer[0]; i < n; ++i)
            {
                *dest++ = *p1++;
            }
        }
        else if( isalpha(m_buffer[0]) &&
            ':' == m_buffer[1])
        {
            // Copy over the drive letter, colon and slash
            *dest++ = *p1++;
            *dest++ = *p1++;
            *dest++ = *p1++;
        }
        else
#endif /* _WIN32 */
        {
            *dest++ = traits_type::path_name_separator();
            ++p1;
        }
    }

    // 1. Parse the path into an uncanonicalised sequence of directory parts
    {
        size_type   i   =   0;

        for(; '\0' != *p1; ++i)
        {
            p2 = next_slash_or_end(p1);

            parts[i].len    =   static_cast<size_type>(p2 - p1);
            parts[i].p      =   p1;
            parts[i].type   =   part::normal;
            switch(parts[i].len)
            {
                case    1:
                    if('.' == p1[0])
                    {
                        parts[i].type   =   part::dot;
                    }
                    break;
                case    2:
                    if('.' == p1[0])
                    {
                        if('.' == p1[1])
                        {
                            parts[i].type   =   part::dotdot;
                        }
                        else if(traits_type::path_name_separator() == p1[1])
                        {
                            parts[i].type   =   part::dot;
                        }
#ifdef _WIN32
                        else if(path_name_separator_alt() == p1[1])
                        {
                            parts[i].type   =   part::dot;
                        }
#endif /* _WIN32 */
                    }
                    break;
                case    3:
                    if( '.' == p1[0] &&
                        '.' == p1[1])
                    {
                        if(traits_type::path_name_separator() == p1[2])
                        {
                            parts[i].type   =   part::dotdot;
                        }
#ifdef _WIN32
                        else if(path_name_separator_alt() == p1[2])
                        {
                            parts[i].type   =   part::dotdot;
                        }
#endif /* _WIN32 */
                    }
                    break;
                default:
                    break;
            }

            p1 = p2;
        }

        parts.resize(i);
    }

    // 2. Process the parts into a canonicalised sequence
    {
        size_type   i   =   0;

        for(i = 0; i < parts.size(); ++i)
        {
            switch(parts[i].type)
            {
                case    part::dot:
                    parts[i].len = 0;
                    break;
                case    part::dotdot:
                    // Now need to track back and find a prior normal element
                    {
                        size_type   prior;

                        for(prior = i; ; )
                        {
                            if(0 == prior)
                            {
                                STLSOFT_THROW_X(unixstl_ns_qual_std(invalid_argument)("No prior part to \"..\" for path canonicalisation"));
                            }
                            else
                            {
                                --prior;

                                if( part::normal == parts[prior].type &&
                                    0 != parts[prior].len)
                                {
                                    parts[i].len = 0;
                                    parts[prior].len = 0;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case    part::normal:
                default:
                    break;
            }
        }
    }

    // 3. Write out all the parts back into the new path instance
    {
        size_type   i   =   0;

#ifdef _DEBUG
        memset(dest, '~', newPath.m_buffer.size() - (dest - &newPath.m_buffer[0]));
#endif /* _DEBUG */

        for(i = 0; i < parts.size(); ++i)
        {
            traits_type::char_copy(dest, parts[i].p, parts[i].len);

            dest += parts[i].len;
        }

        *dest = '\0';
        newPath.m_len = dest - newPath.c_str();
    }

    if(bRemoveTrailingPathNameSeparator)
    {
        newPath.pop_sep();
    }

    swap(newPath);

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::get_file() const
{
    char_type const* slash      =   traits_type::str_rchr(m_buffer.c_str(), traits_type::path_name_separator());
    char_type const* slash_a    =   traits_type::str_rchr(m_buffer.c_str(), path_name_separator_alt());

    if(slash_a > slash)
    {
        slash = slash_a;
    }

    if(NULL == slash)
    {
        slash = m_buffer.c_str();
    }
    else
    {
        ++slash;
    }

    return slash;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::get_ext() const
{
    char_type const         *dot    =   traits_type::str_rchr(this->c_str(), '.');
    char_type const         *file   =   get_file();
    static const char_type  s_empty[1]  =   { '\0' };

    if(NULL == dot)
    {
        return s_empty;
    }
    else if(dot < file)
    {
        return s_empty;
    }
    else
    {
        return dot + 1;
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::size_type basic_path<C, T, A>::length() const
{
    return m_len;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::size_type basic_path<C, T, A>::size() const
{
    return length();
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::size_type
/* static */ basic_path<C, T, A>::max_size()
{
    return buffer_type_::max_size() - 1u;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::bool_type basic_path<C, T, A>::empty() const
{
    return 0 == size();
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::c_str() const
{
    return m_buffer.c_str();
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::char_type const& basic_path<C, T, A>::operator [](ss_typename_type_k basic_path<C, T, A>::size_type index) const
{
    UNIXSTL_MESSAGE_ASSERT("Index out of range", !(size() < index));

    return c_str()[index];
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::exists() const
{
    return traits_type::file_exists(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::is_rooted() const
{
    return traits_type::is_path_rooted(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::is_absolute() const
{
    return traits_type::is_path_absolute(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::has_sep() const
{
    return this->empty() ? false : traits_type::has_dir_end(this->c_str() + (this->size() - 1));
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ss_typename_type_ret_k basic_path<C, T, A>::size_type basic_path<C, T, A>::copy(ss_typename_type_k basic_path<C, T, A>::char_type *buffer, ss_typename_type_k basic_path<C, T, A>::size_type cchBuffer) const
{
    return stlsoft_ns_qual(copy_contents)(buffer, cchBuffer, m_buffer.data(), m_len);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::equivalent(basic_path<C, T, A> const& rhs) const
{
    return equivalent(rhs.c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::equivalent(ss_typename_type_k basic_path<C, T, A>::char_type const* rhs) const
{
    class_type  lhs_(*this);
    class_type  rhs_(rhs);

    return lhs_.make_absolute(false).canonicalise(true) == rhs_.make_absolute(false).canonicalise(true);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::equal(basic_path<C, T, A> const& rhs) const
{
    return equal(rhs.c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline us_bool_t basic_path<C, T, A>::equal(ss_typename_type_k basic_path<C, T, A>::char_type const* rhs) const
{
    return 0 == traits_type::str_compare(m_buffer.c_str(), stlsoft_ns_qual(c_str_ptr)(rhs));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* In the special case of Intel behaving as VC++ 7.0 or earlier on Win32, we
 * illegally insert into the std namespace.
 */
#if defined(STLSOFT_CF_std_NAMESPACE)
# if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
         defined(_MSC_VER))) && \
     _MSC_VER < 1310
namespace std
{
    template<   ss_typename_param_k C
            ,   ss_typename_param_k T
            ,   ss_typename_param_k A
            >
    inline void swap(unixstl_ns_qual(basic_path)<C, T, A>& lhs, unixstl_ns_qual(basic_path)<C, T, A>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

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

#endif /* UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH */

/* ///////////////////////////// end of file //////////////////////////// */
