/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/synch/null_mutex.hpp (originally MLMutex.h, ::SynesisStd)
 *
 * Purpose:     Mutual exclusion model class.
 *
 * Created:     19th December 1997
 * Updated:     10th August 2009
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


/** \file stlsoft/synch/null_mutex.hpp
 *
 * \brief [C++ only] Definition of stlsoft::null_mutex class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_NULL_MUTEX
#define STLSOFT_INCL_STLSOFT_SYNCH_HPP_NULL_MUTEX

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SYNCH_HPP_NULL_MUTEX_MAJOR     4
# define STLSOFT_VER_STLSOFT_SYNCH_HPP_NULL_MUTEX_MINOR     0
# define STLSOFT_VER_STLSOFT_SYNCH_HPP_NULL_MUTEX_REVISION  1
# define STLSOFT_VER_STLSOFT_SYNCH_HPP_NULL_MUTEX_EDIT      41
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
# include <stlsoft/synch/concepts.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */

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

// class null_mutex

/** \brief This class provides a null implementation of the mutex model.
 *
 * \ingroup group__library__synch
 */
class null_mutex
    : public critical_section<  STLSOFT_CRITICAL_SECTION_IS_RECURSIVE
                            ,   STLSOFT_CRITICAL_SECTION_ISNOT_TRYABLE
                            >
{
/// \name Member Types
/// @{
public:
    typedef null_mutex class_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Creates an instance of the mutex
    null_mutex() stlsoft_throw_0()
    {}
/// @}

/// \name Operations
/// @{
public:
    /// \brief Acquires a lock on the mutex, pending the thread until the lock is aquired
    void lock() stlsoft_throw_0()
    {}
    /// \brief Releases an aquired lock on the mutex
    void unlock() stlsoft_throw_0()
    {}
/// @}

/// \name Not to be implemented
/// @{
private:
    null_mutex(class_type const& rhs);
    null_mutex& operator =(class_type const& rhs);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Control shims
 */

/** \brief This \ref group__concept__shims "control shim" aquires a lock on the given mutex
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param mx The mutex on which to aquire the lock.
 */
inline void lock_instance(stlsoft_ns_qual(null_mutex) &mx)
{
    STLSOFT_SUPPRESS_UNUSED(mx); // This is used, instead of the preferred omission of parameter, since it upsets the documentation
}

/** \brief This \ref group__concept__shims "control shim" releases a lock on the given mutex
 *
 * \ingroup group__concept__shim__synchronisation_control
 *
 * \param mx The mutex on which to release the lock
 */
inline void unlock_instance(stlsoft_ns_qual(null_mutex) &mx)
{
    STLSOFT_SUPPRESS_UNUSED(mx); // This is used, instead of the preferred omission of parameter, since it upsets the documentation
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/null_mutex_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_NULL_MUTEX */

/* ///////////////////////////// end of file //////////////////////////// */
