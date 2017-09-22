/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/string/ccombstr_veneer.hpp
 *
 * Purpose:     Contains the definition of the ccombstr_veneer template.
 *
 * Created:     1st October 2002
 * Updated:     10th August 2009
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


/** \file atlstl/string/ccombstr_veneer.hpp
 *
 * \brief [C++] Definition of the atlstl::ccombstr_veneer class
 *   (\ref group__library__string "String" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_STRING_HPP_CCOMBSTR_VENEER
#define ATLSTL_INCL_ATLSTL_STRING_HPP_CCOMBSTR_VENEER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_STRING_HPP_CCOMBSTR_VENEER_MAJOR    5
# define ATLSTL_VER_ATLSTL_STRING_HPP_CCOMBSTR_VENEER_MINOR    3
# define ATLSTL_VER_ATLSTL_STRING_HPP_CCOMBSTR_VENEER_REVISION 1
# define ATLSTL_VER_ATLSTL_STRING_HPP_CCOMBSTR_VENEER_EDIT     76
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#if defined(STLSOFT_COMPILER_IS_DMC)
# ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
#  include <stlsoft/conversion/sap_cast.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */
#endif /* compiler */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
# include <stlsoft/string/string_traits_fwd.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>              // for wcscmp(), _wcsnset()
#endif /* !STLSOFT_INCL_H_WCHAR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::atlstl */
namespace atlstl
{
# else
/* Define stlsoft::atlstl_project */

namespace stlsoft
{

namespace atlstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Applies standard string (std::basic_string) interface to the CComBSTR class
 *
 * \ingroup group__library__string
 *
 * This class adapts the ATL string type CComBSTR to express a standard String-like interface
 *
 * \ingroup concepts_veneer
 */
// [[synesis:class:string: atlstl::ccombstr_veneer]]
class ccombstr_veneer
#ifdef _ATLSTL_CCOMBSTR_VENEER_INHERIT_AS_PUBLIC
    : public CComBSTR
#else /* ? _ATLSTL_CCOMBSTR_VENEER_INHERIT_AS_PUBLIC */
# if defined(STLSOFT_COMPILER_IS_DMC)
    : protected CComBSTR
# else /* ? compiler */
    : private CComBSTR
# endif /* compiler */
#endif /* _ATLSTL_CCOMBSTR_VENEER_INHERIT_AS_PUBLIC */
{
private:
    typedef CComBSTR        parent_class_type;
    typedef ccombstr_veneer class_type;
public:
    /// The value type
    typedef OLECHAR         value_type;
    /// The mutating iterator type
    typedef LPOLESTR        iterator;
    /// The non-mutating (const) iterator type
    typedef LPCOLESTR       const_iterator;
    /// The pointer type
    typedef LPOLESTR        pointer;
    /// The pointer-to-const type
    typedef LPCOLESTR       const_pointer;
    /// The reference type
    typedef OLECHAR&        reference;
    /// The reference-to-const type
    typedef OLECHAR const&  const_reference;
    /// The size type
    typedef as_size_t       size_type;
    /// The difference type
    typedef as_ptrdiff_t    difference_type;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
//    typedef stlsoft_ns_qual(reverse_iterator_base)< iterator
//                                                ,   value_type
//                                                ,   reference
//                                                ,   pointer
//                                                ,   difference_type
//                                                >           reverse_iterator;

    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   const_iterator
                                                    ,   value_type const
                                                    ,   const_reference
                                                    ,   const_pointer
                                                    ,   difference_type
                                                    >       const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
public:
    /// Default constructor
    ccombstr_veneer();
    /// Copy constructor
    ccombstr_veneer(class_type const& rhs);
    /// Copy constructor
    ss_explicit_k ccombstr_veneer(CComBSTR const& rhs);
    /// Construct from an ANSI string
    ss_explicit_k ccombstr_veneer(LPCSTR s);
    /// Construct from a Unicode string
    ss_explicit_k ccombstr_veneer(LPCWSTR s);
    /// Construct from a range
    ccombstr_veneer(LPCOLESTR from, LPCOLESTR to);
    /// Construct from a range
    ccombstr_veneer(size_type length, LPCOLESTR from);
    ///
    ccombstr_veneer(as_size_t cch, OLECHAR ch);

    /// Copy assignment operator
    class_type const& operator =(class_type const& rhs);
    /// Copy assignment operator
    class_type const& operator =(CComBSTR const& rhs);
    /// Assignment operator
    class_type const& operator =(LPCSTR s);
    /// Assignment operator
    class_type const& operator =(LPCWSTR s);

// Iteration
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the start of the reverse sequence
    const_reverse_iterator rbegin() const;
    /// Ends the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the end of the reverse sequence
    const_reverse_iterator rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Access
public:
    reference operator [](size_type index);
    const_reference operator [](size_type index) const;

// Operations
public:
    void swap(class_type& rhs);

// Attributes
public:
    /// Provides access to the underlying CComBSTR type
    CComBSTR        &get_base_type();
    /// Provides access to the underlying CComBSTR type
    CComBSTR const  &get_base_type() const;
    /// Returns the number of elements in the sequence
    size_type       length() const;
    /// Returns the number of elements in the sequence
    size_type       size() const;
    /// \brief Indicates whether the sequence is empty
    as_bool_t       empty() const;
    /// Returns a pointer to constant data representing the managed string
    const_pointer   c_str() const;
    /// Returns a pointer to constant data representing the managed string
    const_pointer   data() const;
};

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Because CComBSTR converts into BSTR, and because BSTR and BSTR are pointers
// which can be combined, and because LPCWSTR is *not* BSTR, and because
// CComBSTR can be implicitly constructed from LPCSTR, BSTR and LPCWSTR, it is
// incredibly easy to think you're doing value comparison but actually be doing
// identity comparison. Filth!!!!!!!
//
// Furthermore, old versions of CComBSTR don't have operator ==(), and even
// newer versions define it as a member function, instead of a free function.
// Triple filth!!!!!
//
// So, to ensure that CComBSTR is compared appropriately, all comparisons must
// filter through this function, which takes parameters by pointer (to avoid the
// implicit conversions) and explicitly calls the operator ==() to avoid identity
// comparison.

inline int atlstl_safe_wcscmp(LPCOLESTR s1, LPCOLESTR s2)
{
    return  (NULL == s1)
                ? (NULL == s2)
                    ? 0
                    : 1
                : (NULL == s2)
                    ? -1
                    : wcscmp(s1, s2);
}

inline int atlstl_safe_wcscmp(LPCOLESTR s1, LPCSTR s2)
{
    USES_CONVERSION;

    return atlstl_safe_wcscmp(s1, (NULL == s2) ? L"" : A2CW(s2));
}

inline int atlstl_safe_wcscmp(LPCSTR s1, LPCOLESTR s2)
{
    USES_CONVERSION;

    return atlstl_safe_wcscmp((NULL == s1) ? L"" : A2CW(s1), s2);
}

#if 0

// _ATL_VER 0x0800
bool CComBSTR::operator==(CComBSTR const& bstrSrc) const throw()
bool CComBSTR::operator==(LPCOLESTR pszSrc) const
bool CComBSTR::operator==(LPOLESTR pszSrc) const
bool CComBSTR::operator==(int nNull) const throw()  // Can you believe it!!
bool CComBSTR::operator==(LPCSTR pszSrc) const

// _ATL_VER 0x0710
bool CComBSTR::operator==(CComBSTR const& bstrSrc) const throw()
bool CComBSTR::operator==(LPCOLESTR pszSrc) const
bool CComBSTR::operator==(LPOLESTR pszSrc) const
bool CComBSTR::operator==(int nNull) const throw()  // Can you believe it!!
bool CComBSTR::operator==(LPCSTR pszSrc) const

// _ATL_VER  0x0700
bool CComBSTR::operator==(CComBSTR const& bstrSrc) const throw()
bool CComBSTR::operator==(LPCOLESTR pszSrc) const
bool CComBSTR::operator==(int nNull) const throw()
bool CComBSTR::operator==(LPCSTR pszSrc) const

// _ATL_VER  0x0300
bool CComBSTR::operator==(BSTR bstrSrc) const
bool CComBSTR::operator==(LPCSTR pszSrc) const

// _ATL_VER  0x0202

#endif /* 0 */


#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

inline as_bool_t operator ==(ccombstr_veneer const& lhs, ccombstr_veneer const& rhs)
{
    CComBSTR const&  lhs_   =   lhs.get_base_type();
    CComBSTR const&  rhs_   =   rhs.get_base_type();

#if _ATL_VER >= 0x0700
    return lhs_ == rhs_;
#else // ? _ATL_VER
    return 0 == atlstl_safe_wcscmp(lhs_, rhs_);
#endif // _ATL_VER
}

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER >= 1200

inline as_bool_t operator ==(LPCSTR lhs, ccombstr_veneer const& rhs)
{
    CComBSTR const&  rhs_ = rhs.get_base_type();

#if _ATL_VER >= 0x0300
    return rhs_ == lhs;
#else /* ? _ATL_VER */
    return 0 == atlstl_safe_wcscmp(lhs, rhs_);
#endif /* _ATL_VER */
}

inline as_bool_t operator ==(ccombstr_veneer const& lhs, LPCSTR rhs)
{
    CComBSTR const&  lhs_ = lhs.get_base_type();

#if _ATL_VER >= 0x0300
    return lhs_ == rhs;
#else /* ? _ATL_VER */
    return 0 == atlstl_safe_wcscmp(lhs_, rhs);
#endif /* _ATL_VER */
}

#endif /* !(_MSC_VER < 1200) */

inline as_bool_t operator ==(LPCWSTR lhs, ccombstr_veneer const& rhs)
{
    CComBSTR const&  rhs_   =   rhs.get_base_type();

#if _ATL_VER >= 0x0700
    return rhs_ == lhs;
#else // ? _ATL_VER
    return 0 == atlstl_safe_wcscmp(lhs, rhs_);
#endif // _ATL_VER
}

inline as_bool_t operator ==(ccombstr_veneer const& lhs, LPCWSTR rhs)
{
    CComBSTR const&  lhs_   =   lhs.get_base_type();

#if _ATL_VER >= 0x0700
    return lhs_ == rhs;
#else // ? _ATL_VER
    return 0 == atlstl_safe_wcscmp(lhs_, rhs);
#endif // _ATL_VER
}

inline as_bool_t operator !=(ccombstr_veneer const& lhs, ccombstr_veneer const& rhs)
{
    return !operator ==(lhs, rhs);
}

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER >= 1200

inline as_bool_t operator !=(LPCSTR lhs, ccombstr_veneer const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline as_bool_t operator !=(ccombstr_veneer const& lhs, LPCSTR rhs)
{
    return !operator ==(lhs, rhs);
}

#endif /* !(_MSC_VER < 1200) */

inline as_bool_t operator !=(LPCWSTR lhs, ccombstr_veneer const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline as_bool_t operator !=(ccombstr_veneer const& lhs, LPCWSTR rhs)
{
    return !operator ==(lhs, rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

inline void swap(ccombstr_veneer& lhs, ccombstr_veneer& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \brief \ref group__concept__shim__string_access__c_str_data for comstl::ccombstr_veneer
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCOLESTR c_str_data(atlstl_ns_qual(ccombstr_veneer) const& str)
{
    return str.data();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
inline LPCOLESTR c_str_data_w(atlstl_ns_qual(ccombstr_veneer) const& str)       { return c_str_data(str); }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief \ref group__concept__shim__string_access__c_str_len for comstl::ccombstr_veneer
 *
 * \ingroup group__concept__shim__string_access
 */
inline as_size_t c_str_len(atlstl_ns_qual(ccombstr_veneer) const& str)
{
    return str.length();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
inline as_size_t c_str_len_w(atlstl_ns_qual(ccombstr_veneer) const& str)        { return c_str_len(str); }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief \ref group__concept__shim__string_access__c_str_ptr for comstl::ccombstr_veneer
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCOLESTR c_str_ptr(atlstl_ns_qual(ccombstr_veneer) const& str)
{
    return str.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
inline LPCOLESTR c_str_ptr_w(atlstl_ns_qual(ccombstr_veneer) const& str)        { return c_str_ptr(str); }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for comstl::ccombstr_veneer
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCOLESTR c_str_ptr_null(atlstl_ns_qual(ccombstr_veneer) const& str)
{
    return !str.empty() ? str.c_str() : NULL;
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
inline LPCOLESTR c_str_ptr_null_w(atlstl_ns_qual(ccombstr_veneer) const& str)   { return c_str_ptr_null(str); }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */




/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for comstl::ccombstr_veneer
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<ss_typename_param_k S>
inline S& operator <<(S& s, atlstl_ns_qual(ccombstr_veneer) const& str)
{
    s << str.c_str();

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/ccombstr_veneer_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline ccombstr_veneer::ccombstr_veneer()
    : parent_class_type()
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(class_type const& rhs)
    : parent_class_type(rhs)
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(CComBSTR const& rhs)
    : parent_class_type(rhs)
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(LPCSTR s)
/* There's a bug in some implementations of ATL 3.x, so we go for the assigment,
 * doing the conversion ourselves
 */
#if _ATL_VER >= 0x0700
    : parent_class_type(s)
#endif /* _ATL_VER */
{
#if _ATL_VER < 0x0700
    USES_CONVERSION;
    *this = A2COLE(s);
#endif /* _ATL_VER */

    ATLSTL_ASSERT(s == NULL || length() == strlen(s));

    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(LPCWSTR s)
#if _ATL_VER >= 0x0300
    : parent_class_type(s)
#endif /* _ATL_VER */
{
#if _ATL_VER < 0x0300
    USES_CONVERSION;
    *this = s;
#endif /* _ATL_VER */

    ATLSTL_ASSERT(s == NULL || length() == wcslen(s));

    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(LPCOLESTR from, LPCOLESTR to)
    : parent_class_type(static_cast<int>(to - from), from)
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline void ccombstr_veneer::swap(ccombstr_veneer::class_type& rhs)
{
    std_swap(m_str, rhs.m_str);
}

inline ccombstr_veneer::ccombstr_veneer(ccombstr_veneer::size_type length, LPCOLESTR from)
    : parent_class_type(static_cast<int>(length), from)
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);
}

inline ccombstr_veneer::ccombstr_veneer(as_size_t cch, OLECHAR ch)
{
    stlsoft_constraint_must_be_same_size(CComBSTR, class_type);

    typedef stlsoft_ns_qual(auto_buffer)<OLECHAR>   buffer_t;

    buffer_t    buffer(cch);

    _wcsnset(&buffer[0], ch, cch);

    class_type(cch, buffer.data()).swap(*this);
}

inline ccombstr_veneer::class_type const& ccombstr_veneer::operator =(ccombstr_veneer::class_type const& rhs)
{
    parent_class_type::operator =(rhs);

    return *this;
}

inline ccombstr_veneer::class_type const& ccombstr_veneer::operator =(CComBSTR const& rhs)
{
    parent_class_type::operator =(rhs);

    return *this;
}

inline ccombstr_veneer::class_type const& ccombstr_veneer::operator =(LPCSTR s)
{
/* There's a bug in some implementations of ATL 3.x, so we go for the assigment,
 * doing the conversion ourselves
 */
#if _ATL_VER >= 0x0700
    parent_class_type::operator =(s);
#else /* ? _ATL_VER */
    USES_CONVERSION;
    *this = A2COLE(s);
#endif /* _ATL_VER */

    return *this;
}

inline ccombstr_veneer::class_type const& ccombstr_veneer::operator =(LPCWSTR s)
{
    parent_class_type::operator =(s);

    return *this;
}

inline ccombstr_veneer::const_iterator ccombstr_veneer::begin() const
{
    return *this;
}

inline ccombstr_veneer::const_iterator ccombstr_veneer::end() const
{
    return begin() + Length();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
inline ccombstr_veneer::const_reverse_iterator ccombstr_veneer::rbegin() const
{
    return const_reverse_iterator(end());
}

inline ccombstr_veneer::const_reverse_iterator ccombstr_veneer::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

inline ccombstr_veneer::reference ccombstr_veneer::operator [](ccombstr_veneer::size_type index)
{
    ATLSTL_MESSAGE_ASSERT("Index out of range", index < length());

    return const_cast<reference>(data()[index]);
}

inline ccombstr_veneer::const_reference ccombstr_veneer::operator [](ccombstr_veneer::size_type index) const
{
    ATLSTL_MESSAGE_ASSERT("Index out of range", index < length());

    return data()[index];
}

inline CComBSTR &ccombstr_veneer::get_base_type()
{
#if defined(STLSOFT_COMPILER_IS_DMC)
    CComBSTR    *this_  =   stlsoft_ns_qual(sap_cast)<CComBSTR*>(this);
#else /* ? compiler */
    CComBSTR    *this_  =   this;
#endif /* compiler */

    return *this_;
}

inline CComBSTR const& ccombstr_veneer::get_base_type() const
{
#if defined(STLSOFT_COMPILER_IS_DMC)
    CComBSTR const  *this_  =   stlsoft_ns_qual(sap_cast)<CComBSTR const*>(this);
#else /* ? compiler */
    CComBSTR const  *this_  =   this;
#endif /* compiler */

    return *this_;
}

inline ccombstr_veneer::size_type ccombstr_veneer::length() const
{
    return Length();
}

inline ccombstr_veneer::size_type ccombstr_veneer::size() const
{
    return length();
}

inline as_bool_t ccombstr_veneer::empty() const
{
    return length() == 0;
}

inline ccombstr_veneer::const_pointer ccombstr_veneer::c_str() const
{
    return empty() ? L"" : static_cast<const_pointer>(*this);
}

inline ccombstr_veneer::const_pointer ccombstr_veneer::data() const
{
    return empty() ? L"" : static_cast<const_pointer>(*this);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace atlstl
# else
} // namespace atlstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

#if defined(STLSOFT_CF_std_NAMESPACE)
namespace stlsoft
{
    inline void swap(atlstl_ns_qual(ccombstr_veneer)& lhs, atlstl_ns_qual(ccombstr_veneer)& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace stlsoft
#endif /* STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _ATLSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::atlstl::c_str_data;
using ::atlstl::c_str_data_w;

using ::atlstl::c_str_len;
using ::atlstl::c_str_len_w;

using ::atlstl::c_str_ptr;
using ::atlstl::c_str_ptr_w;

using ::atlstl::c_str_ptr_null;
using ::atlstl::c_str_ptr_null_w;

/* /////////////////////////////////////////////////////////////////////////
 * Traits
 */

/** Specialisation for atlstl::ccombstr_veneer
 */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits< ::atlstl::ccombstr_veneer>
{
    typedef ::atlstl::ccombstr_veneer                       value_type;
    typedef ::atlstl::ccombstr_veneer::value_type           char_type;  // NOTE: Can't use value_type::value_type here, because of BC++ 5.5.1
    typedef value_type::size_type                           size_type;
    typedef char_type const                                 const_char_type;
    typedef value_type                                      string_type;
    typedef string_type::pointer                            pointer;
    typedef string_type::const_pointer                      const_pointer;
    typedef string_type::iterator                           iterator;
    typedef string_type::const_iterator                     const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
//    typedef string_type::reverse_iterator                   reverse_iterator;
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
        return string_type(len, src.data() + pos);
    }
# ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k I>
    static string_type &assign_inplace(string_type &str, I first, I last)
# else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type &assign_inplace(string_type &str, const_iterator first, const_iterator last)
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        // comstl::bstr cannot assign in-place
        return (str = string_type(last - first, first), str);
    }
};


# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !ATLSTL_INCL_ATLSTL_STRING_HPP_CCOMBSTR_VENEER */

/* ///////////////////////////// end of file //////////////////////////// */
