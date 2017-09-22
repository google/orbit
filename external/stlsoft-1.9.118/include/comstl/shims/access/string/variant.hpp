/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/shims/access/string/variant.hpp
 *
 * Purpose:     Contains classes and functions for dealing with OLE/COM strings.
 *
 * Created:     24th May 2002
 * Updated:     27th April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/shims/access/string/variant.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>VARIANT</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef COMSTL_INCL_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT
#define COMSTL_INCL_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT_MAJOR    5
# define COMSTL_VER_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT_MINOR    0
# define COMSTL_VER_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT_REVISION 6
# define COMSTL_VER_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT_EDIT     115
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS
# include <comstl/string/BSTR_functions.h>
#endif /* !COMSTL_INCL_COMSTL_STRING_H_BSTR_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */

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

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr_null() function, such that the text of a given variant
 * may be accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
class c_str_null_VARIANT_proxy
{
public:
    typedef c_str_null_VARIANT_proxy    class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given BSTR
    ///
    /// \param s The BSTR from which the text will be retrieved
    ss_explicit_k c_str_null_VARIANT_proxy(const BSTR s)
        : m_bstr(s)
        , m_own(false)
    {}

    /// Constructs an instance of the proxy from the given BSTR
    ///
    /// \param ps Pointer to the BSTR from which the text will be retrieved
    ss_explicit_k c_str_null_VARIANT_proxy(BSTR *ps)
        : m_bstr(*ps)
        , m_own(true)
    {
        if(m_own)
        {
            *ps = NULL;
        }
    }

