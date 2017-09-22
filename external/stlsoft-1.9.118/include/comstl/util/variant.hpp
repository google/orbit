/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/variant.hpp (originally MOVriant.h/.cpp, ::SynesisCom)
 *
 * Purpose:     variant class.
 *
 * Created:     12th December 1996
 * Updated:     5th March 2011
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


/** \file comstl/util/variant.hpp
 *
 * \brief [C++ only; requires COM] Definition of the comstl::variant class
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_COMSTL_VARIANT
#define COMSTL_INCL_COMSTL_UTIL_HPP_COMSTL_VARIANT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _COMSTL_VER_COMSTL_UTIL_HPP_COMSTL_VARIANT_MAJOR      2
# define _COMSTL_VER_COMSTL_UTIL_HPP_COMSTL_VARIANT_MINOR      3
# define _COMSTL_VER_COMSTL_UTIL_HPP_COMSTL_VARIANT_REVISION   5
# define _COMSTL_VER_COMSTL_UTIL_HPP_COMSTL_VARIANT_EDIT       158
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
#ifndef COMSTL_INCL_COMSTL_SHIMS_ACCESS_HPP_STRING
# include <comstl/shims/access/string.hpp>
#endif /* !COMSTL_INCL_COMSTL_SHIMS_ACCESS_HPP_STRING */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS
#  include <comstl/error/exceptions.hpp>
# endif /* !COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS */
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS
# include <comstl/util/interface_traits.hpp>
#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS */
#ifndef COMSTL_INCL_COMSTL_UTIL_H_VARIANT_FUNCTIONS
# include <comstl/util/VARIANT_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_H_VARIANT_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */

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

/** \brief Facade for the COM VARIANT type
 *
 * \ingroup group__library__utility__com
 *
 * \remarks comstl::variant publicly derives from \c VARIANT as a measure
 *   of expedience, rather than as an act of design sophistication. Any
 *   manual manipulation of the instances, or their member variables, is
 *   at the user's risk. Notwithstanding, this is helped by the fact that
 *   comstl::variant declares no member variables and no virtual member
 *   functions - for most compilers this means that the Empty Derived
 *   Optimisation (see Section 12.4 of Imperfect C++) will apply.
 */
