/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/toolhelp/sequence_base.hpp
 *
 * Purpose:     TOOLHELP sequence_base class template.
 *
 * Created:     21st May 2005
 * Updated:     10th August 2009
 *
 * Thanks:      To Pablo for contributing this great library.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Pablo Aguilar
 * Copyright (c) 2006-2007, Matthew Wilson
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
 * - Neither the name(s) of Matthew Wilson and Synesis Software, nor Pablo
 *   Aguilar, nor the names of any contributors may be used to endorse or
 *   promote products derived from this software without specific prior written
 *   permission.
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


/** \file winstl/toolhelp/sequence_base.hpp
 *
 * \brief [C++ only] Definition of the
 *  \link winstl::th_sequence_base th_sequence_base\endlink class
 *   (\ref group__library__windows_toolhelp "Windows ToolHelp" Library).
 */

#ifndef WINSTL_INCL_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE
#define WINSTL_INCL_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE_MAJOR     1
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE_MINOR     1
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE_REVISION  4
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE_EDIT      14
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */
#ifndef WINSTL_INCL_WINSTL_TOOLHELP_ERROR_HPP_SEQUENCE_EXCEPTION
# include <winstl/toolhelp/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_TOOLHELP_ERROR_HPP_SEQUENCE_EXCEPTION */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_H_TLHELP32
# define STLSOFT_INCL_H_TLHELP32
# include <tlhelp32.h>
#endif /* !STLSOFT_INCL_H_TLHELP32 */

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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template<ss_typename_param_k V>
struct th_sequence_value_traits;
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

    // NOTE: The following #defines could've been limited to 2, lower & upper
    //       case only. But the heap list API specifies the first/next
    //       functions as Heap32ListX, which is inconsistent with the
    //       other 3 styles
    //
    //       Each #define is #undef'd within the included file
    //
    //  Example for module_sequence:
    //  #define WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE   MODULEENTRY32
    //  #define WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FIRST   Module32First
    //  #define WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_NEXT    Module32Next
    //  #define WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FLAG    TH32CS_SNAPMODULE
    //  #define WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_USE_PID

/** \brief Iterator class for the th_sequence_base class template and its specialisations.
 *
 * \ingroup group__library__windows_toolhelp
 */
template<ss_typename_param_k V>
class th_sequence_const_iterator
    : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(input_iterator_tag)
                                        ,   V
                                        ,   ws_ptrdiff_t
                                        ,   V const*
                                        ,   V const&
                                        >
{
/// \name Member Types
/// @{
public:
    typedef th_sequence_const_iterator<V>               class_type;
    typedef V                                           value_type;
    typedef value_type const&                           const_reference;
    typedef HANDLE                                      handle_type;
    typedef th_sequence_value_traits<value_type>        traits_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance based on a null instance of the requisite handle type.
    th_sequence_const_iterator()
        : m_snapshot(traits_type::null_handle())
    {
        // Do nothing
    }
    /// \brief Constructs an instance based on a snapshot handle.
    th_sequence_const_iterator(handle_type snapshot)
        : m_snapshot(snapshot)
    {
        WINSTL_ASSERT(m_snapshot != traits_type::invalid_handle());

        ::memset(&m_value, 0, sizeof(m_value));
        m_value.dwSize = sizeof(m_value);

        bool ok = traits_type::first(m_snapshot, m_value);
        if( !ok )
        {
            m_snapshot = traits_type::null_handle();
        }
    }
    /// \brief Copy constructor
    th_sequence_const_iterator(class_type const& rhs)
        : m_snapshot(rhs.m_snapshot)
        , m_value   (rhs.m_value)
    {
        WINSTL_ASSERT(m_snapshot != traits_type::invalid_handle());
    }

    // NOTE: Default destructor is good enough
    // NOTE: Default copy assignment operator is good enough
/// @}

/// \name Forward Iterator methods
/// @{
public:
    const_reference operator *() const
    {
        return m_value;
    }

    class_type& operator ++()
    {
        bool ok = traits_type::next(m_snapshot, m_value);
        if( !ok )
            m_snapshot = traits_type::null_handle();

        return *this;
    }

    class_type operator ++(int)
    {
        class_type rhs(*this);
        ++*this;
        return rhs;
    }

    bool equal(class_type const& rhs) const
    {
        WINSTL_MESSAGE_ASSERT("Attempting to compare two non-endpoint iterators from different ranges", (NULL == m_snapshot || NULL == rhs.m_snapshot || m_snapshot == rhs.m_snapshot));

        return m_snapshot == rhs.m_snapshot;
    }

    bool operator ==(class_type const& rhs) const
    {
        return equal(rhs);
    }

    bool operator !=(class_type const& rhs) const
    {
        return !equal(rhs);
    }
/// @}

/// \name Members
/// @{
private:
    handle_type m_snapshot;
    value_type  m_value;
/// @}
};

