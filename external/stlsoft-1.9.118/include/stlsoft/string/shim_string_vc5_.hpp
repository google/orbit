/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/shim_string_vc5_.hpp
 *
 * Purpose:     Contains the basic_shim_string template class for VC++ 5.
 *
 * Created:     16th October 2006
 * Updated:     11th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/string/shim_string_vc5_.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::basic_shim_string class
 *   template for Visual C++ 5.0
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# error shim_string_vc5_.hpp can not be included in isolation: include stlsoft/string/shim_string.hpp instead
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STRING_HPP_SHIM_STRING_VC5__MAJOR      1
# define STLSOFT_VER_STLSOFT_STRING_HPP_SHIM_STRING_VC5_MINOR       1
# define STLSOFT_VER_STLSOFT_STRING_HPP_SHIM_STRING_VC5_REVISION    1
# define STLSOFT_VER_STLSOFT_STRING_HPP_SHIM_STRING_VC5_EDIT        12
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_NEW
# define STLSOFT_INCL_NEW
# include <new>
#endif /* !STLSOFT_INCL_NEW */

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

template<   ss_typename_param_k C
        ,   ss_size_t           N   = 64
        ,   ss_bool_t           U   = false
        >
class basic_shim_string
{
public:
    typedef C                           char_type;
    typedef ss_size_t                   size_type;
    typedef basic_shim_string<C, N, U>  class_type;
    typedef stlsoft_char_traits<C>      traits_type;

public:
    ss_explicit_k basic_shim_string(ss_size_t n)
        : m_buffer(NULL)
        , m_length(0)
    {
        if(NULL == (m_buffer = static_cast<char_type*>(::malloc(sizeof(char_type) * (1 + n)))))
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            m_length    =   n;
            m_buffer[0] = '\0';
            m_buffer[n] = '\0';
        }
    }

    basic_shim_string(char_type const* s, size_type n)
        : m_buffer(NULL)
        , m_length(0)
    {
        if(NULL == (m_buffer = static_cast<char_type*>(::malloc(sizeof(char_type) * (1 + n)))))
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            m_length    =   n;
            if(NULL == s)
            {
                m_buffer[0] = '\0';
            }
            else
            {
                traits_type::copy(&m_buffer[0], s, n);
            }
            m_buffer[n] = '\0';
        }
    }

    basic_shim_string(char_type const* s)
        : m_buffer(NULL)
        , m_length(0)
    {
        const ss_size_t n   =   (NULL == s) ? 0 : traits_type::length(s);

        if(NULL == (m_buffer = static_cast<char_type*>(::malloc(sizeof(char_type) * (1 + n)))))
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            m_length    =   n;
            traits_type::copy(&m_buffer[0], s, n);
            m_buffer[n] = '\0';
        }
    }

#if 0
    basic_shim_string(class_type const& rhs)
        : m_buffer(NULL)
        , m_length(0)
    {
        if(NULL == (m_buffer = static_cast<char_type*>(::malloc(sizeof(char_type) * (1 + rhs.size())))))
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            m_length                =   rhs.size();
            traits_type::copy(&m_buffer[0], &rhs.m_buffer[0], rhs.size());
            m_buffer[rhs.size()]    = '\0';
        }
    }
#else /* ? 0 */
    basic_shim_string(class_type& rhs)
        : m_buffer(rhs.m_buffer)
        , m_length(rhs.m_length)
    {
        rhs.m_buffer    =   NULL;
        rhs.m_length    =   0;
    }
#endif /* 0 */

    ~basic_shim_string()
    {
        ::free(m_buffer);
    }

    /// Swaps the contents of this instance with another
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        std_swap(m_buffer, rhs.m_buffer);
        std_swap(m_length, rhs.m_length);
    }

public:
    void write(char_type const* s)
    {
        traits_type::copy(&m_buffer[0], s, m_buffer.size());
        m_buffer[m_buffer.size() - 1] = '\0';
    }

    void truncate(size_type n)
    {
        STLSOFT_MESSAGE_ASSERT("shim_string truncation size must be <= current size", n <= size());

        m_buffer[n] = '\0';
    }

public:
    size_type       size() const
    {
        return m_length;
    }
    char_type       *data()
    {
        return m_buffer;
    }
    char_type const* data() const
    {
        return m_buffer;
    }

    operator char_type const* () const
    {
        const volatile ss_bool_t    b   =   U;

        return (!b || '\0' != m_buffer[0]) ? data() : NULL;
    }

private:
//  static

private:
    char_type*  m_buffer;
    ss_size_t   m_length;

private:
    class_type& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

template<   ss_typename_param_k C
        >
inline C const* c_str_data(basic_shim_string<C> const& ss)
{
    return ss.data();
}

inline ss_char_a_t const* c_str_data_a(basic_shim_string<ss_char_a_t> const& ss)
{
    return ss.data();
}

inline ss_char_w_t const* c_str_data_w(basic_shim_string<ss_char_w_t> const& ss)
{
    return ss.data();
}



template<   ss_typename_param_k C
        >
inline ss_size_t c_str_len(basic_shim_string<C> const& ss)
{
    return ss.size();
}

inline ss_size_t c_str_len_a(basic_shim_string<ss_char_a_t> const& ss)
{
    return ss.size();
}

inline ss_size_t c_str_len_w(basic_shim_string<ss_char_w_t> const& ss)
{
    return ss.size();
}



template<   ss_typename_param_k C
        >
inline C const* c_str_ptr(basic_shim_string<C> const& ss)
{
    return ss.data();
}

inline ss_char_a_t const* c_str_ptr_a(basic_shim_string<ss_char_a_t> const& ss)
{
    return ss.data();
}

inline ss_char_w_t const* c_str_ptr_w(basic_shim_string<ss_char_w_t> const& ss)
{
    return ss.data();
}



template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline S& operator <<(S& s, basic_shim_string<C> const& ss)
{
    s << ss.data();

    return s;
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ///////////////////////////// end of file //////////////////////////// */
