/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/synch/sleep_functions.h
 *
 * Purpose:     WinSTL time functions.
 *
 * Created:     11th June 2006
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


/** \file winstl/synch/sleep_functions.h
 *
 * \brief [C, C++] Various time functions
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYNCH_H_SLEEP_FUNCTIONS
#define WINSTL_INCL_WINSTL_SYNCH_H_SLEEP_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYNCH_H_SLEEP_FUNCTIONS_MAJOR      2
# define WINSTL_VER_WINSTL_SYNCH_H_SLEEP_FUNCTIONS_MINOR      0
# define WINSTL_VER_WINSTL_SYNCH_H_SLEEP_FUNCTIONS_REVISION   4
# define WINSTL_VER_WINSTL_SYNCH_H_SLEEP_FUNCTIONS_EDIT       15
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
 * Functions
 */

/** \brief [C, C++] Puts the calling thread to sleep for the given number of
 *   microseconds.
\code
winstl__micro_sleep(100000);  // Sleep for 0.1 seconds
winstl__micro_sleep(100);     // Sleep for 0.1 milliseconds
\endcode
 *
 * \param microseconds The number of microseconds to wait
 *
 * \return A boolean value indicating whether the operation was
 *   successful. If not, <code>::GetLastError()</code> will contain an error code
 *   representing the reason for failure.
 *
 * \see winstl::micro_sleep
 */
STLSOFT_INLINE ws_int_t winstl__micro_sleep(ws_uint_t microseconds)
{
    return (STLSOFT_NS_GLOBAL(Sleep)(microseconds / 1000), ws_true_v);
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

/** \brief [C++ only] Puts the calling thread to sleep for the given number of
 *   microseconds.
\code
winstl::micro_sleep(100000);  // Sleep for 0.1 seconds
winstl::micro_sleep(100);     // Sleep for 0.1 milliseconds
\endcode
 *
 * \param microseconds The number of microseconds to wait
 *
 * \return A boolean value indicating whether the operation was
 *   successful. If not, <code>::GetLastError()</code> will contain an error code
 *   representing the reason for failure.
 */
inline ws_int_t micro_sleep(ws_uint_t microseconds)
{
    return winstl__micro_sleep(microseconds);
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

#endif /* !WINSTL_INCL_WINSTL_SYNCH_H_SLEEP_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
