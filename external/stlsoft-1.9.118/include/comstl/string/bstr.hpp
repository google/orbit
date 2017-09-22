/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/string/bstr.hpp (originally MOBStr.h/.cpp, ::SynesisCom)
 *
 * Purpose:     bstr class.
 *
 * Created:     20th December 1996
 * Updated:     5th March 2011
 *
 * Thanks:      To Gabor Fischer for requesting attach().
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1996-2011, Matthew Wilson and Synesis Software
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


/** \file comstl/string/bstr.hpp
 *
 * \brief [C++ only; requires COM] Definition of the comstl::bstr class
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_STRING_HPP_BSTR
#define COMSTL_INCL_COMSTL_STRING_HPP_BSTR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _COMSTL_VER_COMSTL_STRING_HPP_BSTR_MAJOR       2
# define _COMSTL_VER_COMSTL_STRING_HPP_BSTR_MINOR       8
# define _COMSTL_VER_COMSTL_STRING_HPP_BSTR_REVISION    4
# define _COMSTL_VER_COMSTL_STRING_HPP_BSTR_EDIT        62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
//#ifndef COMSTL_INCL_COMSTL_SHIMS_ACCESS_HPP_STRING
//# include <comstl/shims/access/string.hpp>
//#endif /* !COMSTL_INCL_COMSTL_SHIMS_ACCESS_HPP_STRING */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS
#  include <comstl/error/exceptions.hpp>
# endif /* !COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS */
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS
# include <comstl/string/BSTR_functions.h>
#endif /* !COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
# include <stlsoft/string/string_traits_fwd.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# include <stdexcept>
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Facade for the COM BSTR type
 *
 * \ingroup group__library__utility__com
 */
class bstr
{
/// \name Member Types
/// @{
public:
    typedef bstr                                    class_type;
    typedef cs_char_o_t                             char_type;
    typedef char_type                               value_type;
    typedef char_type*                              pointer;
    typedef char_type const*                        const_pointer;
    typedef char_type&                              reference;
    typedef char_type const&                        const_reference;
    typedef cs_ptrdiff_t                            difference_type;
    typedef cs_size_t                               size_type;
    typedef cs_ptrdiff_t                            ssize_type;
    typedef cs_bool_t                               bool_type;
    typedef pointer                                 iterator;
    typedef const_pointer                           const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef stlsoft_ns_qual(reverse_iterator_base)<
        iterator
    ,   value_type
    ,   reference
    ,   pointer
    ,   difference_type
    >                                               reverse_iterator;

    typedef stlsoft_ns_qual(const_reverse_iterator_base)<
        const_iterator
    ,   value_type const
    ,   const_reference
    ,   const_pointer
    ,   difference_type
    >                                               const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

    typedef BSTR                                    resource_type;
/// @}

/// \name Construction
/// @{
public:
    bstr();
    ss_explicit_k bstr(cs_char_a_t const* s, ssize_type len = -1);
    ss_explicit_k bstr(cs_char_w_t const* s, ssize_type len = -1);
    bstr(size_type n, char_type ch);
    /// \brief Copy constructor
    bstr(class_type const& rhs);
    bstr(class_type const& rhs, size_type pos, size_type len);
    ~bstr() stlsoft_throw_0();

    /// \brief Copies the given instance
    class_type& operator =(class_type const& rhs);

    class_type& assign(cs_char_a_t const* s, ssize_type len = -1);
    class_type& assign(cs_char_w_t const* s, ssize_type len = -1);

    class_type& assign(const_iterator from, const_iterator to);

    class_type& operator =(cs_char_a_t const* s);
    class_type& operator =(cs_char_w_t const* s);

    class_type& attach(BSTR bstr);

    BSTR        detach();

    void        clear();
/// @}

/// \name Operations
/// @{
public:
    class_type& append(class_type const& s, ssize_type len = -1);
    class_type& append(cs_char_w_t const* s, ssize_type len = -1);

