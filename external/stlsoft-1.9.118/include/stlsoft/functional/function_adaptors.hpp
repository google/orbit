/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/functional/function_adaptors.hpp (originally ::SynesisStd)
 *
 * Purpose:     Contains the stlsoft::mem_fun and stlsoft::ptr_fun calling
 *              convention-aware function adaptors.
 *
 * Created:     13th June 1999
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/functional/function_adaptors.hpp
 *
 * \brief [C++ only] Main include for function-adapting function classes,
 *   including member functions, non-member functions, and non-member
 *   procedures
 *   (\ref group__library__functional "Functional" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS
#define STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS_MAJOR    4
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS_MINOR    0
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS_REVISION 2
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS_EDIT     59
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS
# include <stlsoft/functional/function_pointer_adaptors.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS */

#if !defined(STLSOFT_COMPILER_IS_GCC) || \
    __GNUC__ > 3 || \
    (   __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 3)
# ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_METHOD_ADAPTORS
#  include <stlsoft/functional/method_adaptors.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_METHOD_ADAPTORS */
#endif /* compiler */
#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS
# include <stlsoft/functional/procedure_adaptors.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
