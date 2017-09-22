/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/integral_printf_traits.hpp (formerly stlsoft/util/printf_traits.hpp)
 *
 * Purpose:     integral_printf_traits classes.
 *
 * Created:     16th January 2002
 * Updated:     10th July 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/util/integral_printf_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::integral_printf_traits class
 *   template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS_MAJOR      5
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS_MINOR      2
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS_REVISION   3
# define STLSOFT_VER_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS_EDIT       70
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifdef STLSOFT_UNITTEST
# include <stlsoft/util/limit_traits.h>
# include <stlsoft/meta/yesno.hpp>
# include <stlsoft/util/std/stdio_overload_detectors.hpp>
# include <stdarg.h>
# include <stdio.h>
# include <string.h>
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
 * Constants & definitions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MIN       -128
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MAX       127

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MIN       0
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MAX       255

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN      -32768
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX      32767

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN      0
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX      65535

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MIN      -2147483648
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MAX      2147483647

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MIN      0
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MAX      4294967295

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN      -9223372036854775808
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX      9223372036854775807

# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN      0
# define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX      18446744073709551615



 /* char */
# if _STLSOFT_SIZEOF_CHAR == 1

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT8_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT8_MAX

#  ifdef STLSOFT_CF_CHAR_IS_UNSIGNED
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MIN
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MAX
#  else /* ? STLSOFT_CF_CHAR_IS_UNSIGNED */
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MIN
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MAX
#  endif /* STLSOFT_CF_CHAR_IS_UNSIGNED */

# else /* ? _STLSOFT_SIZEOF_CHAR */
#  error Cannot operate with a compiler for which sizeof(char) != 1
# endif /* _STLSOFT_SIZEOF_CHAR */


 /* short */
# if _STLSOFT_SIZEOF_SHORT == 2

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MIN         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MAX         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

# elif _STLSOFT_SIZEOF_SHORT == 4

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MIN         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MAX         STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

# else /* ? _STLSOFT_SIZEOF_SHORT */
#  error Cannot operate with a compiler for which sizeof(short) != 2 && sizeof(short) != 4
# endif /* _STLSOFT_SIZEOF_SHORT */


 /* int */
# if _STLSOFT_SIZEOF_INT == 2

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MIN            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MAX            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

# elif _STLSOFT_SIZEOF_INT == 4

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MIN            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MAX            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MAX

# elif _STLSOFT_SIZEOF_INT == 8

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MIN            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MAX            STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX

# else /* ? _STLSOFT_SIZEOF_INT */
#  error Cannot operate with a compiler for which sizeof(int) != 2 && sizeof(int) != 4 && sizeof(int) != 8
# endif /* _STLSOFT_SIZEOF_INT */


 /* long */
# if _STLSOFT_SIZEOF_LONG == 2

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT16_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT16_MAX

# elif _STLSOFT_SIZEOF_LONG == 4

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT32_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT32_MAX

# elif _STLSOFT_SIZEOF_LONG == 8

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MIN           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MAX           STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX

#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MIN          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN
#  define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MAX          STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX

# else /* ? _STLSOFT_SIZEOF_LONG */
#  error Cannot operate with a compiler for which sizeof(long) != 2 && sizeof(long) != 4 && sizeof(long) != 8
# endif /* _STLSOFT_SIZEOF_LONG */


 /* long long */
# ifdef STLSOFT_CF_BUILTIN_long_long_SUPPORT
#  if _STLSOFT_SIZEOF_LONG_LONG == 8

#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_LONG_MIN     STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_LONG_MAX     STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX

#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_LONG_MIN    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN
#   define STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_LONG_MAX    STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX

#  else /* ? _STLSOFT_SIZEOF_LONG_LONG */
#   error Cannot operate with a compiler for which sizeof(long long) != 8
#  endif /* _STLSOFT_SIZEOF_LONG_LONG */
# endif /* STLSOFT_CF_BUILTIN_long_long_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifdef STLSOFT_CF_64_BIT_PRINTF_USES_I64
# undef STLSOFT_CF_64_BIT_PRINTF_USES_I64
#endif /* STLSOFT_CF_64_BIT_PRINTF_USES_I64 */

