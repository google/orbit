/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/std/cstring.hpp
 *
 * Purpose:     Mappings to string string functions
 *
 * Created:     2nd December 2004
 * Updated:     31st March 2010
 *
 * Thanks:      To Anton Sekeris for providing good advice on the naming scheme
 *              for the stlsoft/std headers
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/std/cstring.hpp
 *
 * \brief [C++ only] Mappings of &lt;cstring> string functions that use
 *   \ref group__concept__shim__string_access string
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STD_HPP_CSTRING
#define STLSOFT_INCL_STLSOFT_STD_HPP_CSTRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTRING_MAJOR      1
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTRING_MINOR      5
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTRING_REVISION   4
# define STLSOFT_VER_STLSOFT_STD_HPP_CSTRING_EDIT       33
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_STD_HPP_CBASE_
# include <stlsoft/std/cbase_.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STD_HPP_CBASE_ */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS
# include <stlsoft/string/string_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>
#endif /* !STLSOFT_INCL_H_STRING */
#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */
#if defined(STLSOFT_COMPILER_IS_BORLAND)
# include <malloc.h>
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/// \name Copying and concatenation family
/// @{

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
namespace std_impl
{
#endif /* _STLSOFT_NO_NAMESPACE */

    inline ss_char_a_t* strcpy_a(ss_char_a_t* dest, ss_char_a_t const* src)
    {
        return ::strcpy(dest, src);
    }
    inline ss_char_w_t* strcpy_w(ss_char_w_t* dest, ss_char_w_t const* src)
    {
        return ::wcscpy(dest, src);
    }
    inline ss_char_a_t* strcpy_(ss_char_a_t* dest, ss_char_a_t const* src)
    {
        return strcpy_a(dest, src);
    }
    inline ss_char_w_t* strcpy_(ss_char_w_t* dest, ss_char_w_t const* src)
    {
        return strcpy_w(dest, src);
    }

    inline ss_char_a_t* strcat_a(ss_char_a_t* dest, ss_char_a_t const* src)
    {
        return ::strcat(dest, src);
    }
    inline ss_char_w_t* strcat_w(ss_char_w_t* dest, ss_char_w_t const* src)
    {
        return ::wcscat(dest, src);
    }
    inline ss_char_a_t* strcat_(ss_char_a_t* dest, ss_char_a_t const* src)
    {
        return strcat_a(dest, src);
    }
    inline ss_char_w_t* strcat_(ss_char_w_t* dest, ss_char_w_t const* src)
    {
        return strcat_w(dest, src);
    }

