/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/path.hpp
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


/** \file winstl/filesystem/path.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_path class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_PATH
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_PATH

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_PATH_MAJOR    6
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_PATH_MINOR    6
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_PATH_REVISION 20
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_PATH_EDIT     261
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
#  include <winstl/error/exceptions.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES
# include <stlsoft/memory/allocator_features.hpp>   // for STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES */
#ifndef STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS
# include <stlsoft/string/copy_functions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                            // for std::logic_error
#endif /* !STLSOFT_INCL_STDEXCEPT */

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
 * basic_path
 *
 * This class represents a path, and effectively acts as a C-string of its value.
 */

/** Class used for composing and decomposing file-system paths.
 *
 * \ingroup group__library__filesystem
 *
 * \param C The character type
 * \param T The traits type. On translators that support default template arguments, this defaults to filesystem_traits<C>
 * \param A The allocator type. On translators that support default template arguments, this defaults to processheap_allocator<C>
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
        ,   ss_typename_param_k A = processheap_allocator<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
        ,   ss_typename_param_k A /* = processheap_allocator<C> */
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
    typedef ws_size_t                   size_type;
    /// The Boolean type
    typedef ws_bool_t                   bool_type;

// TODO: Use the slice string, and provide iterators over the directory parts

/// @}

