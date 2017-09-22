/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/synch/semaphore.hpp
 *
 * Purpose:     Semaphore class, based on Win32 kernel semaphore object.
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


/** \file winstl/synch/semaphore.hpp
 *
 * \brief [C++ only] Definition of winstl::semaphore class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SEMAPHORE
#define WINSTL_INCL_WINSTL_SYNCH_HPP_SEMAPHORE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYNCH_HPP_SEMAPHORE_MAJOR    1
# define WINSTL_VER_WINSTL_SYNCH_HPP_SEMAPHORE_MINOR    3
# define WINSTL_VER_WINSTL_SYNCH_HPP_SEMAPHORE_REVISION 4
# define WINSTL_VER_WINSTL_SYNCH_HPP_SEMAPHORE_EDIT     25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
# include <stlsoft/synch/concepts.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */
#ifndef WINSTL_INCL_WINSTL_SYNCH_ERROR_HPP_EXCEPTIONS
# include <winstl/synch/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_ERROR_HPP_EXCEPTIONS */

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

// class semaphore
/** This class acts as an semaphore based on the Win32
 *   kernel semaphore object
 *
 * \ingroup group__library__synch
 */
class semaphore
    : public stlsoft_ns_qual(critical_section)< STLSOFT_CRITICAL_SECTION_ISNOT_RECURSIVE
                                            ,   STLSOFT_CRITICAL_SECTION_IS_TRYABLE
                                            >
    , public stlsoft_ns_qual(synchronisable_object_tag)
{
public:
    /// This type
    typedef semaphore       class_type;
    /// The synchronisation handle type
    typedef HANDLE          synch_handle_type;
    /// The Boolean type
    typedef ws_bool_t       bool_type;
    /// The count type
    typedef ws_size_t       count_type;
    /// The resource type
    typedef HANDLE          resource_type;
private:
#if 0
    typedef LONG            underlying_count_type_;
#else /* ? 0 */
    typedef count_type      underlying_count_type_;
#endif /* 0 */

public:
    enum
    {
        maxCountValue   =   0x7fffffff  ///!< Borrowed from PThreads-win32
    };

/// \name Construction
/// @{
public:
    /// Conversion constructor
    semaphore(synch_handle_type sem, bool_type bTakeOwnership)
        : m_sem(sem)
        , m_maxCount(0)
        , m_bOwnHandle(bTakeOwnership)
    {
        WINSTL_ASSERT(NULL != sem);
    }
    /// Creates an instance of the semaphore
    ss_explicit_k semaphore(count_type initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(NULL, initialCount, maxCount, static_cast<ws_char_a_t const*>(NULL)))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    // This disambiguates cases where the initial count is 0
    ss_explicit_k semaphore(int initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(NULL, static_cast<count_type>(initialCount), maxCount, static_cast<ws_char_a_t const*>(NULL)))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
    /// Creates an instance of the semaphore
    ss_explicit_k semaphore(ws_char_a_t const* name, count_type initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(NULL, initialCount, maxCount, name))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}
    /// Creates an instance of the semaphore
    ss_explicit_k semaphore(ws_char_w_t const* name, count_type initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(NULL, initialCount, maxCount, name))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}
    /// Creates an instance of the semaphore
    ss_explicit_k semaphore(ws_char_a_t const* name, LPSECURITY_ATTRIBUTES psa, count_type initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(psa, initialCount, maxCount, name))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}
    /// Creates an instance of the semaphore
    ss_explicit_k semaphore(ws_char_w_t const* name, LPSECURITY_ATTRIBUTES psa, count_type initialCount, count_type maxCount = maxCountValue)
        : m_sem(create_semaphore_(psa, initialCount, maxCount, name))
        , m_maxCount(maxCount)
        , m_bOwnHandle(true)
    {}

    /// Destroys an instance of the semaphore
    ~semaphore() stlsoft_throw_0()
    {
        if( NULL != m_sem &&
            m_bOwnHandle)
        {
            ::CloseHandle(m_sem);
        }
    }
/// @}

