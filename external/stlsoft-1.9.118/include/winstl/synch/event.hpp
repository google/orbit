/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/synch/event.hpp
 *
 * Purpose:     event class, based on Windows EVENT.
 *
 * Created:     3rd July 2003
 * Updated:     13th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/synch/event.hpp
 *
 * \brief [C++ only] Definition of the winstl::event class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_EVENT
#define WINSTL_INCL_WINSTL_SYNCH_HPP_EVENT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYNCH_HPP_EVENT_MAJOR    4
# define WINSTL_VER_WINSTL_SYNCH_HPP_EVENT_MINOR    3
# define WINSTL_VER_WINSTL_SYNCH_HPP_EVENT_REVISION 2
# define WINSTL_VER_WINSTL_SYNCH_HPP_EVENT_EDIT     60
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

/** \brief Class which wraps the Win32 EVENT synchronisation object
 *
 * \ingroup group__library__synch
 */
class event
    : public stlsoft_ns_qual(synchronisable_object_tag)
{
public:
    /// This type
    typedef event           class_type;
    typedef HANDLE          synch_handle_type;

    typedef HANDLE          resource_type;

/// \name Construction
/// @{
public:
    /// \brief Creates the event
    event(ws_bool_t bManualReset, ws_bool_t bInitialState)
        : m_ev(create_event_(NULL, bManualReset, bInitialState, static_cast<ws_char_a_t const*>(0)))
        , m_bOwnHandle(true)
    {}
    /// \brief Creates the event with the given name
    ss_explicit_k event(ws_char_a_t const* name, ws_bool_t bManualReset, ws_bool_t bInitialState)
        : m_ev(create_event_(NULL, bManualReset, bInitialState, name))
        , m_bOwnHandle(true)
    {}
    /// \brief Creates the event with the given name
    ss_explicit_k event(ws_char_w_t const* name, ws_bool_t bManualReset, ws_bool_t bInitialState)
        : m_ev(create_event_(NULL, bManualReset, bInitialState, name))
        , m_bOwnHandle(true)
    {}
    /// \brief Creates the event with the given name and security attributes
    ss_explicit_k event(ws_char_a_t const* name, ws_bool_t bManualReset, ws_bool_t bInitialState, LPSECURITY_ATTRIBUTES psa)
        : m_ev(create_event_(psa, bManualReset, bInitialState, name))
        , m_bOwnHandle(true)
    {}
    /// \brief Creates the event with the given name and security attributes
    ss_explicit_k event(ws_char_w_t const* name, ws_bool_t bManualReset, ws_bool_t bInitialState, LPSECURITY_ATTRIBUTES psa)
        : m_ev(create_event_(psa, bManualReset, bInitialState, name))
        , m_bOwnHandle(true)
    {}
    /// \brief Destroys the event instance
    ~event() stlsoft_throw_0()
    {
        if( NULL != m_ev &&
            m_bOwnHandle)
        {
            ::CloseHandle(m_ev);
        }
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Sets the state of the event to signalled
    void set()
    {
        WINSTL_ASSERT(NULL != m_ev);

        if(!::SetEvent(m_ev))
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_object_state_change_failed_exception(e, "event set operation failed", Synchronisation_EventSetFailed));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("event set operation failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
    /// \brief Sets the state of the event to signalled
    void reset()
    {
        WINSTL_ASSERT(NULL != m_ev);

        if(!::ResetEvent(m_ev))
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_object_state_change_failed_exception(e, "event reset operation failed", Synchronisation_EventResetFailed));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("event reset operation failed", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief The underlying kernel object handle
    HANDLE  handle()
    {
        return m_ev;
    }
    /// \brief The underlying kernel object handle
    HANDLE  get()
    {
        return m_ev;
    }
/// @}

// Implementation
private:
    static HANDLE create_event_(LPSECURITY_ATTRIBUTES psa, ws_bool_t bManualReset, ws_bool_t bInitialState, ws_char_a_t const* name)
    {
        HANDLE  h = ::CreateEventA(psa, bManualReset, bInitialState, name);

        if(NULL == h)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_creation_exception(e, "failed to create kernel event object"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("failed to create kernel event object", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return h;
    }
    static HANDLE create_event_(LPSECURITY_ATTRIBUTES psa, ws_bool_t bManualReset, ws_bool_t bInitialState, ws_char_w_t const* name)
    {
        HANDLE h = ::CreateEventW(psa, bManualReset, bInitialState, name);

        if(NULL == h)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# if STLSOFT_LEAD_VER >= 0x010a0000
            STLSOFT_THROW_X(synchronisation_creation_exception(e, "failed to create kernel event object"));
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
            STLSOFT_THROW_X(synchronisation_exception("failed to create kernel event object", e));
# endif /* STLSOFT_LEAD_VER >= 1.10 */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return h;
    }

// Members
private:
    HANDLE          m_ev;
    const ws_bool_t m_bOwnHandle;   // Does the instance own the handle?

// Not to be implemented
private:
    event(class_type const& rhs);
    event& operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \brief Overload of the form of the winstl::get_synch_handle() shim for
 *    the winstl::event type.
 *
 * \param ev The winstl::event instance
 *
 * \retval The synchronisation handle of \c ev
 */
inline HANDLE get_synch_handle(event &ev)
{
    return ev.get();
}

/** \brief Overload of the form of the winstl::get_kernel_handle() shim for
 *    the winstl::event type.
 *
 * \ingroup group__library__shims__kernel_handle_attribute
 *
 * \param ev The winstl::event instance
 *
 * \retval The synchronisation handle of \c ev
 */
inline HANDLE get_kernel_handle(event &ev)
{
    return ev.get();
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/event_unittest_.h"
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

#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_EVENT */

/* ///////////////////////////// end of file //////////////////////////// */