    /// Default constructor
    c_str_null_VARIANT_proxy()
        : m_bstr(NULL)
        , m_own(false)
    {}

#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    /// Move constructor
    ///
    /// This <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">move constructor</a>
    /// is for circumstances when the compiler does not, or cannot, apply the
    /// return value optimisation. It causes the contents of \c rhs to be
    /// transferred into the constructing instance. This is completely safe
    /// because the \c rhs instance will never be accessed in its own right, so
    /// does not need to maintain ownership of its contents.
    c_str_null_VARIANT_proxy(class_type& rhs)
        : m_bstr(rhs.m_bstr)
        , m_own(rhs.m_own)
    {
        move_lhs_from_rhs(rhs).m_bstr  =   NULL;
        move_lhs_from_rhs(rhs).m_own   =   false;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_null_VARIANT_proxy(class_type const& rhs)
        : m_bstr(bstr_dup(rhs.m_bstr))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_null_VARIANT_proxy() stlsoft_throw_0()
    {
        if(m_own)
        {
            ::SysFreeString(m_bstr);
        }
    }

// Accessors
public:
    /// Returns a null-terminated string representing the VARIANT contents, or
    /// NULL if the VARIANT contents cannot be converted to text.
    operator LPCOLESTR () const
    {
        return m_bstr;
    }

// Members
private:
    BSTR        m_bstr;
    cs_bool_t   m_own;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr_w() function, such that the text of a given variant
 * may be accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
class c_str_VARIANT_proxy_w
{
public:
    typedef c_str_VARIANT_proxy_w   class_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given BSTR
    ///
    /// \param s The BSTR from which the text will be retrieved
    ss_explicit_k c_str_VARIANT_proxy_w(BSTR &s)
        : m_bstr(s)
    {
        s = NULL;
    }

#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    /// Move constructor
    ///
    /// This <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">move constructor</a>
    /// is for circumstances when the compiler does not, or cannot, apply the
    /// return value optimisation. It causes the contents of \c rhs to be
    /// transferred into the constructing instance. This is completely safe
    /// because the \c rhs instance will never be accessed in its own right, so
    /// does not need to maintain ownership of its contents.
    c_str_VARIANT_proxy_w(class_type& rhs)
        : m_bstr(rhs.m_bstr)
    {
        move_lhs_from_rhs(rhs).m_bstr = NULL;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_VARIANT_proxy_w(class_type const& rhs)
        : m_bstr(bstr_dup(rhs.m_bstr))
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_VARIANT_proxy_w() stlsoft_throw_0()
    {
        ::SysFreeString(m_bstr);
    }

// Accessors
public:
    /// Returns a null-terminated string representing the VARIANT contents.
    operator LPCOLESTR () const
    {
        return (m_bstr == NULL) ? L"" : m_bstr;
    }

// Members
private:
    BSTR    m_bstr;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

/** \brief This class provides an intermediary object that may be returned by the
 * c_str_ptr_a() function, such that the text of a given variant
 * may be accessed as a null-terminated string.
 *
 * \ingroup group__concept__shim__string_access
 *
 */
class c_str_VARIANT_proxy_a
{
public:
    typedef c_str_VARIANT_proxy_a   class_type;
    typedef c_str_VARIANT_proxy_w   class_w_type;

// Construction
public:
    /// Constructs an instance of the proxy from the given c_str_VARIANT_proxy_w
    ///
    /// \param rhs The c_str_VARIANT_proxy_w from which the text will be retrieved
    ss_explicit_k c_str_VARIANT_proxy_a(class_w_type rhs)
        : m_proxyw(rhs)
        , m_buffer(0)
    {}

#ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    /// Move constructor
    ///
    /// This <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">move constructor</a>
    /// is for circumstances when the compiler does not, or cannot, apply the
    /// return value optimisation. It causes the contents of \c rhs to be
    /// transferred into the constructing instance. This is completely safe
    /// because the \c rhs instance will never be accessed in its own right, so
    /// does not need to maintain ownership of its contents.
    c_str_VARIANT_proxy_a(class_type& rhs)
        : m_proxyw(rhs.m_proxyw)
        , m_buffer(rhs.m_buffer)
    {
        move_lhs_from_rhs(rhs).m_buffer = NULL;
    }
#else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    // Copy constructor
    c_str_VARIANT_proxy_a(class_type const& rhs)
        : m_proxyw(rhs.m_proxyw)
        , m_buffer(NULL)
    {}
#endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

    /// Releases any storage aquired by the proxy
    ~c_str_VARIANT_proxy_a() stlsoft_throw_0()
    {
        if(m_buffer != empty_string_())
        {
            ::CoTaskMemFree(m_buffer);
        }
    }

// Accessors
public:
    /// Returns a null-terminated string representing the VARIANT contents.
    operator cs_char_a_t const* () const
    {
        if(NULL == m_buffer)
        {
            LPCOLESTR   w_value     =   m_proxyw;
            cs_char_a_t *&buffer_   =   const_cast<class_type*>(this)->m_buffer;

            if( NULL == w_value ||
                L'\0' == *w_value)
            {
                buffer_ = empty_string_();
            }
            else
            {
                cs_size_t cch = ::SysStringLen((BSTR)w_value);

                buffer_ = static_cast<cs_char_a_t *>(::CoTaskMemAlloc((1 + cch) * sizeof(cs_char_a_t)));

                if(NULL == buffer_)
                {
                    buffer_ = empty_string_();
                }
                else
                {
                    int n = ::WideCharToMultiByte(0, 0, w_value, -1, buffer_, static_cast<int>(cch + 1), NULL, NULL);

#ifdef WIN32
                    if(0 == n)
#else /* ? WIN32 */
# error Not currently implemented for operating systems other than Win32
#endif /* WIN32 */
                    {
                        // TODO: report failure to convert via exception
                    }
                }
            }
        }

        return m_buffer;
    }

// Implementation
private:
    static cs_char_a_t *empty_string_()
    {
        // This character array is initialised to 0, which conveniently happens to
        // be the empty string, by the module/application load, so it is
        // guaranteed to be valid, and there are no threading/race conditions
        static cs_char_a_t  s_empty[1];

        COMSTL_ASSERT(s_empty[0] == '\0'); // Paranoid check

        return s_empty;
    }

// Members
private:
    c_str_VARIANT_proxy_w   m_proxyw;
    cs_char_a_t             *m_buffer;

// Not to be implemented
private:
    void operator =(class_type const& rhs);
};

/* /////////////////////////////////////////////////////////////////////////
 * Equivalence testing
 */

// c_str_null_VARIANT_proxy
inline cs_bool_t operator ==(LPCOLESTR lhs, c_str_null_VARIANT_proxy const& rhs)
{
    return lhs == static_cast<LPCOLESTR>(rhs);
}

inline cs_bool_t operator ==(c_str_null_VARIANT_proxy const& lhs, LPCOLESTR rhs)
{
    return static_cast<LPCOLESTR>(lhs) == rhs;
}

inline cs_bool_t operator !=(LPCOLESTR lhs, c_str_null_VARIANT_proxy const& rhs)
{
    return lhs != static_cast<LPCOLESTR>(rhs);
}

inline cs_bool_t operator !=(c_str_null_VARIANT_proxy const& lhs, LPCOLESTR rhs)
{
    return static_cast<LPCOLESTR>(lhs) != rhs;
}

// c_str_VARIANT_proxy_a
inline cs_bool_t operator ==(LPCSTR lhs, c_str_VARIANT_proxy_a const& rhs)
{
    return lhs == static_cast<LPCSTR>(rhs);
}

inline cs_bool_t operator ==(c_str_VARIANT_proxy_a const& lhs, LPCSTR rhs)
{
    return static_cast<LPCSTR>(lhs) == rhs;
}

inline cs_bool_t operator !=(LPCSTR lhs, c_str_VARIANT_proxy_a const& rhs)
{
    return lhs != static_cast<LPCSTR>(rhs);
}

inline cs_bool_t operator !=(c_str_VARIANT_proxy_a const& lhs, LPCSTR rhs)
{
    return static_cast<LPCSTR>(lhs) != rhs;
}

// c_str_VARIANT_proxy_w
inline cs_bool_t operator ==(LPCOLESTR lhs, c_str_VARIANT_proxy_w const& rhs)
{
    return lhs == static_cast<LPCOLESTR>(rhs);
}

inline cs_bool_t operator ==(c_str_VARIANT_proxy_w const& lhs, LPCOLESTR rhs)
{
    return static_cast<LPCOLESTR>(lhs) == rhs;
}

inline cs_bool_t operator !=(LPCOLESTR lhs, c_str_VARIANT_proxy_w const& rhs)
{
    return lhs != static_cast<LPCOLESTR>(rhs);
}

inline cs_bool_t operator !=(c_str_VARIANT_proxy_w const& lhs, LPCOLESTR rhs)
{
    return static_cast<LPCOLESTR>(lhs) != rhs;
}

/* /////////////////////////////////////////////////////////////////////////
 * IOStream compatibility
 */

template <ss_typename_param_k S>
inline S& operator <<(S& s, c_str_null_VARIANT_proxy const& shim)
{
    s << static_cast<LPCOLESTR>(shim);

    return s;
}

template <ss_typename_param_k S>
inline S& operator <<(S& s, c_str_VARIANT_proxy_w const& shim)
{
    s << static_cast<LPCOLESTR>(shim);

    return s;
}

template <ss_typename_param_k S>
inline S& operator <<(S& s, c_str_VARIANT_proxy_a const& shim)
{
    s << static_cast<cs_char_a_t const*>(shim);

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_data
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline c_str_VARIANT_proxy_a c_str_data_a(VARIANT const& v)
{
    VARIANT vs;
    HRESULT hr;

    ::VariantInit(&vs);

    hr  =   ::VariantChangeTypeEx(&vs, const_cast<VARIANT *>(&v), LOCALE_USER_DEFAULT, VARIANT_ALPHABOOL, VT_BSTR);

    if(FAILED(hr))
    {
        vs.bstrVal = NULL;
    }

    return c_str_VARIANT_proxy_a(c_str_VARIANT_proxy_w(vs.bstrVal));
}

inline c_str_VARIANT_proxy_w c_str_data_w(VARIANT const& v)
{
    VARIANT vs;
    HRESULT hr;

    ::VariantInit(&vs);

    hr  =   ::VariantChangeTypeEx(&vs, const_cast<VARIANT *>(&v), LOCALE_USER_DEFAULT, VARIANT_ALPHABOOL, VT_BSTR);

    if(FAILED(hr))
    {
        vs.bstrVal = NULL;
    }

    return c_str_VARIANT_proxy_w(vs.bstrVal);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Returns the corresponding possibly unterminated C-string pointer of the VARIANT \c v
 *
 * \ingroup group__concept__shim__string_access
 *
 */
#ifdef UNICODE
inline c_str_VARIANT_proxy_w c_str_data(VARIANT const& v)
#else /* ? UNICODE */
inline c_str_VARIANT_proxy_a c_str_data(VARIANT const& v)
#endif /* UNICODE */
{
#ifdef UNICODE
    return c_str_data_w(v);
#else /* ? UNICODE */
    return c_str_data_a(v);
#endif /* UNICODE */
}


/* /////////////////////////////////////////////////////////////////////////
 * c_str_len
 *
 * This can be applied to an expression, and the return value is the number of
 * characters in the character string in the expression.
 */

/** \brief Returns the length (in characters) of the VARIANT \c v, <b><i>not</i></b> including the null-terminating character
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline cs_size_t c_str_len_a(VARIANT const& v)
{
    if( v.vt == VT_NULL ||
        v.vt == VT_EMPTY)
    {
        return 0;
    }
    else
    {
        return stlsoft_ns_qual(c_str_len_a)(c_str_data_a(v));
    }
}

/** \brief Returns the length (in characters) of the VARIANT \c v, <b><i>not</i></b> including the null-terminating character
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline cs_size_t c_str_len_w(VARIANT const& v)
{
    if(v.vt == VT_BSTR)
    {
        return v.bstrVal != NULL ? ::SysStringLen(v.bstrVal) : 0;
    }
    else if(v.vt == VT_NULL ||
            v.vt == VT_EMPTY)
    {
        return 0;
    }
    else
    {
        return stlsoft_ns_qual(c_str_len_w)(c_str_data_w(v));
    }
}

/** \brief Returns the length (in characters) of the VARIANT \c v, <b><i>not</i></b> including the null-terminating character
 *
 * \ingroup group__concept__shim__string_access
 *
 */
inline cs_size_t c_str_len(VARIANT const& v)
{
#ifdef UNICODE
    return c_str_len_w(v);
#else /* ? UNICODE */
    return c_str_len_a(v);
#endif /* UNICODE */
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or to an empty string.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline c_str_VARIANT_proxy_a c_str_ptr_a(VARIANT const& v)
{
    return c_str_data_a(v);
}

inline c_str_VARIANT_proxy_w c_str_ptr_w(VARIANT const& v)
{
    return c_str_data_w(v);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Returns the corresponding C-string pointer of the VARIANT \c v
 *
 * \ingroup group__concept__shim__string_access
 *
 */
#ifdef UNICODE
inline c_str_VARIANT_proxy_w c_str_ptr(VARIANT const& v)
#else /* ? UNICODE */
inline c_str_VARIANT_proxy_a c_str_ptr(VARIANT const& v)
#endif /* UNICODE */
{
#ifdef UNICODE
    return c_str_ptr_w(v);
#else /* ? UNICODE */
    return c_str_ptr_a(v);
#endif /* UNICODE */
}

/* /////////////////////////////////////////////////////////////////////////
 * c_str_ptr_null
 *
 * This can be applied to an expression, and the return value is either a
 * pointer to the character string or NULL.
 */

/** \brief Returns the corresponding ANSI C-string pointer of the VARIANT \c v, or a null pointer
 *
 * \ingroup group__concept__shim__string_access
 *
 */
//inline c_str_null_VARIANT_proxy<cs_char_a_t> c_str_ptr_null_a(VARIANT const& v);
//inline c_str_null_VARIANT_proxy<cs_char_w_t> c_str_ptr_null_w(VARIANT const& v);
//inline c_str_null_VARIANT_proxy<cs_char_o_t> c_str_ptr_null_o(VARIANT const& v);

inline c_str_null_VARIANT_proxy c_str_ptr_null_w(VARIANT const& v)
{
    if(v.vt == VT_BSTR)
    {
        return c_str_null_VARIANT_proxy(v.bstrVal);
    }
    else if(v.vt == VT_NULL ||
            v.vt == VT_EMPTY)
    {
        return c_str_null_VARIANT_proxy();
    }
    else
    {
        VARIANT vs;
        HRESULT hr;

        ::VariantInit(&vs);

        hr  =   ::VariantChangeTypeEx(&vs, const_cast<VARIANT *>(&v), LOCALE_USER_DEFAULT, VARIANT_ALPHABOOL, VT_BSTR);

        if(FAILED(hr))
        {
            vs.bstrVal = NULL;
        }

        return c_str_null_VARIANT_proxy(&vs.bstrVal);
    }
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/variant_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

using ::comstl::c_str_data_a;
using ::comstl::c_str_data_w;
//using ::comstl::c_str_data_o;
using ::comstl::c_str_data;

using ::comstl::c_str_len_a;
using ::comstl::c_str_len_w;
//using ::comstl::c_str_len_o;
using ::comstl::c_str_len;

using ::comstl::c_str_ptr_a;
using ::comstl::c_str_ptr_w;
//using ::comstl::c_str_ptr_o;
using ::comstl::c_str_ptr;

//using ::comstl::c_str_ptr_null_a;
using ::comstl::c_str_ptr_null_w;
//using ::comstl::c_str_ptr_null_o;
//using ::comstl::c_str_ptr_null;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Global namespace shims
 */

/* This defines stream inserter shim functions for the converters for use
 * with the Visual C++ <7.1 standard library.
 */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

# include <iosfwd>

inline comstl_ns_qual_std(basic_ostream)<char>& operator <<(comstl_ns_qual_std(basic_ostream)<char> &stm, comstl_ns_qual(c_str_VARIANT_proxy_a) const& v)
{
    return stm << static_cast<char const*>(v);
}

inline comstl_ns_qual_std(basic_ostream)<wchar_t>& operator <<(comstl_ns_qual_std(basic_ostream)<wchar_t> &stm, comstl_ns_qual(c_str_VARIANT_proxy_w) const& v)
{
    return stm << static_cast<wchar_t const*>(v);
}

#endif /* library */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_SHIMS_ACCESS_STRING_HPP_VARIANT */

/* ///////////////////////////// end of file //////////////////////////// */
