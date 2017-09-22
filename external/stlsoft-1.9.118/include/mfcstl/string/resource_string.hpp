/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/string/resource_string.hpp (originally MWResStr.h: ::SynesisWin)
 *
 * Purpose:     resource_string class.
 *
 * Created:     1st November 1994
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2009, Matthew Wilson and Synesis Software
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


/** \file mfcstl/string/resource_string.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::resource_string class
 *   (\ref group__library__string "String" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_STRING_HPP_RESOURCE_STRING
#define MFCSTL_INCL_MFCSTL_STRING_HPP_RESOURCE_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_STRING_HPP_RESOURCE_STRING_MAJOR      4
# define MFCSTL_VER_MFCSTL_STRING_HPP_RESOURCE_STRING_MINOR      0
# define MFCSTL_VER_MFCSTL_STRING_HPP_RESOURCE_STRING_REVISION   3
# define MFCSTL_VER_MFCSTL_STRING_HPP_RESOURCE_STRING_EDIT       80
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# pragma warn -8022 /* Suppresses "'f()' hides virtual function 'g()'" */
# pragma warn -8084 /* Suppresses "Suggest parentheses to clarify precedence in function 'f()'" */
#endif /* compiler */

#ifndef STLSOFT_INCL_H_AFXWIN
# define STLSOFT_INCL_H_AFXWIN
# include <afxwin.h>    // for AfxThrowResourceException()
#endif /* !STLSOFT_INCL_H_AFXWIN */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# pragma warn .8022 /* Suppresses "'f()' hides virtual function 'g()'" */
# pragma warn .8084 /* Suppresses "Suggest parentheses to clarify precedence in function 'f()'" */
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

// resource_string
/** \brief Instances of this class represent Windows string resources, and are
 * constructed from instance identifiers.
 *
 * \ingroup group__library__string
 */
class resource_string
    : public CString
    , public stlsoft_ns_qual(stl_collection_tag)
{
private:
    typedef CString         parent_class_type;
    typedef resource_string class_type;
public:
    /// The type of the const (non-mutating) iterator
    typedef LPCTSTR         const_iterator;
    /// The size type
    typedef UINT            size_type;

// Construction
public:
    /// Constructs an around the string loaded from the given \c id
    ///
    /// \param id identifier of the string resource to load
    ss_explicit_k resource_string(ms_uint_t id) stlsoft_throw_2(CMemoryException*, CResourceException*);
    /// Constructs an around the string loaded from the given \c id and \c hinst
    ///
    /// \param hinst The module from which to load the string
    /// \param id identifier of the string resource to load
    resource_string(HINSTANCE hinst, ms_uint_t id) stlsoft_throw_2(CMemoryException*, CResourceException*);
    /// Copy constructor
    ///
    /// \param rhs The instance from which to copy-construct
    resource_string(resource_string const& rhs);
    /// Copy constructor
    ///
    /// \param rhs The instance from which to copy-construct
    resource_string(CString const& rhs);

    /// Copy assignment operator
    ///
    /// \param rhs The instance from which to copy-assign
    resource_string const& operator =(resource_string const& rhs);
    /// Copy assignment operator
    ///
    /// \param rhs The instance from which to copy-assign
    resource_string const& operator =(CString const& rhs);

// Iteration
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator  begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const;

// Attributes
public:
    /// Returns the number of elements in the sequence
    size_type size() const;
    /// Returns the number of elements in the sequence
    size_type length() const;
    /// Indicates whether the string is empty
    ms_bool_t empty() const;
    /// Returns a pointer to constant data representing the managed string
    LPCTSTR c_str() const;
    /// Returns a possibly unterminated pointer to constant data representing the managed string
    LPCTSTR data() const;
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

inline LPCTSTR c_str_ptr_null(resource_string const& s)
{
    return s.empty() ? NULL : s.c_str();
}
#ifdef UNICODE
inline LPCTSTR c_str_ptr_null_w(resource_string const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_ptr_null_a(resource_string const& s)
#endif /* UNICODE */
{
    return c_str_ptr_null(s);
}

inline LPCTSTR c_str_ptr(resource_string const& s)
{
    return s.c_str();
}
#ifdef UNICODE
inline LPCTSTR c_str_ptr_w(resource_string const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_ptr_a(resource_string const& s)
#endif /* UNICODE */
{
    return c_str_ptr(s);
}

inline LPCTSTR c_str_data(resource_string const& s)
{
    return s.data();
}
#ifdef UNICODE
inline LPCTSTR c_str_data_w(resource_string const& s)
#else /* ? UNICODE */
inline LPCTSTR c_str_data_a(resource_string const& s)
#endif /* UNICODE */
{
    return c_str_data(s);
}

inline ms_size_t c_str_len(resource_string const& s)
{
    return s.length();
}



template<ss_typename_param_k S>
inline S& operator <<(S& s, resource_string const& str)
{
    s << str.c_str();

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/resource_string_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

inline resource_string::resource_string(ms_uint_t id) stlsoft_throw_2(CMemoryException*, CResourceException*)
{
    if(!parent_class_type::LoadString(id))
    {
        AfxThrowResourceException();
    }
}

inline resource_string::resource_string(HINSTANCE hinst, ms_uint_t id) stlsoft_throw_2(CMemoryException*, CResourceException*)
{
    TCHAR sz[1024];

    if(0 == ::LoadString(hinst, id, sz, STLSOFT_NUM_ELEMENTS(sz)))
    {
        AfxThrowResourceException();
    }
    else
    {
        parent_class_type   *pThis = this;

        pThis->operator =(sz);
    }
}

inline resource_string::resource_string(resource_string const& rhs)
  : parent_class_type(rhs)
{}

inline resource_string::resource_string(CString const& rhs)
  : parent_class_type(rhs)
{}

inline resource_string const& resource_string::operator =(resource_string const& rhs)
{
    parent_class_type   *pThis = this;

    pThis->operator =(rhs);

    return *this;
}

inline resource_string const& resource_string::operator =(CString const& rhs)
{
    parent_class_type   *pThis = this;

    pThis->operator =(rhs);

    return *this;
}

inline resource_string::const_iterator resource_string::begin() const
{
    return *this;
}

inline resource_string::const_iterator resource_string::end() const
{
    return begin() + length();
}

inline resource_string::size_type resource_string::size() const
{
    return GetLength();
}

inline resource_string::size_type resource_string::length() const
{
    return GetLength();
}

inline ms_bool_t resource_string::empty() const
{
    return 0 == length();
}

inline LPCTSTR resource_string::c_str() const
{
    return *this;
}

inline LPCTSTR resource_string::data() const
{
    return c_str();
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::mfcstl::c_str_ptr_null;
#if defined(UNICODE)
using ::mfcstl::c_str_ptr_null_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_ptr_null_a;
#endif /* UNICODE */

using ::mfcstl::c_str_ptr;
#if defined(UNICODE)
using ::mfcstl::c_str_ptr_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_ptr_a;
#endif /* UNICODE */

using ::mfcstl::c_str_data;
#if defined(UNICODE)
using ::mfcstl::c_str_data_w;
#else /* ? UNICODE */
using ::mfcstl::c_str_data_a;
#endif /* UNICODE */

using ::mfcstl::c_str_len;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_STRING_HPP_RESOURCE_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
