/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/integral_range.hpp
 *
 * Purpose:     Integral range class.
 *
 * Created:     4th November 2003
 * Updated:     5th March 2011
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


/** \file rangelib/integral_range.hpp Integral range class */

#ifndef RANGELIB_INCL_RANGELIB_HPP_INTEGRAL_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_INTEGRAL_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_INTEGRAL_RANGE_MAJOR    2
# define RANGELIB_VER_RANGELIB_HPP_INTEGRAL_RANGE_MINOR    6
# define RANGELIB_VER_RANGELIB_HPP_INTEGRAL_RANGE_REVISION 5
# define RANGELIB_VER_RANGELIB_HPP_INTEGRAL_RANGE_EDIT     56
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC:     _MSC_VER < 1200
STLSOFT_COMPILER_IS_MWERKS:   (__MWERKS__ & 0xFF00) < 0x3000
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGELIB
# include <rangelib/rangelib.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGELIB */
#ifndef RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES
# include <rangelib/range_categories.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_RANGE_CATEGORIES */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL
# include <stlsoft/util/operator_bool.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPERATOR_BOOL */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS
# include <stlsoft/error/exceptions.hpp>      // for null_exception_policy
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE
# include <stlsoft/meta/is_character_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CHARACTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE
# include <stlsoft/meta/is_numeric_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_PRINTF_TRAITS
# include <stlsoft/util/printf_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_PRINTF_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::out_of_range
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifndef STLSOFT_INCL_H_STDIO
# define STLSOFT_INCL_H_STDIO
# include <stdio.h>                     // for sprintf
#endif /* !STLSOFT_INCL_H_STDIO */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::rangelib */
namespace rangelib
{
# else
/* Define stlsoft::rangelib_project */

namespace stlsoft
{

namespace rangelib_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Error policy class for integral_range
 *
 * \ingroup group__library__rangelib
 */
struct invalid_integral_range_policy
{
public:
    /// The thrown type
    typedef std::out_of_range   thrown_type;

public:
    /// Function call operator, taking three 32-bit signed integer parameters
    void operator ()(ss_sint32_t first, ss_sint32_t last, ss_sint32_t increment) const
    {
        static const char   s_format[]  =   "Invalid integral range [%ld, %ld), %ld";
        char                message[3 * 21 + STLSOFT_NUM_ELEMENTS(s_format)];
        const ss_size_t     cch         =   static_cast<ss_size_t>(::sprintf(message, s_format, long(first), long(last), long(increment)));

        STLSOFT_ASSERT(cch < STLSOFT_NUM_ELEMENTS(message));
        STLSOFT_SUPPRESS_UNUSED(cch);

        STLSOFT_THROW_X(thrown_type(message));
    }
    /// Function call operator, taking three 32-bit unsigned integer parameters
    void operator ()(ss_uint32_t first, ss_uint32_t last, ss_uint32_t increment) const
    {
        static const char   s_format[] = "Invalid integral range [%lu, %lu), %lu";
        char                message[3 * 21 + STLSOFT_NUM_ELEMENTS(s_format)];
        const ss_size_t     cch         =   static_cast<ss_size_t>(::sprintf(message, "Invalid integral range [%lu, %lu), %lu", ss_ulong_t(first), ss_ulong_t(last), ss_ulong_t(increment)));

        STLSOFT_ASSERT(cch < STLSOFT_NUM_ELEMENTS(message));
        STLSOFT_SUPPRESS_UNUSED(cch);
#if defined(STLSOFT_COMPILER_IS_COMO)
        STLSOFT_SUPPRESS_UNUSED(s_format);
#endif /* compiler */

        STLSOFT_THROW_X(thrown_type(message));
    }
#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
    /// Function call operator, taking three 32-bit signed integer parameters
    void operator ()(int first, int last, int increment) const
    {
        operator ()(ss_sint32_t(first), ss_sint32_t(last), ss_sint32_t(increment));
    }
    /// Function call operator, taking three 32-bit unsigned integer parameters
    void operator ()(unsigned int first, unsigned int last, unsigned int increment) const
    {
        operator ()(ss_uint32_t(first), ss_uint32_t(last), ss_uint32_t(increment));
    }
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
private:
    static char const* format_sint64()
    {
        return printf_traits<ss_sint64_t>::format_a();
    }
    static char const* format_uint64()
    {
        return printf_traits<ss_uint64_t>::format_a();
    }

public:
    /// Function call operator, taking three 64-bit signed integer parameters
    void operator ()(ss_sint64_t first, ss_sint64_t last, ss_sint64_t increment) const
    {
        static const char   s_fmtfmt[]  =   "Invalid integral range [%s, %s), %s";
        char                format[3 * 4 + STLSOFT_NUM_ELEMENTS(s_fmtfmt)];
        char                message[3 * 21 + STLSOFT_NUM_ELEMENTS(s_fmtfmt)];
        const ss_size_t     cch1        =   static_cast<ss_size_t>(::sprintf(format, s_fmtfmt, format_sint64(), format_sint64(), format_sint64()));
        const ss_size_t     cch2        =   static_cast<ss_size_t>(::sprintf(message, format, first, last, increment));

        STLSOFT_ASSERT(cch1 < STLSOFT_NUM_ELEMENTS(format));
        STLSOFT_SUPPRESS_UNUSED(cch1);
        STLSOFT_ASSERT(cch2 < STLSOFT_NUM_ELEMENTS(message));
        STLSOFT_SUPPRESS_UNUSED(cch2);

        STLSOFT_THROW_X(thrown_type(message));
    }
    /// Function call operator, taking three 64-bit unsigned integer parameters
    void operator ()(ss_uint64_t first, ss_uint64_t last, ss_uint64_t increment) const
    {
        static const char   s_fmtfmt[] = "Invalid integral range [%s, %s), %s";
        char                format[3 * 4 + STLSOFT_NUM_ELEMENTS(s_fmtfmt)];
        char                message[3 * 21 + STLSOFT_NUM_ELEMENTS(s_fmtfmt)];
        const ss_size_t     cch1        =   static_cast<ss_size_t>(::sprintf(format, s_fmtfmt, format_uint64(), format_uint64(), format_uint64()));
        const ss_size_t     cch2        =   static_cast<ss_size_t>(::sprintf(message, format, first, last, increment));

        STLSOFT_ASSERT(cch1 < STLSOFT_NUM_ELEMENTS(format));
        STLSOFT_SUPPRESS_UNUSED(cch1);
        STLSOFT_ASSERT(cch2 < STLSOFT_NUM_ELEMENTS(message));
        STLSOFT_SUPPRESS_UNUSED(cch2);

        STLSOFT_THROW_X(thrown_type(message));
    }
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
};

/** \brief This range class represents an integral range.
 *
 * \ingroup group__library__rangelib
 *
 * It is categoried as a Notional Range
 *
 * It could be used as follows
\code
  // Create a range of integer values, in the range [-100, 200), in increments of 5
  stlsoft::integral_range<int>   r(-100, +100, 5);

  // Calculate the total
  int total = stlsoft::r_accumulate(r, 0);
\endcode
 */
template<   ss_typename_param_k T
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   ss_typename_param_k XP = invalid_integral_range_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   ss_typename_param_k XP = null_exception_policy
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        >
class integral_range
    : public notional_range_tag
{
/// \name Types
/// @{
public:
    typedef T                       value_type;
    typedef T const&                const_reference;
    typedef XP                      exception_policy_type;
    typedef notional_range_tag      range_tag_type;
    typedef integral_range<T, XP>   class_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs from a start and end position, and an increment
    integral_range(value_type first, value_type last, value_type increment = +1)
        : m_position(first)
        , m_last(last)
        , m_increment(increment)
    {
        if(m_last < m_position)
        {
//          std_swap(m_position, m_last);

            if(m_increment > 0)
            {
                m_increment = -m_increment;
            }
        }

        validate_range(m_position, m_last, m_increment);
    }
    /// Destructor
    ~integral_range() stlsoft_throw_0()
    {
        // These constraints are to ensure that this template is not used
        // for any other types, especially floating point types!!
        STLSOFT_STATIC_ASSERT(0 != is_integral_type<value_type>::value);
        STLSOFT_STATIC_ASSERT(0 != is_numeric_type<value_type>::value || 0 != is_character_type<value_type>::value);
    }
/// @}

/// \name Notional Range methods
/// @{
private:
    STLSOFT_DEFINE_OPERATOR_BOOL_TYPES_T(class_type, operator_bool_generator_type, operator_bool_type);
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        return m_position != m_last;
    }
    /// Returns the current value in the range
    const_reference current() const
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to access the value of a closed range", is_open());

