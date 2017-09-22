/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/process_mutex.hpp
 *
 * Purpose:     Intra-process mutext, based on PTHREADS.
 *
 * Created:     15th May 2002
 * Updated:     13th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2014, Matthew Wilson and Synesis Software
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


/** \file unixstl/synch/process_mutex.hpp
 *
 * \brief [C++ only] Definition of the unixstl::process_mutex class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX
#define UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX_MAJOR      4
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX_MINOR      6
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX_REVISION   4
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX_EDIT       74
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES
# include <unixstl/synch/util/features.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES */
#ifndef UNIXSTL_USING_PTHREADS
# error unixstl/synch/process_mutex.hpp cannot be included in non-multithreaded compilation. _REENTRANT and/or _POSIX_THREADS must be defined
#endif /* !UNIXSTL_USING_PTHREADS */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_ERROR_HPP_EXCEPTIONS
#  include <unixstl/synch/error/exceptions.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_ERROR_HPP_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
# include <stlsoft/synch/concepts.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */

#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */
#ifndef STLSOFT_INCL_H_PTHREAD
# define STLSOFT_INCL_H_PTHREAD
# include <pthread.h>
#endif /* !STLSOFT_INCL_H_PTHREAD */

#ifdef STLSOFT_UNITTEST
# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

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

// class process_mutex
/** \brief This class provides an implementation of the mutex model based on
 *   the PTHREADS pthread_mutex_t.
 *
 * \ingroup group__library__synch
 */
