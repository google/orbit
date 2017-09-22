/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/synch/process_mutex.hpp
 *
 * Purpose:     Definition of the process_mutex type.
 *
 * Created:     20th March 2005
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


/** \file platformstl/synch/process_mutex.hpp
 *
 * \brief [C++ only] Definition of the platformstl::process_mutex type
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX
#define PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX_MAJOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX_MINOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX_REVISION   1
# define PLATFORMSTL_VER_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX_EDIT       18
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX
#  include <unixstl/synch/process_mutex.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_SYNCH_HPP_PROCESS_MUTEX */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_PROCESS_MUTEX
#  include <winstl/synch/process_mutex.hpp>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_PROCESS_MUTEX */
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
namespace platformstl
{
#else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)

    /** \brief A process mutex class
     *
     * The class is not actually defined in the
     * \link ::platformstl platformstl\endlink namespace. Rather, it
     * resolves to the appropriate type for the given platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::process_mutex
     * type resolves to the unixstl::process_mutex class. On Windows
     * platforms it resolves to the winstl::process_mutex class. It is not
     * defined for other platforms.
     */
class process_mutex
{
/// \name Member Types
/// @{
public:
#if defined(PLATFORMSTL_OS_IS_UNIX)
    typedef pthread_mutex_t *handle_type;
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
    typedef HANDLE          handle_type;
#else /* ? OS */
# error Operating system not discriminated
#endif /* OS */
    typedef process_mutex   class_type;

    typedef handle_type     resource_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs a mutex object
    process_mutex();
    /// \brief Destroys a mutex object
    ~process_mutex();
/// @}

/// \name Operations
/// @{
public:
    /// \brief Acquires a lock on the mutex, pending the thread until the lock is aquired
    void lock();
    /// \brief Attempts to lock the mutex
    ///
    /// \return <b>true</b> if the mutex was aquired, or <b>false</b> if not
    bool try_lock();
    /// \brief Releases an aquired lock on the mutex
    void unlock();
/// @}

/// \name Accessors
/// @{
public:
    /// \brief The underlying kernel object handle
    handle_type get();
/// @}
};

#elif defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _UNIXSTL_NO_NAMESPACE
    using ::process_mutex;
# else /* ? _UNIXSTL_NO_NAMESPACE */
    using ::unixstl::process_mutex;
# endif /* _UNIXSTL_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
    using ::process_mutex;
# else /* ? _WINSTL_NO_NAMESPACE */
    using ::winstl::process_mutex;
# endif /* _WINSTL_NO_NAMESPACE */

#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYNCH_HPP_PROCESS_MUTEX */

/* ///////////////////////////// end of file //////////////////////////// */
