/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/internal/safestr.h
 *
 * Purpose:     Handles detection of safe string library.
 *
 * Created:     24th March 2008
 * Updated:     10th August 2009
 *
 * Author:      Matthew Wilson
 *
 * Home:        http://www.stlsoft.org/
 *
 * Copyright (c) 2008-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/internal/safestr.h
 *
 * \brief [C, C++] Handles detection of safe string library.
 */

#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
#define STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_H_STLSOFT_INTERNAL_H_SAFESTR_MAJOR     1
# define STLSOFT_VER_H_STLSOFT_INTERNAL_H_SAFESTR_MINOR     1
# define STLSOFT_VER_H_STLSOFT_INTERNAL_H_SAFESTR_REVISION  1
# define STLSOFT_VER_H_STLSOFT_INTERNAL_H_SAFESTR_EDIT      4
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if (   defined(STLSOFT_COMPILER_IS_MSVC) || \
        (   defined(STLSOFT_COMPILER_IS_INTEL) && \
            defined(_MSC_VER))) && \
    _MSC_VER >= 1400
# include <crtdefs.h>
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Symbols
 */

#ifdef __STDC_SECURE_LIB__
# if defined(__STDC_WANT_SECURE_LIB__) && \
     __STDC_WANT_SECURE_LIB__ == 1
#  define STLSOFT_USING_SAFE_STR_FUNCTIONS
# endif /* __STDC_WANT_SECURE_LIB__ */
#endif /* __STDC_SECURE_LIB__ */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

/* ///////////////////////////// end of file //////////////////////////// */
