/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/shims/logical/is_empty/util/features.hpp
 *
 * Purpose:     Detects compiler features required by the is_empty shim.
 *
 * Created:     20th December 2006
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


/** \file stlsoft/shims/logical/is_empty/util/features.hpp
 *
 * \brief [C++] Detects compiler features required by the is_empty shim
 *   (\ref group__concept__shim__collection_logical__is_empty "is_empty Collection Logical Shim").
 */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES
#define STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES_MAJOR     2
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES_MINOR     0
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES_REVISION  2
# define STLSOFT_VER_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES_EDIT      6
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1310
# define STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP
#endif /* compiler */

#ifdef STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
#  include <stlsoft/meta/yesno.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#endif /* STLSOFT_SHIM_LOGICAL_IS_EMPTY_NEEDS_HELP */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_LOGICAL_IS_EMPTY_UTIL_HPP_FEATURES */

/* ///////////////////////////// end of file //////////////////////////// */
