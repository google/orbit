/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/util/struct_initialisers.hpp
 *
 * Purpose:     Functions for initialising Win32 structures.
 *
 * Created:     20th October 1994
 * Updated:     29th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/util/struct_initialisers.hpp
 *
 * \brief [C++ only] Definition of the winstl::zero_struct and
 *   winstl::init_struct structure initialiser functions
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef WINSTL_INCL_WINSTL_UTIL_HPP_STRUCT_INITIALISERS
#define WINSTL_INCL_WINSTL_UTIL_HPP_STRUCT_INITIALISERS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_UTIL_HPP_STRUCT_INITIALISERS_MAJOR       4
# define WINSTL_VER_WINSTL_UTIL_HPP_STRUCT_INITIALISERS_MINOR       1
# define WINSTL_VER_WINSTL_UTIL_HPP_STRUCT_INITIALISERS_REVISION    1
# define WINSTL_VER_WINSTL_UTIL_HPP_STRUCT_INITIALISERS_EDIT        221
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

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
 * Functions
 */

/** \brief Initialises all elements of a structure to zero
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline void zero_struct(T &t)
{
    ::ZeroMemory(&t, sizeof(T));
}

template <ss_typename_param_k T>
struct init_traits;


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

struct struct_has
{
    struct no_init_member_type
    {};

    struct cb_member_type
    {};

    struct cBytes_member_type
    {};

    struct cbSize_member_type
    {};

    struct dwLength_member_type
    {};

    struct dwOSVersionInfoSize_member_type
    {};

    struct dwSize_member_type
    {};

    struct nLength_member_type
    {};

    struct uSize_member_type
    {};


    template <ss_typename_param_k T>
    static void init(T &t, no_init_member_type)
    {
        zero_struct(t);
    }

    template <ss_typename_param_k T>
    static void init(T &t, cb_member_type)
    {
        zero_struct(t);
        t.cb        =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, cBytes_member_type)
    {
        zero_struct(t);
        t.cBytes    =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, cbSize_member_type)
    {
        zero_struct(t);
        t.cbSize    =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, dwLength_member_type)
    {
        zero_struct(t);
        t.dwLength  =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, dwOSVersionInfoSize_member_type)
    {
        zero_struct(t);
        t.dwOSVersionInfoSize   =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, dwSize_member_type)
    {
        zero_struct(t);
        t.dwSize    =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, nLength_member_type)
    {
        zero_struct(t);
        t.nLength   =   sizeof(T);
    }

    template <ss_typename_param_k T>
    static void init(T &t, uSize_member_type)
    {
        zero_struct(t);
        t.uSize    =   sizeof(T);
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Initialises all members of a structure to zero, and
 *    sets the size member to the size of the structure.
 *
 * \ingroup group__library__utility
 */
template <ss_typename_param_k T>
inline void init_struct(T &t)
{
    typedef ss_typename_type_k init_traits<T>::type     discriminator_t;

    struct_has::init(t, discriminator_t());
}

// Specialisations

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define WINSTL_The_structure_(S, t)                \
                                                    \
    STLSOFT_TEMPLATE_SPECIALISATION                 \
    struct init_traits<S>                           \
    {                                               \
        typedef struct_##t  type;                   \
    }

// WinBase.h:

