/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/synch/tss_index.hpp (originally in MWTlsFns.h, ::SynesisWin)
 *
 * Purpose:     Wrapper class for Win32 TSS key.
 *
 * Created:     20th January 1999
 * Updated:     13th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/synch/tss_index.hpp
 *
 * \brief [C++ only] Definition of winstl::tss_index class
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_TSS_INDEX
#define WINSTL_INCL_WINSTL_SYNCH_HPP_TSS_INDEX

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYNCH_HPP_TSS_INDEX_MAJOR    4
# define WINSTL_VER_WINSTL_SYNCH_HPP_TSS_INDEX_MINOR    0
# define WINSTL_VER_WINSTL_SYNCH_HPP_TSS_INDEX_REVISION 5
# define WINSTL_VER_WINSTL_SYNCH_HPP_TSS_INDEX_EDIT     38
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef WINSTL_INCL_WINSTL_SYNCH_ERROR_HPP_EXCEPTIONS
#  include <winstl/synch/error/exceptions.hpp>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_ERROR_HPP_EXCEPTIONS */
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

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
/** \brief Indicates that a TSS key cannot be allocated.
 *
 * \note This exception indicates an irrecoverable condition.
 *
 * \ingroup group__library__synch
 */
class tss_exception
    : public synchronisation_exception
{
/// \name Types
/// @{
public:
    typedef synchronisation_exception           parent_class_type;
    typedef tss_exception                       class_type;
    typedef parent_class_type::error_code_type  error_code_type;
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k tss_exception(error_code_type err)
# if STLSOFT_LEAD_VER >= 0x010a0000
        : parent_class_type(err, "", Synchronisation_TssIndexCreationFailed)
# else /* ? STLSOFT_LEAD_VER >= 1.10 */
        : parent_class_type("", err)
# endif /* STLSOFT_LEAD_VER >= 1.10 */
    {}
/// @}

/// \name Accessors
/// @{
public:
    virtual char const* what() const stlsoft_throw_0()
    {
        return "failed to allocate a TSS key";
    }
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */


/** \brief Wrapper for a Win32 TSS key (TLS index).
 *
 * \ingroup group__library__synch
 */
class tss_index
{
/// \name Types
/// @{
public:
    /// \brief This class
    typedef tss_index   class_type;
    /// \brief The type of the TSS key
    typedef ws_dword_t  key_type;
    /// \brief The type of the TSS key
    ///
    /// \deprecated Deprecated in favour of key_type
    typedef key_type    index_type;
    /// \brief The type of the slot values
    typedef void*       value_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Allocates a TSS key
    ss_explicit_k tss_index()
        : m_index(index_create_())
    {}
    /// \brief Releases the TSS key
    ~tss_index() stlsoft_throw_0()
    {
        index_destroy_(m_index);
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Sets the value in the slot for the current thread
    void        set_value(value_type value)
    {
        class_type::set_slot_value_(m_index, value);
    }
    /// \brief Gets the value in the slot for the current thread
    value_type  get_value() const
    {
        return class_type::get_slot_value_(m_index);
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Implicit conversion operator to the
    operator key_type () const
    {
        return m_index;
    }
/// @}

/// \name Implementation
/// @{
private:
    static key_type index_create_()
    {
        key_type const key = ::TlsAlloc();

        if(0xFFFFFFFF == key)
        {
            DWORD const e = ::GetLastError();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(tss_exception(e));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            ::OutputDebugStringA("fatal: could not allocate a TSS key!\n");
            ::RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, 0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return key;
    }

    static void index_destroy_(key_type key)
    {
        ::TlsFree(key);
    }

    static void set_slot_value_(key_type key, value_type value)
    {
        ::TlsSetValue(key, value);
    }

    static value_type get_slot_value_(key_type key)
    {
        return ::TlsGetValue(key);
    }
/// @}

/// \name Members
/// @{
private:
    key_type    m_index;
/// @}

/// \name Not to be implemented
/// @{
private:
    tss_index(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

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

#endif /* WINSTL_INCL_WINSTL_SYNCH_HPP_TSS_INDEX */

/* ///////////////////////////// end of file //////////////////////////// */
