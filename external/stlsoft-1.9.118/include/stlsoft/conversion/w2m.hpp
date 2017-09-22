/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/w2m.hpp
 *
 * Purpose:     Character-encoding scheme interconversion components.
 *
 * Created:     3rd September 2008
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2008-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/w2m.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::wide2multibyte class
 *  template
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_W2M
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_W2M

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_W2M_MAJOR       1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_W2M_MINOR       0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_W2M_REVISION    1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_W2M_EDIT        2
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:  __GNUC__<3
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 3
# error stlsoft/conversion/char_conversions.hpp is not compatible with GNU C++ prior to 3.0
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1100
# error stlsoft/conversion/char_conversions.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS
# include <stlsoft/conversion/char_conversions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_W2M */

/* ///////////////////////////// end of file //////////////////////////// */
