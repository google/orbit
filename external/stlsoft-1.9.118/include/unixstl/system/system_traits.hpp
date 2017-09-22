/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/system/system_traits.hpp
 *
 * Purpose:     Contains the system_traits template class, and ANSI and
 *              Unicode specialisations thereof.
 *
 * Created:     15th November 2002
 * Updated:     10th September 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2011, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file unixstl/system/system_traits.hpp
 *
 * \brief [C++ only] Definition of the unixstl::system_traits traits
 *  class
 *   (\ref group__library__system "System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS
#define UNIXSTL_INCL_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS_MAJOR     5
# define UNIXSTL_VER_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS_MINOR     4
# define UNIXSTL_VER_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS_REVISION  1
# define UNIXSTL_VER_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS_EDIT      111
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#if defined(_WIN32) || \
    defined(_WIN64)
# include <ctype.h>
#endif /* Windows */
#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */
#ifndef STLSOFT_INCL_H_FCNTL
# define STLSOFT_INCL_H_FCNTL
# include <fcntl.h>
#endif /* !STLSOFT_INCL_H_FCNTL */
#if defined(_WIN32) || \
    defined(_WIN64)
# include <io.h>
# if defined(STLSOFT_COMPILER_IS_INTEL) || \
     defined(STLSOFT_COMPILER_IS_MSVC)
#  include <direct.h>
# endif /* os && compiler */
#endif /* Windows */
#ifndef STLSOFT_INCL_H_DLFCN
# define STLSOFT_INCL_H_DLFCN
# include <dlfcn.h>
#endif /* !STLSOFT_INCL_H_DLFCN */
#ifndef STLSOFT_INCL_H_LIMITS
# define STLSOFT_INCL_H_LIMITS
# include <limits.h>
#endif /* !STLSOFT_INCL_H_LIMITS */
#ifndef STLSOFT_INCL_H_STDIO
# define STLSOFT_INCL_H_STDIO
# include <stdio.h>
#endif /* !STLSOFT_INCL_H_STDIO */
#ifndef STLSOFT_INCL_H_STDLIB
# define STLSOFT_INCL_H_STDLIB
# include <stdlib.h>
#endif /* !STLSOFT_INCL_H_STDLIB */
#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */
#ifndef STLSOFT_INCL_H_UNISTD
# define STLSOFT_INCL_H_UNISTD
# include <unistd.h>
#endif /* !STLSOFT_INCL_H_UNISTD */
#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */
#ifndef STLSOFT_INCL_SYS_H_TYPES
# define STLSOFT_INCL_SYS_H_TYPES
# include <sys/types.h>
#endif /* !STLSOFT_INCL_SYS_H_TYPES */
#ifndef STLSOFT_INCL_SYS_H_STAT
# define STLSOFT_INCL_SYS_H_STAT
# include <sys/stat.h>
#endif /* !STLSOFT_INCL_SYS_H_STAT */

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
 * Classes
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** Traits for accessing the correct system functions for a given
 *   character type.
 *
 * \ingroup group__library__system
 *
 * system_traits is a traits class for determining the correct system
 * structures and functions for a given character type.
 *
 * \param C The character type (e.g. \c char, \c wchar_t)
 */
template <ss_typename_param_k C>
struct system_traits
{
/// \name Types
/// @{
public:
    /// The character type
    typedef C                                       char_type;
    /// The size type
    typedef us_size_t                               size_type;
    /// The difference type
    typedef us_ptrdiff_t                            difference_type;
    /// The current instantion of the type
    typedef system_traits<C>                        class_type;
    /// The (signed) integer type
    typedef us_int_t                                int_type;
    /// The Boolean type
    typedef us_bool_t                               bool_type;
    /// The type of a handle to a dynamically loaded module
    typedef void*                                   module_type;
    /// The type of a handle to a kernel object
    typedef int                                     handle_type;
    /// The type of system result codes
    typedef int                                     result_code_type;
    /// The type of system error codes
    typedef int                                     error_type;
/// @}

/// \name General string handling
/// @{
public:
    /// Copies a specific number of characters from the source to the destination
    static char_type*   char_copy(char_type* dest, char_type const* src, size_type n);
#if !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS) || \
    defined(_CRT_SECURE_NO_DEPRECATE)
    /// Copies the contents of \c src to \c dest
    static char_type*   str_copy(char_type* dest, char_type const* src);
    /// Copies the contents of \c src to \c dest, up to cch \c characters
    static char_type*   str_n_copy(char_type* dest, char_type const* src, size_type cch);
    /// Appends the contents of \c src to \c dest
    static char_type*   str_cat(char_type* dest, char_type const* src);
    /// Appends the contents of \c src to \c dest, up to cch \c characters
    static char_type*   str_n_cat(char_type* dest, char_type const* src, size_type cch);
#endif /* !STLSOFT_USING_SAFE_STR_FUNCTIONS || _CRT_SECURE_NO_DEPRECATE */
    /// Comparies the contents of \c src and \c dest
    static int_type     str_compare(char_type const* s1, char_type const* s2);
    /// Comparies the contents of \c src and \c dest in a case-insensitive fashion
    static int_type     str_compare_no_case(char_type const* s1, char_type const* s2);
    /// Comparies the contents of \c src and \c dest up to \c cch characters
    static int_type     str_n_compare(char_type const* s1, char_type const* s2, size_type cch);
    /// Comparies the contents of \c src and \c dest up to \c cch characters
    static int_type     str_n_compare_no_case(char_type const* s1, char_type const* s2, size_type cch);
    /// Evaluates the length of \c src
    static size_type    str_len(char_type const* src);
    /// Finds the given character \c ch in \c s
    static char_type*   str_chr(char_type const* s, char_type ch);
    /// Finds the rightmost instance \c ch in \c s
    static char_type*   str_rchr(char_type const* s, char_type ch);
    /// Finds the given substring \c sub in \c s
    static char_type*   str_str(char_type const* s, char_type const* sub);
    /// Finds one of a set of characters in \c s
    static char_type*   str_pbrk(char_type const* s, char_type const* charSet);
    /// Returns a pointer to the end of the string
    static char_type*   str_end(char_type const* s);
    /// Sets each character in \c s to the character \c c
    ///
    /// \return s + n
    static char_type*   str_set(char_type* s, size_type n, char_type c);
/// @}

/// \name Dynamic Loading
/// @{
public:
    /// Loads the given executable module
    static module_type  load_library(char_type const* name);
    /// Closes the given executable module
    static bool_type    free_library(module_type hModule);
    /// Retrieves the given symbol from the library
    static void*        find_symbol(module_type hModule, char const* symbolName);
/// @}

/// \name Kernel object control
/// @{
public:
    /// Closes the given operating system handle
    static bool_type    close_handle(handle_type h);
/// @}

/// \name Error
/// @{
public:
    /// Gives the last error
    static error_type   get_last_error();
    /// Sets the last error
    static void         set_last_error(error_type er);
/// @}

/// \name Environment
/// @{
public:
    /// Gets an environment variable into the given buffer
    ///
    /// \param name The name of the variable to find
    /// \param buffer The buffer in which to write the variable. If this is NULL, then the required length is returned
    /// \param cchBuffer The size of the buffer, in characters
    static size_type    get_environment_variable(char_type const* name, char_type* buffer, size_type cchBuffer);
    /// Expands environment strings in \c src into \c buffer, up to a maximum \c cchDest characters
    static size_type    expand_environment_strings(char_type const* src, char_type* buffer, size_type cchBuffer);
/// @}
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k C>
struct system_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct system_traits<us_char_a_t>
{
public:
    typedef us_char_a_t                 char_type;
    typedef us_size_t                   size_type;
    typedef us_ptrdiff_t                difference_type;
    typedef system_traits<us_char_a_t>  class_type;
    typedef us_int_t                    int_type;
    typedef us_bool_t                   bool_type;
    typedef void*                       module_type;
    typedef int                         handle_type;
    typedef int                         result_code_type;
    typedef int                         error_type;

public:
    static char_type* char_copy(char_type* dest, char_type const* src, size_type n)
    {
        return static_cast<char_type*>(::memcpy(dest, src, sizeof(char_type) * n));
    }

#if !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS) || \
    defined(_CRT_SECURE_NO_DEPRECATE)
    static char_type* str_copy(char_type* dest, char_type const* src)
    {
        return ::strcpy(dest, src);
    }

    static char_type* str_n_copy(char_type* dest, char_type const* src, size_type cch)
    {
        return ::strncpy(dest, src, cch);
    }

    static char_type* str_cat(char_type* dest, char_type const* src)
    {
        return ::strcat(dest, src);
    }

    static char_type* str_n_cat(char_type* dest, char_type const* src, size_type cch)
    {
        return ::strncat(dest, src, cch);
    }
#endif /* !STLSOFT_USING_SAFE_STR_FUNCTIONS || _CRT_SECURE_NO_DEPRECATE */

    static int_type str_compare(char_type const* s1, char_type const* s2)
    {
        return ::strcmp(s1, s2);
    }

    static int_type str_compare_no_case(char_type const* s1, char_type const* s2);

    static int_type str_n_compare(char_type const* s1, char_type const* s2, size_type cch)
    {
        return ::strncmp(s1, s2, cch);
    }

#ifdef _MSC_VER
    static int_type str_n_compare_no_case(char_type const* s1, char_type const* s2, size_type cch)
    {
        UNIXSTL_ASSERT(NULL != s1);
        UNIXSTL_ASSERT(NULL != s2);

        return ::_strnicmp(s1, s2, cch);
    }
#else
    static int_type str_n_compare_no_case(char_type const* s1, char_type const* s2, size_type cch);
#endif

    static size_type str_len(char_type const* src)
    {
        return static_cast<size_type>(::strlen(src));
    }

    static char_type* str_chr(char_type const* s, char_type ch)
    {
        return const_cast<char_type*>(::strchr(s, ch));
    }

    static char_type* str_rchr(char_type const* s, char_type ch)
    {
        return const_cast<char_type*>(::strrchr(s, ch));
    }

    static char_type* str_str(char_type const* s, char_type const* sub)
    {
        return const_cast<char_type*>(::strstr(s, sub));
    }

    static char_type* str_pbrk(char_type const* s, char_type const* charSet)
    {
        return const_cast<char_type*>(::strpbrk(s, charSet));
    }

    static char_type* str_end(char_type const* s)
    {
        UNIXSTL_ASSERT(NULL != s);

        for(; *s != '\0'; ++s)
        {}

        return const_cast<char_type*>(s);
    }

    static char_type* str_set(char_type* s, size_type n, char_type c)
    {
        UNIXSTL_ASSERT(NULL != s || 0u == n);

        for(; 0u != n; --n, ++s)
        {
            *s = c;
        }

        return s;
    }

public:
    static module_type load_library(char_type const* name)
    {
        return ::dlopen(name, RTLD_NOW);
    }

    static bool_type free_library(module_type hModule)
    {
        return 0 == ::dlclose(hModule);
    }

    static void* find_symbol(module_type hModule, char const* symbolName)
    {
        return ::dlsym(hModule, symbolName);
    }

public:
    static bool_type close_handle(handle_type h)
    {
#if defined(_WIN32) && \
    (   defined(STLSOFT_COMPILER_IS_MSVC) || \
        defined(STLSOFT_COMPILER_IS_INTEL))
        return 0 == ::_close(h);
#else /* ? _WIN32 */
        return 0 == ::close(h);
#endif /* _WIN32 */
    }

public:
    static error_type get_last_error()
    {
        return errno;
    }

    static void set_last_error(error_type er = error_type())
    {
        errno = er;
    }

public:
#if (   defined(STLSOFT_COMPILER_IS_MSVC) || \
        defined(STLSOFT_COMPILER_IS_INTEL)) && \
    defined(STLSOFT_USING_SAFE_STR_FUNCTIONS)
# if _MSC_VER >= 1200
#  pragma warning(push)
# endif /* compiler */
# pragma warning(disable : 4996)
#endif /* compiler */

    static size_type get_environment_variable(char_type const* name, char_type* buffer, size_type cchBuffer)
    {
        char const* var = ::getenv(name);

        if(NULL == var)
        {
            return 0;
        }
        else
        {
            us_size_t var_len = str_len(var);

            if(NULL == buffer)
            {
                return var_len;
            }
            else
            {
                size_type writtenLen = (var_len < cchBuffer) ? var_len : cchBuffer;

                char_copy(buffer, var, writtenLen);
                if(writtenLen < cchBuffer)
                {
                    buffer[writtenLen] = '\0';
                }

                return (var_len < cchBuffer) ? var_len : cchBuffer;
            }
        }
    }

#if (   defined(STLSOFT_COMPILER_IS_MSVC) || \
        defined(STLSOFT_COMPILER_IS_INTEL)) && \
    defined(STLSOFT_USING_SAFE_STR_FUNCTIONS)
# if _MSC_VER >= 1200
#  pragma warning(pop)
# else /* ? compiler */
#  pragma warning(default : 4996)
# endif /* _MSC_VER */
#endif /* compiler */

    static size_type expand_environment_strings(char_type const* src, char_type* buffer, size_type cchBuffer);
};

