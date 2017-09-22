/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/pid_sequence.hpp
 *
 * Purpose:     Process Id sequence class.
 *
 * Created:     24th June 2005
 * Updated:     25th March 2010
 *
 * Thanks to:   Adi Shavit for spotting a small inefficiency in the
 *              resize()-ing, during the review of Extended STL volume 1
 *              (see http://extendedstl.com/).
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/system/pid_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::pid_sequence class
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_PID_SEQUENCE
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_PID_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_PID_SEQUENCE_MAJOR    2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_PID_SEQUENCE_MINOR    2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_PID_SEQUENCE_REVISION 2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_PID_SEQUENCE_EDIT     51
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_COMO:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
#  include <winstl/error/exceptions.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_VERSION
# include <winstl/system/system_version.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_VERSION */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD
# include <stlsoft/algorithms/pod.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#if !defined(_PSAPI_H_) && \
    !defined(_PSAPI_H)
# ifndef WINSTL_INCL_WINSTL_DL_HPP_DL_CALL
#  include <winstl/dl/dl_call.hpp>
# endif /* !WINSTL_INCL_WINSTL_DL_HPP_DL_CALL */
#endif /* psapi */

#if !defined(STLSOFT_UNITTEST)
# include <algorithm>
#endif /* !STLSOFT_UNITTEST */

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

/** \brief Process Id sequence
 *
 * \ingroup group__library__system
 */
class pid_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// The value type
    typedef DWORD                                                           value_type;
    /// The allocator type
    typedef processheap_allocator<value_type>                               allocator_type;
    /// The class type
    typedef pid_sequence                                                    class_type;
    /// The non-mutating (const) pointer type
    typedef value_type const*                                               const_pointer;
    /// The non-mutating (const) reference type
    typedef value_type const&                                               const_reference;
    /// The non-mutating (const) iterator type
    typedef stlsoft_ns_qual(pointer_iterator)<  value_type
                                            ,   const_pointer
                                            ,   const_reference
                                            >::type                         const_iterator;
    /// The size type
    typedef ws_size_t                                                       size_type;
    /// The difference type
    typedef ws_ptrdiff_t                                                    difference_type;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The non-mutating (const) reverse iterator type
    typedef stlsoft_ns_qual(const_reverse_bidirectional_iterator_base)< const_iterator
                                                                    ,   value_type
                                                                    ,   const_reference
                                                                    ,   const_pointer
                                                                    ,   difference_type
                                                                    >       const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

    enum
    {
            elideIdle   =   0x0001
        ,   elideSystem =   0x0002
        ,   sort        =   0x0004
    };
/// @}

/// \name Construction
/// @{
public:
    /// Constructs a sequence from the current processes in the host system
    ss_explicit_k pid_sequence(ws_uint32_t flags = elideIdle | elideSystem);
    /// Copies the contents of the sequence
    pid_sequence(class_type const& rhs);
    /// Releases the storage associated with the process id list
    ~pid_sequence() stlsoft_throw_0();
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator  begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Element Access
/// @{
public:
    /// Returns a reference to the element at the given index
    ///
    /// \param index The required index. Behaviour is undefined, if not within the range [0, size())
    const_reference operator [](size_type index) const;
/// @}

/// \name Attributes
/// @{
public:
    /// Indicates whether the sequence is empty
    ws_bool_t   empty() const;
    /// Returns the number of identifiers in the sequence
    size_type   size() const;
/// @}

/// \name System Traits
/// @{
public:
    /// \brief The process identifier of the Idle process
    ///
    /// \note The Idle process is a pseudo-process. You should not attempt to
    ///        manipulate it using the process control functions
    static value_type   idleProcessId()
    {
        return 0;
    }
    /// \brief The process identifier of the System process
    ///
    /// \note The System process is a pseudo-process. You should not attempt to
    ///        manipulate it using the process control functions
    static value_type   systemProcessId()
    {
        ws_uint32_t major   =   system_version::major();
        ws_uint32_t minor   =   system_version::minor();

        if(4 == major)
        {
            return 2;   // NT 4
        }
        else
        {
            if( 5 == major &&
                0 == minor)
            {
                return 8; // Win2K
            }
            else if(5 == major &&
                    1 == minor)
            {
                return 4; // WinXP
            }
            else
            {
                return 4; // Longhorn and above - this value is a guess!!
            }
        }
    }
/// @}

/// \name Members
/// @{
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   value_type
                                            ,   allocator_type
                                            ,   64
                                            >       buffer_type_;

    buffer_type_    m_pids;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/pid_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline pid_sequence::pid_sequence(ws_uint32_t flags)
    : m_pids(buffer_type_::internal_size())
{
    DWORD   cbReturned;

    for(;;)
    {
#if defined(_PSAPI_H_) || \
    defined(_PSAPI_H)
        if(!::EnumProcesses(&m_pids[0], sizeof(value_type) * m_pids.size(), &cbReturned))
#else /* ? psapi */
        if(!dl_call<BOOL>(  "PSAPI.DLL"
                        ,   WINSTL_DL_CALL_WINx_STDCALL_LITERAL("EnumProcesses")
                        ,   &m_pids[0]
                        ,   sizeof(value_type) * m_pids.size()
                        ,   &cbReturned))
#endif /* psapi */
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(windows_exception("Failed to enumerate processes", ::GetLastError()));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            m_pids.resize(0);

            break;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            const size_type n = cbReturned / sizeof(value_type);

            if(n < m_pids.size())
            {
                m_pids.resize(n);

                break;
            }
            else
            {
                const size_type size = m_pids.size();

                m_pids.resize(1); // Read "Extended STL, volume 1" to find out what this is for

                if(!m_pids.resize(2 * size))
                {
                    // This will only ever be executed when compiled in the
                    // absence of throwing bad_alloc on memory exhaustion
                    m_pids.resize(0);

                    break;
                }
            }
        }
    }

    if(flags & (elideIdle | elideSystem))
    {
        value_type* begin   =   &*m_pids.begin();
        value_type* end     =   &*m_pids.end();
        value_type* pIdle   =   (flags & elideIdle) ? stlsoft_ns_qual_std(find)(begin, end, idleProcessId()) : end;
        value_type* pSystem =   (flags & elideSystem) ? stlsoft_ns_qual_std(find)(begin, end, systemProcessId()) : end;

        // Optimise for the special case where idle is [0] and system is [1]
        if( end != pIdle &&
            end != pSystem &&
            pSystem == pIdle + 1)
        {
            pod_move(pSystem + 1, end, begin);
            m_pids.resize(m_pids.size() - 2);
        }
        else
        {
            if(end != pIdle)
            {
                pod_move(pIdle + 1, end, pIdle);
                m_pids.resize(m_pids.size() - 1);
            }
            if(end != pSystem)
            {
                pod_move(pSystem + 1, end, pSystem);
                m_pids.resize(m_pids.size() - 1);
            }
        }
    }

    if(flags & sort)
    {
        stlsoft_ns_qual_std(sort)(m_pids.begin(), m_pids.end());
    }
}

inline pid_sequence::pid_sequence(pid_sequence const& rhs)
    : m_pids(rhs.m_pids.size())
{
    stlsoft_ns_qual_std(copy)(rhs.m_pids.begin(), rhs.m_pids.end(), m_pids.begin());
}

inline pid_sequence::~pid_sequence() stlsoft_throw_0()
{}

inline pid_sequence::const_iterator pid_sequence::begin() const
{
    return &*m_pids.begin();
}

inline pid_sequence::const_iterator pid_sequence::end() const
{
    return &*m_pids.end();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
inline pid_sequence::const_reverse_iterator pid_sequence::rbegin() const
{
    return const_reverse_iterator(end());
}

inline pid_sequence::const_reverse_iterator pid_sequence::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

inline pid_sequence::const_reference pid_sequence::operator [](pid_sequence::size_type index) const
{
    WINSTL_MESSAGE_ASSERT("Index out of range", index < size());

    return m_pids[index];
}

inline ws_bool_t pid_sequence::empty() const
{
    return m_pids.empty();
}

inline pid_sequence::size_type pid_sequence::size() const
{
    return m_pids.size();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

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

#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_PID_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
