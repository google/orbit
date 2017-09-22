/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/error/active_end_iterator_exhaustion.hpp
 *
 * Purpose:     An exception thrown when an active end iterator is exhausted.
 *
 * Created:     30th November 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/error/active_end_iterator_exhaustion.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::active_end_iterator_exhaustion
 *   exception class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION
#define STLSOFT_INCL_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION_MAJOR     2
# define STLSOFT_VER_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION_MINOR     0
# define STLSOFT_VER_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION_REVISION  1
# define STLSOFT_VER_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION_EDIT      13
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_ITERATION_INTERRUPTION
# include <stlsoft/error/iteration_interruption.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_ITERATION_INTERRUPTION */

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

/** \brief An exception thrown when an active end iterator is exhausted
 *
 * \ingroup group__library__error
 */
class active_end_iterator_exhaustion
    : public iteration_interruption
{
/// \name Member Types
/// @{
public:
    typedef iteration_interruption          parent_class_type;
    typedef active_end_iterator_exhaustion  class_type;
/// @}

/// \name Construction
/// @{
public:
    active_end_iterator_exhaustion()
        : parent_class_type()
    {}
    ss_explicit_k active_end_iterator_exhaustion(char const* message)
        : parent_class_type(message)
    {}
    active_end_iterator_exhaustion(char const* message, long errorCode)
        : parent_class_type(message, errorCode)
    {}
    virtual ~active_end_iterator_exhaustion() throw()
    {}
/// @}

/// \name Implementation
/// @{
private:
    virtual char const* real_what_() const throw()
    {
        return "active end iterator invalidation";
    }
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/active_end_iterator_exhaustion_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_ACTIVE_END_ITERATOR_EXHAUSTION */

/* ///////////////////////////// end of file //////////////////////////// */
