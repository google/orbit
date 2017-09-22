/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/any_cast.hpp
 *
 * Purpose:     A dangerous, but sometimes necessary, tool for handling bad
 *              libraries.
 *
 * Created:     12th May 2004
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


/** \file stlsoft/conversion/any_cast.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::any_caster cast class
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_ANY_CAST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_ANY_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_ANY_CAST_MAJOR      4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_ANY_CAST_MINOR      0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_ANY_CAST_REVISION   1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_ANY_CAST_EDIT       33
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

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

/** \brief Helper class used by any_caster, to define invalid conversions.
 *
 * \ingroup group__library__conversion
 *
 */
template <ss_size_t N>
struct any_caster_invalid_type
{
/// \name Member Types
/// @{
public:
    typedef any_caster_invalid_type<N>  class_type;
/// @}

/// \name Not to be implemented
/// @{
#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? compiler */
private:
#endif /* compiler */
    any_caster_invalid_type();
    any_caster_invalid_type(class_type const&);
/// @}
};

/** \brief A dangerous, but sometimes necessary, tool for handling bad libraries.
 *
 * \ingroup group__library__conversion
 *
 * This class helps overcome problems when external libraries have errors in
 * their function parameter declarations, resulting in compilation-time type
 * errors with some versions of the libraries, and not with others.
 *
 * \note See inetstl::filesystem_traits<char>::find_first_file() for an
 * example of its use.
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        ,   ss_typename_param_k T3  =   any_caster_invalid_type<3>*
        ,   ss_typename_param_k T4  =   any_caster_invalid_type<4>*
        ,   ss_typename_param_k T5  =   any_caster_invalid_type<5>*
        ,   ss_typename_param_k T6  =   any_caster_invalid_type<6>*
        ,   ss_typename_param_k T7  =   any_caster_invalid_type<7>*
        ,   ss_typename_param_k T8  =   any_caster_invalid_type<8>*
        >
union any_caster
{
/// \name Member Types
/// @{
public:
    typedef any_caster<T, T1, T2, T3, T4, T5, T6, T7, T8>   class_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs an instance from the source type
    ss_explicit_k any_caster(T t_)
    {
        this->t = t_;
    }
/// @}

/// \name Conversion
/// @{
public:
    operator T1 () const
    {
        return t1;
    }
    operator T2 () const
    {
        return t2;
    }
    operator T3 () const
    {
        return t3;
    }
    operator T4 () const
    {
        return t4;
    }
    operator T5 () const
    {
        return t5;
    }
    operator T6 () const
    {
        return t6;
    }
    operator T7 () const
    {
        return t7;
    }
    operator T8 () const
    {
        return t8;
    }
/// @}

/// \name Members
/// @{
private:
    T   t;
    T1  t1;
    T2  t2;
    T3  t3;
    T4  t4;
    T5  t5;
    T6  t6;
    T7  t7;
    T8  t8;
/// @}

/// \name Not to be implemented
/// @{
private:
    any_caster(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/any_caster_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_ANY_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
