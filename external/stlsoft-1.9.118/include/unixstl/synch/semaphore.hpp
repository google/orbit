/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/semaphore.hpp
 *
 * Purpose:     Semaphore class, based on POSIX semaphore object.
 *
 * Created:     30th May 2006
 * Updated:     13th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2014, Matthew Wilson and Synesis Software
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


/** \file unixstl/synch/semaphore.hpp
 *
 * \brief [C++ only] Definition of unixstl::semaphore class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SEMAPHORE
#define UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SEMAPHORE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SEMAPHORE_MAJOR    1
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SEMAPHORE_MINOR    2
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SEMAPHORE_REVISION 3
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SEMAPHORE_EDIT     21
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
# include <stlsoft/synch/concepts.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_ERROR_HPP_EXCEPTIONS
# include <unixstl/synch/error/exceptions.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_ERROR_HPP_EXCEPTIONS */

#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */
#ifndef STLSOFT_INCL_H_SEMAPHORE
# define STLSOFT_INCL_H_SEMAPHORE
# include <semaphore.h>
#endif /* !STLSOFT_INCL_H_SEMAPHORE */

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

// class semaphore
/** \brief This class acts as an semaphore based on the POSIX
 *   semaphore object
 *
 * \ingroup group__library__synch
 */
class semaphore
    : public stlsoft_ns_qual(critical_section)< STLSOFT_CRITICAL_SECTION_ISNOT_RECURSIVE
                                            ,   STLSOFT_CRITICAL_SECTION_IS_TRYABLE
                                            >
    , public stlsoft_ns_qual(synchronisable_object_tag)
{
/// \name Member Types
/// @{
public:
    typedef semaphore       class_type;
    typedef sem_t*          handle_type;
    typedef us_bool_t       bool_type;
    typedef us_size_t       count_type;

    typedef sem_t*          resource_type;
/// @}

/// \name Member Constants
/// @{
public:
    enum
    {
        maxCountValue   =   _POSIX_SEM_VALUE_MAX    // Borrowed from PThreads-win32
    };
/// @}

/// \name Construction
/// @{
public:
    /// \brief Conversion constructor
    semaphore(handle_type sem, bool_type bTakeOwnership)
        : m_sem(sem)
        , m_bOwnHandle(bTakeOwnership)
    {
        UNIXSTL_ASSERT(NULL != sem);
    }
    /// \brief Creates an instance of the semaphore
    ss_explicit_k semaphore(count_type initialCount, bool_type bInterProcessShared = false)
        : m_sem(create_semaphore_(&m_semInternal, initialCount, bInterProcessShared))
        , m_bOwnHandle(true)
    {}

    /// \brief Destroys an instance of the semaphore
    ~semaphore() stlsoft_throw_0()
    {
        if( NULL != m_sem &&
            m_bOwnHandle)
        {
            ::sem_destroy(m_sem);
        }
    }

#if 0
    void close() stlsoft_throw_0()
    {
        if( NULL != m_sem &&
            m_bOwnHandle)
        {
            ::sem_destroy(m_sem);
            m_sem = NULL;
        }
    }
#endif /* 0 */

/// @}

/// \name Operations
/// @{
public:
    /// \brief Acquires a lock on the semaphore, pending the thread until the lock is aquired
    void lock()
    {
        UNIXSTL_ASSERT(NULL != m_sem);

        if(::sem_wait(m_sem) < 0)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(synchronisation_exception("semaphore wait failed", errno));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
    /// \brief Attempts to lock the semaphore
    ///
    /// \return <b>true</b> if the semaphore was aquired, or <b>false</b> if not
    bool_type try_lock()
    {
        UNIXSTL_ASSERT(NULL != m_sem);

        int res =   ::sem_trywait(m_sem);

        if(0 == res)
        {
            return true;
        }
        else
        {
            if(EAGAIN != res)
            {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                STLSOFT_THROW_X(synchronisation_exception("semaphore wait failed", errno));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
        }

        return false;
    }
    /// \brief Releases an aquired lock on the semaphore, increasing the
    ///  semaphore's counter by one.
    void unlock()
    {
        UNIXSTL_ASSERT(NULL != m_sem);

        if(::sem_post(m_sem) < 0)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(synchronisation_exception("semaphore release failed", errno));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief The underlying kernel object handle
    handle_type handle() stlsoft_throw_0()
    {
        return m_sem;
    }
    /// \brief The underlying kernel object handle
    handle_type get() stlsoft_throw_0()
    {
        return m_sem;
    }
/// @}

// Implementation
private:
    static handle_type create_semaphore_(sem_t* internal, count_type initialCount, bool_type bInterProcessShared)
    {
        UNIXSTL_ASSERT(initialCount <= maxCountValue);

        handle_type sem;

        if(::sem_init(internal, bInterProcessShared, initialCount) < 0)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(synchronisation_exception("failed to create kernel semaphore object", errno));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            sem = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            sem = internal;
        }

        return sem;
    }

// Members
private:
    sem_t               m_semInternal;  // The actual object if internally initialised
    handle_type         m_sem;          // Handle to the underlying semaphore object
    const bool_type     m_bOwnHandle;   // Does the instance own the handle?

// Not to be implemented
private:
    semaphore(class_type const& rhs);
    semaphore& operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/** \brief This \ref group__concept__shims "control shim" aquires a lock on the given semaphore
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param sem The semaphore on which to aquire the lock.
 */
inline void lock_instance(unixstl_ns_qual(semaphore) &sem)
{
    sem.lock();
}

/** \brief This \ref group__concept__shims "control shim" releases a lock on the given semaphore
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param sem The semaphore on which to release the lock
 */
inline void unlock_instance(unixstl_ns_qual(semaphore) &sem)
{
    sem.unlock();
}

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace unixstl {
# else
namespace unixstl_project {
#  if defined(STLSOFT_COMPILER_IS_BORLAND)
using ::stlsoft::lock_instance;
using ::stlsoft::unlock_instance;
#  endif /* compiler */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * lock_traits
 */

// class lock_traits
/** \brief Traits for the semaphore class
 *
 * \ingroup group__library__synch
 */
struct semaphore_lock_traits
{
public:
    /// The lockable type
    typedef semaphore                lock_type;
    typedef semaphore_lock_traits    class_type;

// Operations
public:
    /// Lock the given semaphore instance
    static void lock(semaphore &c)
    {
        lock_instance(c);
    }

    /// Unlock the given semaphore instance
    static void unlock(semaphore &c)
    {
        unlock_instance(c);
    }
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/semaphore_unittest_.h"
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

#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SEMAPHORE */

/* ///////////////////////////// end of file //////////////////////////// */