/** \brief [IMPLEMENTATION] A utility class template that is used to
 *   to block the non-default constructor for specialisations of
 *   th_sequence_base that do not require the process Id.
 * \ingroup group__library__windows_toolhelp
 */
template<typename V>
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct sequence_doesnt_use_pid;
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
struct sequence_doesnt_use_pid {};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Class template for TOOLHELP STL collections.
 *
 * \ingroup group__library__windows_toolhelp
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k E = throw_exception_policy<toolhelp_exception>
        >
class th_sequence_base
{
/// \name Member Types
/// @{
public:
    /// \brief The size type.
    typedef th_sequence_base<V>                                 class_type;
    /// \brief The non-mutating (const) iterator type.
    typedef th_sequence_const_iterator<V>                       const_iterator;
    /// \brief The value type.
    typedef ss_typename_type_k const_iterator::value_type       value_type;
    /// \brief The mutating (non-const) reference type.
    typedef ss_typename_type_k const_iterator::reference        reference;
    /// \brief The non-mutating (const) type.
    typedef ss_typename_type_k const_iterator::const_reference  const_reference;
    /// \brief The handle type.
    typedef ss_typename_type_k const_iterator::handle_type      handle_type;
    /// \brief The traits type.
    typedef ss_typename_type_k const_iterator::traits_type      traits_type;
    /// \brief The exception policy type.
    typedef E                                                   exception_policy;
    /// \brief The size type.
    typedef ws_size_t                                           size_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance on a TOOLHELP snapshot for the calling process.
    th_sequence_base()
        : m_snapshot(::CreateToolhelp32Snapshot(
              traits_type::flag()
            , 0
        ))
    {
        verify_construction();
    }

    /// \brief Constructs an instance on a TOOLHELP snapshot for the specified process.
#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER > 1200
    th_sequence_base(DWORD process_id, DWORD = sizeof(sequence_doesnt_use_pid<value_type>))
#else /* ? compiler */
    th_sequence_base(DWORD process_id)
#endif /* compiler */
        : m_snapshot(::CreateToolhelp32Snapshot(
              traits_type::flag()
            , process_id
        ))
    {
        verify_construction();
    }

    /// \brief Destructor
    ~th_sequence_base()
    {
        if(traits_type::invalid_handle() != m_snapshot)
        {
            ::CloseHandle(m_snapshot);
        }
    }
/// @}

/// \name Iteration
/// @{
public:
    /// \brief Begins the iteration.
    ///
    /// \return An iterator representing the start of the sequence.
    const_iterator begin() const
    {
        return const_iterator(m_snapshot);
    }
    /// \brief Ends the iteration.
    ///
    /// \return An iterator representing the end of the sequence.
    const_iterator end() const
    {
        return const_iterator();
    }
/// @}

/// \name Attributes
/// @{
public:
    /// \brief Indicates whether the sequence is empty.
    bool empty() const
    {
        // NOTE: Not necessarily efficient
        //       First answer could be cached
        //       This is also provided just as a convenience
        return (begin() == end());
    }
/// @}

/// \name Members
/// @{
private:
    handle_type m_snapshot;
/// @}

/// \name Not to be implemented
/// @{
private:
    th_sequence_base(class_type const&);
    class_type& operator =(class_type const&);
/// @}

    void verify_construction()
    {
        if(traits_type::invalid_handle() == m_snapshot)
        {
            exception_policy    xp;
            DWORD               dwErr   =   ::GetLastError();

            xp(traits_type::create_snapshot_fail_message(), dwErr);
        }
    }
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

#endif /* WINSTL_INCL_WINSTL_TOOLHELP_HPP_SEQUENCE_BASE */

/* ///////////////////////////// end of file //////////////////////////// */
