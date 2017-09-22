/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/util/shared_handles.hpp
 *
 * Purpose:     Contains the shared_handle and monitored_shared_handle classes.
 *
 * Created:     19th January 2002
 * Updated:     10th August 2009
 *
 * Thanks:      To Austin Ziegler for fixes to defects evident on x64.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/util/shared_handles.hpp
 *
 *\brief [C++ only] [IMPLEMENTATION] Contains the 
 *  \link winstl::registry_util::shared_handle shared_handle\endlink
 *  and
 *  \link winstl::registry_util::monitored_shared_handle monitored_shared_handle\endlink
 *  classes that are used to provide shared context between iterators implement
 *  the \ref group__pattern__externally_invalidatable_iterator "Externally Invalidatable Iterator"
 *  pattern
 *   (\ref group__library__windows_registry "Windows Registry" Library).
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES
#define WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES_MAJOR       2
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES_MINOR       0
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES_REVISION    5
# define WINSTL_VER_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES_EDIT        29
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REGFWD
# include <winstl/registry/regfwd.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REGFWD */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS
# include <winstl/registry/util/defs.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS
# include <winstl/registry/reg_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_KEY
# include <winstl/registry/reg_key.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_KEY */
#ifndef WINSTL_INCL_SYNCH_WINSTL_HPP_EVENT
# include <winstl/synch/event.hpp>
#endif /* !WINSTL_INCL_SYNCH_WINSTL_HPP_EVENT */
#if !defined(STLSOFT_COMPILER_IS_COMO) && \
    !defined(STLSOFT_COMPILER_IS_WATCOM)
# ifndef WINSTL_INCL_WINSTL_DL_HPP_DL_CALL
#  include <winstl/dl/dl_call.hpp>
# endif /* !WINSTL_INCL_WINSTL_DL_HPP_DL_CALL */
#endif /* compiler */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXTERNAL_ITERATOR_INVALIDATION
#  include <stlsoft/error/external_iterator_invalidation.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXTERNAL_ITERATOR_INVALIDATION */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

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

#ifndef _STLSOFT_NO_NAMESPACES
/** \brief Internal/implementation namespace containing shared handles.
 *
 * \ingroup group__library__windows_registry
 */
namespace registry_util
{
#endif /* !_STLSOFT_NO_NAMESPACES */