        return m_position;
    }
    /// Advances the current position in the range
    class_type& advance()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to advance a closed range", is_open());
        STLSOFT_MESSAGE_ASSERT("Attempting to increment the range past its end point", ((m_increment > 0 && m_position < m_last) || (m_increment < 0 && m_position > m_last)));

        m_position += m_increment;

        return *this;
    }

    /// Indicates whether the range is open
    operator operator_bool_type() const
    {
        return operator_bool_generator_type::translate(is_open());
    }
    /// Returns the current value in the range
    const_reference operator *() const
    {
        return current();
    }
    /// Advances the current position in the range
    class_type& operator ++()
    {
        return advance();
    }
    /// Advances the current position in the range, returning a copy of the
    /// range prior to its being advanced
    class_type operator ++(int)
    {
        class_type  ret(*this);

        operator ++();

        return ret;
    }
/// @}

/// \name Comparison
/// @{
public:
    /// Evaluates whether two ranges are equal
    bool operator ==(class_type const& rhs) const
    {
        STLSOFT_MESSAGE_ASSERT("Comparing unrelated ranges!", m_last == rhs.m_last);

        return m_position == rhs.m_position;
    }
    /// Evaluates whether two ranges are unequal
    bool operator !=(class_type const& rhs) const
    {
        return ! operator ==(rhs);
    }