    inline ss_char_a_t* strncpy_a(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
    {
        return ::strncpy(dest, src, n);
    }
    inline ss_char_w_t* strncpy_w(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
    {
        return ::wcsncpy(dest, src, n);
    }
    inline ss_char_a_t* strncpy_(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
    {
        return strncpy_a(dest, src, n);
    }
    inline ss_char_w_t* strncpy_(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
    {
        return strncpy_w(dest, src, n);
    }

    inline ss_char_a_t* strncat_a(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
    {
        return ::strncat(dest, src, n);
    }
    inline ss_char_w_t* strncat_w(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
    {
        return ::wcsncat(dest, src, n);
    }
    inline ss_char_a_t* strncat_(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
    {
        return strncat_a(dest, src, n);
    }
    inline ss_char_w_t* strncat_(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
    {
        return strncat_w(dest, src, n);
    }

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace std_impl
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strcpy(ss_typename_type_k string_traits<S>::char_type* dest, S const& src)
{
    return stlsoft_std_ns_qual(strcpy_)(dest, stlsoft_ns_qual(c_str_ptr)(src));
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

inline ss_char_a_t* strcpy(ss_char_a_t* dest, ss_char_a_t const* src)
{
    return stlsoft_std_ns_qual(strcpy_)(dest, src);
}

inline ss_char_w_t* strcpy(ss_char_w_t* dest, ss_char_w_t const* src)
{
    return stlsoft_std_ns_qual(strcpy_)(dest, src);
}



#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strcat(ss_typename_type_k string_traits<S>::char_type* dest, S const& src)
{
    return stlsoft_std_ns_qual(strcat_)(dest, stlsoft_ns_qual(c_str_ptr)(src));
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

inline ss_char_a_t* strcat(ss_char_a_t* dest, ss_char_a_t const* src)
{
    return stlsoft_std_ns_qual(strcat_)(dest, src);
}

inline ss_char_w_t* strcat(ss_char_w_t* dest, ss_char_w_t const* src)
{
    return stlsoft_std_ns_qual(strcat_)(dest, src);
}


#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strncpy(ss_typename_type_k string_traits<S>::char_type* dest, S const& src)
{
    return stlsoft_std_ns_qual(strncpy_)(dest, stlsoft_ns_qual(c_str_data)(src), stlsoft_ns_qual(c_str_len)(src));
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */


#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strncat(ss_typename_type_k string_traits<S>::char_type* dest, S const& src)
{
    return stlsoft_std_ns_qual(strncat_)(dest, stlsoft_ns_qual(c_str_data)(src), stlsoft_ns_qual(c_str_len)(src));
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */


#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strncpy(ss_typename_type_k string_traits<S>::char_type* dest, S const& src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncpy_)(dest, stlsoft_ns_qual(c_str_data)(src), n);
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

inline ss_char_a_t* strncpy(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncpy_)(dest, src, n);
}

inline ss_char_w_t* strncpy(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncpy_)(dest, src, n);
}


#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strncat(ss_typename_type_k string_traits<S>::char_type* dest, S const& src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncat_)(dest, stlsoft_ns_qual(c_str_data)(src), n);
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

inline ss_char_a_t* strncat(ss_char_a_t* dest, ss_char_a_t const* src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncat_)(dest, src, n);
}

inline ss_char_w_t* strncat(ss_char_w_t* dest, ss_char_w_t const* src, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncat_)(dest, src, n);
}

/// @}

/// \name Length and comparison family
/// @{

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
namespace std_impl
{
#endif /* _STLSOFT_NO_NAMESPACE */

    inline int strcmp_a(ss_char_a_t const* s1, ss_char_a_t const* s2)
    {
        return ::strcmp(s1, s2);
    }
    inline int strcmp_w(ss_char_w_t const* s1, ss_char_w_t const* s2)
    {
        return ::wcscmp(s1, s2);
    }
    inline int strcmp_(ss_char_a_t const* s1, ss_char_a_t const* s2)
    {
        return strcmp_a(s1, s2);
    }
    inline int strcmp_(ss_char_w_t const* s1, ss_char_w_t const* s2)
    {
        return strcmp_w(s1, s2);
    }

    inline int strncmp_a(ss_char_a_t const* s1, ss_char_a_t const* s2, ss_size_t n)
    {
        return ::strncmp(s1, s2, n);
    }
    inline int strncmp_w(ss_char_w_t const* s1, ss_char_w_t const* s2, ss_size_t n)
    {
        return ::wcsncmp(s1, s2, n);
    }
    inline int strncmp_(ss_char_a_t const* s1, ss_char_a_t const* s2, ss_size_t n)
    {
        return strncmp_a(s1, s2, n);
    }
    inline int strncmp_(ss_char_w_t const* s1, ss_char_w_t const* s2, ss_size_t n)
    {
        return strncmp_w(s1, s2, n);
    }

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace std_impl
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


template <ss_typename_param_k S>
inline ss_size_t strlen(S const& s)
{
    return stlsoft_ns_qual(c_str_len)(s);
}

inline ss_size_t strlen(ss_char_a_t const* s)
{
    return ::strlen(s);
}

inline ss_size_t strlen(ss_char_w_t const* s)
{
    return ::wcslen(s);
}


template <ss_typename_param_k S>
inline int strcmp(S const& s1, S const& s2)
{
    return stlsoft_std_ns_qual(strcmp_)(stlsoft_ns_qual(c_str_ptr)(s1), stlsoft_ns_qual(c_str_ptr)(s2));
}

inline int strcmp(ss_char_a_t const* s1, ss_char_a_t const* s2)
{
    return ::strcmp(s1, s2);
}

inline int strcmp(ss_char_w_t const* s1, ss_char_w_t const* s2)
{
    return ::wcscmp(s1, s2);
}


template <ss_typename_param_k S>
inline int strncmp(S const& s1, S const& s2, ss_size_t n)
{
    return stlsoft_std_ns_qual(strncmp_)(stlsoft_ns_qual(c_str_ptr)(s1), stlsoft_ns_qual(c_str_ptr)(s2), n);
}

inline int strncmp(ss_char_a_t const* s1, ss_char_a_t const* s2, ss_size_t n)
{
    return ::strncmp(s1, s2, n);
}

inline int strncmp(ss_char_w_t const* s1, ss_char_w_t const* s2, ss_size_t n)
{
    return ::wcsncmp(s1, s2, n);
}

/// @}



/// \name Searching family
/// @{

inline ss_char_a_t const* strchr(ss_char_a_t const* s, ss_char_a_t ch)
{
    return ::strchr(s, ch);
}

inline ss_char_w_t const* strchr(ss_char_w_t const* s, ss_char_w_t ch)
{
    return ::wcschr(s, ch);
}


inline ss_char_a_t const* strrchr(ss_char_a_t const* s, ss_char_a_t ch)
{
    return ::strrchr(s, ch);
}

inline ss_char_w_t const* strrchr(ss_char_w_t const* s, ss_char_w_t ch)
{
    return ::wcsrchr(s, ch);
}


inline ss_char_a_t const* strstr(ss_char_a_t const* s, ss_char_a_t const* charSet)
{
    return ::strstr(s, charSet);
}

inline ss_char_w_t const* strstr(ss_char_w_t const* s, ss_char_w_t const* charSet)
{
    return ::wcsstr(s, charSet);
}

#if 0
// NOTE: Can't do the ones that return pointers, since the shims may return temporaries. That would *not be good
ss_size_t  __cdecl strcspn(char const*, char const*);
ss_size_t  __cdecl strspn(char const*, char const*);
char *  __cdecl strpbrk(char const*, char const*);




#endif /* 0 */

/// @}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
namespace std_impl
{
#endif /* _STLSOFT_NO_NAMESPACE */

    inline ss_char_a_t* strdup_a(ss_char_a_t const* s)
    {
        return ::strdup(s);
    }
    inline ss_char_w_t* strdup_w(ss_char_w_t const* s)
    {
#  if defined(STLSOFT_COMPILER_IS_BORLAND) || \
      (   defined(STLSOFT_COMPILER_IS_GCC) && \
          !defined(WIN32)) || \
      defined(STLSOFT_COMPILER_IS_WATCOM)
        const ss_size_t len =   ::wcslen(s);
        ss_char_w_t     *sz =   static_cast<ss_char_w_t*>(::malloc(sizeof(ss_char_w_t) * (1 + len)));

        if(NULL != sz)
        {
            ::wcscpy(sz, s);
        }

        return sz;
#  else /* ? compiler */
        return ::wcsdup(const_cast<ss_char_w_t*>(s));
#  endif /* compiler */
    }
    inline ss_char_a_t* strdup_(ss_char_a_t const* s)
    {
        return strdup_a(s);
    }
    inline ss_char_w_t* strdup_(ss_char_w_t const* s)
    {
        return strdup_w(s);
    }

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace std_impl
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Duplicates the given string
 *
 * \ingroup group__library__utility
 *
 * \note The returned string is allocated by the standard strdup() function,
 * which uses malloc() to allocate the memory, and so must be freed using free()
 */
#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_traits<S>::char_type* strdup(S const& s)
{
    return stlsoft_std_ns_qual(strdup_)(stlsoft_ns_qual(c_str_ptr)(s));
}
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/** \brief Duplicates the multibyte C-string
 *
 * \ingroup group__library__utility
 */

inline ss_char_a_t* strdup(ss_char_a_t const* s)
{
    return stlsoft_std_ns_qual(strdup_)(s);
}

/** \brief Duplicates the wide C-string
 *
 * \ingroup group__library__utility
 */

inline ss_char_w_t* strdup(ss_char_w_t const* s)
{
    return stlsoft_std_ns_qual(strdup_)(s);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STD_HPP_CSTRING */

/* ///////////////////////////// end of file //////////////////////////// */
