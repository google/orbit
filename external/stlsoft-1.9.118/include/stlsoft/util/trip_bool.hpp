/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/trip_bool.hpp
 *
 * Purpose:     Defines the stlsoft::trip_bool class template.
 *
 * Created:     22nd October 2006
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


/** \file stlsoft/util/trip_bool.hpp
 *
 * \brief [C++ only] Defines the stlsoft::trip_bool class template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_TRIP_BOOL
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_TRIP_BOOL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_TRIP_BOOL_MAJOR       1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_TRIP_BOOL_MINOR       0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_TRIP_BOOL_REVISION    2
# define STLSOFT_VER_STLSOFT_UTIL_HPP_TRIP_BOOL_EDIT        8
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

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/** \brief
 *
 * \note This template was inspired by Bill Rubin's "Trip Switch Booleans in
 *  C++" article in August 2006 issue of CVu.
 */

template <ss_bool_t V>
class trip_bool
{
/// \name Member Types and Constants
/// @{
public:
    /// \brief The value type
    typedef ss_bool_t       value_type;
    /// \brief The current instantiation of the template
    typedef trip_bool<V>    class_type;

    enum { value = V };
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance of the class
    ///
    /// \param v The initial value of the trip instance. Defaults to !V
    trip_bool(bool v = !V)
        : m_v(v)
    {}
/// @}

/// \name Accessors
/// @{
public:
    /// \brief
    operator value_type () const
    {
        return m_v;
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief
    void trip()
    {
        m_v = V;
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_v;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/trip_bool_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_TRIP_BOOL */

/* ///////////////////////////// end of file //////////////////////////// */