STLSOFT_TEMPLATE_SPECIALISATION
struct system_traits<us_char_w_t>
{
public:
    typedef us_char_w_t                 char_type;
    typedef us_size_t                   size_type;
    typedef us_ptrdiff_t                difference_type;
    typedef system_traits<us_char_a_t>  class_type;
    typedef us_int_t                    int_type;
    typedef us_bool_t                   bool_type;
    typedef void*                       module_type;
    typedef int                         handle_type;
    typedef int                         result_code_type;
    typedef int                         error_type;

public:
    static char_type* char_copy(char_type* dest, char_type const* src, size_type n)
    {
        return static_cast<char_type*>(::memcpy(dest, src, sizeof(char_type) * n));
    }

#if !defined(STLSOFT_USING_SAFE_STR_FUNCTIONS) || \
    defined(_CRT_SECURE_NO_DEPRECATE)
    static char_type* str_copy(char_type* dest, char_type const* src)
    {
        return ::wcscpy(dest, src);
    }

    static char_type* str_n_copy(char_type* dest, char_type const* src, size_type cch)
    {
        return ::wcsncpy(dest, src, cch);
    }

    static char_type* str_cat(char_type* dest, char_type const* src)
    {
        return ::wcscat(dest, src);
    }

    static char_type* str_n_cat(char_type* dest, char_type const* src, size_type cch)
    {
        return ::wcsncat(dest, src, cch);
    }
#endif /* !STLSOFT_USING_SAFE_STR_FUNCTIONS || _CRT_SECURE_NO_DEPRECATE */