class variant
    : public VARIANT
{
/// Member Types
public:
    typedef variant         class_type;
    typedef cs_bool_t       bool_type;
    typedef cs_size_t       size_type;

/// Conversion
public:
    /** Default constructor
     *
     * Initialises the instance
     *
     * \post <code>assert(VT_EMPTY == this->vt)</code>
     *
     * \exception - Does not throw an exception
     */
    variant();

    /** Copying constructor
     *
     * Initialises the instance with a copy of the given \c VARIANT
     *
     * \post <code>assert(rhs == *this)</code>
     *
     * \exception comstl::com_exception If the copy fails
     */
    variant(VARIANT const& rhs);

    /** Copy constructor
     *
     * \post <code>assert(rhs == *this)</code>
     *
     * \exception comstl::com_exception If the copy fails
     */
    variant(class_type const& rhs);

    /** Copy assignment operator
     *
     * \post <code>assert(rhs == *this)</code>
     *
     * \exception comstl::com_exception If the copy fails
     */
    class_type& operator =(class_type const& rhs);

    /** Conversion constructor
     *
     * Initialises the instance with the given boolean value
     *
     * \post <code>assert(VT_BOOL == this->vt)</code>
     * \post <code>assert(b == (VARIANT_TRUE == this->boolVal))</code>
     *
     * \exception - Does not throw an exception
     */
    variant(bool b);

    /** Conversion constructor
     *
     * Initialises the instance with the given 8-bit signed integer value
     *
     * \post <code>assert(VT_I1 == this->vt)</code>
     * \post <code>assert(i == this->cVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::sint8_t i);

    /** Conversion constructor
     *
     * Initialises the instance with the given 8-bit unsigned integer value
     *
     * \post <code>assert(VT_UI1 == this->vt)</code>
     * \post <code>assert(i == this->bVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::uint8_t i);

    /** Conversion constructor
     *
     * Initialises the instance with the given 16-bit signed integer value
     *
     * \post <code>assert(VT_I2 == this->vt)</code>
     * \post <code>assert(i == this->iVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::sint16_t i);

    /** Conversion constructor
     *
     * Initialises the instance with the given 16-bit unsigned integer value
     *
     * \post <code>assert(VT_UI2 == this->vt)</code>
     * \post <code>assert(i == this->uiVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::uint16_t i);

    /** Conversion constructor
     *
     * Initialises the instance with the given 32-bit signed integer value
     *
     * \post <code>assert(VT_I4 == this->vt)</code>
     * \post <code>assert(i == this->lVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::sint32_t i);

    /** Conversion constructor
     *
     * Initialises the instance with the given 32-bit unsigned integer value
     *
     * \post <code>assert(VT_UI4 == this->vt)</code>
     * \post <code>assert(i == this->ulVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(stlsoft::uint32_t i);

//#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
//  variant(stlsoft::sint64_t i);
//  variant(stlsoft::uint64_t i);
//#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
    /** Conversion constructor
     *
     * Initialises the instance with the given \c short value
     *
     * \post <code>assert(VT_I2 == this->vt)</code>
     * \post <code>assert(i == this->iVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(short i);

    /** Conversion constructor
     *
     * Initialises the instance with the given
     * <code>unsigned short</code> value
     *
     * \post <code>assert(VT_UI2 == this->vt)</code>
     * \post <code>assert(i == this->uiVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(unsigned short i);
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
    /** Conversion constructor
     *
     * Initialises the instance with the given \c int value
     *
     * \post <code>assert(VT_I4 == this->vt)</code>
     * \post <code>assert(i == this->lVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(int i);

    /** Conversion constructor
     *
     * Initialises the instance with the given
     * <code>unsigned int</code> value
     *
     * \post <code>assert(VT_UI4 == this->vt)</code>
     * \post <code>assert(i == this->ulVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(unsigned int i);
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
    /** Conversion constructor
     *
     * Initialises the instance with the given \c long value
     *
     * \post <code>assert(VT_I4 == this->vt)</code>
     * \post <code>assert(i == this->lVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(long i);

    /** Conversion constructor
     *
     * Initialises the instance with the given
     * <code>unsigned long</code> value
     *
     * \post <code>assert(VT_UI4 == this->vt)</code>
     * \post <code>assert(i == this->ulVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(unsigned long i);
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

    /** Conversion constructor
     *
     * Initialises the instance with the given \c float value
     *
     * \post <code>assert(VT_R4 == this->vt)</code>
     * \post <code>assert(r == this->fltVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(float r);

    /** Conversion constructor
     *
     * Initialises the instance with the given \c double value
     *
     * \post <code>assert(VT_R8 == this->vt)</code>
     * \post <code>assert(r == this->dblVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(double r);

    /** Conversion constructor
     *
     * Initialises the instance with the given currency (\c CY) value
     *
     * \post <code>assert(VT_CY == this->vt)</code>
     * \post <code>assert(r == this->cyVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(CY cy);

    /** Conversion constructor
     *
     * Initialises the instance with the given \c DECIMAL value
     *
     * \post <code>assert(VT_DECIMAL == this->vt)</code>
     * \post <code>assert(dec == this->decVal)</code>
     *
     * \exception - Does not throw an exception
     */
    variant(DECIMAL const& dec);


    variant(LPUNKNOWN punk, bool_type bAddRef);
    variant(LPDISPATCH pdisp, bool_type bAddRef);
    variant(cs_char_a_t const* s, int len = -1);
    variant(cs_char_w_t const* s, int len = -1);
    variant(VARIANT const& var, VARTYPE vt);

    /** Releases any resources associated with the underlying
     *   <code>VARIANT</code>
     */
    ~variant() stlsoft_throw_0()
    {
        stlsoft_constraint_must_be_same_size(class_type, VARIANT);

        ::VariantClear(this);
    }

    /** Clears the variant
     *
     * \post <code>assert(VT_EMPTY == this->vt)</code>
     */
    void clear();

/// Operations
public:
    HRESULT     try_conversion_copy(VARIANT const& var, VARTYPE vt);
    HRESULT     try_convert(VARTYPE vt);
    class_type& convert(VARTYPE vt);

    /** Returns a pointer to a specified interface on an object to which
     * a client currently holds an interface pointer.
     *
     * \return An <code>HRESULT</code> code indicating the success of the
     *   operation.
     * \retval <code>S_OK</code> The interface is supported:
     *   <code>*ppv</code> will hold the pointer to the requested interface
     * \retval <code>E_INTERFACE</code> The interface is not supported: the
     *   value of <code>*ppv</code> is undefined.
     *
     * \pre <code>NULL != ppv</code>
     */
    HRESULT     QueryInterface(REFIID riid, void** ppv) const;

    /** Returns a pointer to a specified interface on an object to which
     * a client currently holds an interface pointer.
     *
     * \return An <code>HRESULT</code> code indicating the success of the
     *   operation.
     * \retval <code>S_OK</code> The interface is supported:
     *   <code>*ppi</code> will hold the pointer to the requested interface
     * \retval <code>E_INTERFACE</code> The interface is not supported: the
     *   value of <code>*ppi</code> is undefined.
     *
     * \pre <code>NULL != ppi</code>
     */
    template <ss_typename_param_k I>
    HRESULT QueryInterfaceValue(I** ppi)
    {
        return QueryInterface(IID_traits<I>::iid(), reinterpret_cast<void**>(ppi));
    }

public:
    /** Swaps the contents with another instance
     */
    void swap(class_type& rhs);

/// Comparison
public:
    bool_type   equal(class_type const& rhs) const;
    bool_type   equal(VARIANT const& rhs) const;

/// Operators
public:

private:
    static void swap_(VARIANT& lhs, VARIANT& rhs);
    void handle_error_(char const* message, HRESULT hr);
};