/// \name Operations
/// @{
public:
    /// Acquires a lock on the semaphore, pending the thread until the lock is aquired
    void lock()
    {
        WINSTL_ASSERT(NULL != m_sem);

        DWORD const dwRes = ::WaitForSingleObject(m_sem, INFINITE);

        if(WAIT_OBJECT_0 != dwRes)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(wait_failed_logic_exception(e, "semaphore wait failed"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("semaphore wait failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
    /// Acquires a lock on the semaphore, pending the thread until the lock is aquired
    bool_type lock(ws_dword_t wait)
    {
        WINSTL_ASSERT(NULL != m_sem);

        DWORD const dwRes = ::WaitForSingleObject(m_sem, wait);

        if( WAIT_OBJECT_0 != dwRes &&
            WAIT_TIMEOUT != dwRes)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(wait_failed_logic_exception(e, "semaphore wait failed"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("semaphore wait failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return (dwRes == WAIT_OBJECT_0);
    }
    /// Attempts to lock the semaphore
    ///
    /// \return <b>true</b> if the semaphore was aquired, or <b>false</b> if not
    bool_type try_lock()
    {
        return lock(0);
    }
    /// Releases an aquired lock on the semaphore, increasing the
    ///  semaphore's counter by one.
    ///
    /// \note Equivalent to \link winstl::semaphore::unlock(count_type) unlock()\endlink.
    void unlock()
    {
        WINSTL_ASSERT(NULL != m_sem);

        if(!::ReleaseSemaphore(m_sem, 1, NULL))
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_object_state_change_failed_exception(e, "semaphore release failed", Synchronisation_SemaphoreReleaseFailed));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("semaphore release failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
    /// Releases a number of aquired "locks" on the semaphore,
    ///  increasing the semaphore's counter by the given value.
    ///
    /// \param numLocksToRelease The number by which to increment the
    ///  semaphore's counter. If this is greater than the available
    ///  value, the function fails. (It will throw an exception, if
    ///  exception handling is enabled, or return -1 otherwise.)
    ///
    /// \return Returns the value of the semaphore's counter prior to the
    ///  change effected by the call.
    /// \retval -1 Indicates call failure (only if exception handling is not
    ///  enabled).
    ws_long_t unlock(count_type numLocksToRelease)
    {
        WINSTL_ASSERT(NULL != m_sem);
        WINSTL_ASSERT(numLocksToRelease > 0);
        WINSTL_ASSERT(static_cast<LONG>(numLocksToRelease) > 0);

        LONG    previousCount   =   0;

        if(!::ReleaseSemaphore(m_sem, static_cast<LONG>(numLocksToRelease), &previousCount))
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_object_state_change_failed_exception(e, "semaphore release failed", Synchronisation_SemaphoreReleaseFailed));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("semaphore release failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            return -1;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return static_cast<ws_long_t>(previousCount);
    }
/// @}

/// \name Accessors
/// @{
public:
    /// The underlying kernel object handle
    synch_handle_type handle()
    {
        return m_sem;
    }
    /// The underlying kernel object handle
    synch_handle_type get()
    {
        return m_sem;
    }
/// @}

// Implementation
private:
    static HANDLE CreateSemaphore_A_arguments_adjusted_(
        LPSECURITY_ATTRIBUTES   psa
    ,   underlying_count_type_  initialCount
    ,   underlying_count_type_  maxCount
    ,   ws_char_a_t const*      name
    )
    {
        return STLSOFT_NS_GLOBAL(CreateSemaphoreA)(psa, static_cast<LONG>(initialCount), static_cast<LONG>(maxCount), name);
    }
    static HANDLE CreateSemaphore_W_arguments_adjusted_(
        LPSECURITY_ATTRIBUTES   psa
    ,   underlying_count_type_  initialCount
    ,   underlying_count_type_  maxCount
    ,   ws_char_w_t const*      name
    )
    {
        return STLSOFT_NS_GLOBAL(CreateSemaphoreW)(psa, static_cast<LONG>(initialCount), static_cast<LONG>(maxCount), name);
    }


    static synch_handle_type create_semaphore_(
        LPSECURITY_ATTRIBUTES   psa
    ,   underlying_count_type_  initialCount
    ,   underlying_count_type_  maxCount
    ,   ws_char_a_t const*      name
    )
    {
        WINSTL_MESSAGE_ASSERT("Maximum semaphore count must be > 0", maxCount > 0);
        WINSTL_ASSERT(initialCount <= maxCount);

        synch_handle_type sem = CreateSemaphore_A_arguments_adjusted_(psa, initialCount, maxCount, name);

        if(NULL == sem)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_creation_exception(e, "failed to create kernel semaphore object"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("failed to create kernel semaphore object", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return sem;
    }
    static synch_handle_type create_semaphore_(
        LPSECURITY_ATTRIBUTES   psa
    ,   underlying_count_type_  initialCount
    ,   underlying_count_type_  maxCount
    ,   ws_char_w_t const*      name
    )
    {
        WINSTL_MESSAGE_ASSERT("Maximum semaphore count must be > 0", maxCount > 0);
        WINSTL_ASSERT(initialCount <= maxCount);

        synch_handle_type sem = CreateSemaphore_W_arguments_adjusted_(psa, initialCount, maxCount, name);

        if(NULL == sem)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_creation_exception(e, "failed to create kernel semaphore object"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("failed to create kernel semaphore object", e)));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return sem;
    }

// Members
private:
    synch_handle_type   m_sem;          // The underlying semaphore object
    const count_type    m_maxCount;     // Record of the maximum counter value
    const bool_type     m_bOwnHandle;   // Does the instance own the handle?

// Not to be implemented
private:
    semaphore(class_type const& rhs);
    semaphore& operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** Overload of the form of the winstl::get_synch_handle() shim for
 *    the winstl::semaphore type.
 *
 * \param sem The winstl::semaphore instance
 *
 * \retval The synchronisation handle of \c sem
 */
inline HANDLE get_synch_handle(semaphore &sem)
{
    return sem.get();
}

/** Overload of the form of the winstl::get_kernel_handle() shim for
 *    the winstl::semaphore type.
 *
 * \ingroup group__library__shims__kernel_handle_attribute
 *
 * \param sem The winstl::semaphore instance
 *
 * \retval The synchronisation handle of \c sem
 */
inline HANDLE get_kernel_handle(semaphore &sem)
{
    return sem.get();
}


#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/** This \ref group__concept__shims "control shim" aquires a lock on the given semaphore
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param sem The semaphore on which to aquire the lock.
 */
inline void lock_instance(winstl_ns_qual(semaphore) &sem)
{
    sem.lock();
}

/** This \ref group__concept__shims "control shim" releases a lock on the given semaphore
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param sem The semaphore on which to release the lock
 */
inline void unlock_instance(winstl_ns_qual(semaphore) &sem)
{
    sem.unlock();
}

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace winstl {
# else
namespace winstl_project {
#  if defined(STLSOFT_COMPILER_IS_BORLAND)
using ::stlsoft::lock_instance;
using ::stlsoft::unlock_instance;
#  endif /* compiler */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * lock_traits
 */

// class lock_traits
/** Traits for the semaphore class
 *
 * \ingroup group__library__synch
 */
struct semaphore_lock_traits
{
public:
    /// The lockable type
    typedef semaphore                lock_type;
    /// This type
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

#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SEMAPHORE */

/* ///////////////////////////// end of file //////////////////////////// */
