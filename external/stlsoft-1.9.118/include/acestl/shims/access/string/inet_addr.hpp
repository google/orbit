/* /////////////////////////////////////////////////////////////////////////
 * File:        acestl/shims/access/string/inet_addr.hpp
 *
 * Purpose:     Helper functions for ACE strings.
 *
 * Created:     23rd September 2004
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


/** \file acestl/shims/access/string/inet_addr.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>ACE_INET_Addr</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR
#define ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR_MAJOR      2
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR_MINOR      0
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR_REVISION   6
# define ACESTL_VER_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR_EDIT       43
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ACESTL_INCL_ACESTL_HPP_ACESTL
# include <acestl/acestl.hpp>
#endif /* !ACESTL_INCL_ACESTL_HPP_ACESTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_HPP_STD_BASIC_STRING
# include <stlsoft/shims/access/string/std/basic_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_HPP_BASIC_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */

#ifndef STLSOFT_INCL_ACE_H_INET_ADDR
# define STLSOFT_INCL_ACE_H_INET_ADDR
# include <ace/INET_Addr.h>                     // for ACE_INET_Addr
#endif /* !STLSOFT_INCL_ACE_H_INET_ADDR */
#ifndef STLSOFT_INCL_ACE_H_ACE_WCHAR
# define STLSOFT_INCL_ACE_H_ACE_WCHAR
# include <ace/ace_wchar.h>                     // for ACE_Wide_To_Ascii, ACE_Ascii_To_Wide
#endif /* !STLSOFT_INCL_ACE_H_ACE_WCHAR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::acestl */
namespace acestl
{
# else
/* Define stlsoft::acestl_project */

namespace stlsoft
{

namespace acestl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ACESTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace acestl_inet_addr_access_string_util
{
    template <ss_typename_param_k S>
    void stream_insert(S& s, ACE_INET_Addr const& addr)
    {
        typedef ::stlsoft::auto_buffer_old< ACE_TCHAR
                                        ,   ::stlsoft::allocator_selector<ACE_TCHAR>::allocator_type
                                        >       buffer_t;

        buffer_t    buffer(buffer_t::internal_size());

        for(; 0 != addr.addr_to_string(&buffer[0], buffer.size()); buffer.resize(2 * buffer.size()))
        {}

        buffer.resize(buffer.size() + 1);
        buffer[buffer.size() - 1] = '\0';

        s << buffer.data();
    }

    inline int invoke_addr_to_string(ACE_TCHAR const*, ACE_INET_Addr const& addr, ACE_TCHAR buffer[], as_size_t size, int fmt)
    {
        return addr.addr_to_string(buffer, size, fmt);
    }

# ifdef ACE_USES_WCHAR
    inline int invoke_addr_to_string(as_char_w_t const*, ACE_INET_Addr const& addr, as_char_a_t buffer[], as_size_t size, int fmt)
# else /* ? ACE_USES_WCHAR */
    inline int invoke_addr_to_string(as_char_a_t const*, ACE_INET_Addr const& addr, as_char_w_t buffer[], as_size_t size, int fmt)
# endif /* ACE_USES_WCHAR */
    {
# ifdef ACE_USES_WCHAR
        stlsoft::auto_buffer<as_char_w_t>   buff(1 + size);
# else /* ? ACE_USES_WCHAR */
        stlsoft::auto_buffer<as_char_a_t>   buff(1 + size);
# endif /* ACE_USES_WCHAR */
        int                                 res =   addr.addr_to_string(&buff[0], buff.size(), fmt);

        if(0 == res)
        {
            ACESTL_ASSERT(static_cast<ss_size_t>(res) < buff.size());

            buff[size] = '\0';

# ifdef ACE_USES_WCHAR
            ::strncpy(&buffer[0], ACE_TEXT_ALWAYS_CHAR(buff.data()), size);
# else /* ? ACE_USES_WCHAR */
            ::wcsncpy(&buffer[0], ACE_TEXT_ALWAYS_WCHAR(buff.data()), size);
# endif /* ACE_USES_WCHAR */
        }

        return res;
    }

