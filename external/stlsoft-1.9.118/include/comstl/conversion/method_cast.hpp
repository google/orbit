/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/conversion/method_cast.hpp
 *
 * Purpose:     COM memory functions.
 *
 * Created:     20th December 2003
 * Updated:     26th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/conversion/method_cast.hpp
 *
 * \brief [C++ only] Definition of the comstl::method_cast suite of
 *   cast functions
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef COMSTL_INCL_COMSTL_CONVERSION_HPP_METHOD_CAST
#define COMSTL_INCL_COMSTL_CONVERSION_HPP_METHOD_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_CONVERSION_HPP_METHOD_CAST_MAJOR     2
# define COMSTL_VER_COMSTL_CONVERSION_HPP_METHOD_CAST_MINOR     2
# define COMSTL_VER_COMSTL_CONVERSION_HPP_METHOD_CAST_REVISION  1
# define COMSTL_VER_COMSTL_CONVERSION_HPP_METHOD_CAST_EDIT      33
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:     __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

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
 * Functions
 */

#if 0
template<   ss_typename_param_k R
        ,   ss_typename_param_k V
        >
inline R& transfer_resource(R& r, V v)
{
//  COMSTL_MESSAGE_ASSERT("Resource destination not empty!", 0 == r);
    r = v;

    return r;
}
#endif /* 0 */

inline ss_sint8_t& transfer_resource(ss_sint8_t& r, ss_sint8_t v)               { return ((r = v), r); }
inline ss_uint8_t& transfer_resource(ss_uint8_t& r, ss_uint8_t v)               { return ((r = v), r); }
inline ss_sint16_t& transfer_resource(ss_sint16_t& r, ss_sint16_t v)            { return ((r = v), r); }
inline ss_uint16_t& transfer_resource(ss_uint16_t& r, ss_uint16_t v)            { return ((r = v), r); }
inline ss_sint32_t& transfer_resource(ss_sint32_t& r, ss_sint32_t v)            { return ((r = v), r); }
inline ss_uint32_t& transfer_resource(ss_uint32_t& r, ss_uint32_t v)            { return ((r = v), r); }
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
inline ss_sint64_t& transfer_resource(ss_sint64_t& r, ss_sint64_t v)            { return ((r = v), r); }
inline ss_uint64_t& transfer_resource(ss_uint64_t& r, ss_uint64_t v)            { return ((r = v), r); }
#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */

#ifdef STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
inline signed char& transfer_resource(signed char& r, signed char v)            { return ((r = v), r); }
inline unsigned char& transfer_resource(unsigned char& r, unsigned char v)      { return ((r = v), r); }
#endif /* STLSOFT_CF_CHAR_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
inline signed short& transfer_resource(signed short& r, signed short v)         { return ((r = v), r); }
inline unsigned short& transfer_resource(unsigned short& r, unsigned short v)   { return ((r = v), r); }
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
inline signed int& transfer_resource(signed int& r, signed int v)               { return ((r = v), r); }
inline unsigned int& transfer_resource(unsigned int& r, unsigned int v)         { return ((r = v), r); }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
inline signed long& transfer_resource(signed long& r, signed long v)            { return ((r = v), r); }
inline unsigned long& transfer_resource(unsigned long& r, unsigned long v)      { return ((r = v), r); }
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

inline float& transfer_resource(float& r, float v)                              { return ((r = v), r); }
inline double& transfer_resource(double& r, double v)                           { return ((r = v), r); }
inline long double& transfer_resource(long double& r, long double v)            { return ((r = v), r); }


inline bool& transfer_resource(bool& r, VARIANT_BOOL v)                         { return ((r = VARIANT_FALSE != v), r); }

template<   ss_typename_param_k R
        ,   ss_typename_param_k V
        >
inline R*& transfer_resource(R*& r, V* v)
{
    COMSTL_MESSAGE_ASSERT("Resource destination not empty!", 0 == r);

    return (r = v);
}


/** \brief
 *
 * \ingroup group__library__conversion
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k V
        >
inline R method_cast(C& c, HRESULT (C::*pfn)(V*))
{
    R       r   =   R();
    V       v;
    HRESULT hr  =   (c.*pfn)(&v);

    if(SUCCEEDED(hr))
    {
        return transfer_resource(r, v);
    }

    return r;
}

/** \brief
 *
 * \ingroup group__library__conversion
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k V
        >
inline R method_cast(C *c, HRESULT (STDAPICALLTYPE C::*pfn)(V*))
{
    R       r   =   R();
    V       v;
    HRESULT hr  =   (c->*pfn)(&v);

    if(SUCCEEDED(hr))
    {
        return transfer_resource(r, v);
    }

    return r;
}

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

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_CONVERSION_HPP_METHOD_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