/// \name Construction
/// @{
public:
    /// Constructs an empty path.
    ///
    /// \code
    /// winstl::path  p;
    ///
    /// assert(0 == p.size());
    /// assert("" == p);
    /// \endcode
    basic_path();
    /// Constructs a path from a nul-terminated character string.
    ///
    /// \code
    /// winstl::path  p("C:\\Windows");
    ///
    /// assert(10 == p.size());
    /// assert("C:\\Windows" == p);
    /// assert("C:\\WINDOWS" == p);
    /// assert("c:\\windows" == p);
    /// \endcode
    ///
    /// \pre len(path) <= size()
    ss_explicit_k basic_path(char_type const* path);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    /// Constructs a path from a string object.
    ///
    /// \remarks The string object may be any type for which the
    ///  <b>c_str_len</b> and <b>c_str_data</b>
    ///  \ref group__concept__shim__string_access "String Access Shims" are
    ///  defined. The following example shows the use of a window handle
    ///  (HWND). Other types supported are <code>std::exception</code>,
    ///  <code>std::string</code>, <code>stlsoft::simple_string</code>,
    ///  <code>winstl::error_desc</code>, and many more
    ///
    /// \code
    /// HWND          hwndParent = . . .
    /// HWND          hwnd       = winstl::CreateEdit(hwndParent, "C:\\Windows", ES_LEFT, 0, 0, 10, 10, 1001);
    /// winstl::path  p(hwnd);
    ///
    /// assert(10 == p.size());
    /// assert("C:\\Windows" == p);
    /// assert("C:\\WINDOWS" == p);
    /// assert("c:\\windows" == p);
    /// \endcode
    template<ss_typename_param_k S>
    ss_explicit_k basic_path(S const& s)
    {
        m_len = stlsoft_ns_qual(c_str_len)(s);

        traits_type::char_copy(&m_buffer[0], stlsoft_ns_qual(c_str_data)(s), m_len);
        m_buffer[m_len] = '\0';
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    /// Constructs a path from a slice of a character string.
    ///
    /// \endcode
    /// winstl::path  p("C:\\Windows\\system32", 10);
    ///
    /// assert(10 == p.size());
    /// assert("C:\\Windows" == p);
    /// assert("C:\\WINDOWS" == p);
    /// assert("c:\\windows" == p);
    /// \endcode
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
    ///
    /// \remarks If the path currently contains path name separators, and
    ///   they are all forward slashes '/', then a forward slash will be
    ///   used; otherwise a backslash '\\' will be used.
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
    ///
    /// \note This method is idempotent.
    class_type& pop_sep() stlsoft_throw_0();
    /// Removes the extension, if any, from the file component of the path
    ///
    /// \note This method is idempotent.
    class_type& pop_ext() stlsoft_throw_0();

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
    /// Canonicalises the path
    ///
    /// Canonicalises the path, removing all "./" parts and evaluating all
    /// "../" parts. Any path with only one part will not be canonicalised.
    /// A leading '.' will be preserved if no other '..' or "normal" parts
    /// are contained.
    ///
    /// \param bRemoveTrailingPathNameSeparator Removes any trailing
    ///   separator, even if no other changes have been made.
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
    char_type const*  get_file() const;
    /// Returns a pointer to the extension, or to the empty string if there is no extension
    char_type const*  get_ext() const;
    /// Returns the length of the converted path
    size_type         length() const;
    /// Returns the length of the converted path
    ///
    /// \remarks Equivalent to length()
    size_type         size() const;
    /// The maximum possible length of a path
    static size_type  max_size();
    /// Determines whether the path is empty
    bool_type         empty() const;
    /// Conversion to a non-mutable (const) pointer to the path
    char_type const*  c_str() const;
    /// Returns a non-mutable (const) reference to the character at
    ///  the given index
    ///
    /// \note The behaviour is undefined if <code>index >= size()</code>.
    char_type const&  operator [](size_type index) const;
    /// Indicates whether the path represents an existing file system entry
    bool_type         exists() const;
    /// Indicates whether the path is rooted
    bool_type         is_rooted() const;
    /// Indicates whether the path is absolute
    bool_type         is_absolute() const;
    /// Indicates whether the path has a trailing separator
    bool_type         has_sep() const;

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
    /// Determines whether the instance holds a string that refers to
    ///  the same file-system entity as the given string.
    ///
    /// \htmlonly
    /// <pre>
    /// winstl::path  p("C:\\Windows\\system32\\..");
    ///
    /// assert(p != "C:\\WINDOWS\\");
    /// assert(!p.equal("C:\\WINDOWS\\"));
    /// assert(p.equivalent("C:\\WINDOWS\\"));
    /// </pre>
    /// \endhtmlonly
    bool_type equivalent(char_type const* rhs) const;

    /// Evaluates whether the two instances hold strings that refer
    ///  to the same file-system entity.
    ///
    /// \remarks See \link winstl::basic_path::equivalent(C const* ) equivalent()\endlink for an example.
    ///
    /// \note The string comparison is case-insensitive.
    bool_type equivalent(class_type const& rhs) const;

    /// Evaluates whether the instance holds an identical string.
    ///
    /// \note The string comparison is case-insensitive.
    bool_type equal(char_type const* rhs) const;
    /// Evaluates whether the two instances hold identical strings.
    ///
    /// \note The string comparison is case-insensitive.
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

    class_type&             push_(char_type const* rhs, size_type cch, bool_type bAddPathNameSeparator);
    class_type&             push_sep_(char_type sep);
    void                    swap(class_type& rhs);
    class_type&             concat_(char_type const* rhs, size_type cch);

    bool_type               has_dir_end_() const;

    static char_type const* last_slash_(char_type const* buffer, size_type len);

    static char_type const* next_slash_or_end(char_type const* p);
    static char_type const* next_part_or_end(char_type const* p);
    static char_type        path_name_separator();
    static char_type        path_name_separator_alt();

// Member Types
private:
    typedef basic_file_path_buffer< char_type
                                ,   allocator_type
                                >                   buffer_type_;

    struct part_type
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

#ifdef STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT
    typedef ss_typename_type_k A::ss_template_qual_k rebind<part_type>::other   part_ator_type_;
#else /* ? STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */
# ifdef WIN32
    typedef ss_typename_type_k processheap_allocator<part_type>                 part_ator_type_;
# else /* ? OS */
    typedef ss_typename_type_k allocator_selector<part_type>::allocator_type    part_ator_type_;
# endif /* OS */
#endif /* STLSOFT_LF_ALLOCATOR_REBIND_SUPPORT */

    typedef stlsoft_ns_qual(auto_buffer_old)<   part_type
                                            ,   part_ator_type_
# ifdef WIN32
                                            ,   WINSTL_CONST_MAX_PATH / 2
# endif /* OS */
                                            >                                   part_buffer_type_;


    static size_type coalesce_parts_(part_buffer_type_& parts);

// Member Variables
private:
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
typedef basic_path<ws_char_a_t, filesystem_traits<ws_char_a_t> >       path_a;
/** Specialisation of the basic_path template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_path<ws_char_w_t, filesystem_traits<ws_char_w_t> >       path_w;
/** Specialisation of the basic_path template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_path<TCHAR, filesystem_traits<TCHAR> >                   path;

/* /////////////////////////////////////////////////////////////////////////
 * Support for PlatformSTL redefinition by inheritance+namespace, for confused
 * compilers (e.g. VC++ 6)
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

    template<   ss_typename_param_k C
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
            ,   ss_typename_param_k T = filesystem_traits<C>
            ,   ss_typename_param_k A = processheap_allocator<C>
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            ,   ss_typename_param_k T /* = filesystem_traits<C> */
            ,   ss_typename_param_k A /* = processheap_allocator<C> */
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            >
    class basic_path__
        : public winstl_ns_qual(basic_path)<C, T, A>
    {
    private:
        typedef winstl_ns_qual(basic_path)<C, T, A>                     parent_class_type;
        typedef winstl_ns_qual(basic_path__)<C, T, A>                   class_type;
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
inline ws_bool_t operator ==(basic_path<C, T, A> const& lhs, ss_typename_type_k basic_path<C, T, A>::char_type const* rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t operator !=(basic_path<C, T, A> const& lhs, ss_typename_type_k basic_path<C, T, A>::char_type const* rhs)
{
    return !lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t operator ==(ss_typename_type_k basic_path<C, T, A>::char_type const* lhs, basic_path<C, T, A> const& rhs)
{
    return rhs.equal(lhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t operator !=(ss_typename_type_k basic_path<C, T, A>::char_type const* lhs, basic_path<C, T, A> const& rhs)
{
    return !rhs.equal(lhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t operator ==(basic_path<C, T, A> const& lhs, basic_path<C, T, A> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t operator !=(basic_path<C, T, A> const& lhs, basic_path<C, T, A> const& rhs)
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

/** \ref group__concept__shim__string_access__c_str_data for winstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_data(winstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_a_t const* c_str_data_a(winstl_ns_qual(basic_path)<ws_char_a_t, T, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_w_t const* c_str_data_w(winstl_ns_qual(basic_path)<ws_char_w_t, T, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_len for winstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_size_t c_str_len(winstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_size_t c_str_len_a(winstl_ns_qual(basic_path)<ws_char_a_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_a)(b.c_str());
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_size_t c_str_len_w(winstl_ns_qual(basic_path)<ws_char_w_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \ref group__concept__shim__string_access__c_str_ptr for winstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr(winstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_a_t const* c_str_ptr_a(winstl_ns_qual(basic_path)<ws_char_a_t, T, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_w_t const* c_str_ptr_w(winstl_ns_qual(basic_path)<ws_char_w_t, T, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \ref group__concept__shim__string_access__c_str_ptr_null for winstl::basic_path
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr_null(winstl_ns_qual(basic_path)<C, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_a_t const* c_str_ptr_null_a(winstl_ns_qual(basic_path)<ws_char_a_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_a)(b.c_str());
}
template <ss_typename_param_k T, ss_typename_param_k A>
inline ws_char_w_t const* c_str_ptr_null_w(winstl_ns_qual(basic_path)<ws_char_w_t, T, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */





/** \ref group__concept__shim__stream_insertion "stream insertion shim" for winstl::basic_path
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline S& operator <<(S& s, winstl_ns_qual(basic_path)<C, T, A> const& b)
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

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::bool_type
basic_path<C, T, A>::has_dir_end_() const
{
    return empty() ? false : traits_type::has_dir_end(m_buffer.c_str() + (m_len - 1));
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type const*
basic_path<C, T, A>::last_slash_(
    ss_typename_param_k basic_path<C, T, A>::char_type const*   buffer
,   ss_typename_param_k basic_path<C, T, A>::size_type          /* len */
)
{
    char_type*  slash   =   traits_type::str_rchr(buffer, path_name_separator());
    char_type*  slash_a =   traits_type::str_rchr(buffer, path_name_separator_alt());

    if(slash_a > slash)
    {
        slash = slash_a;
    }

    return slash;
}


template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::next_slash_or_end(ss_typename_param_k basic_path<C, T, A>::char_type const* p)
{
    for(; ; )
    {
        switch(*p)
        {
            case    '/':
            case    '\\':
            case    '\0':
                return p;
            default:
                ++p;
                break;
        }
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type const* basic_path<C, T, A>::next_part_or_end(ss_typename_param_k basic_path<C, T, A>::char_type const* p)
{
    for(; ; )
    {
        switch(*p)
        {
            case    '/':
            case    '\\':
                ++p;
            case    '\0':
                return p;
            default:
                ++p;
                break;
        }
    }
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type basic_path<C, T, A>::path_name_separator_alt()
{
    return '/';
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ ss_typename_param_k basic_path<C, T, A>::char_type
basic_path<C, T, A>::path_name_separator()
{
    WINSTL_ASSERT('\\' == traits_type::path_name_separator());

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
inline /* static */ ss_typename_param_k basic_path<C, T, A>::size_type basic_path<C, T, A>::coalesce_parts_(ss_typename_param_k basic_path<C, T, A>::part_buffer_type_& parts)
{
    ss_typename_param_k part_buffer_type_::iterator src     =   parts.begin();
    ss_typename_param_k part_buffer_type_::iterator dest    =   parts.begin();

    { for(size_type i = 0; i < parts.size(); ++i, ++src)
    {
        if(0 == parts[i].len)
        {
            ; // Skip/overwrite this element
        }
        else
        {
            if(dest != src)
            {
                *dest = *src;
            }

            ++dest;
        }
    }}

    size_type   n = static_cast<size_type>(dest - parts.begin());

    parts.resize(n);

    return n;
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

        WINSTL_MESSAGE_ASSERT("path too long", cch < m_buffer.size());

        traits_type::char_copy(&m_buffer[0], path, cch);

        m_len = cch;
    }

    m_buffer[m_len] = '\0';
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>::basic_path(ss_typename_type_k basic_path<C, T, A>::char_type const* path, ss_typename_type_k basic_path<C, T, A>::size_type cch)
    : m_len(cch)
{
    WINSTL_ASSERT((NULL != path) || (0 == cch));

    if(0 != cch)
    {
        WINSTL_MESSAGE_ASSERT("path too long", cch < m_buffer.size());

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
inline basic_path<C, T, A>& basic_path<C, T, A>::push(class_type const& rhs, ws_bool_t bAddPathNameSeparator /* = false */)
{
    // TODO: Change to use data(), when push_() properly implemented
    return push_(rhs.c_str(), rhs.size(), bAddPathNameSeparator);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push(char_type const* rhs, ws_bool_t bAddPathNameSeparator /* = false */)
{
    WINSTL_ASSERT(NULL != rhs);

    return push_(rhs, traits_type::str_len(rhs), bAddPathNameSeparator);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>&
basic_path<C, T, A>::push_(
    char_type const*    rhs
,   size_type           cch
,   ws_bool_t           bAddPathNameSeparator
)
{
    if(0 != cch)
    {
        if(traits_type::is_path_rooted(rhs))
        {
            class_type newPath(rhs, cch);

            if( bAddPathNameSeparator &&
                !newPath.has_dir_end_())
            {
                newPath.push_sep();
            }

            swap(newPath);
        }
        else
        {
            WINSTL_MESSAGE_ASSERT("path too long", size() + 1 + cch < m_buffer.size());

            // In an attempt to maintain slash/backslash consistency, we
            // locate the next slash to help guide the push_sep_() method.

            class_type          newPath(*this);
            char_type const*    psep    =   next_slash_or_end(c_str());
            char_type           sep     =   ('\0' == *psep) ? char_type(0) : psep[0];

            newPath.push_sep_(sep);
            newPath.concat_(rhs, cch);
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
inline basic_path<C, T, A>& basic_path<C, T, A>::push_ext(class_type const& rhs, ws_bool_t bAddPathNameSeparator /* = false */)
{
}
#endif /* 0 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push_ext(char_type const* rhs, ws_bool_t bAddPathNameSeparator /* = false */)
{
    WINSTL_ASSERT(NULL != rhs);

    class_type  newPath(*this);

    newPath.pop_sep();
    if('.' != *rhs)
    {
        static char_type const s_dot[] = { '.', '\0' };

        newPath.concat_(s_dot, 1u);
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
    char_type   sep = path_name_separator();

    char_type*  slash   =   traits_type::str_chr(m_buffer.c_str(), path_name_separator());
    char_type*  slash_a =   traits_type::str_chr(m_buffer.c_str(), path_name_separator_alt());

    if( NULL == slash &&
        NULL != slash_a)
    {
        sep = path_name_separator_alt();
    }

    return push_sep_(sep);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::push_sep_(ss_typename_type_k basic_path<C, T, A>::char_type sep)
{
    if(0 == sep)
    {
        sep = path_name_separator();
    }

    WINSTL_MESSAGE_ASSERT("You can only push a path name separator character recognised by your operating system!", traits_type::is_path_name_separator(sep));

    if(0 != m_len)
    {
        char_type&  last = m_buffer[m_len - 1];

        if(traits_type::is_path_name_separator(last))
        {
            if(last != sep)
            {
                last = sep;
            }
        }
        else
        {
            WINSTL_ASSERT(m_len + 1 < m_buffer.size());

            m_buffer[m_len]     =   sep;
            m_buffer[m_len + 1] =   '\0';
            ++m_len;
        }
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop(ws_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    char_type* slash = const_cast<char_type*>(last_slash_(m_buffer.data(), m_len));

    if(NULL != slash)
    {
        if(static_cast<size_type>(slash - m_buffer.data()) + 1 == m_len)
        {
            bool shouldRemoveTrailingSlash = true;

            // The last slash is just a trailing separator
            //
            // Is it just a volume, or just a UNC, or just a root slash
            if(traits_type::is_path_rooted(m_buffer.c_str()))
            {
                if(traits_type::is_path_UNC(m_buffer.c_str()))
                {
                    char_type const* share = next_part_or_end(m_buffer.c_str() + 2);

                    if(NULL == share)
                    {
                        shouldRemoveTrailingSlash = false;
                    }
                }
                else if(traits_type::is_path_absolute(m_buffer.c_str()))
                {
                    if(3 == m_len)
                    {
                        shouldRemoveTrailingSlash = false;
                    }
                }
                else
                {
                    if(1 == m_len)
                    {
                        shouldRemoveTrailingSlash = false;
                    }
                }
            }

            if(shouldRemoveTrailingSlash)
            {
                m_buffer[--m_len] = '\0';

                slash = const_cast<char_type*>(last_slash_(m_buffer.data(), m_len));
            }
        }
    }

    if(NULL != slash)
    {
        if(traits_type::is_path_UNC(m_buffer.c_str()))
        {
            char_type const* shareSlash = next_slash_or_end(m_buffer.c_str() + 2);

            if(shareSlash == slash)
            {
                slash = NULL;
            }
        }
        else if(traits_type::is_path_absolute(m_buffer.c_str()) &&
                3 == m_len)
        {
            slash = NULL;
        }
        else if(traits_type::is_path_rooted(m_buffer.c_str()) &&
                1 == m_len)
        {
            slash = NULL;
        }
    }

    if(NULL != slash)
    {
        *(slash + 1) = '\0';
        m_len = static_cast<size_type>((slash + 1) - m_buffer.c_str());

        if(bRemoveTrailingPathNameSeparator)
        {
            this->pop_sep();
        }
    }

    if(NULL == slash)
    {
        clear();
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop_sep() stlsoft_throw_0()
{
    if(0 != m_len)
    {
        if( 1 == m_len &&
            traits_type::is_path_name_separator(m_buffer[0]))
        {
            // It's / or \ - ignore
        }
        else if(3 == m_len &&
                ':' == m_buffer[1] &&
                traits_type::is_path_name_separator(m_buffer[2]))
        {
            // It's drive rooted - ignore
        }
        else
        {
            // We can pop a separator off anything else, including a UNC host
            char_type* last = &m_buffer[m_len - 1];

            if(*last == path_name_separator())
            {
                m_buffer[m_len-- - 1] = '\0';
            }
            else if(*last == path_name_separator_alt())
            {
                m_buffer[m_len-- - 1] = '\0';
            }
        }
    }

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline basic_path<C, T, A>& basic_path<C, T, A>::pop_ext() stlsoft_throw_0()
{
    { for(ws_size_t len = m_len; 0 != len; --len)
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
inline basic_path<C, T, A>& basic_path<C, T, A>::make_absolute(ws_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    if(0 != size())
    {
        buffer_type_    buffer;
        size_type       cch = traits_type::get_full_path_name(c_str(), buffer.size(), &buffer[0]);

        if(0 == cch)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(windows_exception("could not determine the absolute path", ::GetLastError()));
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
inline basic_path<C, T, A>& basic_path<C, T, A>::canonicalise(ws_bool_t bRemoveTrailingPathNameSeparator /* = true */)
{
    if(0 == size())
    {
        return *this;
    }

    class_type  newPath(*this);

#ifdef _DEBUG
    memset(&newPath.m_buffer[0], '~', newPath.m_buffer.size());
#endif /* _DEBUG */

    // Basically we scan through the path looking for ./ .\ ..\ and ../

    // 0. Handle special path prefixes

    part_buffer_type_   parts(this->length() / 2);  // Uncanonicalised directory parts
    char_type*          dest   =   &newPath.m_buffer[0];
    char_type const*    p1     =   this->c_str();
    char_type const*    p2;

    if(traits_type::is_path_UNC(this->c_str()))
    {
        WINSTL_ASSERT('\\' == m_buffer[0]);
        WINSTL_ASSERT('\\' == m_buffer[1]);
        WINSTL_ASSERT('\\' != m_buffer[2]);

        char_type const* slash0 =   next_slash_or_end(&m_buffer[3]);
        char_type const* slash1 =   next_slash_or_end(slash0);

        for(ws_size_t i = 0, n = static_cast<ws_size_t>(slash1 - &m_buffer[0]); i < n; ++i)
        {
            *dest++ = *p1++;
        }
    }
    else if(this->is_absolute())
    {
        // Copy over the drive letter, colon and slash
        *dest++ = *p1++;
        *dest++ = *p1++;
        *dest++ = *p1++;
    }
    else if(this->is_rooted())
    {
        *dest++ = path_name_separator();
        ++p1;
    }

    // 1. Parse the path into an uncanonicalised sequence of directory parts
    {
        size_type   i   =   0;

        for(; '\0' != *p1; ++i)
        {
            p2 = next_part_or_end(p1);

            parts[i].len    =   static_cast<size_type>(p2 - p1);
            parts[i].p      =   p1;
            parts[i].type   =   part_type::normal;
            switch(parts[i].len)
            {
                case    1:
                    if('.' == p1[0])
                    {
                        parts[i].type   =   part_type::dot;
                    }
                    break;
                case    2:
                    if('.' == p1[0])
                    {
                        if('.' == p1[1])
                        {
                            parts[i].type   =   part_type::dotdot;
                        }
                        else if(path_name_separator() == p1[1])
                        {
                            parts[i].type   =   part_type::dot;
                        }
                        else if(path_name_separator_alt() == p1[1])
                        {
                            parts[i].type   =   part_type::dot;
                        }
                    }
                    break;
                case    3:
                    if( '.' == p1[0] &&
                        '.' == p1[1])
                    {
                        if(path_name_separator() == p1[2])
                        {
                            parts[i].type   =   part_type::dotdot;
                        }
                        else if(path_name_separator_alt() == p1[2])
                        {
                            parts[i].type   =   part_type::dotdot;
                        }
                    }
                    break;
                default:
                    break;
            }

            p1 = p2;
        }

        parts.resize(i);
    }

    // 2.a Remove all '.' parts
    { for(size_type i = 0; i < parts.size(); ++i)
    {
        WINSTL_ASSERT(0 != parts[i].len);

        part_type&  part = parts[i];

        if(part_type::dot == part.type)
        {
            part.len = 0;
        }
    }}

    coalesce_parts_(parts);

    // 2.b Process the '..' parts
    { for(size_type i = 1; i < parts.size(); ++i)
    {
        WINSTL_ASSERT(0 != parts[i].len);

        part_type&  part = parts[i];

        if(part_type::dotdot == part.type)
        {
            { for(size_type prior = i; ; )
            {
                if(0 == prior)
                {
                    break;
                }
                else
                {
                    --prior;

                    if(0 != parts[prior].len)
                    {
                        if(part_type::normal == parts[prior].type)
                        {
                            part.len = 0;
                            parts[prior].len = 0;
                            break;
                        }
                    }
                }
            }}
        }
    }}

    coalesce_parts_(parts);

    // 2.c "insert" a '.' if we've removed everything.
    if( !this->is_rooted() &&
        parts.empty())
    {
        static const char_type  s_dot[] = { '.', '/' };

        parts.resize(1);
        parts[0].type   =   part_type::dot;
        parts[0].len    =   1;
        parts[0].p      =   s_dot;
    }

    // 3. Write out all the parts back into the new path instance
    {
#ifdef _DEBUG
        ::memset(dest, '~', newPath.m_buffer.size() - (dest - &newPath.m_buffer[0]));
#endif /* _DEBUG */

        for(size_type i = 0; i < parts.size(); ++i)
        {
            traits_type::char_copy(dest, parts[i].p, parts[i].len);

            dest += parts[i].len;
        }

        *dest = '\0';
        newPath.m_len = static_cast<size_type>(dest - newPath.c_str());
    }

    // Now we determine whether to leave a trailing separator or
    // not and, if so, what type it should be.

    WINSTL_ASSERT(m_len > 0);

    char_type last = m_buffer[m_len - 1];

    if( !bRemoveTrailingPathNameSeparator &&
        traits_type::is_path_name_separator(last))
    {
        newPath.push_sep_(last);
    }
    else
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
    char_type const* slash = last_slash_(m_buffer.data(), m_len);

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
    WINSTL_MESSAGE_ASSERT("Index out of range", !(size() < index));

    return c_str()[index];
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::exists() const
{
    return traits_type::file_exists(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::is_rooted() const
{
    return traits_type::is_path_rooted(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::is_absolute() const
{
    return traits_type::is_path_absolute(this->c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::has_sep() const
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
inline ws_bool_t basic_path<C, T, A>::equivalent(basic_path<C, T, A> const& rhs) const
{
    return equivalent(rhs.c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::equivalent(ss_typename_type_k basic_path<C, T, A>::char_type const* rhs) const
{
    class_type  lhs_(*this);
    class_type  rhs_(rhs);

    return lhs_.make_absolute(false).canonicalise(true) == rhs_.make_absolute(false).canonicalise(true);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::equal(basic_path<C, T, A> const& rhs) const
{
    return equal(rhs.c_str());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline ws_bool_t basic_path<C, T, A>::equal(ss_typename_type_k basic_path<C, T, A>::char_type const* rhs) const
{
    return 0 == traits_type::str_compare_no_case(m_buffer.c_str(), stlsoft_ns_qual(c_str_ptr)(rhs));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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
    inline void swap(winstl_ns_qual(basic_path)<C, T, A>& lhs, winstl_ns_qual(basic_path)<C, T, A>& rhs)
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

#ifndef _WINSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::winstl::c_str_data;
using ::winstl::c_str_data_a;
using ::winstl::c_str_data_w;

using ::winstl::c_str_len;
using ::winstl::c_str_len_a;
using ::winstl::c_str_len_w;

using ::winstl::c_str_ptr;
using ::winstl::c_str_ptr_a;
using ::winstl::c_str_ptr_w;

using ::winstl::c_str_ptr_null;
using ::winstl::c_str_ptr_null_a;
using ::winstl::c_str_ptr_null_w;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_PATH */

/* ///////////////////////////// end of file //////////////////////////// */
