/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/truncation_test.hpp
 *
 * Purpose:     Runtime checking for numeric conversions.
 *
 * Created:     10th August 2006
 * Updated:     24th November 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2011, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/truncation_test.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::truncation_test functions
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST_MAJOR      1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST_MINOR      0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST_REVISION   6
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST_EDIT       48
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS
# include <stlsoft/util/limit_traits.h>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_H_LIMIT_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS
# include <stlsoft/util/sign_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SIGNED_TYPE
# include <stlsoft/meta/is_signed_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SIGNED_TYPE */

#ifdef STLSOFT_UNITTEST
# include <limits.h>
#endif /* STLSOFT_UNITTEST */
#if defined(STLSOFT_UNITTEST) || \
    defined(_DEBUG)
# include <typeinfo>
# if defined(STLSOFT_COMPILER_IS_MSVC)
#  include <crtdbg.h>
# endif /* VC++ */
#endif /* STLSOFT_UNITTEST || _DEBUG */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 *
 *
 * Assume 11 types:
 *     char, signed char, unsigned char
 *     short, unsigned short
 *     int, unsigned int
 *     long, unsigned long
 *     long long, unsigned long long
 *
 * That gives 121 permutations:
 *     (a) 11 where the type is the same
 *     (b) 20 where the sign is the same and sizeof(FROM) <= sizeof(TO); superset of (a)
 *     (c) 10 where FROM is unsigned and sizeof(FROM) < sizeof(TO)
 *     (d) 80 that must be determined dynamically
 *
 *
 * The strategy is as follows:
 *

if( signof(FROM) == signof(TO) &&       // Compile-time test 1
    sizeof(FROM) <= sizeof(TO))
{
    return true;
}
else if(unsigned == signof(FROM) &&     // Compile-time test 2
        sizeof(FROM) < sizeof(TO))
{
    return true;
}
else
{
    return runtime_test<TO>(from);
}

 *
 * The runtime test evaluates to the following:
 *
 *  If the FROM type is signed, then

 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

    // The permutations are:
    //
    // 1a FROM signed | TO signed | sizeof(FROM) < sizeof(TO)       =>  Always yes
    // 1b FROM signed | TO signed | sizeof(FROM) = sizeof(TO)       =>  Always yes
    // 1c FROM signed | TO signed | sizeof(FROM) > sizeof(TO)       =>  Runtime test
    //
    // 2a FROM unsigned | TO signed | sizeof(FROM) < sizeof(TO)     =>  Always yes
    // 2b FROM unsigned | TO signed | sizeof(FROM) = sizeof(TO)     =>  Runtime test
    // 2c FROM unsigned | TO signed | sizeof(FROM) > sizeof(TO)     =>  Runtime test
    //
    // 3a FROM signed | TO unsigned | sizeof(FROM) < sizeof(TO)     =>  Runtime test
    // 3b FROM signed | TO unsigned | sizeof(FROM) = sizeof(TO)     =>  Runtime test
    // 3c FROM signed | TO unsigned | sizeof(FROM) > sizeof(TO)     =>  Runtime test
    //
    // 4a FROM unsigned | TO unsigned | sizeof(FROM) < sizeof(TO)   =>  Always yes
    // 4b FROM unsigned | TO unsigned | sizeof(FROM) = sizeof(TO)   =>  Always yes
    // 4c FROM unsigned | TO unsigned | sizeof(FROM) > sizeof(TO)   =>  Runtime test



