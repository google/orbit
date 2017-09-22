/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/synch/spin_mutex.hpp (originally MWSpinMx.h, ::SynesisWin)
 *
 * Purpose:     Intra-process mutex, based on spin waits.
 *
 * Created:     27th August 1997
 * Updated:     10th August 2009
 *
 * Thanks:      To Rupert Kittinger, for pointing out that the prior
 *              implementation that always yielded was not really "spinning".
 *
 *              Brad Cox, for helping out in testing and fixing the
 *              implementation for MAC OSX (Intel).
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1997-2009, Matthew Wilson and Synesis Software
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


/** \file unixstl/synch/spin_mutex.hpp
 *
 * \brief [C++ only] Definition of the unixstl::spin_mutex class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SPIN_MUTEX
#define UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SPIN_MUTEX

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SPIN_MUTEX_MAJOR     5
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SPIN_MUTEX_MINOR     0
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SPIN_MUTEX_REVISION  3
# define UNIXSTL_VER_UNIXSTL_SYNCH_HPP_SPIN_MUTEX_EDIT      60
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
!UNIXSTL_OS_IS_MACOSX:
[Incompatibilies-end]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES
# include <unixstl/synch/util/features.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_UTIL_H_FEATURES */
#ifndef UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS
# error unixstl/synch/spin_mutex.hpp requires support for atomic integer operations. Consult unixstl/synch/util/features.h for details
#endif /* !UNIXSTL_HAS_ATOMIC_INTEGER_OPERATIONS */
#ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS
# include <unixstl/synch/atomic_functions.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_H_ATOMIC_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
# include <stlsoft/synch/concepts.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_SPIN_POLICIES
# include <stlsoft/synch/spin_policies.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_SPIN_POLICIES */

#ifndef STLSOFT_INCL_H_SCHED
# define STLSOFT_INCL_H_SCHED
# include <sched.h>
#endif /* !STLSOFT_INCL_H_SCHED */

#ifdef STLSOFT_UNITTEST
# include <stlsoft/synch/lock_scope.hpp>
#endif /* STLSOFT_UNITTEST */

#if defined(_DEBUG)
# define    STLSOFT_SPINMUTEX_COUNT_LOCKS
#endif /* _DEBUG */

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

// class spin_mutex
/** \brief This class provides an implementation of the mutex model based on
 *    a spinning mechanism.
 *
 * \ingroup group__library__synch
 *
 * \note A spin mutex is not recursive. If you re-enter it your thread will
 *   be in irrecoverable deadlock.
 */
