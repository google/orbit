/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/performance/processtimes_counter.hpp
 *
 * Purpose:     UNIXSTL process-time performance counter class.
 *
 * Created:     9th June 2006
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


/** \file unixstl/performance/processtimes_counter.hpp
 *
 * \brief [C++ only] Definition of the
 *  \link unixstl::processtimes_counter processtimes_counter\endlink class
 *   (\ref group__library__performance "Performance" Library).
 */

#ifndef WINSTL_INCL_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER
#define UNIXSTL_INCL_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_MAJOR     1
# define UNIXSTL_VER_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_MINOR     0
# define UNIXSTL_VER_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_REVISION  7
# define UNIXSTL_VER_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER_EDIT      13
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */

#ifndef STLSOFT_INCL_SYS_H_TIME
# define STLSOFT_INCL_SYS_H_TIME
# include <sys/time.h>
#endif /* !STLSOFT_INCL_SYS_H_TIME */
#ifndef STLSOFT_INCL_SYS_H_RESOURCE
# define STLSOFT_INCL_SYS_H_RESOURCE
# include <sys/resource.h>
#endif /* !STLSOFT_INCL_SYS_H_RESOURCE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class processtimes_counter
/** \brief A performance counter that provides process-specific performance timings
 *
 * \ingroup group__library__performance
 *
 * This class uses the operating system's performance monitoring facilities to provide timing
 * information pertaining to the calling process only, irrespective of the activities of other
 * processes on the system. This class does not provide meaningful timing information on operating
 * systems that do not provide process-specific monitoring.
 */
class processtimes_counter
{
public:
    typedef processtimes_counter    class_type;
    typedef us_sint64_t             epoch_type;

public:
    /// \brief The interval type
    ///
    /// The type of the interval measurement, a 64-bit signed integer
    typedef us_sint64_t             interval_type;

// Construction
public:
    processtimes_counter();

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

// Members
private:
    typedef struct timeval timeval_t;

    timeval_t   m_kernelStart;
    timeval_t   m_kernelEnd;
    timeval_t   m_userStart;
    timeval_t   m_userEnd;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/processtimes_counter_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline processtimes_counter::processtimes_counter()
{
    // Note that the constructor does nothing, for performance reasons. Calling
    // any of the Attribute methods before having gone through a start()-stop()
    // cycle will yield undefined results.
}

// Operations
inline void processtimes_counter::start()
{
    struct rusage   r_usage;

    ::getrusage(RUSAGE_SELF, &r_usage);

    m_kernelStart   =   r_usage.ru_stime;
    m_userStart     =   r_usage.ru_utime;
}

inline void processtimes_counter::stop()
{
    struct rusage   r_usage;

    ::getrusage(RUSAGE_SELF, &r_usage);

    m_kernelEnd     =   r_usage.ru_stime;
    m_userEnd       =   r_usage.ru_utime;
}

// Kernel
inline processtimes_counter::interval_type processtimes_counter::get_kernel_period_count() const
{
    return get_kernel_microseconds();
}

inline processtimes_counter::interval_type processtimes_counter::get_kernel_seconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_kernelStart.tv_sec <= m_kernelEnd.tv_sec);

    long    secs    =   m_kernelEnd.tv_sec - m_kernelStart.tv_sec;
    long    usecs   =   m_kernelEnd.tv_usec - m_kernelStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs + usecs / (1000 * 1000);
}

inline processtimes_counter::interval_type processtimes_counter::get_kernel_milliseconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_kernelStart.tv_sec <= m_kernelEnd.tv_sec);

    long    secs    =   m_kernelEnd.tv_sec - m_kernelStart.tv_sec;
    long    usecs   =   m_kernelEnd.tv_usec - m_kernelStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * 1000 + usecs / 1000;
}

inline processtimes_counter::interval_type processtimes_counter::get_kernel_microseconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_kernelStart.tv_sec <= m_kernelEnd.tv_sec);

    long    secs    =   m_kernelEnd.tv_sec - m_kernelStart.tv_sec;
    long    usecs   =   m_kernelEnd.tv_usec - m_kernelStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * (1000 * 1000) + usecs;
}

// User
inline processtimes_counter::interval_type processtimes_counter::get_user_period_count() const
{
    return get_user_microseconds();
}

inline processtimes_counter::interval_type processtimes_counter::get_user_seconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_userStart.tv_sec <= m_userEnd.tv_sec);

    long    secs    =   m_userEnd.tv_sec - m_userStart.tv_sec;
    long    usecs   =   m_userEnd.tv_usec - m_userStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs + usecs / (1000 * 1000);
}

inline processtimes_counter::interval_type processtimes_counter::get_user_milliseconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_userStart.tv_sec <= m_userEnd.tv_sec);

    long    secs    =   m_userEnd.tv_sec - m_userStart.tv_sec;
    long    usecs   =   m_userEnd.tv_usec - m_userStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * 1000 + usecs / 1000;
}

inline processtimes_counter::interval_type processtimes_counter::get_user_microseconds() const
{
    UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_userStart.tv_sec <= m_userEnd.tv_sec);

    long    secs    =   m_userEnd.tv_sec - m_userStart.tv_sec;
    long    usecs   =   m_userEnd.tv_usec - m_userStart.tv_usec;

    UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * (1000 * 1000) + usecs;
}

// Total
inline processtimes_counter::interval_type processtimes_counter::get_period_count() const
{
    return get_kernel_period_count() + get_user_period_count();
}

inline processtimes_counter::interval_type processtimes_counter::get_seconds() const
{
    return get_period_count() / interval_type(10000000);
}

inline processtimes_counter::interval_type processtimes_counter::get_milliseconds() const
{
    return get_period_count() / interval_type(10000);
}

inline processtimes_counter::interval_type processtimes_counter::get_microseconds() const
{
    return get_period_count() / interval_type(10);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_PERFORMANCE_HPP_PROCESSTIMES_COUNTER */

/* ///////////////////////////// end of file //////////////////////////// */
