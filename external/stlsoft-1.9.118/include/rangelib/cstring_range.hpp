/* /////////////////////////////////////////////////////////////////////////
 * File:        rangelib/cstring_range.hpp
 *
 * Purpose:     Range adaptor for C-strings.
 *
 * Created:     17th May 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file rangelib/cstring_range.hpp Range adaptor for C-strings */

#ifndef RANGELIB_INCL_RANGELIB_HPP_CSTRING_RANGE
#define RANGELIB_INCL_RANGELIB_HPP_CSTRING_RANGE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define RANGELIB_VER_RANGELIB_HPP_CSTRING_RANGE_MAJOR     2
# define RANGELIB_VER_RANGELIB_HPP_CSTRING_RANGE_MINOR     3
# define RANGELIB_VER_RANGELIB_HPP_CSTRING_RANGE_REVISION  3
# define RANGELIB_VER_RANGELIB_HPP_CSTRING_RANGE_EDIT      37
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
#ifndef RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS
# include <rangelib/operator_adaptors.hpp>
#endif /* !RANGELIB_INCL_RANGELIB_HPP_OPERATOR_ADAPTORS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_BOOL_TYPE
# include <stlsoft/meta/is_bool_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_BOOL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE
# include <stlsoft/meta/is_integral_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_INTEGRAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE
# include <stlsoft/meta/is_numeric_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_NUMERIC_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */

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

/** \brief Traits type for cstring_range
 *
 * \ingroup group__library__rangelib
 */
template <ss_typename_param_k C>
struct cstring_range_traits
{
/// \name Types
/// @{
public:
    /// The value type
    typedef C           value_type;
    /// The non-mutable (const) reference type
    typedef C const&    const_reference;
/// @}
};


/** \brief This class represents a C-style string as a range
 *
 * \ingroup group__library__rangelib
 *
 * It is categoried as a Notional Range
 *
 * It could be used as follows
\code
  // Create a range based on a C-string
  stlsoft::cstring_range  r("This is a literal string");

  // Count the number of i's in the string
  size_t  num_Is = stlsoft::r_count(r, 'i');
\endcode
 */
template <ss_typename_param_k C>
class cstring_range
    : public non_mutating_operator_adaptor< cstring_range<C>
                                        ,   cstring_range_traits<C>
                                        >       // This provides the operator forms of the methods
    , public notional_range_tag
{
/// \name Types
/// @{
public:
    /// The value type
    typedef C                                               value_type;
    /// The range tag type
    typedef notional_range_tag                              range_tag_type;
    /// The current parameterisation of the type
    typedef cstring_range<C>                                class_type;
    /// The non-mutable (const) reference type
    typedef value_type const&                               const_reference;
/// @}

/// \name Construction
/// @{
public:
    /// Constructor
    ///
    /// \param s The C-string for which this instance will act as a range
    cstring_range(value_type const* s)
        : m_s(s)
    {
        STLSOFT_MESSAGE_ASSERT("NULL string passed to cstring_range constructor", NULL != s);
    }
    /// Destructor
    ~cstring_range() stlsoft_throw_0()
    {
        // This is a constraint to ensure that this template is not used
        // for any non-character types.
        STLSOFT_STATIC_ASSERT(0 != is_integral_type<value_type>::value);
        STLSOFT_STATIC_ASSERT(0 == is_numeric_type<value_type>::value);
        STLSOFT_STATIC_ASSERT(0 == is_bool_type<value_type>::value);
    }
/// @}

/// \name Range methods
/// @{
public:
    /// Indicates whether the range is open
    ss_bool_t is_open() const
    {
        STLSOFT_ASSERT(NULL != m_s);

        return '\0' != *m_s;
    }
    /// Returns the current value in the range
    const_reference current() const
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to access the value of a closed range", is_open());

        return *m_s;
    }
    /// Advances the current position in the range
    class_type& advance()
    {
        STLSOFT_MESSAGE_ASSERT("Attempting to advance a closed range", is_open());

        ++m_s;

        return *this;
    }
/// @}

// Members
private:
    value_type const    *m_s;
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstring_range_unittest_.h"
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

#endif /* !RANGELIB_INCL_RANGELIB_HPP_CSTRING_RANGE */

/* ///////////////////////////// end of file //////////////////////////// */