#ifdef STLSOFT_CF_64_BIT_PRINTF_USES_LL
# undef STLSOFT_CF_64_BIT_PRINTF_USES_LL
#endif /* STLSOFT_CF_64_BIT_PRINTF_USES_LL */

# if defined(STLSOFT_COMPILER_IS_BORLAND) || \
     (  defined(STLSOFT_COMPILER_IS_COMO) && \
        defined(_MSC_VER)) || \
     (  defined(WIN32) && \
        (   defined(STLSOFT_COMPILER_IS_GCC)) || \
            defined(STLSOFT_COMPILER_IS_INTEL)) || \
     (  defined(STLSOFT_COMPILER_IS_MSVC) /* && \
        _MSC_VER < 1400 */) || \
     defined(STLSOFT_COMPILER_IS_VECTORC) || \
     defined(STLSOFT_COMPILER_IS_WATCOM)
#  define STLSOFT_CF_64_BIT_PRINTF_USES_I64
# elif defined(STLSOFT_COMPILER_IS_COMO) || \
       defined(STLSOFT_COMPILER_IS_DMC) || \
       defined(STLSOFT_COMPILER_IS_GCC) || \
       defined(STLSOFT_COMPILER_IS_INTEL) || \
       (    defined(STLSOFT_COMPILER_IS_MSVC) && \
            _MSC_VER >= 1400) || \
       defined(STLSOFT_COMPILER_IS_MWERKS) || \
       defined(STLSOFT_COMPILER_IS_SUNPRO)
#  define STLSOFT_CF_64_BIT_PRINTF_USES_LL
# else /* ? compiler */
#  error Further compiler discrimination is required
# endif /* printf-64 */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// struct integral_printf_traits

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \brief Traits for determining the size, in printf-ed form, of the minimum and
 * maximum values of types
 *
 * \ingroup group__library__utility
 *
 * integral_printf_traits is a traits class for acquiring enum values representing the
 * lengths, when expressed in string form, of the minimum and maximum values of
 * the type, and the maximum of the two. The lengths are inclusive of the
 * null terminator.
 *
\code
  assert(stlsoft::integral_printf_traits<ss_sint16_t>::size_min == 6);
  assert(stlsoft::integral_printf_traits<ss_sint16_t>::size_max == 7);
  assert(stlsoft::integral_printf_traits<ss_sint16_t>::size == 7);
\endcode
 *
 * \param T The type
 *
 */
template <ss_typename_param_k T>
struct integral_printf_traits
{
    enum
    {
            /// [DEPRECATED] The number of decimal characters (+ null) in
            /// the minimum decimal value
            ///
            /// \deprecated This member constant is deprecated, and will be
            ///   removed in a future version of STLSoft
            size_min
            /// [DEPRECATED] The number of decimal characters (+ null) in
            /// the maximum decimal value
            ///
            /// \deprecated This member constant is deprecated, and will be
            ///   removed in a future version of STLSoft
        ,   size_max
            /// [DEPRECATED] The maximum of \c size_min and \c size_max
            ///
            /// \deprecated This member constant is deprecated, and will be
            ///   removed in a future version of STLSoft
        ,   size
    };

    /// Returns the appropriate decimal integral printf format for the type
    static ss_char_a_t const* decimal_format_a();
    /// Returns the appropriate decimal integral wprintf format for the type
    static ss_char_w_t const* decimal_format_w();

