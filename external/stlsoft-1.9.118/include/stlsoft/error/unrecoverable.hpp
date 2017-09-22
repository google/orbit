/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/error/unrecoverable.hpp
 *
 * Purpose:     Definition of the \c unrecoverable exception class.
 *
 * Created:     14th October 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/error/unrecoverable.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::unrecoverable
 *   exception class
 *   (\ref group__library__error "Error" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_UNRECOVERABLE
#define STLSOFT_INCL_STLSOFT_ERROR_HPP_UNRECOVERABLE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ERROR_HPP_UNRECOVERABLE_MAJOR      2
# define STLSOFT_VER_STLSOFT_ERROR_HPP_UNRECOVERABLE_MINOR      0
# define STLSOFT_VER_STLSOFT_ERROR_HPP_UNRECOVERABLE_REVISION   2
# define STLSOFT_VER_STLSOFT_ERROR_HPP_UNRECOVERABLE_EDIT       29
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifdef STLSOFT_CF_std_NAMESPACE
# include <exception>
#else /* ? STLSOFT_CF_std_NAMESPACE */
# if defined(STLSOFT_COMPILER_IS_WATCOM)
#  include <except.h>       // for terminate()
#  include <stdexcep.h>     // for 'std' exceptions
# else /* ? compiler */
#  error No other non-std compiler supported
# endif /* compiler */
#endif /* STLSOFT_CF_std_NAMESPACE */
#if defined(STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS) || \
    defined(STLSOFT_COMPILER_IS_MWERKS)
# include <stdlib.h>        // for EXIT_FAILURE / exit()
#endif /* compiler */
#if defined(STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS)
# include <windows.h>       // for ExitProcess()
#endif /* STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS */

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

/** \brief Unrecoverable exception class
 *
 * \ingroup group__library__error
 *
 * Exceptions deriving from this class may be caught, but they result in
 * process termination at the end of the catch clause, or if they're not caught.
 */
class unrecoverable
    : public stlsoft_ns_qual_std(exception)
{
/// \name Types
/// @{
public:
    /// The parent type
    typedef stlsoft_ns_qual_std(exception)  parent_class_type;
    /// The type of the current instantiation
    typedef unrecoverable                   class_type;
/// @}

/// \name Construction
/// @{
protected:
    /// Default constructor
    ss_explicit_k unrecoverable(void (*pfnHandler)() = NULL)
        : m_refcnt(new long(1))
        , m_pfnHandler(pfnHandler)
    {}
public:
    /// Copy constructor
    ///
    /// \note The copy constructor effects a sharing of the internal 'instance count'
    /// in order to provide
    unrecoverable(class_type const& rhs)
        : m_refcnt(rhs.m_refcnt)
        , m_pfnHandler(rhs.m_pfnHandler)
    {
        ++*m_refcnt;
    }
    /// Destructor
    ///
    /// \note The destructor of each instance decrements the reference count of the
    ///
    virtual ~unrecoverable() stlsoft_throw_0()
    {
        if(0 == --*m_refcnt)
        {
            delete m_refcnt; // We could forgo this, but might as well be clean

            // Invoke the supplied handler
            if(NULL != m_pfnHandler)
            {
                (*m_pfnHandler)();
            }

            // If no handler was supplied, or it didn't close the process, we
            // call terminate to make sure. Terminate is chosen since it often
            // results in an "uglier" closedown than exit(), and that's a good
            // thing for unrecoverable exceptions
#if defined(STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS)
            ::ExitProcess(EXIT_FAILURE);
#else /* ? STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS */
# if defined(STLSOFT_COMPILER_IS_MWERKS)
            exit(EXIT_FAILURE);
# else /* ? compiler */
            {   // Because many compilers (including Borland, DMC++, GCC, etc.) place
                // terminate() within std, we introduce a block here, and 'use' the
                // std namespace. (Not a thing one normally does, you understand.)
#  ifdef STLSOFT_CF_std_NAMESPACE
                using namespace ::std;
#  endif /* !STLSOFT_CF_std_NAMESPACE */

                terminate();
            }
# endif /* compiler */
#endif /* STLSOFT_UNRECOVERABLE_EXCEPTION_USE_WIN32_EXITPROCESS */
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    /// Returns a human-readable string describing the exception condition
    virtual char const* what() const throw()
    {
        return "unrecoverable condition";
    }
/// @}

/// \name Members
/// @{
private:
    long *const m_refcnt;
    void        (*m_pfnHandler)();
/// @}

// Not to be implemented
private:
    class_type& operator =(class_type const&);

    void *operator new(ss_size_t );
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_UNRECOVERABLE */

/* ///////////////////////////// end of file //////////////////////////// */
