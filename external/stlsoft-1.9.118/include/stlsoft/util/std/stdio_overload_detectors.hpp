/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std/stdio_overload_detectors.hpp
 *
 * Purpose:     Detects characteristics of certain stdio functions that are, or
 *              will be, subject to upgrade in standardisation.
 *
 * Created:     30th May 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/util/std/stdio_overload_detectors.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::vswprintf_detector and
 *   stlsoft:: swprintf_detector classes, which detects characteristics of
 *   certain stdio functions that are, or will be, subject to upgrade in
 *   standardisation
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS
#define STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS_MAJOR    1
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS_MINOR    0
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS_REVISION 4
# define STLSOFT_VER_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS_EDIT     14
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
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES
# include <stlsoft/meta/n_types.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES */

#ifndef STLSOFT_INCL_H_STDARG
# define STLSOFT_INCL_H_STDARG
# include <stdarg.h>
#endif /* !STLSOFT_INCL_H_STDARG */
#ifndef STLSOFT_INCL_H_STDIO
# define STLSOFT_INCL_H_STDIO
# include <stdio.h>
#endif /* !STLSOFT_INCL_H_STDIO */
#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// NOTE: Only have to split these into base and derived, and qualify in sizeof()
// because of silly old GCC (pre 3.4)

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

    
struct vswprintf_detector_base
{
protected:
    typedef int (*vswprintf_3_t)(wchar_t *, wchar_t const*, va_list);
    typedef int (*vswprintf_4_t)(wchar_t *, ss_size_t, wchar_t const*, va_list);

    // NOTE: The four functions have to have different return types, otherwise a number
    // of compilers mess up the detection
    static one_t    has_3_param_vswprintf(vswprintf_3_t);
    static two_t    has_3_param_vswprintf(...);

    static three_t  has_4_param_vswprintf(vswprintf_4_t);
    static four_t   has_4_param_vswprintf(...);
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** Traits class that determines whether the C standard function
 *   <code>vswprintf()</code> takes 3 or 4 parameters
 */
struct vswprintf_detector
    : public vswprintf_detector_base
{
public:
    enum
    {
        /// Indicates whether <code>vswprintf()</code> takes 3 parameters 
        has_3_param = (sizeof(one_t) == sizeof(vswprintf_detector_base::has_3_param_vswprintf(::vswprintf)))
    };
    enum
    {
        /// Indicates whether <code>vswprintf()</code> takes 4 parameters 
        has_4_param = (sizeof(three_t) == sizeof(vswprintf_detector_base::has_4_param_vswprintf(::vswprintf)))
    };
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

struct swprintf_detector_base
{
protected:
    typedef int (*swprintf_3_t)(wchar_t *, wchar_t const*, ...);
    typedef int (*swprintf_4_t)(wchar_t *, ss_size_t, wchar_t const*, ...);

    // NOTE: The four functions have to have different return types, otherwise a number
    // of compilers mess up the detection
    static one_t    has_3_param_swprintf(swprintf_3_t);
    static two_t    has_3_param_swprintf(...);

    static three_t  has_4_param_swprintf(swprintf_4_t);
    static four_t   has_4_param_swprintf(...);
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** Traits class that determines whether the C standard function
 *   <code>swprintf()</code> takes 3 or 4 parameters
 */
struct swprintf_detector
    : public swprintf_detector_base
{
public:
    enum
    {
        /// Indicates whether <code>swprintf()</code> takes 3 parameters 
        has_3_param = (sizeof(one_t) == sizeof(swprintf_detector_base::has_3_param_swprintf(::swprintf)))
    };
    enum
    {
        /// Indicates whether <code>swprintf()</code> takes 4 parameters 
        has_4_param = (sizeof(three_t) == sizeof(swprintf_detector_base::has_4_param_swprintf(::swprintf)))
    };
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_STDIO_OVERLOAD_DETECTORS */

/* ///////////////////////////// end of file //////////////////////////// */