    static int_type str_compare(char_type const* s1, char_type const* s2)
    {
        return ::wcscmp(s1, s2);
    }

    static int_type str_compare_no_case(char_type const* s1, char_type const* s2);

    static int_type str_n_compare(char_type const* s1, char_type const* s2, size_type cch)
    {
        return ::wcsncmp(s1, s2, cch);
    }

#ifdef _MSC_VER
    static int_type str_n_compare_no_case(char_type const* s1, char_type const* s2, size_type cch)
    {
        UNIXSTL_ASSERT(NULL != s1);
        UNIXSTL_ASSERT(NULL != s2);

        return ::_wcsnicmp(s1, s2, cch);
    }
#else
    static int_type str_n_compare_no_case(char_type const* s1, char_type const* s2, size_type cch);
#endif

    static size_type str_len(char_type const* src)
    {
        return static_cast<size_type>(::wcslen(src));
    }

    static char_type* str_chr(char_type const* s, char_type ch)
    {
        return const_cast<char_type*>(::wcschr(s, ch));
    }

    static char_type* str_rchr(char_type const* s, char_type ch)
    {
        return const_cast<char_type*>(::wcsrchr(s, ch));
    }

    static char_type* str_str(char_type const* s, char_type const* sub)
    {
        return const_cast<char_type*>(::wcsstr(s, sub));
    }

