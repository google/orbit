/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/shims/conversion/to_uint64/stat.hpp
 *
 * Purpose:     Contains the to_uint64 access shim overload for (struct stat).
 *
 * Created:     1st April 2010
 * Updated:     1st April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/shims/conversion/to_uint64/stat.hpp
 *
 * \brief [C++] Integer conversion shims for the UNIX type stat.
 *   \ref group__concept__shim__integer_conversion__to_uint64 "to_uint64 Integer Attribute Shim".
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT
#define UNIXSTL_INCL_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT_MAJOR      1
# define UNIXSTL_VER_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT_MINOR      0
# define UNIXSTL_VER_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT_REVISION   1
# define UNIXSTL_VER_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT_EDIT       1
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_HPP_DEGENERATE
# include <stlsoft/shims/conversion/to_uint64/degenerate.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_CONVERSION_TO_UINT64_HPP_DEGENERATE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
namespace stlsoft
{
#endif /* _UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * to_uint64 functions
 */

/** \brief Converts a struct stat instance to an unsigned
 *    64-bit integer value, by taking its \c st_size member value.
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline stlsoft_ns_qual(ss_uint64_t) to_uint64(struct stat const& st)
{
    return static_cast<stlsoft_ns_qual(ss_uint64_t)>(st.st_size);
}

/** \brief Converts a pointer to a struct stat instance to an unsigned
 *    64-bit integer value, by taking its \c st_size member value.
 * \ingroup group__concept__shim__integer_conversion__to_uint64
 */
inline stlsoft_ns_qual(ss_uint64_t) to_uint64(struct stat const* st)
{
    return (NULL == st) ? 0 : to_uint64(*st);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
} // namespace stlsoft
#endif /* _UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_SHIMS_CONVERSION_TO_UINT64_HPP_STAT */

/* ///////////////////////////// end of file //////////////////////////// */
