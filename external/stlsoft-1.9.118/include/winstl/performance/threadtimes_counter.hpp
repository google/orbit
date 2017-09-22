/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/performance/threadtimes_counter.hpp
 *
 * Purpose:     WinSTL thread-time performance counter class.
 *
 * Created:     22nd March 2002
 * Updated:     6th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/performance/threadtimes_counter.hpp
 *
 * \brief [C++ only] Definition of the
 *  \link winstl::threadtimes_counter threadtimes_counter\endlink class
 *   (\ref group__library__performance "Performance" Library).
 */

#ifndef WINSTL_INCL_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER
#define WINSTL_INCL_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER_MAJOR    4
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER_MINOR    0
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER_REVISION 3
# define WINSTL_VER_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER_EDIT     48
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

// class threadtimes_counter
/** \brief A performance counter that provides thread-specific performance timings
 *
 * \ingroup group__library__performance
 *
 * This class uses the operating system's performance monitoring facilities to provide timing
 * information pertaining to the calling thread only, irrespective of the activities of other
 * threads on the system. This class does not provide meaningful timing information on operating
 * systems that do not provide thread-specific monitoring.
 */
class threadtimes_counter
{
public:
    /// This type
    typedef threadtimes_counter class_type;

private:
    typedef ws_sint64_t         epoch_type;
public:
    /// \brief The interval type
    ///
    /// The type of the interval measurement, a 64-bit signed integer
    typedef ws_sint64_t         interval_type;

// Construction
public:
    /// \brief Constructor
    ///
    /// Creates an instance of the class, and caches the thread token so that measurements will
    /// be taken with respect to the thread in which the class was created.
    threadtimes_counter();

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
    // Kernel

    /// \brief The elapsed count in the measurement period for kernel mode activity
    ///
    /// This represents the extent, in machine-specific increments, of the measurement period for kernel mode activity
    interval_type   get_kernel_period_count() const;
    /// \brief The number of whole seconds in the measurement period for kernel mode activity
    ///
    /// This represents the extent, in whole seconds, of the measurement period for kernel mode activity
    interval_type   get_kernel_seconds() const;
    /// \brief The number of whole milliseconds in the measurement period for kernel mode activity
    ///
    /// This represents the extent, in whole milliseconds, of the measurement period for kernel mode activity
    interval_type   get_kernel_milliseconds() const;
    /// \brief The number of whole microseconds in the measurement period for kernel mode activity
    ///
    /// This represents the extent, in whole microseconds, of the measurement period for kernel mode activity
    interval_type   get_kernel_microseconds() const;

    // User

    /// \brief The elapsed count in the measurement period for user mode activity
    ///
    /// This represents the extent, in machine-specific increments, of the measurement period for user mode activity
    interval_type   get_user_period_count() const;
    /// \brief The number of whole seconds in the measurement period for user mode activity
    ///
    /// This represents the extent, in whole seconds, of the measurement period for user mode activity
    interval_type   get_user_seconds() const;
    /// \brief The number of whole milliseconds in the measurement period for user mode activity
    ///
    /// This represents the extent, in whole milliseconds, of the measurement period for user mode activity
    interval_type   get_user_milliseconds() const;
    /// \brief The number of whole microseconds in the measurement period for user mode activity
    ///
    /// This represents the extent, in whole microseconds, of the measurement period for user mode activity
    interval_type   get_user_microseconds() const;

    // Total

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

private: // Implementation
    static epoch_type convert_(FILETIME const& ft);

// Members
private:
    epoch_type  m_kernelStart;
    epoch_type  m_kernelEnd;
    epoch_type  m_userStart;
    epoch_type  m_userEnd;
    HANDLE      m_thread;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/threadtimes_counter_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline /* static */ threadtimes_counter::epoch_type threadtimes_counter::convert_(FILETIME const& ft)
{
    epoch_type r = ft.dwHighDateTime;

    r <<= 32;

    r += ft.dwLowDateTime;

    return r;
}

inline threadtimes_counter::threadtimes_counter()
    : m_thread(::GetCurrentThread())
{
    // Note that the constructor does nothing, for performance reasons. Calling
    // any of the Attribute methods before having gone through a start()-stop()
    // cycle will yield undefined results.
}

// Operations
inline void threadtimes_counter::start()
{
    FILETIME    creationTime;
    FILETIME    exitTime;
    FILETIME    kernelTime;
    FILETIME    userTime;

    if(!::GetThreadTimes(m_thread, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        m_kernelStart   =   0;
        m_userStart     =   0;

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ; // TODO: throw
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        m_kernelStart   =   convert_(kernelTime);
        m_userStart     =   convert_(userTime);
    }
}

inline void threadtimes_counter::stop()
{
    FILETIME    creationTime;
    FILETIME    exitTime;
    FILETIME    kernelTime;
    FILETIME    userTime;

    if(!::GetThreadTimes(m_thread, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        m_kernelEnd     =   0;
        m_userEnd       =   0;

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ; // TODO: throw
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        m_kernelEnd     =   convert_(kernelTime);
        m_userEnd       =   convert_(userTime);
    }
}

// Attributes

// Kernel
inline threadtimes_counter::interval_type threadtimes_counter::get_kernel_period_count() const
{
    return static_cast<interval_type>(m_kernelEnd - m_kernelStart);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_kernel_seconds() const
{
    return get_kernel_period_count() / interval_type(10000000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_kernel_milliseconds() const
{
    return get_kernel_period_count() / interval_type(10000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_kernel_microseconds() const
{
    return get_kernel_period_count() / interval_type(10);
}

// User
inline threadtimes_counter::interval_type threadtimes_counter::get_user_period_count() const
{
    return static_cast<interval_type>(m_userEnd - m_userStart);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_user_seconds() const
{
    return get_user_period_count() / interval_type(10000000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_user_milliseconds() const
{
    return get_user_period_count() / interval_type(10000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_user_microseconds() const
{
    return get_user_period_count() / interval_type(10);
}

// Total
inline threadtimes_counter::interval_type threadtimes_counter::get_period_count() const
{
    return get_kernel_period_count() + get_user_period_count();
}

inline threadtimes_counter::interval_type threadtimes_counter::get_seconds() const
{
    return get_period_count() / interval_type(10000000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_milliseconds() const
{
    return get_period_count() / interval_type(10000);
}

inline threadtimes_counter::interval_type threadtimes_counter::get_microseconds() const
{
    return get_period_count() / interval_type(10);
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

#endif /* !WINSTL_INCL_WINSTL_PERFORMANCE_HPP_THREADTIMES_COUNTER */

/* ///////////////////////////// end of file //////////////////////////// */