WINSTL_The_structure_(PROCESS_INFORMATION, has::no_init_member_type);
WINSTL_The_structure_(OVERLAPPED, has::no_init_member_type);
WINSTL_The_structure_(FILETIME, has::no_init_member_type);
WINSTL_The_structure_(SYSTEMTIME, has::no_init_member_type);
WINSTL_The_structure_(COMMPROP, has::no_init_member_type);
WINSTL_The_structure_(COMSTAT, has::no_init_member_type);
WINSTL_The_structure_(COMMTIMEOUTS, has::no_init_member_type);
WINSTL_The_structure_(SYSTEM_INFO, has::no_init_member_type);
WINSTL_The_structure_(EXCEPTION_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(CREATE_THREAD_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(CREATE_PROCESS_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(EXIT_THREAD_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(LOAD_DLL_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(UNLOAD_DLL_DEBUG_INFO, has::no_init_member_type);
WINSTL_The_structure_(OUTPUT_DEBUG_STRING_INFO, has::no_init_member_type);
WINSTL_The_structure_(RIP_INFO, has::no_init_member_type);
WINSTL_The_structure_(DEBUG_EVENT, has::no_init_member_type);
WINSTL_The_structure_(PROCESS_HEAP_ENTRY, has::no_init_member_type);
WINSTL_The_structure_(BY_HANDLE_FILE_INFORMATION, has::no_init_member_type);
WINSTL_The_structure_(TIME_ZONE_INFORMATION, has::no_init_member_type);
WINSTL_The_structure_(WIN32_STREAM_ID, has::no_init_member_type);
WINSTL_The_structure_(WIN32_FIND_DATAA, has::no_init_member_type);
WINSTL_The_structure_(WIN32_FIND_DATAW, has::no_init_member_type);
WINSTL_The_structure_(WIN32_FILE_ATTRIBUTE_DATA, has::no_init_member_type);
WINSTL_The_structure_(SYSTEM_POWER_STATUS, has::no_init_member_type);
#ifdef _WINCON_
WINSTL_The_structure_(CONSOLE_SCREEN_BUFFER_INFO, has::no_init_member_type);
WINSTL_The_structure_(CONSOLE_SCREEN_BUFFER_INFOEX, has::cbSize_member_type);
#endif /* _WINCON_ */
#ifdef WINTRUST_H
WINSTL_The_structure_(WIN_CERTIFICATE, has::no_init_member_type);
WINSTL_The_structure_(WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT, has::no_init_member_type);
WINSTL_The_structure_(WIN_TRUST_ACTDATA_SUBJECT_ONLY, has::no_init_member_type);
WINSTL_The_structure_(WIN_TRUST_SUBJECT_FILE, has::no_init_member_type);
WINSTL_The_structure_(WIN_TRUST_SUBJECT_FILE_AND_DISPLAY, has::no_init_member_type);
WINSTL_The_structure_(WIN_SPUB_TRUSTED_PUBLISHER_DATA, has::no_init_member_type);
#endif /* WINTRUST_H */


WINSTL_The_structure_(STARTUPINFOA, has::cb_member_type);
WINSTL_The_structure_(STARTUPINFOW, has::cb_member_type);

WINSTL_The_structure_(OFSTRUCT, has::cBytes_member_type);

WINSTL_The_structure_(MEMORYSTATUS, has::dwLength_member_type);

WINSTL_The_structure_(OSVERSIONINFOA, has::dwOSVersionInfoSize_member_type);
WINSTL_The_structure_(OSVERSIONINFOW, has::dwOSVersionInfoSize_member_type);
#if !defined(STLSOFT_COMPILER_IS_DMC) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1200)
WINSTL_The_structure_(OSVERSIONINFOEXA, has::dwOSVersionInfoSize_member_type);
WINSTL_The_structure_(OSVERSIONINFOEXW, has::dwOSVersionInfoSize_member_type);
#endif /* compiler */

WINSTL_The_structure_(COMMCONFIG, has::dwSize_member_type);

WINSTL_The_structure_(SECURITY_ATTRIBUTES, has::nLength_member_type);

inline void init_struct(DCB &dcb)
{
    zero_struct(dcb);
    dcb.DCBlength = sizeof(dcb);
}

// ShellApi.h

#if defined(_INC_SHELLAPI) || \
    defined(_SHELLAPI_H) || \
    !defined(WIN32_LEAN_AND_MEAN)

WINSTL_The_structure_(SHFILEOPSTRUCTA, has::no_init_member_type);
WINSTL_The_structure_(SHFILEOPSTRUCTW, has::no_init_member_type);
#if !defined(STLSOFT_COMPILER_IS_GCC)
WINSTL_The_structure_(SHNAMEMAPPINGA, has::no_init_member_type);
WINSTL_The_structure_(SHNAMEMAPPINGW, has::no_init_member_type);
#endif /* compiler */
WINSTL_The_structure_(SHFILEINFOA, has::no_init_member_type);
WINSTL_The_structure_(SHFILEINFOW, has::no_init_member_type);


#if !defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(STLSOFT_COMPILER_IS_GCC) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1200) && \
    defined(WINVER) && \
    WINVER >= 0x0400
WINSTL_The_structure_(DRAGINFOA, has::uSize_member_type);
WINSTL_The_structure_(DRAGINFOW, has::uSize_member_type);
#endif /* WINVER */

WINSTL_The_structure_(APPBARDATA, has::cbSize_member_type);
WINSTL_The_structure_(SHELLEXECUTEINFOA, has::cbSize_member_type);
WINSTL_The_structure_(SHELLEXECUTEINFOW, has::cbSize_member_type);
#if !defined(STLSOFT_COMPILER_IS_DMC) && \
    (   !defined(STLSOFT_COMPILER_IS_GCC) || \
        __GNUC__ > 3 || \
        (   __GNUC__ == 3 && \
            __GNUC_MINOR__ > 2)) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1200)
WINSTL_The_structure_(SHQUERYRBINFO, has::cbSize_member_type);
#endif /* compiler */
WINSTL_The_structure_(NOTIFYICONDATAA, has::cbSize_member_type);
WINSTL_The_structure_(NOTIFYICONDATAW, has::cbSize_member_type);

#endif /* _INC_SHELLAPI || _SHELLAPI_H || !WIN32_LEAN_AND_MEAN */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/struct_initialisers_unittest_.h"
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

#endif /* !WINSTL_INCL_WINSTL_UTIL_HPP_STRUCT_INITIALISERS */

/* ///////////////////////////// end of file //////////////////////////// */
