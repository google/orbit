/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/char_conversions.hpp (originally MLStrCnv.h, ::SynesisStd)
 *
 * Purpose:     Character-encoding scheme interconversion components.
 *
 * Created:     31st May 2003
 * Updated:     29th January 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2011, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/char_conversions.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::multibyte2wide and
 *  stlsoft::wide2multibyte class templates
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS_MAJOR    5
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS_MINOR    2
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS_REVISION 1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS_EDIT     95
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:  __GNUC__<3
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 3
# error stlsoft/conversion/char_conversions.hpp is not compatible with GNU C++ prior to 3.0
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1100
# error stlsoft/conversion/char_conversions.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR
#  include <stlsoft/error/conversion_error.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_CONVERSION_ERROR */
# include <errno.h>
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

#ifdef STLSOFT_UNITTEST
# include <wchar.h>
#endif /* STLSOFT_UNITTEST */

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

/** \brief Converts a multibyte (<code>char</code>-based) string to a wide
 *   (<code>whar_t</code>-based) string.
 *
 * \ingroup group__library__conversion
 */
template <ss_size_t CCH>
class multibyte2wide
    : public auto_buffer<ss_char_w_t, CCH>
{
/// \name Member Types
/// @{
private:
    typedef auto_buffer<ss_char_w_t, CCH>                       parent_class_type;
public:
    /// \brief The character type
    typedef ss_char_w_t                                         char_type;
    /// \brief The alternate character type
    typedef ss_char_a_t                                         alt_char_type;
    /// \brief The size type
    typedef ss_typename_type_k parent_class_type::size_type     size_type;
    /// \brief The pointer type
    typedef ss_typename_type_k parent_class_type::pointer       pointer;
/// @}

/// \name Construction
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    ss_explicit_k multibyte2wide(S const& s)
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    ss_explicit_k multibyte2wide(alt_char_type const* s)
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
        : parent_class_type(calc_length_(s) + 1)
    {
        prepare_(stlsoft_ns_qual(c_str_ptr_a)(s));
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    multibyte2wide(S const& s, size_type cch)
#else
    multibyte2wide(alt_char_type const* s, size_type cch)
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
        : parent_class_type(cch + 1)
    {
        prepare_(stlsoft_ns_qual(c_str_data_a)(s), cch);
    }

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
/// @}

/// \name Implementation
/// @{
private:
    template <ss_typename_param_k S>
    static size_type calc_length_(S const& s)
    {
        return stlsoft_ns_qual(c_str_len_a)(s);
    }

    void prepare_(alt_char_type const* s)
    {
        prepare_(s, parent_class_type::size() - 1);
    }

    void prepare_(alt_char_type const* s, size_type size)
    {
        const pointer data = parent_class_type::data();

        // If the auto_buffer failed to allocate the required memory, and
        // we're not in an exception-environment, then size() will be zero
        if(0 == size)
        {
            // Since we know that auto_buffer's parameterising size must
            // always be greater that 0, then
            data[0] = '\0';
        }
        else
        {
            int         err;
#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
            ss_size_t   numConverted;

            err = ::mbstowcs_s(&numConverted, data, size + 1, s, size);

            if(0 != err)
            {
#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
            if(static_cast<ss_size_t>(-1) == ::mbstowcs(data, s, size))
            {
                err = errno;
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                STLSOFT_THROW_X(conversion_error("failed to convert multibyte string to wide string", err));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                data[0] = '\0';
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            } // '}'
            else
            {
                data[size] = '\0';
            }
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    char_type const* data() const
    {
        return parent_class_type::data();
    }
    char_type const* c_str() const
    {
        return parent_class_type::data();
    }

    size_type size() const
    {
        size_type n = parent_class_type::size();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_ASSERT(0 != n);
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        if(0 == n)
        {
            return 0;
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return n - 1;
    }
/// @}

/// \name Operators
/// @{
public:
    operator char_type const* () const
    {
        return parent_class_type::data();
    }
/// @}

/// \name Not to be implemented
/// @{
private:
# if defined(STLSOFT_COMPILER_IS_GCC)
public:
    multibyte2wide(multibyte2wide const& rhs)
        : parent_class_type(rhs.parent_class_type::size())
    {
        ::memcpy(this->parent_class_type::data(), rhs.parent_class_type::data(), this->parent_class_type::size());
    }
private:
# else /* compiler */
    multibyte2wide(multibyte2wide const&);
# endif /* compiler */
    multibyte2wide& operator =(multibyte2wide const&);

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}
};

/** \brief Converts a wide (<code>whar_t</code>-based) string to a
 *    multibyte (<code>char</code>-based) string.
 *
 * \ingroup group__library__conversion
 */
template <ss_size_t CCH>
class wide2multibyte
    : public auto_buffer<ss_char_a_t, CCH>
{
/// \name Member Types
/// @{
private:
    typedef auto_buffer<ss_char_a_t, CCH>                       parent_class_type;
public:
    /// \brief The character type
    typedef ss_char_a_t                                         char_type;
    /// \brief The alternate character type
    typedef ss_char_w_t                                         alt_char_type;
    /// \brief The size type
    typedef ss_typename_type_k parent_class_type::size_type     size_type;
    /// \brief The pointer type
    typedef ss_typename_type_k parent_class_type::pointer       pointer;
/// @}

/// \name Construction
/// @{
public:
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    ss_explicit_k wide2multibyte(S const& s)
#else
    ss_explicit_k wide2multibyte(alt_char_type const* s)
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
        : parent_class_type(stlsoft_ns_qual(c_str_len)(s) + 1)
    {
        prepare_(stlsoft_ns_qual(c_str_ptr_w)(s));
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    ss_explicit_k wide2multibyte(S const& s, size_type cch)
#else
    ss_explicit_k wide2multibyte(alt_char_type const* s, size_type cch)
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
        : parent_class_type(cch + 1)
    {
        prepare_(stlsoft_ns_qual(c_str_data_w)(s), cch);
    }
/// @}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Implementation
private:
    void prepare_(alt_char_type const* s)
    {
        prepare_(s, parent_class_type::size() - 1);
    }

    void prepare_(alt_char_type const* s, size_type size)
    {
        const pointer data = parent_class_type::data();

        // If the auto_buffer failed to allocate the required memory, and
        // we're not in an exception-environment, then size() will be zero
        if(0 == size)
        {
            // Since we know that auto_buffer's parameterising size must
            // always be greater that 0, then
            data[0] = '\0';
        }
        else
        {
            int         err;
#ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
            ss_size_t   numConverted;

            err = ::wcstombs_s(&numConverted, data, size + 1, s, size);

            if(0 != err)
            {
#else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
            if(static_cast<ss_size_t>(-1) == ::wcstombs(data, s, size))
            {
                err = errno;
#endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                STLSOFT_THROW_X(conversion_error("failed to convert wide string to multibyte string", err));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                data[0] = '\0';
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
            else
            {
                data[size] = '\0';
            }
        }
    }

/// \name Accessors
/// @{
public:
    char_type const* data() const
    {
        return parent_class_type::data();
    }
    char_type const* c_str() const
    {
        return parent_class_type::data();
    }

    size_type size() const
    {
        size_type n = parent_class_type::size();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_ASSERT(0 != n);
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        if(0 == n)
        {
            return 0;
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return n - 1;
    }
/// @}

/// \name Operators
/// @{
public:
    operator char_type const* () const
    {
        return parent_class_type::data();
    }
/// @}

/// \name Not to be implemented
/// @{
private:
# if defined(STLSOFT_COMPILER_IS_GCC)
public:
    wide2multibyte(wide2multibyte const& rhs)
        : parent_class_type(rhs.parent_class_type::size())
    {
        ::memcpy(this->parent_class_type::data(), rhs.parent_class_type::data(), this->parent_class_type::size());
    }
private:
# else /* compiler */
    wide2multibyte(wide2multibyte const&);
# endif /* compiler */
    wide2multibyte& operator =(wide2multibyte const&);
/// @}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
class encoding2encoding
{
public:
    encoding2encoding(C const* s)
        : m_s(s)
    {}
    encoding2encoding(C *s)
        : m_s(s)
    {}
    template <ss_typename_param_k S>
    encoding2encoding(S const& s)
        : m_s(s.c_str())
    {}

public:
    C const* c_str() const
    {
        return m_s;
    }
    operator C const* () const
    {
        return m_s;
    }

private:
    C const* m_s;
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

/** \brief Type that converts a multibyte string to a wide string.
 *
 * \ingroup group__library__conversion
 */
typedef multibyte2wide<256>               m2w;
/** \brief Type that converts a wide string to a multibyte string.
 *
 * \ingroup group__library__conversion
 */
typedef wide2multibyte<256>               w2m;

/** \brief [Deprecated] Type that converts a multibyte string to a wide string.
 *
 * \ingroup group__library__conversion
 *
 * \deprecated This name is deprecated in favour of stlsoft::m2w
 */
typedef multibyte2wide<256>               a2w;
/** \brief [Deprecated] Type that converts a wide string to a multibyte string.
 *
 * \ingroup group__library__conversion
 *
 * \deprecated This name is deprecated in favour of stlsoft::w2m
 */
typedef wide2multibyte<256>               w2a;

//#if defined(UNICODE)
//typedef encoding2encoding<ss_char_w_t>  t2w;
//typedef encoding2encoding<ss_char_w_t>  w2t;
//typedef w2a                             t2a;
//typedef a2w                             a2t;
//#else /* ? UNICODE */
//typedef encoding2encoding<ss_char_a_t>  t2a;
//typedef encoding2encoding<ss_char_a_t>  a2t;
//typedef a2w                             t2w;
//typedef w2a                             w2t;
//#endif /* UNICODE */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for stlsoft::multibyte2wide
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_ptr_null(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_ptr_null_w(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief \ref group__concept__shim__string_access__c_str_ptr for stlsoft::multibyte2wide
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_ptr(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_ptr_w(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for stlsoft::multibyte2wide
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_data(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.data();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_w_t const* c_str_data_w(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.data();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for stlsoft::multibyte2wide
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_size_t c_str_len(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.size();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_size_t c_str_len_w(stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return b.size();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for stlsoft::wide2multibyte
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_ptr_null(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_ptr_null_a(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for stlsoft::wide2multibyte
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_ptr(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.c_str();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_ptr_a(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for stlsoft::wide2multibyte
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_data(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.data();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_char_a_t const* c_str_data_a(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.data();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for stlsoft::wide2multibyte
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_size_t   CCH
        >
inline ss_size_t c_str_len(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.size();
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_size_t   CCH
        >
inline ss_size_t c_str_len_a(stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return b.size();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */




/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for stlsoft::multibyte2wide
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        ,   ss_size_t           CCH
        >
inline S& operator <<(S& s, stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    s << b.c_str();

    return s;
}

/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for stlsoft::wide2multibyte
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        ,   ss_size_t           CCH
        >
inline S& operator <<(S& s, stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    s << b.c_str();

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/char_conversions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Global namespace shims
 */

/* This defines stream inserter shim function templates for the converters
 * for use with the Visual C++ <7.1 standard library.
 */

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

# include <iosfwd>

template <stlsoft_ns_qual(ss_size_t) CCH>
inline stlsoft_ns_qual_std(basic_ostream)<char>& operator <<(stlsoft_ns_qual_std(basic_ostream)<char> &stm, stlsoft_ns_qual(wide2multibyte)<CCH> const& b)
{
    return stm << b.c_str();
}

template <stlsoft_ns_qual(ss_size_t) CCH>
inline stlsoft_ns_qual_std(basic_ostream)<wchar_t>& operator <<(stlsoft_ns_qual_std(basic_ostream)<wchar_t> &stm, stlsoft_ns_qual(multibyte2wide)<CCH> const& b)
{
    return stm << b.c_str();
}

#endif /* library */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS */

/* ///////////////////////////// end of file //////////////////////////// */