    class_type& operator +=(class_type const& s);
    class_type& operator +=(cs_char_w_t const* s);
/// @}

/// \name Accessors
/// @{
public:
    const_pointer   data() const;
    const_pointer   c_str() const;
    size_type       length() const;
    size_type       size() const;
    bool_type       empty() const;
    BSTR            get() const;
    reference       operator [](size_type);
    const_reference operator [](size_type) const;

    const_pointer   *NonDestructiveAddress() const;
    BSTR            *NonDestructiveAddress();
    BSTR            *DestructiveAddress();
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator                begin();
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator                end();
    /// Begins the iteration
    ///
    /// \return A non-mutable (const) iterator representing the start of the sequence
    const_iterator          begin() const;
    /// Ends the iteration
    ///
    /// \return A non-mutable (const) iterator representing the end of the sequence
    const_iterator          end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// Ends the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the end of the reverse sequence
    const_reverse_iterator  rend() const;
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator        rbegin();
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator        rend();
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Comparison
/// @{
public:
    /// \brief Evaluates whether the value is equivalent to the given argument
    bool_type equal(class_type const& rhs) const;
    /// \brief Evaluates whether the value is equivalent to the given argument
    bool_type equal(BSTR const& rhs) const;
/// @}

/// \name Operations
/// @{
public:
    /// \brief Swaps the contents with the given instance
    void swap(class_type& rhs) stlsoft_throw_0();
    /// \brief Swaps the contents with the given BSTR
    void swap(BSTR& rhs) stlsoft_throw_0();
/// @}

/// \name Members
/// @{
private:
    BSTR    m_bstr;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * String access shims
 */

/** \brief \ref group__concept__shim__string_access__c_str_data for comstl::bstr
 *
 * \ingroup group__concept__shim__string_access
 */
inline bstr::const_pointer c_str_data(comstl_ns_qual(bstr) const& b)
{
    return b.data();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline bstr::const_pointer c_str_data_w(comstl_ns_qual(bstr) const& b)
{
    return b.data();
}

inline bstr::const_pointer c_str_data_o(comstl_ns_qual(bstr) const& b)
{
    return b.data();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for comstl::bstr
 *
 * \ingroup group__concept__shim__string_access
 */
inline cs_size_t c_str_len(comstl_ns_qual(bstr) const& b)
{
    return b.length();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline cs_size_t c_str_len_w(comstl_ns_qual(bstr) const& b)
{
    return b.length();
}

inline cs_size_t c_str_len_o(comstl_ns_qual(bstr) const& b)
{
    return b.length();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for comstl::bstr
 *
 * \ingroup group__concept__shim__string_access
 */
inline bstr::const_pointer c_str_ptr(comstl_ns_qual(bstr) const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline bstr::const_pointer c_str_ptr_w(comstl_ns_qual(bstr) const& b)
{
    return b.c_str();
}

inline bstr::const_pointer c_str_ptr_o(comstl_ns_qual(bstr) const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for comstl::bstr
 *
 * \ingroup group__concept__shim__string_access
 */
inline bstr::const_pointer c_str_ptr_null(comstl_ns_qual(bstr) const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline bstr::const_pointer c_str_ptr_null_w(comstl_ns_qual(bstr) const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null_w)(b.c_str());
}

inline bstr::const_pointer c_str_ptr_null_o(comstl_ns_qual(bstr) const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for comstl::bstr
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        >
inline S& operator <<(S& stm, comstl_ns_qual(bstr) const& str)
{
    STLSOFT_STATIC_ASSERT(sizeof(OLECHAR) == sizeof(ss_typename_type_k S::char_type));

    stm << str.c_str();

    return stm;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

inline cs_bool_t operator ==(bstr const& lhs, bstr const& rhs)
{
    return lhs.equal(rhs);
}

inline cs_bool_t operator !=(bstr const& lhs, bstr const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline cs_bool_t operator ==(bstr const& lhs, BSTR const& rhs)
{
    return lhs.equal(rhs);
}

inline cs_bool_t operator !=(bstr const& lhs, BSTR const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline cs_bool_t operator ==(BSTR const& lhs, bstr const& rhs)
{
    return rhs.equal(lhs);
}

inline cs_bool_t operator !=(BSTR const& lhs, bstr const& rhs)
{
    return !operator ==(lhs, rhs);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/bstr_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Construction
inline bstr::bstr()
    : m_bstr(NULL)
{}

inline /* explicit */ bstr::bstr(cs_char_a_t const* s, ssize_type len /* = -1 */)
{
    // Precondition tests
    COMSTL_MESSAGE_ASSERT("Default length must be specified by -1. No other -ve value allowed", (len >= 0 || len == -1));
    COMSTL_MESSAGE_ASSERT("Cannot pass in NULL pointer and -1 (default) length", (NULL != s || len >= 0));

    // There's a potential problem here (which has actually occurred!):
    //
    // If s is non-NULL and len is non-negative, it's possible for
    // the underlying SysAllocStringLen() to walk into invalid
    // memory while searching s.

    ssize_type actualLen = static_cast<ssize_type>(stlsoft_ns_qual(c_str_len)(s));

    if( NULL != s &&
        len > actualLen)
    {
        m_bstr = bstr_create(static_cast<cs_char_w_t const*>(NULL), static_cast<cs_size_t>(len));

        if(NULL != m_bstr)
        {
# ifdef _WIN64
            int buffLen = static_cast<int>(actualLen + 1);
# else /* ? _WIN64 */
            int buffLen = actualLen + 1;
# endif /* _WIN64 */

            ::MultiByteToWideChar(0, 0, s, buffLen, m_bstr, buffLen);
        }
    }
    else
    {
        if(-1 == len)
        {
            len = actualLen;
        }

        m_bstr = bstr_create(s, static_cast<cs_size_t>(len));
    }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( NULL == m_bstr &&
        NULL != s &&
        0 != len &&
        '\0' != 0[s])
    {
        STLSOFT_THROW_X(com_exception("failed to allocate string", HRESULT_FROM_WIN32(::GetLastError())));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline /* explicit */ bstr::bstr(cs_char_w_t const* s, ssize_type len /* = -1 */)
{
    // Precondition tests
    COMSTL_MESSAGE_ASSERT("Default length must be specified by -1. No other -ve value allowed", (len >= 0 || len == -1));
    COMSTL_MESSAGE_ASSERT("Cannot pass in NULL pointer and -1 (default) length", (NULL != s || len >= 0));

    // There's a potential problem here (which has actually occurred!):
    //
    // If s is non-NULL and len is non-negative, it's possible for
    // the underlying SysAllocStringLen() to walk into invalid
    // memory while searching s.

    ssize_type actualLen = static_cast<ssize_type>(stlsoft_ns_qual(c_str_len)(s));

    if( NULL != s &&
        len > actualLen)
    {
        m_bstr = bstr_create(static_cast<cs_char_w_t const*>(NULL), static_cast<cs_size_t>(len));

        if(NULL != m_bstr)
        {
#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
            ::wcscpy_s(m_bstr, static_cast<cs_size_t>(actualLen), s);
#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
            ::wcscpy(m_bstr, s);
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */
        }
    }
    else
    {
        if(-1 == len)
        {
            len = actualLen;
        }

        m_bstr = bstr_create(s, static_cast<cs_size_t>(len));
    }

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( NULL == m_bstr &&
        NULL != s &&
        0 != len &&
        '\0' != 0[s])
    {
        STLSOFT_THROW_X(com_exception("failed to allocate string", HRESULT_FROM_WIN32(::GetLastError())));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline bstr::bstr(bstr::size_type n, bstr::char_type ch)
    : m_bstr(bstr_create_w(NULL, n))
{
    if(NULL == m_bstr)
    {
        if(0 != n)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(com_exception("failed to allocate string", HRESULT_FROM_WIN32(::GetLastError())));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
    else
    {
        { for(size_type i = 0; i < n; ++i)
        {
            m_bstr[i] = ch;
        }}
    }
}

inline bstr::bstr(bstr::class_type const& rhs)
    : m_bstr(bstr_dup(rhs.m_bstr))
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( NULL == m_bstr &&
        !rhs.empty())
    {
        STLSOFT_THROW_X(com_exception("failed to allocate string", HRESULT_FROM_WIN32(::GetLastError())));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline bstr::bstr(bstr::class_type const& rhs, bstr::size_type pos, bstr::size_type len)
    : m_bstr(NULL)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(pos > rhs.size())
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("Position out of range"));
    }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
    COMSTL_MESSAGE_ASSERT("Position out of range", pos <= rhs.size());
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    if(pos + len > rhs.size())
    {
        len = rhs.size() - pos;
    }

    m_bstr = bstr_create(rhs.data() + pos, len);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if( NULL == m_bstr &&
        !rhs.empty())
    {
        STLSOFT_THROW_X(com_exception("failed to allocate string", HRESULT_FROM_WIN32(::GetLastError())));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline bstr::~bstr() stlsoft_throw_0()
{
    ::SysFreeString(m_bstr);
}

inline bstr::class_type& bstr::operator =(bstr::class_type const& rhs)
{
    class_type  t(rhs);

    t.swap(*this);

    return *this;
}

inline bstr::class_type& bstr::assign(cs_char_a_t const* s, ssize_type len /* = -1 */)
{
    class_type  t(s, len);

    t.swap(*this);

    return *this;
}

inline bstr::class_type& bstr::assign(cs_char_w_t const* s, ssize_type len /* = -1 */)
{
    class_type  t(s, len);

    t.swap(*this);

    return *this;
}

inline bstr::class_type& bstr::assign(bstr::const_iterator from, bstr::const_iterator to)
{
    return assign(from, to - from);
}

inline bstr::class_type& bstr::operator =(cs_char_a_t const* s)
{
    return assign(s);
}

inline bstr::class_type& bstr::operator =(cs_char_w_t const* s)
{
    return assign(s);
}

inline bstr::class_type& bstr::attach(BSTR bstr)
{
    *DestructiveAddress() = bstr;

    return *this;
}

inline BSTR bstr::detach()
{
    BSTR    str =   m_bstr;

    m_bstr = NULL;

    return str;
}

inline void bstr::clear()
{
    ::SysFreeString(m_bstr);

    m_bstr = NULL;
}

inline bstr::class_type& bstr::append(bstr::class_type const& s, ssize_type len /* = -1 */)
{
    return append(s.data(), len);
}

inline bstr::class_type& bstr::append(cs_char_w_t const* s, ssize_type len /* = -1 */)
{
    if(empty())
    {
        bstr    rhs(s, len);

        rhs.swap(*this);
    }
    else
    {
        if(len < 0)
        {
            len = (NULL == s) ? 0 : static_cast<ssize_type>(::wcslen(s));
        }

        if(0 != len)
        {
            size_type   totalLen = size() + len;
            bstr        rhs(data(), static_cast<ssize_type>(totalLen));

#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
            ::wcsncpy_s(&rhs[0] + size(), static_cast<cs_size_t>(totalLen), s, static_cast<cs_size_t>(len));
#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
            ::wcsncpy(&rhs[0] + size(), s, static_cast<cs_size_t>(len));
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */

            rhs.swap(*this);
        }
    }

    return *this;
}

inline bstr::class_type& bstr::operator +=(bstr::class_type const& s)
{
    return append(s);
}

inline bstr::class_type& bstr::operator +=(cs_char_w_t const* s)
{
    return append(s);
}

// Operations

// Accessors
inline bstr::const_pointer bstr::data() const
{
    return this->c_str();
}

inline bstr::const_pointer bstr::c_str() const
{
    return (NULL == m_bstr) ? L"" : m_bstr;
}

inline bstr::size_type bstr::length() const
{
    return static_cast<size_type>(::SysStringLen(m_bstr));
}

inline bstr::size_type bstr::size() const
{
    return this->length();
}

inline bstr::bool_type bstr::empty() const
{
    return 0 == this->size();
}

inline BSTR bstr::get() const
{
    return m_bstr;
}

inline bstr::reference bstr::operator [](bstr::size_type index)
{
    COMSTL_MESSAGE_ASSERT("invalid index", index < size());

    return index[m_bstr];
}

inline bstr::const_reference bstr::operator [](bstr::size_type index) const
{
    COMSTL_MESSAGE_ASSERT("invalid index", index <= size());

    return index[data()];
}

inline bstr::const_pointer *bstr::NonDestructiveAddress() const
{
    return const_cast<const_pointer*>(&m_bstr);
}

inline BSTR* bstr::NonDestructiveAddress()
{
    return &m_bstr;
}

inline BSTR* bstr::DestructiveAddress()
{
    clear();

    return &m_bstr;
}

inline bstr::iterator bstr::begin()
{
    return get();
}

inline bstr::iterator bstr::end()
{
    return get() + size();
}

inline bstr::const_iterator bstr::begin() const
{
    return get();
}

inline bstr::const_iterator bstr::end() const
{
    return get() + size();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
inline bstr::const_reverse_iterator bstr::rbegin() const
{
    return const_reverse_iterator(end());
}

inline bstr::const_reverse_iterator bstr::rend() const
{
    return const_reverse_iterator(begin());
}

inline bstr::reverse_iterator bstr::rbegin()
{
    return reverse_iterator(end());
}

inline bstr::reverse_iterator bstr::rend()
{
    return reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

inline bstr::bool_type bstr::equal(bstr::class_type const& rhs) const
{
    return 0 == bstr_compare(this->get(), rhs.get());
}

inline bstr::bool_type bstr::equal(BSTR const& rhs) const
{
    return 0 == bstr_compare(this->get(), rhs);
}

// Operations

inline void bstr::swap(bstr::class_type& rhs) stlsoft_throw_0()
{
    std_swap(m_bstr, rhs.m_bstr);
}

inline void bstr::swap(BSTR& rhs) stlsoft_throw_0()
{
    std_swap(m_bstr, rhs);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace stlsoft::comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _COMSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::comstl::c_str_data;
using ::comstl::c_str_data_w;
using ::comstl::c_str_data_o;

using ::comstl::c_str_len;
using ::comstl::c_str_len_w;
using ::comstl::c_str_len_o;

using ::comstl::c_str_ptr;
using ::comstl::c_str_ptr_w;
using ::comstl::c_str_ptr_o;

using ::comstl::c_str_ptr_null;
using ::comstl::c_str_ptr_null_w;
using ::comstl::c_str_ptr_null_o;

/* /////////////////////////////////////////////////////////////////////////
 * Traits
 */

/** Specialisation for comstl::bstr
 */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits< ::comstl::bstr>
{
    typedef ::comstl::bstr                                  value_type;
    typedef ::comstl::bstr::value_type                      char_type;  // NOTE: Can't use value_type::value_type here, because of BC++ 5.5.1
    typedef value_type::size_type                           size_type;
    typedef char_type const                                 const_char_type;
    typedef value_type                                      string_type;
    typedef string_type::pointer                            pointer;
    typedef string_type::const_pointer                      const_pointer;
    typedef string_type::iterator                           iterator;
    typedef string_type::const_iterator                     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef string_type::reverse_iterator                   reverse_iterator;
    typedef string_type::const_reverse_iterator             const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    enum
    {
            is_pointer          =   false
        ,   is_pointer_to_const =   false
        ,   char_type_size      =   sizeof(char_type)
    };

    static string_type empty_string()
    {
        return string_type();
    }
    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(src, pos, len);
    }
# ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k I>
    static string_type& assign_inplace(string_type& str, I first, I last)
# else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type& assign_inplace(string_type& str, const_iterator first, const_iterator last)
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        // comstl::bstr cannot assign in-place
        return (str = string_type(first, last - first), str);
    }
};


# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_STRING_HPP_BSTR */

/* ///////////////////////////// end of file //////////////////////////// */