class process_mutex
    : public stlsoft_ns_qual(critical_section)< STLSOFT_CRITICAL_SECTION_ISNOT_RECURSIVE
                                            ,   STLSOFT_CRITICAL_SECTION_IS_TRYABLE
                                            >
{
/// \name Member Types
/// @{
public:
    typedef process_mutex       class_type;
    typedef us_bool_t           bool_type;

    typedef pthread_mutex_t*    resource_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Creates an instance of the mutex
    ///
    /// \note This creates a recursive mutex. Use the other constructor(s) to
    ///  obtain a non-recursive mutex.
    ///
    /// \note On systems that support shared mutexes, this will be not shared. Use
    ///  the other constructor to obtain a shared mutex
    process_mutex()
        : m_mx(&m_mx_)
#if defined(_POSIX_THREAD_PROCESS_SHARED)
        , m_error(create_(&m_mx_, PTHREAD_PROCESS_PRIVATE, true))
#else /* ? _POSIX_THREAD_PROCESS_SHARED */
        , m_error(create_(&m_mx_, 0, true))
#endif /* _POSIX_THREAD_PROCESS_SHARED */
        , m_bOwnHandle(true)
    {}

    /// \brief Conversion constructor
    ///
    /// \param mx The raw mutex object handle that this instance will use
    /// \param bTakeOwnership If true, the handle is closed when this instance is destroyed
    process_mutex(pthread_mutex_t* mx, bool_type bTakeOwnership)
        : m_mx(mx)
        , m_error(0)
        , m_bOwnHandle(bTakeOwnership)
    {
        UNIXSTL_ASSERT(NULL != mx);
    }

    /// \brief Creates an instance of the mutex
    ///
    /// \note On systems that support shared mutexes, this will be not shared. Use
    /// the two-parameter constructor to obtain a shared mutex
    ss_explicit_k process_mutex(bool_type bRecursive)
        : m_mx(&m_mx_)
#if defined(_POSIX_THREAD_PROCESS_SHARED)
        , m_error(create_(&m_mx_, PTHREAD_PROCESS_PRIVATE, bRecursive))
#else /* ? _POSIX_THREAD_PROCESS_SHARED */
        , m_error(create_(&m_mx_, 0, bRecursive))
#endif /* _POSIX_THREAD_PROCESS_SHARED */
        , m_bOwnHandle(true)
    {}
#if defined(_POSIX_THREAD_PROCESS_SHARED)
    /// \brief Creates an instance of the mutex, optionally recursive and/or shared between processes
    ///
    /// \param pshared A value from the PTHREADS_PROCESS_* group that determines the sharing
    ///  characteristics of the mutex.
    /// \param bRecursive A boolean value denoting whether the mutex should be recursive or not
    process_mutex(int pshared, bool_type bRecursive)
        : m_mx(&m_mx_)
        , m_error(create_(&m_mx_, pshared, bRecursive))
        , m_bOwnHandle(true)
    {}
#endif /* _POSIX_THREAD_PROCESS_SHARED */
    /// \brief Destroys an instance of the mutex
    ~process_mutex() stlsoft_throw_0()
    {
        if( 0 == m_error &&
            m_bOwnHandle)
        {
            ::pthread_mutex_destroy(m_mx);
        }
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Acquires a lock on the mutex, pending the thread until the lock is aquired
    ///
    /// \exception unixstl::synchronisation_exception When compiling with exception support, this will throw
    /// unixstl::synchronisation_exception if the lock cannot be acquired. When
    /// compiling absent exception support, failure to acquire the lock
    /// will be reflected in a non-zero return from get_error().
    void lock()
    {
        m_error = ::pthread_mutex_lock(m_mx);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        if(0 != m_error)
        {
            STLSOFT_THROW_X(synchronisation_exception("Mutex lock failed", m_error));
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    /// \brief Attempts to lock the mutex
    ///
    /// \return <b>true</b> if the mutex was aquired, or <b>false</b> if not.
    ///
    /// \exception unixstl::synchronisation_exception When compiling with exception support, this will throw
    /// unixstl::synchronisation_exception if the lock cannot be acquired for a reason
    /// other than a timeout (<code>EBUSY</code>). When compiling absent
    /// exception support, failure to acquire the lock (for any other
    /// reason) will be reflected in a non-zero return from get_error().
    bool try_lock()
    {
        m_error = ::pthread_mutex_trylock(m_mx);

        if(0 == m_error)
        {
            return true;
        }
        else
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            if(EBUSY != m_error)
            {
                STLSOFT_THROW_X(synchronisation_exception("Mutex try-lock failed", m_error));
            }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

            return false;
        }
    }
    /// \brief Releases an aquired lock on the mutex
    ///
    /// \exception unixstl::synchronisation_exception When compiling with exception support, this will throw
    /// unixstl::synchronisation_exception if the lock cannot be released. When
    /// compiling absent exception support, failure to release the lock
    /// will be reflected in a non-zero return from get_error().
    void unlock()
    {
        m_error = ::pthread_mutex_unlock(m_mx);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        if(0 != m_error)
        {
            STLSOFT_THROW_X(synchronisation_exception("Mutex unlock failed", m_error));
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    /// \brief Contains the last failed error code from the underlying PTHREADS API
    int get_error() const stlsoft_throw_0()
    {
        return m_error;
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief The underlying kernel object handle
    pthread_mutex_t* handle() stlsoft_throw_0()
    {
        return m_mx;
    }
    /// \brief The underlying kernel object handle
    pthread_mutex_t* get() stlsoft_throw_0()
    {
        return m_mx;
    }
/// @}

/// \name Implementation
/// @{
private:
#if defined(STLSOFT_COMPILER_IS_SUNPRO)
    static int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
    {
        return ::pthread_mutexattr_destroy(attr);
    }
#endif /* compiler */
    static int create_(pthread_mutex_t* mx, int pshared, bool_type bRecursive)
    {
        pthread_mutexattr_t attr;
        int                 res = 0;

        if(0 == (res = ::pthread_mutexattr_init(&attr)))
        {
            stlsoft::scoped_handle<pthread_mutexattr_t*>    attr_(&attr, pthread_mutexattr_destroy);

            if( !bRecursive ||
                0 == (res = ::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)))
            {
#if defined(_POSIX_THREAD_PROCESS_SHARED)
                if(0 != (res = ::pthread_mutexattr_setpshared(&attr, pshared)))
                {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                    STLSOFT_THROW_X(synchronisation_exception("failed to set process-sharing attribute for PTHREADS mutex", res));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                }
                else
#else /* ? _POSIX_THREAD_PROCESS_SHARED */
                STLSOFT_SUPPRESS_UNUSED(pshared);
#endif /* _POSIX_THREAD_PROCESS_SHARED */
                {
                    if(0 == (res = ::pthread_mutex_init(mx, &attr)))
                    {
                    }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                    else
                    {
                        STLSOFT_THROW_X(synchronisation_exception("failed to set initialise PTHREADS mutex", res));
                    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                }
            }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            else
            {
                STLSOFT_THROW_X(synchronisation_exception("failed to set recursive attribute to PTHREADS mutex", res));
            }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        else
        {
            STLSOFT_THROW_X(synchronisation_exception("failed to initialise PTHREADS mutex attributes", res));
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return res;
    }
/// @}

/// \name Members
/// @{
private:
    pthread_mutex_t         m_mx_;          // The mutex used when created and owned by the instance
    pthread_mutex_t* const  m_mx;           // The mutex "handle"
    int                     m_error;        // The last PThreads error
    const bool_type         m_bOwnHandle;   // Does the instance own the handle?
/// @}

/// \name Not to be implemented
/// @{
private:
    process_mutex(class_type const& rhs);
    process_mutex& operator =(class_type const& rhs);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Control shims
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/** \brief This \ref group__concept__shims "control shim" aquires a lock on the given mutex
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param mx The mutex on which to aquire the lock.
 */
inline void lock_instance(unixstl_ns_qual(process_mutex) &mx)
{
    mx.lock();
}

/** \brief This \ref group__concept__shims "control shim" releases a lock on the given mutex
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param mx The mutex on which to release the lock
 */
inline void unlock_instance(unixstl_ns_qual(process_mutex) &mx)
{
    mx.unlock();
}


#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace unixstl
{
# else /* ? _STLSOFT_NO_NAMESPACE */
namespace unixstl_project
{
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * lock_traits
 */

// class lock_traits
/** \brief Traits for the process_mutex class
 *
 * \ingroup group__library__synch
 */
struct process_mutex_lock_traits
{
public:
    /// The lockable type
    typedef process_mutex                lock_type;
    typedef process_mutex_lock_traits    class_type;

// Operations
public:
    /// Lock the given process_mutex instance
    static void lock(process_mutex &c)
    {
#if defined(STLSOFT_COMPILER_IS_BORLAND)
        // Borland requires that we explicitly qualify the shim functions, even
        // though they're defined in the enclosing namespace of this one.
        stlsoft_ns_qual(lock_instance)(c);
#else /* ? compiler */
        lock_instance(c);
#endif /* compiler */
    }

    /// Unlock the given process_mutex instance
    static void unlock(process_mutex &c)
    {
#if defined(STLSOFT_COMPILER_IS_BORLAND)
        // Borland requires that we explicitly qualify the shim functions, even
        // though they're defined in the enclosing namespace of this one.
        stlsoft_ns_qual(unlock_instance)(c);
#else /* ? compiler */
        unlock_instance(c);
#endif /* compiler */
    }
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/process_mutex_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX */

/* ///////////////////////////// end of file //////////////////////////// */