    inline int invoke_addr_to_string(ACE_INET_Addr const& addr, as_char_a_t buffer[], as_size_t size, int fmt = 1)
    {
        return invoke_addr_to_string(static_cast<ACE_TCHAR const*>(0), addr, buffer, size, fmt);
    }
    inline int invoke_addr_to_string(ACE_INET_Addr const& addr, as_char_w_t buffer[], as_size_t size, int fmt = 1)
    {
        return invoke_addr_to_string(static_cast<ACE_TCHAR const*>(0), addr, buffer, size, fmt);
    }

    template <typename C>
    inline ::stlsoft::basic_shim_string<C> c_str_ptr_(ACE_INET_Addr const& addr)
    {
        typedef C                                                   char_t;
        typedef ::stlsoft::basic_shim_string<char_t>::buffer_type   buffer_t;

        ::stlsoft::basic_shim_string<char_t>    retVal(buffer_t::internal_size());
        buffer_t                                &buffer =   retVal.get_buffer();

        for(;0 != invoke_addr_to_string(addr, &buffer[0], buffer.size()); buffer.resize((3 * buffer.size()) / 2))
        {}

        buffer.resize(buffer.size() + 1);
        buffer[buffer.size() - 1] = '\0';

        return retVal;
    }

} // namespace acestl_inet_addr_access_string_util
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * String Access Shims
 */

// c_str_data

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_data_a(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_a_t>(addr);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_data_w(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_w_t>(addr);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_data for ACE_INET_Addr
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_data(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<ACE_TCHAR>(addr);
}

// c_str_len

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline as_size_t c_str_len_a(ACE_INET_Addr const& addr)
{
    return c_str_data_a(addr).size();
}
inline as_size_t c_str_len_w(ACE_INET_Addr const& addr)
{
    return c_str_data_w(addr).size();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_len for ACE_INET_Addr
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline as_size_t c_str_len(ACE_INET_Addr const& addr)
{
    return c_str_data(addr).size();
}

// c_str_ptr

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_ptr_a(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_a_t>(addr);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_ptr_w(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_w_t>(addr);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_ptr for ACE_INET_Addr
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_ptr(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<ACE_TCHAR>(addr);
}

// c_str_ptr_null

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ::stlsoft::basic_shim_string<as_char_a_t> c_str_ptr_null_a(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_a_t>(addr);
}
inline ::stlsoft::basic_shim_string<as_char_w_t> c_str_ptr_null_w(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<as_char_w_t>(addr);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** An overload of \ref group__concept__shim__string_access__c_str_ptr_null for ACE_INET_Addr
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline ::stlsoft::basic_shim_string<ACE_TCHAR> c_str_ptr_null(ACE_INET_Addr const& addr)
{
    return acestl_inet_addr_access_string_util::c_str_ptr_<ACE_TCHAR>(addr);
}


/* /////////////////////////////////////////////////////////////////////////
 * Stream Insertion Shims
 */

/** The \ref group__concept__shim__stream_insertion "stream insertion shim" for ACE_INET_Addr
 *
 * \ingroup group__concept__shim__stream_insertion
 *
 */
template <ss_typename_param_k S>
inline S& operator <<(S& s, ACE_INET_Addr const& addr)
{
    acestl_inet_addr_access_string_util::stream_insert(s, addr);

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/inet_addr_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ACESTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace acestl
# else
} // namespace acestl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */

/** The \ref group__concept__shim__stream_insertion "stream insertion shim" for ACE_INET_Addr.
 *
 * \ingroup group__concept__shim__stream_insertion
 *
 */
template <ss_typename_param_k S>
inline S& operator <<(S& s, ACE_INET_Addr const& addr)
{
    ::acestl::acestl_inet_addr_access_string_util::stream_insert(s, addr);

    return s;
}


namespace stlsoft
{

    using ::acestl::c_str_data;
    using ::acestl::c_str_data_a;
    using ::acestl::c_str_data_w;

    using ::acestl::c_str_len;
    using ::acestl::c_str_len_a;
    using ::acestl::c_str_len_w;

    using ::acestl::c_str_ptr;
    using ::acestl::c_str_ptr_a;
    using ::acestl::c_str_ptr_w;

    using ::acestl::c_str_ptr_null;
    using ::acestl::c_str_ptr_null_a;
    using ::acestl::c_str_ptr_null_w;

} // namespace stlsoft

#endif /* !_ACESTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* ACESTL_INCL_ACESTL_SHIMS_ACCESS_STRING_HPP_INET_ADDR */

/* ///////////////////////////// end of file //////////////////////////// */