template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_helper_runtime_test_different_sign_FROM_is_signed(FROM from, yes_type, TO)
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    enum {  TO_is_signed            =   is_signed_type<TO>::value                   };
    enum {  FROM_is_signed          =   is_signed_type<FROM>::value                 };

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    STLSOFT_SUPPRESS_UNUSED(sizeofFROM);
    STLSOFT_SUPPRESS_UNUSED(sizeofTO);

    STLSOFT_STATIC_ASSERT((0 == int(TO_is_signed)) != (0 == int(FROM_is_signed)));
    STLSOFT_STATIC_ASSERT(0 != int(FROM_is_signed));
    STLSOFT_STATIC_ASSERT(0 == int(TO_is_signed));

    // FROM is signed
    // TO is unsigned
    //
    // Truncation occurs if:
    //
    // - from < 0
    // - from > toMax
    // - from

    if(from < 0)
    {
        return false;
    }
    else if(sizeofFROM < sizeofTO)               // 3a
    {
        return true;
    }
    else
    {
        if(sizeofFROM == sizeofTO)
        {
            TO  toMax   =   limit_traits<TO>::maximum();

            if(toMax < static_cast<TO>(from))
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        else                                        // 3b & 3c
        {
            FROM    toMax   =   static_cast<FROM>(limit_traits<TO>::maximum());

            if(from > toMax)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
}

template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_helper_runtime_test_different_sign_FROM_is_signed(FROM from, no_type, TO)
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    enum {  TO_is_signed            =   is_signed_type<TO>::value                   };
    enum {  FROM_is_signed          =   is_signed_type<FROM>::value                 };

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    STLSOFT_SUPPRESS_UNUSED(sizeofFROM);
    STLSOFT_SUPPRESS_UNUSED(sizeofTO);

    STLSOFT_STATIC_ASSERT((0 == int(TO_is_signed)) != (0 == int(FROM_is_signed)));
    STLSOFT_STATIC_ASSERT(0 == int(FROM_is_signed));
    STLSOFT_STATIC_ASSERT(0 != int(TO_is_signed));

    // FROM is unsigned
    // TO is signed
    //
    // Truncation occurs if from > toMax

    FROM    toMax   =   static_cast<FROM>(limit_traits<TO>::maximum());

    if(from > toMax)
    {
        return false;
    }
    else
    {
        return true;
    }
}


template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_helper_runtime_test_same_sign(FROM from, yes_type, TO) // The use of the dummy variable is to fix a bug with VC++ 5-7.0
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    STLSOFT_STATIC_ASSERT(sizeofTO < sizeofFROM);

    // This is a fully runtime test: does FROM fit into TO's limits?
    //
    // To do this we elicit TO's min and max. The values are held in
    // FROM, which involves no truncation because sizeof(FROM) > sizeof(TO)

    FROM    toMax   =   static_cast<FROM>(limit_traits<TO>::maximum());
    FROM    toMin   =   static_cast<FROM>(limit_traits<TO>::minimum());

    if( from < toMin ||
        from > toMax)
    {
        return false;
    }
    else
    {
        return true;
    }
}

template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_helper_runtime_test_same_sign(FROM from, no_type, TO)
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    enum {  TO_is_signed            =   is_signed_type<TO>::value                   };
    enum {  FROM_is_signed          =   is_signed_type<FROM>::value                 };

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    STLSOFT_SUPPRESS_UNUSED(sizeofFROM);
    STLSOFT_SUPPRESS_UNUSED(sizeofTO);

    STLSOFT_STATIC_ASSERT((0 == int(TO_is_signed)) != (0 == int(FROM_is_signed)));

    typedef ss_typename_param_k value_to_yesno_type<FROM_is_signed>::type  same_sign_yesno_t;

    return truncation_test_helper_runtime_test_different_sign_FROM_is_signed<TO>(from, same_sign_yesno_t(), TO());
}



template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_helper_runtime_test(FROM from, no_type, TO ) // The use of the dummy variable is to fix a bug with VC++ 5-7.0
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    // Types are different

    // Next test for same sign
    enum {  TO_is_signed            =   is_signed_type<TO>::value                   };
    enum {  FROM_is_signed          =   is_signed_type<FROM>::value                 };

    enum {  types_have_same_sign    =   int(TO_is_signed) == int(FROM_is_signed)    };

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    STLSOFT_STATIC_ASSERT(sizeofFROM >= sizeofTO || FROM_is_signed);

    typedef ss_typename_param_k value_to_yesno_type<types_have_same_sign>::type  same_sign_yesno_t;

    return truncation_test_helper_runtime_test_same_sign<TO>(from, same_sign_yesno_t(), TO());
}

template <typename T>
inline bool truncation_test_helper_runtime_test(T, yes_type, ...)
{
    return true;
}