/* /////////////////////////////////////////////////////////////////////////
 * String access shims
 */

// No string access shims are defined, because there're already a set
// defined for VARIANT, in comstl/shims/access/string.hpp, which is included
// by this file.

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

inline cs_bool_t operator ==(variant const& lhs, variant const& rhs)
{
    return lhs.equal(rhs);
}

inline cs_bool_t operator !=(variant const& lhs, variant const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline cs_bool_t operator ==(variant const& lhs, VARIANT const& rhs)
{
    return lhs.equal(rhs);
}

inline cs_bool_t operator !=(variant const& lhs, VARIANT const& rhs)
{
    return !operator ==(lhs, rhs);
}

inline cs_bool_t operator ==(VARIANT const& lhs, variant const& rhs)
{
    return rhs.equal(lhs);
}

inline cs_bool_t operator !=(VARIANT const& lhs, variant const& rhs)
{
    return !operator ==(lhs, rhs);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/variant_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline void variant::handle_error_(char const* message, HRESULT hr)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT

    STLSOFT_THROW_X(com_exception(message, hr));

#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */

    STLSOFT_SUPPRESS_UNUSED(message);

    ::VariantClear(this);

    this->vt    =   VT_ERROR;
    this->scode =   hr;

#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline /* static */ void variant::swap_(VARIANT& lhs, VARIANT& rhs)
{
    VARIANT t;

    ::memcpy(&t, &lhs, sizeof(VARIANT));
    ::memcpy(&lhs, &rhs, sizeof(VARIANT));
    ::memcpy(&rhs, &t, sizeof(VARIANT));
}




inline variant::variant()
{
    ::VariantInit(this);
}

inline variant::variant(class_type const& rhs)
{
    ::VariantInit(this);

    class_type& rhs_    =   const_cast<class_type&>(rhs);
    HRESULT     hr      =   ::VariantCopy(this, &rhs_);

    if(FAILED(hr))
    {
        handle_error_("failed to copy variant", hr);
    }
}

inline variant::variant(VARIANT const& rhs)
{
    ::VariantInit(this);

    HRESULT hr = ::VariantCopy(this, const_cast<VARIANT*>(&rhs));

    if(FAILED(hr))
    {
        handle_error_("failed to copy variant", hr);
    }
}

inline variant::class_type& variant::operator =(variant::class_type const& rhs)
{
    class_type r(rhs);

    r.swap(*this);

    return *this;
}

inline variant::variant(bool b)
{
    ::VariantInit(this);

    this->vt        =   VT_BOOL;
    this->boolVal   =   b ? VARIANT_TRUE : VARIANT_FALSE;
}

inline variant::variant(stlsoft::sint8_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_I1;
    this->cVal  =   static_cast<CHAR>(i);
}

inline variant::variant(stlsoft::uint8_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI1;
    this->bVal  =   static_cast<BYTE>(i);
}

inline variant::variant(stlsoft::sint16_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_I2;
    this->iVal  =   static_cast<SHORT>(i);
}

inline variant::variant(stlsoft::uint16_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI2;
    this->uiVal =   static_cast<USHORT>(i);
}

inline variant::variant(stlsoft::sint32_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_I4;
    this->lVal  =   static_cast<LONG>(i);
}

inline variant::variant(stlsoft::uint32_t i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI4;
    this->ulVal =   static_cast<ULONG>(i);
}

#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
inline variant::variant(short i)
{
    ::VariantInit(this);

    this->vt    =   VT_I2;
    this->iVal  =   i;
}
inline variant::variant(unsigned short i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI2;
    this->uiVal =   i;
}
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline variant::variant(int i)
{
    ::VariantInit(this);

    this->vt    =   VT_I4;
    this->lVal  =   i;
}
inline variant::variant(unsigned int i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI4;
    this->ulVal =   i;
}
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline variant::variant(long i)
{
    ::VariantInit(this);

    this->vt    =   VT_I4;
    this->lVal  =   i;
}
inline variant::variant(unsigned long i)
{
    ::VariantInit(this);

    this->vt    =   VT_UI4;
    this->ulVal =   i;
}
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */


inline variant::variant(float r)
{
    ::VariantInit(this);

    this->vt        =   VT_R4;
    this->fltVal    =   r;
}

inline variant::variant(double r)
{
    ::VariantInit(this);

    this->vt        =   VT_R8;
    this->dblVal    =   r;
}

inline variant::variant(CY cy)
{
    ::VariantInit(this);

    this->vt    =   VT_CY;
    this->cyVal =   cy;
}

inline variant::variant(DECIMAL const& dec)
{
    ::VariantInit(this);

    this->vt            =   VT_DECIMAL;
    this->decVal.scale  =   dec.scale;
    this->decVal.sign   =   dec.sign;
    this->decVal.Hi32   =   dec.Hi32;
    this->decVal.Mid32  =   dec.Mid32;
    this->decVal.Lo32   =   dec.Lo32;
}

inline variant::variant(LPUNKNOWN punk, bool_type bAddRef)
{
    ::VariantInit(this);

    this->vt        =   VT_UNKNOWN;
    this->punkVal   =   punk;

    if( bAddRef &&
        NULL != punk)
    {
        punk->AddRef();
    }
}

inline variant::variant(LPDISPATCH pdisp, bool_type bAddRef)
{
    ::VariantInit(this);

    this->vt        =   VT_DISPATCH;
    this->pdispVal  =   pdisp;

    if( bAddRef &&
        NULL != pdisp)
    {
        pdisp->AddRef();
    }
}

inline variant::variant(cs_char_a_t const* s, int len /* = -1 */)
{
    ::VariantInit(this);

    this->vt        =   VT_BSTR;
    this->bstrVal   =   (len < 0) ? bstr_create(s) : bstr_create(s, static_cast<size_type>(len));

    if(NULL == this->bstrVal)
    {
        if( NULL != s &&
            '\0' != 0[s])
        {
            handle_error_("could not initialise from string", E_OUTOFMEMORY);
        }
    }
}

inline variant::variant(cs_char_w_t const* s, int len /* = -1 */)
{
    ::VariantInit(this);

    this->vt        =   VT_BSTR;
    this->bstrVal   =   (len < 0) ? bstr_create(s) : bstr_create(s, static_cast<size_type>(len));

    if(NULL == this->bstrVal)
    {
        if( NULL != s &&
            '\0' != 0[s])
        {
            handle_error_("could not initialise from string", E_OUTOFMEMORY);
        }
    }
}

inline variant::variant(VARIANT const& var, VARTYPE vt)
{
    ::VariantInit(this);

    class_type  copy;
    HRESULT     hr = ::VariantChangeType(&copy, const_cast<VARIANT*>(&var), 0, vt);

    if(FAILED(hr))
    {
        handle_error_("could not convert variant to requested type", hr);
    }
    else
    {
        copy.swap(*this);
    }
}

inline void variant::clear()
{
    ::VariantClear(this);
}

inline HRESULT variant::try_conversion_copy(VARIANT const& var, VARTYPE vt)
{
    HRESULT hr;

    if(vt == this->vt)
    {
        hr = S_FALSE;
    }
    else
    {
        class_type  copy;

        hr  =   ::VariantChangeType(&copy, const_cast<VARIANT*>(&var), 0, vt);

        if(SUCCEEDED(hr))
        {
            copy.swap(*this);
        }
    }

    return hr;
}

inline HRESULT variant::try_convert(VARTYPE vt)
{
    return try_conversion_copy(*this, vt);
}

inline variant::class_type& variant::convert(VARTYPE vt)
{
    HRESULT hr  =   try_convert(vt);

    if(FAILED(hr))
    {
        handle_error_("could not convert variant to requested type", hr);
    }

    return *this;
}


inline HRESULT variant::QueryInterface(REFIID riid, void** ppv) const
{
    COMSTL_ASSERT(NULL != ppv);

    if( VT_UNKNOWN == this->vt ||
        VT_DISPATCH == this->vt)
    {
        return (NULL == this->punkVal) ? E_POINTER : this->punkVal->QueryInterface(riid, ppv);
    }

    return DISP_E_BADVARTYPE;
}


inline void variant::swap(variant::class_type& rhs)
{
    swap_(*this, rhs);
}

inline variant::bool_type variant::equal(variant::class_type const& rhs) const
{
    return equal(static_cast<VARIANT const&>(rhs));
}

inline variant::bool_type variant::equal(VARIANT const& rhs) const
{
    HRESULT comparisonSucceeded;
    int     areEqual = VARIANT_equal(*this, rhs, &comparisonSucceeded);

    if(FAILED(comparisonSucceeded))
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(comstl::com_exception("support for comparison of variant type not currently supported", comparisonSucceeded));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        OutputDebugStringA("support for comparison of variant type not currently supported\n");

        return false;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return 0 != areEqual;
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

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_COMSTL_VARIANT */

/* ///////////////////////////// end of file //////////////////////////// */