    /// Returns the appropriate hexadecimal integral printf format for the type
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase);
    /// Returns the appropriate hexadecimal integral wprintf format for the type
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase);

    /// Returns the appropriate octal integral printf format for the type
    static ss_char_a_t const* octal_format_a();
    /// Returns the appropriate octal integral wprintf format for the type
    static ss_char_w_t const* octal_format_w();


    /// Returns the appropriate integral printf format for the type
    ///
    /// \deprecated This method is deprecated, and may be removed in a
    ///   future version; use decimal_format_a().
    static ss_char_a_t const* format_a();
    /// Returns the appropriate integral wprintf format for the type
    ///
    /// \deprecated This method is deprecated, and may be removed in a
    ///   future version; use decimal_format_w().
    static ss_char_w_t const* format_w();

    /// Returns the appropriate hexadecimal printf format for the type
    ///
    /// \deprecated This method is deprecated, and may be removed in a
    ///   future version; use hexadecimal_format_a(false).
    static ss_char_a_t const* hex_format_a();
    /// Returns the appropriate hexadecimal wprintf format for the type
    ///
    /// \deprecated This method is deprecated, and may be removed in a
    ///   future version; use hexadecimal_format_w(false).
    static ss_char_w_t const* hex_format_w();
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k T>
struct integral_printf_traits;

STLSOFT_OPEN_WORKER_NS_(ximpl_integral_printf_traits)

template <ss_typename_param_k T>
struct integral_printf_traits_base;


 /* char */

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<void>
{
	static ss_char_a_t const* get_hexadecimal_format_pcx_a_(int upperCase)
	{
		static ss_char_a_t const* const s_strings[] =
		{
				"%x"
			,	"%X"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_w_t const* get_hexadecimal_format_pcx_w_(int upperCase)
	{
		static ss_char_w_t const* const s_strings[] =
		{
				L"%x"
			,	L"%X"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_a_t const* get_hexadecimal_format_pclx_a_(int upperCase)
	{
		static ss_char_a_t const* const s_strings[] =
		{
				"%lx"
			,	"%lX"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_w_t const* get_hexadecimal_format_pclx_w_(int upperCase)
	{
		static ss_char_w_t const* const s_strings[] =
		{
				L"%lx"
			,	L"%lX"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_a_t const* get_hexadecimal_format_pci64x_a_(int upperCase)
	{
		static ss_char_a_t const* const s_strings[] =
		{
				"%I64x"
			,	"%I64X"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_w_t const* get_hexadecimal_format_pci64x_w_(int upperCase)
	{
		static ss_char_w_t const* const s_strings[] =
		{
				L"%I64x"
			,	L"%I64X"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_a_t const* get_hexadecimal_format_pcllx_a_(int upperCase)
	{
		static ss_char_a_t const* const s_strings[] =
		{
				"%llx"
			,	"%llX"
		};

		return s_strings[0 != upperCase];
	}

	static ss_char_w_t const* get_hexadecimal_format_pcllx_w_(int upperCase)
	{
		static ss_char_w_t const* const s_strings[] =
		{
				L"%llx"
			,	L"%llX"
		};

		return s_strings[0 != upperCase];
	}

};


STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<char>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_CHAR_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
#  ifdef STLSOFT_CF_CHAR_IS_UNSIGNED
        return  "%u";
#  else /* ? STLSOFT_CF_CHAR_IS_UNSIGNED */
        return  "%d";
#  endif /* STLSOFT_CF_CHAR_IS_UNSIGNED */
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
#  ifdef STLSOFT_CF_CHAR_IS_UNSIGNED
        return L"%u";
#  else /* ? STLSOFT_CF_CHAR_IS_UNSIGNED */
        return L"%d";
#  endif /* STLSOFT_CF_CHAR_IS_UNSIGNED */
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<unsigned char>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UCHAR_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%u";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%u";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<signed char>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SCHAR_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%d";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%d";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};


 /* short */

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<short>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SHORT_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%d";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%d";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<unsigned short>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_USHORT_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%u";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%u";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};


 /* int */

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<int>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_INT_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%d";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%d";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<unsigned int>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%u";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%u";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%o";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%o";
    }
};


 /* long */

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<long>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_LONG_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%ld";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%ld";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pclx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pclx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%lo";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%lo";
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<unsigned long>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_ULONG_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
        return  "%lu";
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
        return L"%lu";
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pclx_a_(upperCase);
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
        return integral_printf_traits_base<void>::get_hexadecimal_format_pclx_w_(upperCase);
    }

    static ss_char_a_t const* octal_format_a()
    {
        return "%lo";
    }
    static ss_char_w_t const* octal_format_w()
    {
        return L"%lo";
    }
};


 /* long long */

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<ss_sint64_t>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_SINT64_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return  "%I64d";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return  "%lld";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return L"%I64d";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return L"%lld";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pci64x_a_(upperCase);
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcllx_a_(upperCase);
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pci64x_w_(upperCase);
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcllx_w_(upperCase);
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }

    static ss_char_a_t const* octal_format_a()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return "%I64o";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return "%llo";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    static ss_char_w_t const* octal_format_w()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return L"%I64o";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return L"%llo";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits_base<ss_uint64_t>
{
    enum
    {
            size_min = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MIN)))
        ,   size_max = int(sizeof(STLSOFT_STRINGIZE(STLSOFT_INTEGRAL_PRINTF_TRAITS_UINT64_MAX)))
        ,   size = (size_min < size_max) ? size_max : size_min
    };

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_a, decimal_format_a)
    static ss_char_a_t const* format_a()
    {
        return decimal_format_a();
    }
    static ss_char_a_t const* decimal_format_a()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return  "%I64u";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return  "%llu";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(format_w, decimal_format_w)
    static ss_char_w_t const* format_w()
    {
        return decimal_format_w();
    }
    static ss_char_w_t const* decimal_format_w()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return L"%I64u";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return L"%llu";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }

    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_a, hexadecimal_format_a)
    static ss_char_a_t const* hex_format_a()
    {
        return hexadecimal_format_a(false);
    }
    static ss_char_a_t const* hexadecimal_format_a(bool upperCase)
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pci64x_a_(upperCase);
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcllx_a_(upperCase);
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(hex_format_w, hexadecimal_format_w)
    static ss_char_w_t const* hex_format_w()
    {
        return hexadecimal_format_w(false);
    }
    static ss_char_w_t const* hexadecimal_format_w(bool upperCase)
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pci64x_w_(upperCase);
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return integral_printf_traits_base<void>::get_hexadecimal_format_pcllx_w_(upperCase);
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }

    static ss_char_a_t const* octal_format_a()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return "%I64o";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return "%llo";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
    static ss_char_w_t const* octal_format_w()
    {
#if defined(STLSOFT_CF_64_BIT_PRINTF_USES_I64)
        return L"%I64o";
#elif defined(STLSOFT_CF_64_BIT_PRINTF_USES_LL)
        return L"%llo";
#else
# error Further compiler discrimination is required
#endif /* printf-64 */
    }
};


