/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/performance/tick_counter.hpp
 *
 * Purpose:     WinSTL tick-count performance counter class.
 *
 * Created:     19th October 1998
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/performance/tick_counter.hpp
 *
 * \brief [C++ only] Definition of the
 *  \link winstl::tick_counter tick_counter\endlink class
 *   (\ref group__library__performance "Performance" Library).
 */

#ifndef WINSTL_INCL_WINSTL_PERFORMANCE_HPP_TICK_COUNTER
#define WINSTL_INCL_WINSTL_PERFORMANCE_HPP_TICK_COUNTER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_TICK_COUNTER_MAJOR       4
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_TICK_COUNTER_MINOR       0
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_TICK_COUNTER_REVISION    2
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_TICK_COUNTER_EDIT        44
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class tick_counter
/** \brief A low-cost, low-resolution performance counter
 *
 * \ingroup group__library__performance
 *
 * This class provides low-resolution, but low-latency, performance monitoring
 * and is guaranteed to be meaningful on all operating systems.
 */
class tick_counter
{
public:
    /// This type
    typedef tick_counter    class_type;

private:
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    typedef ws_sint64_t    epoch_type;
#else
    typedef ws_sint32_t    epoch_type;
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
public:
    /// \brief The interval type
    ///
    /// The type of the interval measurement, a 64-bit signed integer
    typedef epoch_type     interval_type;

// Construction
public:
    tick_counter();

// Operations
public:
    /// \brief Starts measurement
    ///
    /// Begins the measurement period
    void        start();
    /// \brief Ends measurement
    ///
    /// Ends the measurement period
    void        stop();

// Attributes
public:
    /// \brief The elapsed count in the measurement period
    ///
    /// This represents the extent, in machine-specific increments, of the measurement period
    interval_type   get_period_count() const;
    /// \brief The number of whole seconds in the measurement period
    ///
    /// This represents the extent, in whole seconds, of the measurement period
    interval_type   get_seconds() const;
    /// \brief The number of whole milliseconds in the measurement period
    ///
    /// This represents the extent, in whole milliseconds, of the measurement period
    interval_type   get_milliseconds() const;
    /// \brief The number of whole microseconds in the measurement period
    ///
    /// This represents the extent, in whole microseconds, of the measurement period
    interval_type   get_microseconds() const;

// Members
private:
    ws_dword_t  m_start;    // start of measurement period
    ws_dword_t  m_end;      // End of measurement period
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/tick_counter_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline tick_counter::tick_counter()
{
    // Note that the constructor does nothing, for performance reasons. Calling
    // any of the Attribute methods before having gone through a start()-stop()
    // cycle will yield undefined results.
}

// Operations
inline void tick_counter::start()
{
    m_start = ::GetTickCount();
}

inline void tick_counter::stop()
{
    m_end = ::GetTickCount();
}

// Attributes
inline tick_counter::interval_type tick_counter::get_period_count() const
{
    return static_cast<interval_type>(m_end - m_start);
}

inline tick_counter::interval_type tick_counter::get_seconds() const
{
    return get_period_count() / interval_type(1000);
}

inline tick_counter::interval_type tick_counter::get_milliseconds() const
{
    return get_period_count();
}

inline tick_counter::interval_type tick_counter::get_microseconds() const
{
    return get_period_count() * interval_type(1000);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_PERFORMANCE_HPP_TICK_COUNTER */

/* ///////////////////////////// end of file //////////////////////////// */
