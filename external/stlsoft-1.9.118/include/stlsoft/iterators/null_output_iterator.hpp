/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/null_output_iterator.hpp
 *
 * Purpose:     Null output iterator.
 *
 * Created:     18th February 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/null_output_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::null_output_iterator class
 *   template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR_MAJOR       1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR_MINOR       0
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR_REVISION    4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR_EDIT        16
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_STRING
# define STLSOFT_INCL_STRING
# include <string>
#endif /* !STLSOFT_INCL_STRING */

/* For some reason, VC++ 6.0 has an ICE if the following is not included */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1200
# include <iterator>
# include <stlsoft/string/char_traits.hpp>
#endif /* compiler */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <sstream>
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

/** \brief \ref group__pattern__null_object "Null Object" pattern applied to
 *   the \ref group__concept__output_iterator "Output Iterator" concept.
 *
 * \ingroup group__library__iterators
 *
 *
 * This class template provides full backwards-compatibility with
 * std::null_output_iterator, but offers the additional capability to supply both
 * prefix <i>and</i> suffix to each delimited item.
 *
 * For example, the following code indents each entry in the sequence as well
 * as appending a carriage return.
 *
\code

std::vector<int>  ints(10);

{ for(size_t i = 0; i < ints.size(); ++i)
{
  ints[i] = i;
}}

std::cout << "Elements:" << std::endl;
std::copy(ints.begin(), ints.end(), stlsoft::null_output_iterator<int>());
\endcode
 *
 * This has no effect, and produces no output. It does, however, cause the
 * sequence to be enumerated, and its compilation ensures that all iterators
 * support the codified operations.
 *
 * \param V The value type.
 */
template<   ss_typename_param_k V
        >
// [[synesis:class:iterator: null_output_iterator<T<V>, T<C>, T<T>, T<S>>]]
class null_output_iterator
    : public stlsoft_ns_qual(iterator_base)<stlsoft_ns_qual_std(output_iterator_tag), void, void, void, void>
{
/// \name Member Types
/// @{
public:
    /// The value type
    typedef V                                           assigned_type;
    /// The class type
    typedef null_output_iterator<V>                     class_type;
private:
    class deref_proxy;
/// @}

/// \name Construction
/// @{
public:
/// @}

/// \name Implementation
/// @{
private:
    class deref_proxy
    {
    public:
        deref_proxy(null_output_iterator *it)
            : m_it(it)
        {}

    public:
        void operator =(assigned_type const& value)
        {
            STLSOFT_SUPPRESS_UNUSED(value);
        }

    private:
        null_output_iterator  *const m_it;

    // Not to be implemented
    private:
        void operator =(deref_proxy const&);
    };
/// @}

/// \name Ouput Iterator Methods
/// @{
public:
    deref_proxy operator *()
    {
        return deref_proxy(this);
    }
    class_type& operator ++()
    {
        return *this;
    }
    class_type& operator ++(int)
    {
        return *this;
    }
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
//# include "./unittest/ostream_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_NULL_OUTPUT_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
