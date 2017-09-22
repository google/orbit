/* /////////////////////////////////////////////////////////////////////////
 * File:        dotnetstl/util/dispose_functions.hpp
 *
 * Purpose:     A useful tool for accessing a String object's content as a c-string.
 *
 * Created:     24th June 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file dotnetstl/util/dispose_functions.hpp
 *
 * \brief [C++ only] Utility functions for handling object disposal
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS
#define DOTNETSTL_INCL_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define DOTNETSTL_VER_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS_MAJOR       4
# define DOTNETSTL_VER_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS_MINOR       0
# define DOTNETSTL_VER_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS_REVISION    1
# define DOTNETSTL_VER_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS_EDIT        31
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL
# include <dotnetstl/dotnetstl.hpp>
#endif /* !DOTNETSTL_INCL_DOTNETSTL_HPP_DOTNETSTL */

/* ////////////////////////////////////////////////////////////////////// */

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
 * Classes
 */

/** \brief Disposes the managed type, and resets the pointer
 *
 * \ingroup group__library__utility
 *
 * \param pt The pointer on which to call Dispose(). Can be null, but otherwise must be convertible to IDispose
 */
template <ss_typename_param_k T>
inline void dispose_set_null(T *&pt)
{
    if(0 != pt)
    {
        System::IDisposable *disposable =   pt;

        disposable->Dispose();
        pt = 0;
    }
}

/** \brief Disposes all the items in a container
 *
 * \ingroup group__library__utility
 *
 * \param pc The container on whose items to call Dispose().
 */
template <ss_typename_param_k C>
inline void dispose_contents(C *pc)
{
    for(int i = 0, count = pc->get_Count(); i < count; ++i)
    {
        System::IDisposable *o  =   dynamic_cast<System::IDisposable*>(pc->get_Item(i));

        dispose_set_null(o);
    }
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/dispose_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifdef _STLSOFT_NO_NAMESPACE
} // namespace dotnetstl
#else
} // namespace dotnetstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* DOTNETSTL_INCL_DOTNETSTL_UTIL_HPP_DISPOSE_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
