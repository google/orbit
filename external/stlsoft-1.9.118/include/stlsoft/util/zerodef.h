/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/zerodef.h
 *
 * Purpose:     Include for defining ZERO to be the ZERO_v template class.
 *
 * Created:     29th July 2003
 * Updated:     3rd February 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2012, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/zerodef.h
 *
 * \brief [C++ only] Defines the ZERO preprocessor symbol as
 *   stlsoft::ZERO_v()
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_H_ZERODEF
#define STLSOFT_INCL_STLSOFT_UTIL_H_ZERODEF

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_H_ZERODEF_MAJOR    4
# define STLSOFT_VER_STLSOFT_UTIL_H_ZERODEF_MINOR    0
# define STLSOFT_VER_STLSOFT_UTIL_H_ZERODEF_REVISION 2
# define STLSOFT_VER_STLSOFT_UTIL_H_ZERODEF_EDIT     24
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_HPP_ZERO
# include <stlsoft/zero.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_HPP_ZERO */

#ifndef STLSOFT_INCL_H_STDDEF
# define STLSOFT_INCL_H_STDDEF
# include <stddef.h>    // Always make sure that this is included, irrespective of
                        // its potential inclusion within stlsoft/stlsoft.h
#endif /* !STLSOFT_INCL_H_STDDEF */

/* /////////////////////////////////////////////////////////////////////////
 * Definitions
 */

#ifdef ZERO
# ifdef STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
#  pragma message("ZERO is already defined in this compilation unit. Continuing is potentially dangerous. You are advised to verify that the redefinition is compatible, or to refrain from using stlsoft_zerodef.h")
# endif /* STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT */
#endif /* !ZERO */

#ifdef __cplusplus
# ifdef ZERO
#  undef ZERO
# endif /* ZERO */
 /// \def ZERO
 ///
 /// By including this file, \c ZERO is (re-)defined to be <code>stlsoft::ZERO_v()</code>
 /// which means that any use of \c ZERO must be with integral types.
# define ZERO   stlsoft_ns_qual(ZERO_v)::create()
#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_H_ZERODEF */

/* ///////////////////////////// end of file //////////////////////////// */
