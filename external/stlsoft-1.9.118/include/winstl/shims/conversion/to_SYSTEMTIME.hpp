/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shims/conversion/to_SYSTEMTIME.hpp
 *
 * Purpose:     Includes all winstl::to_SYSTEMTIME conversion shims.
 *
 * Created:     15th December 2006
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


/** \file winstl/shims/conversion/to_SYSTEMTIME.hpp
 *
 * \brief [C++] Includes all winstl::to_SYSTEMTIME conversion shims
 *   (\ref group__concept__shim__time_conversion__to_SYSTEMTIME "winstl::to_SYSTEMTIME" Time Conversion Shim).
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME
#define WINSTL_INCL_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME_MAJOR     1
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME_MINOR     1
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME_REVISION  3
# define WINSTL_VER_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME_EDIT      6
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_DATE
# include <winstl/shims/conversion/to_SYSTEMTIME/DATE.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_DATE */
#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_FILETIME
# include <winstl/shims/conversion/to_SYSTEMTIME/FILETIME.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_FILETIME */
#ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME
# include <winstl/shims/conversion/to_SYSTEMTIME/SYSTEMTIME.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_SYSTEMTIME */
#ifdef WINSTL_UDATE_DEFINED
# ifndef WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE
#  include <winstl/shims/conversion/to_SYSTEMTIME/UDATE.hpp>
# endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_TO_SYSTEMTIME_HPP_UDATE */
#endif /* WINSTL_UDATE_DEFINED */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHIMS_CONVERSION_HPP_TO_SYSTEMTIME */

/* ///////////////////////////// end of file //////////////////////////// */