    /// \brief [IMPLEMENTATION] Non-monitoring shared registry key context
    ///
    /// \ingroup group__library__windows_registry
#ifdef _STLSOFT_NO_NAMESPACES
    struct registry_util::shared_handle
#else /* ? _STLSOFT_NO_NAMESPACES */
    struct shared_handle
#endif /* _STLSOFT_NO_NAMESPACES */
    {
    /// \name Member Types
    /// @{
    public:
        typedef shared_handle   class_type;
        typedef HKEY            handle_type;
    /// @}

    /// \name Members
    /// @{
    public:
        handle_type m_hkey;
    private:
        ss_sint32_t m_refCount;
    /// @}

    /// \name Construction
    /// @{
    public:
        ss_explicit_k shared_handle(handle_type hkey)
            : m_hkey(hkey)
            , m_refCount(1)
        {}
    protected:
        shared_handle(handle_type hkey, ss_sint32_t refCount)
            : m_hkey(hkey)
            , m_refCount(refCount)
        {}
    protected:
        virtual ~shared_handle() stlsoft_throw_0()
        {
            WINSTL_MESSAGE_ASSERT("Shared search handle being destroyed with outstanding references!", 0 == m_refCount);

            if(NULL != m_hkey)
            {
                ::RegCloseKey(m_hkey);
            }
        }
    /// @}

    /// \name Operations
    /// @{
    public:
        ss_sint32_t AddRef()
        {
            return ++m_refCount;
        }
        ss_sint32_t Release()
        {
            ss_sint32_t rc = --m_refCount;

            if(0 == rc)
            {
                delete this;
            }

            return rc;
        }

        virtual void test_reset_and_throw()
        {}
    /// @}

    /// \name Not to be implemented
    /// @{
    private:
        shared_handle(class_type const&);
        class_type& operator =(class_type const&);
    /// @}
    };

#if defined(STLSOFT_CF_EXCEPTION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_COMO) && \
    !defined(STLSOFT_COMPILER_IS_WATCOM)
    /// \brief [IMPLEMENTATION] Monitoring shared registry key context
    ///
    /// \ingroup group__library__windows_registry
    ///
    /// \note This class is not defined in compilation modes that do not
    ///   support exception throwing/handling.
    struct monitored_shared_handle
        : public shared_handle
    {
    public:
        typedef shared_handle           parent_class_type;
        typedef monitored_shared_handle class_type;

    public:
        monitored_shared_handle(handle_type hkey, int eventType)
            : parent_class_type(hkey, 0)
            , m_eventType(eventType)
            , m_monitor(true, false)
        {
            set();

            AddRef();
        }

    /// \name Operations
    /// @{
    private:
        virtual void test_reset_and_throw()
        {
            // 1. Test, . . .
            if(WAIT_OBJECT_0 == ::WaitForSingleObject(m_monitor.handle(), 0))
            {
                // Must set to watch again here, because several iterators from the same
                // same reg_key_sequence could be open simultaneously

                // 2. Reset, . . .
                set();

                // 3. . . . and Throw
                STLSOFT_THROW_X(stlsoft_ns_qual(external_iterator_invalidation)("registry contents changed"));
            }
        }
    private:
        void set()
        {
            try
            {
                dl_call<LONG>(  "ADVAPI32.DLL"
#if defined(WINSTL_OS_IS_WIN64)
                            ,   "C:RegNotifyChangeKeyValue"
#elif defined(WINSTL_OS_IS_WIN32)
                            ,   "S:RegNotifyChangeKeyValue"
#else /* ? WIN?? */
# error Windows operating system not recognised
#endif /* WIN?? */
                            ,   m_hkey
                            ,   false
                            ,   (int)m_eventType
                            ,   m_monitor.handle()
                            ,   true);
            }
            catch(missing_entry_point_exception &)
            {
                if( 0 != (::GetVersion() & 0x80000000) &&
                    LOBYTE(LOWORD(GetVersion())) == 4 &&
                    HIBYTE(LOWORD(GetVersion())) < 10)
                {
                    // If it's Windows 95, which doesn't support this function, we need to
                    // quench it, and simply not do any external iterator invalidation
                    // checking
                    ;
                }
                else
                {
                    throw;
                }
            }
        }
    /// @}

    /// \name Members
    /// @{
    private:
        const int   m_eventType;
        event       m_monitor;  // The event that will monitor changes to the API.
    /// @}

    /// \name Not to be implemented
    /// @{
    private:
        monitored_shared_handle(class_type const&);
        class_type& operator =(class_type const&);
    /// @}
    };
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    /// \brief [IMPLEMENTATION] Simple factory function for creating an appropriate shared handle
    ///
    /// \ingroup group__library__windows_registry
    ///
    /// Used by basic_reg_key_sequence and basic_reg_value_sequence.
    ///
    /// \param hkey The registry key handle to be owned
    /// \param bMonitorExternalInvalidation If non-zero, the given \c eventType event is monitored.
    /// \param eventType The type of the event to monitor. One of REG_NOTIFY_CHANGE_NAME or REG_NOTIFY_CHANGE_LAST_SET
    ///
    /// \note The <code>bMonitorExternalInvalidation</code> and
    ///   <code>eventType</code> parameters are ignored in compilation
    ///   modes that do not support exception throwing/handling.
    static shared_handle *create_shared_handle(HKEY hkey, ws_bool_t bMonitorExternalInvalidation, int eventType)
    {
#if defined(STLSOFT_CF_EXCEPTION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_COMO) && \
    !defined(STLSOFT_COMPILER_IS_WATCOM)

        if(bMonitorExternalInvalidation)
        {
            return new monitored_shared_handle(hkey, eventType);
        }
        else
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        STLSOFT_SUPPRESS_UNUSED(bMonitorExternalInvalidation);
        STLSOFT_SUPPRESS_UNUSED(eventType);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        {
            return new shared_handle(hkey);
        }
    }

#ifndef _STLSOFT_NO_NAMESPACES
} // namespace registry_util
#endif /* !_STLSOFT_NO_NAMESPACES */

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

#endif /* WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES */

/* ///////////////////////////// end of file //////////////////////////// */
