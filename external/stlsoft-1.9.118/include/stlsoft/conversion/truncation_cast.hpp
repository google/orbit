/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/truncation_cast.hpp
 *
 * Purpose:     Runtime checking for numeric conversions.
 *
 * Created:     10th August 2006
 * Updated:     5th March 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2011, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/truncation_cast.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::truncation_cast cast class
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST_MAJOR      1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST_MINOR      0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST_REVISION   4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST_EDIT       42
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST
# include <stlsoft/conversion/truncation_test.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>
#endif /* !STLSOFT_INCL_STDEXCEPT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline TO truncation_cast(FROM from)
{
    if(!truncation_test<TO>(from))
    {
        STLSOFT_THROW_X(std::out_of_range("Cast would result in truncation"));
    }

    return static_cast<TO>(from);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/truncation_cast_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
