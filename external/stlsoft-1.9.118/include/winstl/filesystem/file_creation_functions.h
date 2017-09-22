/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/file_creation_functions.h
 *
 * Purpose:     File creation functions.
 *
 * Created:     12th September 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/filesystem/file_creation_functions.h
 *
 * \brief [C, C++] File creation functions
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_H_FILE_CREATION_FUNCTIONS
#define WINSTL_INCL_WINSTL_H_FILE_CREATION_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_H_FILE_CREATION_FUNCTIONS_MAJOR      2
# define WINSTL_VER_WINSTL_H_FILE_CREATION_FUNCTIONS_MINOR      0
# define WINSTL_VER_WINSTL_H_FILE_CREATION_FUNCTIONS_REVISION   1
# define WINSTL_VER_WINSTL_H_FILE_CREATION_FUNCTIONS_EDIT       13
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

#if !defined(_WINSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
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
 * C functions
 */

/** \brief Opens an existing file for shared read access.
 *
 * \ingroup group__library__filesystem
 */
STLSOFT_INLINE HANDLE winstl__open_file_read_shared_a(ws_char_a_t const* fileName, DWORD dwShareMode)
{
    return STLSOFT_NS_GLOBAL(CreateFileA)(fileName, GENERIC_READ, dwShareMode, NULL, OPEN_ALWAYS, 0, NULL);
}

/** \brief Opens an existing file for shared read access.
 *
 * \ingroup group__library__filesystem
 */
STLSOFT_INLINE HANDLE winstl__open_file_read_shared_w(ws_char_w_t const* fileName, DWORD dwShareMode)
{
    return STLSOFT_NS_GLOBAL(CreateFileW)(fileName, GENERIC_READ, dwShareMode, NULL, OPEN_ALWAYS, 0, NULL);
}

/** \brief Opens an existing file for exclusive read access.
 *
 * \ingroup group__library__filesystem
 */
STLSOFT_INLINE HANDLE winstl__open_file_exclusive_a(ws_char_a_t const* fileName)
{
    return STLSOFT_NS_GLOBAL(CreateFileA)(fileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
}

/** \brief Opens an existing file for exclusive read access.
 *
 * \ingroup group__library__filesystem
 */
STLSOFT_INLINE HANDLE winstl__open_file_exclusive_w(ws_char_w_t const* fileName)
{
    return STLSOFT_NS_GLOBAL(CreateFileW)(fileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
}

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace winstl
{
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C++ functions
 */

#ifdef __cplusplus

/** \brief Opens an existing file for shared read access.
 *
 * \ingroup group__library__filesystem
 */
inline HANDLE open_file_read_shared(ws_char_a_t const* fileName, DWORD dwShareMode)
{
    return winstl__open_file_read_shared_a(fileName, dwShareMode);
}
/** \brief Opens an existing file for shared read access.
 *
 * \ingroup group__library__filesystem
 */
inline HANDLE open_file_read_shared(ws_char_w_t const* fileName, DWORD dwShareMode)
{
    return winstl__open_file_read_shared_w(fileName, dwShareMode);
}

/** \brief Opens an existing file for exclusive read access.
 *
 * \ingroup group__library__filesystem
 */
inline HANDLE open_file_exclusive(ws_char_a_t const* fileName)
{
    return winstl__open_file_exclusive_a(fileName);
}
/** \brief Opens an existing file for exclusive read access.
 *
 * \ingroup group__library__filesystem
 */
inline HANDLE open_file_exclusive(ws_char_w_t const* fileName)
{
    return winstl__open_file_exclusive_w(fileName);
}

#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_H_FILE_CREATION_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