#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

STLSOFT_CLOSE_WORKER_NS_(ximpl_integral_printf_traits)


STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<char>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<char>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<signed char>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<signed char>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<unsigned char>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned char>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<short>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<short>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<unsigned short>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned short>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<int>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<int>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<unsigned int>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned int>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<long>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<long>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<unsigned long>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned long>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_sint64_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<ss_sint64_t>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_uint64_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<ss_uint64_t>
{};


#ifdef STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
# if _STLSOFT_SIZEOF_CHAR == 1
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_sint8_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<signed char>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_uint8_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned char>
{};
# endif /* _STLSOFT_SIZEOF_CHAR == 1 */
#endif /* STLSOFT_CF_CHAR_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
# if _STLSOFT_SIZEOF_SHORT == 2
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_sint16_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<short>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_uint16_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned short>
{};
# endif /* _STLSOFT_SIZEOF_SHORT == 2 */
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
# if _STLSOFT_SIZEOF_INT == 4
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_sint32_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<int>
{};
STLSOFT_TEMPLATE_SPECIALISATION
struct integral_printf_traits<ss_uint32_t>
    : public STLSOFT_WORKER_NS_QUAL_(ximpl_integral_printf_traits, integral_printf_traits_base)<unsigned int>
{};
# endif /* _STLSOFT_SIZEOF_INT == 4 */
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/integral_printf_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_INTEGRAL_PRINTF_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