template <ss_typename_param_k SP>
class spin_mutex_base
    : public stlsoft_ns_qual(critical_section)< STLSOFT_CRITICAL_SECTION_ISNOT_RECURSIVE
                                            ,   STLSOFT_CRITICAL_SECTION_ISNOT_TRYABLE
                                            >
{
/// \name Member Types
/// @{
private:
    /// \brief The spin-policy class
    typedef SP                              spin_policy_class;
public:
    /// \brief This class
    typedef spin_mutex_base<SP>             class_type;
    /// \brief The atomic integer type
    typedef unixstl_ns_qual(atomic_int_t)   atomic_int_type;
    /// \brief The count type
    typedef us_sint32_t                     count_type;
    /// \brief The bool type
    typedef us_bool_t                       bool_type;
/// @}

/// \name Construction
/// @{
public:
#ifdef __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE
# pragma message(_sscomp_fileline_message("Create stlsoft/synch/spin_mutex_base.hpp, and factor out"))
#endif /* __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE */

    /// \brief Creates an instance of the mutex
    ///
    /// \param p Pointer to an external counter variable. May be NULL, in
    ///  which case an internal member is used for the counter variable.
    ///
    /// \note
    ss_explicit_k spin_mutex_base(atomic_int_type *p = NULL) stlsoft_throw_0()
        : m_spinCount((NULL != p) ? p : &m_internalCount)
        , m_internalCount(0)
#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
        , m_cLocks(0)
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS
        , m_spunCount(0)
        , m_bYieldOnSpin(spin_policy_class::value)
    {}
    /// \brief Creates an instance of the mutex
    ///
    /// \param p Pointer to an external counter variable. May be NULL, in
    ///  which case an internal member is used for the counter variable.
    /// \param bYieldOnSpin
    spin_mutex_base(atomic_int_type *p, bool_type bYieldOnSpin) stlsoft_throw_0()
        : m_spinCount((NULL != p) ? p : &m_internalCount)
        , m_internalCount(0)
#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
        , m_cLocks(0)
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS
        , m_spunCount(0)
        , m_bYieldOnSpin(bYieldOnSpin)
    {}
    /// Destroys an instance of the mutex
    ~spin_mutex_base() stlsoft_throw_0()
    {
#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
        UNIXSTL_ASSERT(0 == m_cLocks);
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Acquires a lock on the mutex, pending the thread until the lock is aquired
    void lock() stlsoft_throw_0()
    {
#ifdef UNIXSTL_SPINMUTEX_CHECK_INIT
        // If the dynamic initialisation phase has been skipped, the
        // members will all be assigned to 0, which is correct for
        // all except m_spinCount, which must be assigned to
        // &m_internalCount
        if(NULL == m_spinCount)
        {
            m_spinCount = &m_internalCount;
        }
#endif /* UNIXSTL_SPINMUTEX_CHECK_INIT  */

        UNIXSTL_MESSAGE_ASSERT("A global instance of spin_mutex has skipped dynamic initialisation. You must #define UNIXSTL_SPINMUTEX_CHECK_INIT if your compilation causes dynamic initialisation to be skipped.", NULL != m_spinCount);

#if defined(UNIXSTL_OS_IS_LINUX) && \
    !defined(UNIXSTL_ARCH_IS_INTEL)
        for(m_spunCount = 1; 0 != ::atomic_inc_and_test(m_spinCount); ++m_spunCount)
#elif defined(UNIXSTL_OS_IS_MACOSX)
        for(m_spunCount = 1; !::OSAtomicCompareAndSwap32Barrier(0, 1, m_spinCount); ++m_spunCount)
#elif defined(UNIXSTL_HAS_ATOMIC_WRITE)
        for(m_spunCount = 1; 0 != atomic_write(m_spinCount, 1); ++m_spunCount)
#else /* ? arch */
# error Your platform does not support atomic_write(), or has provides it in a manner unknown to the authors of UNIXSTL. If you know of the correct implementation, please send a patch.
#endif /* arch */
        {
            if(m_bYieldOnSpin)
            {
                ::sched_yield();
            }
        }

#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
        UNIXSTL_ASSERT(0 != ++m_cLocks);
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS
    }
    /// \brief Releases an aquired lock on the mutex
    void unlock() stlsoft_throw_0()
    {
#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
        UNIXSTL_ASSERT(m_cLocks-- != 0);
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS

        m_spunCount = 0;

#if defined(UNIXSTL_OS_IS_LINUX) && \
    !defined(UNIXSTL_ARCH_IS_INTEL)
        ::atomic_dec(m_spinCount);
#elif defined(UNIXSTL_OS_IS_MACOSX)
        ::OSAtomicDecrement32Barrier(m_spinCount);
#else /* ? arch */
        atomic_write(m_spinCount, 0);
#endif /* arch */
    }
/// @}

/// \name Attributes
/// @{
public:
    /// \brief An indicator as to the level of contention on the mutex.
    ///
    /// \note The value returned is only meaningful after lock() has been
    ///  called and before a corresponding unlock() has been called.
    ///
    /// \note The value returned is only reliable when an external counter
    ///  variable is being used, and when each spin_mutex instance is
    ///  thread-specific. In all other cases, the spun count is subject to
    ///  race conditions (that do <b>not</b> affect the good functioning of
    ///  the spin_mutex) and value returned may be, at best, used only as a
    ///  guide as to contention.
    count_type  spun_count() const
    {
        return m_spunCount;
    }
/// @}

/// \name Members
/// @{
private:
    atomic_int_type *m_spinCount;
    atomic_int_type m_internalCount;
#ifdef STLSOFT_SPINMUTEX_COUNT_LOCKS
    count_type      m_cLocks;       // Used as check on matched Lock/Unlock calls
#endif // STLSOFT_SPINMUTEX_COUNT_LOCKS
    count_type      m_spunCount;
    const bool_type m_bYieldOnSpin;
/// @}

/// \name Not to be implemented
/// @{
private:
    spin_mutex_base(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
/// @}
};

typedef spin_mutex_base<stlsoft_ns_qual(spin_yield)>        spin_mutex_yield;
typedef spin_mutex_base<stlsoft_ns_qual(spin_no_yield)>     spin_mutex_no_yield;

#ifdef STLSOFT_OLD_SPIN_MUTEX_BEHAVIOUR
typedef spin_mutex_no_yield                                 spin_mutex;
#else /* ? STLSOFT_OLD_SPIN_MUTEX_BEHAVIOUR */
typedef spin_mutex_yield                                    spin_mutex;
#endif /* STLSOFT_OLD_SPIN_MUTEX_BEHAVIOUR */

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
template <ss_typename_param_k SP>
inline void lock_instance(unixstl_ns_qual(spin_mutex_base)<SP> &mx)
{
    mx.lock();
}

/** \brief This \ref group__concept__shims "control shim" releases a lock on the given mutex
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param mx The mutex on which to release the lock
 */
template <ss_typename_param_k SP>
inline void unlock_instance(unixstl_ns_qual(spin_mutex_base)<SP> &mx)
{
    mx.unlock();
}


#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace unixstl
{
# else
namespace unixstl_project
{
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
/** \brief Traits for the spin_mutex class
 *
 * \ingroup group__library__synch
 */
struct spin_mutex_lock_traits
{
public:
    /// The lockable type
    typedef spin_mutex                lock_type;
    typedef spin_mutex_lock_traits    class_type;

// Operations
public:
    /// Lock the given spin_mutex instance
    static void lock(spin_mutex &c)
    {
        lock_instance(c);
    }

    /// Unlock the given spin_mutex instance
    static void unlock(spin_mutex &c)
    {
        unlock_instance(c);
    }
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/spin_mutex_unittest_.h"
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

#endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_SPIN_MUTEX */

/* ///////////////////////////// end of file //////////////////////////// */