/// @}

// Implementation
private:
    static void validate_range(value_type first, value_type last, value_type increment)
    {
        ss_bool_t   bValid = true;

        // Check modulus
        if(bValid)
        {
            if( first != last &&
                0 != increment)
            {
                bValid = (0 == ((last - first) % increment));
            }
        }

        // Check direction
        if(bValid)
        {
            if( (   last < first &&
                    increment > 0) ||
                (   first < last &&
                    increment < 0))
            {
                bValid = false;
            }
        }

//        STLSOFT_MESSAGE_ASSERT("The range you have specified will not close with the given increment", (first == last || (increment > 0 && last > first) || (increment < 0 && last < first)));
//        STLSOFT_MESSAGE_ASSERT("The range you have specified will not close with the given increment", 0 == ((last - first) % increment));

        if(!bValid)
        {
            exception_policy_type()(first, last, increment);
        }

        // Assert here, in case using a null exception policy
        STLSOFT_MESSAGE_ASSERT("invalid integral range", bValid);
    }

// Members
private:
    value_type  m_position;
    value_type  m_last;
    value_type  m_increment;
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

template <ss_typename_param_k T>
inline integral_range<T> make_integral_range(T first, T last)
{
    return integral_range<T>(first, last);
}

template <ss_typename_param_k T>
inline integral_range<T> make_integral_range(T first, T last, T increment)
{
    return integral_range<T>(first, last, increment);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/integral_range_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef RANGELIB_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace rangelib
# else
} // namespace rangelib_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !RANGELIB_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !RANGELIB_INCL_RANGELIB_HPP_INTEGRAL_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