    static char_type* str_pbrk(char_type const* s, char_type const* charSet)
    {
        return const_cast<char_type*>(::wcspbrk(s, charSet));
    }

    static char_type* str_end(char_type const* s)
    {
        UNIXSTL_ASSERT(NULL != s);

        for(; *s != L'\0'; ++s)
        {}

        return const_cast<char_type*>(s);
    }

    static char_type* str_set(char_type* s, size_type n, char_type c)
    {
        UNIXSTL_ASSERT(NULL != s || 0u == n);

        for(; 0u != n; --n, ++s)
        {
            *s = c;
        }

        return s;
    }

public:
    static module_type load_library(char_type const* name);

    static bool_type free_library(module_type hModule)
    {
        return 0 == ::dlclose(hModule);
    }

    static void* find_symbol(module_type hModule, char const* symbolName)
    {
        return ::dlsym(hModule, symbolName);
    }

public:
    static bool_type close_handle(handle_type h)
    {
#if defined(_WIN32) && \
    defined(STLSOFT_COMPILER_IS_MSVC)
        return 0 == ::_close(h);
#else /* ? _WIN32 */
        return 0 == ::close(h);
#endif /* _WIN32 */
    }

public:
    static error_type get_last_error()
    {
        return errno;
    }

    static void set_last_error(error_type er = error_type())
    {
        errno = er;
    }

public:
    static size_type get_environment_variable(char_type const* name, char_type* buffer, size_type cchBuffer);
    static size_type expand_environment_strings(char_type const* src, char_type* buffer, size_type cchBuffer);
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/system_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

/* ////////////////////////////////////////////////////////////////////// */

#endif /* UNIXSTL_INCL_UNIXSTL_SYSTEM_HPP_SYSTEM_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
