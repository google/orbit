/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/nulldef.h
 *
 * Purpose:     Include for defining NULL to be the NULL_v template class.
 *
 * Created:     17th December 2002
 * Updated:     3rd February 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/nulldef.h
 *
 * \brief [C++ only] Defines the NULL preprocessor symbol as
 *   stlsoft::NULL_v()
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_H_NULLDEF
#define STLSOFT_INCL_STLSOFT_UTIL_H_NULLDEF

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_H_NULLDEF_MAJOR    4
# define STLSOFT_VER_STLSOFT_UTIL_H_NULLDEF_MINOR    0
# define STLSOFT_VER_STLSOFT_UTIL_H_NULLDEF_REVISION 3
# define STLSOFT_VER_STLSOFT_UTIL_H_NULLDEF_EDIT     29
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_NULL
# include <stlsoft/util/null.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_NULL */

#ifndef STLSOFT_INCL_H_STDDEF
# define STLSOFT_INCL_H_STDDEF
# include <stddef.h>    // Always make sure that this is included, irrespective of
                        // its potential inclusion within stlsoft/stlsoft.h
#endif /* !STLSOFT_INCL_H_STDDEF */

/* /////////////////////////////////////////////////////////////////////////
 * Definitions
 */

#ifndef NULL
# ifdef STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
#  pragma message("NULL not defined. This is potentially dangerous. You are advised to include its defining header before stlsoft_nulldef.h")
# endif /* STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT */
#endif /* !NULL */

#ifdef _STLSOFT_NULL_v_DEFINED
# ifdef __cplusplus
#  ifdef NULL
#   undef NULL
#  endif /* NULL */
  /// \def NULL
  ///
  /// By including this file, \c NULL is (re-)defined to be <code>stlsoft::NULL_v()</code>
  /// which means that any use of \c NULL must be with pointer types.
#  define NULL   stlsoft_ns_qual(NULL_v)::create()
# endif /* __cplusplus */
#endif /* _STLSOFT_NULL_v_DEFINED */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_H_NULLDEF */

/* ///////////////////////////// end of file //////////////////////////// */