template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
inline bool truncation_test_(FROM from, TO dummy = TO())    // The use of the dummy variable is to fix a bug with VC++ 5-7.0
{
#ifdef _DEBUG
# if defined(STLSOFT_COMPILER_IS_MSVC)
	int const flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags & ~(_CRTDBG_ALLOC_MEM_DF));
# endif /* VC++ */

    char const* TO_     =   typeid(TO).name();
    char const* FROM_   =   typeid(FROM).name();

    STLSOFT_SUPPRESS_UNUSED(TO_);
    STLSOFT_SUPPRESS_UNUSED(FROM_);
# if defined(STLSOFT_COMPILER_IS_MSVC)
	_CrtSetDbgFlag(flags);
# endif /* VC++ */
#endif /* _DEBUG */

    // First, we must check that the types are compatible, with constraints

    // Both types must be integral
    STLSOFT_STATIC_ASSERT(0 != is_integral_type<TO>::value);
    STLSOFT_STATIC_ASSERT(0 != is_integral_type<FROM>::value);

    // Now calculate the sizes

    const ss_size_t sizeofFROM  =   sizeof(FROM);
    const ss_size_t sizeofTO    =   sizeof(TO);

    // Now determine the signs

    enum {  TO_is_signed        =   is_signed_type<TO>::value           };
    enum {  FROM_is_signed      =   is_signed_type<FROM>::value         };

    // We know at compile time that FROM fits into TO if:
    //
    // - they have the same sign, and sizeof(FROM) <= sizeof(TO), OR
    // - FROM is unsigned (and TO is signed), and sizeof(FROM) < sizeof(TO)
    //
    // If either of these hold, then the answer is true: the yes_type overload
    //  of truncation_test_helper_runtime_test() is selected.
    //
    // If not, then a runtime test is required: the no_type overload
    //  of truncation_test_helper_runtime_test() is selected.

    enum { types_are_statically_compatible  =
                                                (   int(TO_is_signed) == int(FROM_is_signed) &&
                                                    sizeofFROM <= sizeofTO)
                                            ||
                                                (   !FROM_is_signed &&
                                                    sizeofFROM < sizeofTO)  };

    typedef ss_typename_param_k value_to_yesno_type<types_are_statically_compatible>::type  yesno_t;

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     defined(_Wp64) && \
     !defined(_WIN64)
#  pragma warning(push)
#  pragma warning(disable : 4267)
# endif /* VC++ + Win32 + _Wp32 */

    return truncation_test_helper_runtime_test<TO>(from, yesno_t(), dummy);

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     defined(_Wp64) && \
     !defined(_WIN64)
#  pragma warning(pop)
# endif /* VC++ + Win32 + _Wp32 */
}


#if 0
template<ss_typename_param_k TO>
class truncation_test
{
public:
    template <ss_typename_param_k FROM>
    truncation_test(FROM from)
        : m_b(truncation_test_(from, get_to_()))
    {}

public:
    operator bool () const
    {
        return m_b;
    }

private:
    static TO get_to_()
    {
        return TO();
    }

private:
    const bool m_b;
};
#else /* ? 0 */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** Indicates whether a given value can be cast to a given type without
 * truncation
 *
 * \ingroup group__library__conversion
 *
 * Example:
<pre>
  truncation_cast&lt;unsigned>(-1); // Will return false, since negatives cannot fit in unsigned
  truncation_cast&lt;short>(30000); // Will return true, since 30000 will fit inside short (assuming short has >= 16-bits)
</pre>
 *
 * \param from The value to be tested
 *
 * \retval false The value will experience truncation
 * \retval true The value will not be truncated
 */
template<   ss_typename_param_k TO
        ,   ss_typename_param_k FROM
        >
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
inline bool truncation_test(FROM from, TO dummy = TO())    // The use of the dummy variable is to fix a bug with VC++ 5-7.0
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
inline bool truncation_test(FROM from)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
{
    return truncation_test_(from, dummy);
}

#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/truncation_test_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_TRUNCATION_TEST */

/* ///////////////////////////// end of file //////////////////////////// */
