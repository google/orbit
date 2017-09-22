/* /////////////////////////////////////////////////////////////////////////
 * File:        dotnetstl/conversion/check_cast.hpp
 *
 * Purpose:     A cast operator function that performs runtime verification
 *              on the cast instance in debug builds.
 *
 * Created:     9th August 2006
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


/** \file dotnetstl/conversion/check_cast.hpp
 *
 * \brief [C++ only] Definition of the dotnetstl::check_cast cast function
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_CONVERSION_HPP_CHECK_CAST
#define DOTNETSTL_INCL_DOTNETSTL_CONVERSION_HPP_CHECK_CAST

#ifndef DOTNETSTL_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_VER_DOTNETSTL_CONVERSION_HPP_CHECK_CAST_MAJOR    1
# define DOTNETSTL_VER_DOTNETSTL_CONVERSION_HPP_CHECK_CAST_MINOR    0
# define DOTNETSTL_VER_DOTNETSTL_CONVERSION_HPP_CHECK_CAST_REVISION 1
# define DOTNETSTL_VER_DOTNETSTL_CONVERSION_HPP_CHECK_CAST_EDIT     4
#endif /* !DOTNETSTL_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
# include <dotnetstl/dotnetstl.hpp>
#endif /* !DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL */

#ifdef STLSOFT_UNITTEST
# include <exception>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef _STLSOFT_NO_NAMESPACE
/* There is no stlsoft namespace, so must define ::dotnetstl */
namespace dotnetstl
{
#else
/* Define stlsoft::dotnet_project */

namespace stlsoft
{

namespace dotnetstl_project
{

#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief A cast operator function that performs runtime verification
 *     on the cast instance in debug builds.
 *
 * \ingroup group__library__conversion
 *
 * In debug builds, application of check_cast is equivalent to using
 * __try_cast. In release builds it is equivalent to using static_cast.
 */
template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline TO check_cast(FROM from)
{
#ifdef _DEBUG
    return __try_cast<TO>(from);
#else // ? _DEBUG
    return static_cast<TO>(from);
#endif // _DEBUG
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/check_cast_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef _STLSOFT_NO_NAMESPACE
} // namespace dotnetstl
#else
} // namespace dotnetstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHECK_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
