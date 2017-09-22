/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/charset_tokeniser.hpp
 *
 * Purpose:     String token parsing class using char-sets.
 *
 * Created:     17th October 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/string/charset_tokeniser.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::charset_tokeniser class
 *  template
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHARSET_TOKENISER
#define STLSOFT_INCL_STLSOFT_STRING_HPP_CHARSET_TOKENISER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STRING_HPP_CHARSET_TOKENISER_MAJOR     2
# define STLSOFT_VER_STLSOFT_STRING_HPP_CHARSET_TOKENISER_MINOR     0
# define STLSOFT_VER_STLSOFT_STRING_HPP_CHARSET_TOKENISER_REVISION  4
# define STLSOFT_VER_STLSOFT_STRING_HPP_CHARSET_TOKENISER_EDIT      25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_DMC:  __DMC__<0x0839
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/string/charset_tokeniser.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TOKENISER
# include <stlsoft/string/string_tokeniser.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TOKENISER */

#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */

#ifdef STLSOFT_UNITTEST
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
#  include <stlsoft/string/simple_string.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW
#  include <stlsoft/string/string_view.hpp>
# endif /* STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW */
# include <algorithm>
# include <string>
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

/** \brief Comparator for the stlsoft::charset_tokeniser class template.
 *
 * \ingroup group__library__string
 */
template <ss_typename_param_k S>
struct charset_comparator
{
public:
    typedef S                   delimiter_type;

private:
    template <ss_typename_param_k const_iterator>
    static bool equal_(delimiter_type const& delimiter, const_iterator &it)
    {
        return delimiter.end() != stlsoft_ns_qual_std(find)(delimiter.begin(), delimiter.end(), *it);
    }

    template <ss_typename_param_k const_iterator>
    static const_iterator advance_(const_iterator it, delimiter_type const& delimiter)
    {
        return it + 1;
    }

public:
    ///
    ///
    /// \note This is only for compatibility with earlier versions of the string tokeniser
    template <ss_typename_param_k const_iterator>
    static bool not_equal(delimiter_type const& delimiter, const_iterator &it)
    {
        return !equal_(delimiter, it);
    }

    static ss_size_t length(delimiter_type const& /* delimiter */)
    {
        return 1;
    }

    template <ss_typename_param_k const_iterator>
    static bool test_start_token_advance(const_iterator &it, const_iterator end, delimiter_type const& delimiter)
    {
        return equal_(delimiter, it) ? (it = advance_(it, delimiter), true) : false;
    }

    template <ss_typename_param_k const_iterator>
    static bool test_end_token_advance(const_iterator &it, const_iterator end, delimiter_type const& delimiter)
    {
        return equal_(delimiter, it) ? (it = advance_(it, delimiter), true) : false;
    }

    template <ss_typename_param_k const_iterator>
    static const_iterator nonskip_move_to_start(const_iterator it, const_iterator end, delimiter_type const& delimiter)
    {
        return it;
    }

    template <ss_typename_param_k const_iterator>
    static bool test_end_token(const_iterator it, const_iterator end, delimiter_type const& delimiter)
    {
        return equal_(delimiter, it);
    }

    template <ss_typename_param_k const_iterator>
    static const_iterator find_next_start(const_iterator it, const_iterator end, delimiter_type const& delimiter)
    {
        return advance_(it, delimiter);
    }
};

/** \brief A class template that provides string tokenising behaviour, where the delimiter is a character set, a la <code>strtok()</code>
 *
 * \ingroup group__library__string
 *
 * This class takes a string, and a character-set delimiter, and fashions
 * a sequence from the given string, with each element determined with
 * respect to the delimiter. It is derived from stlsoft::string_tokeniser,
 * and effectively defines a specialisation of it, in order to make it
 * simpler to specialise. All that's usually required is to specialise
 * the string type and, optionally, the blanks policy.
 *
 * \param S The string type
 * \param B The blank skipping policy type. Defaults to skip_blank_tokens&lt;true&gt;
 * \param V The value type (the string type that will be used for the values). Defaults to \c S
 * \param T The string type traits type. Defaults to string_tokeniser_type_traits&lt;S, V&gt;
 * \param D The delimiter type (can be a string type or a character type). Defaults to \c S
 * \param P The tokeniser comparator type. Defaults to string_tokeniser_comparator&lt;D, S, T&gt;
 *
 */
template<   ss_typename_param_k S
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k B = skip_blank_tokens<true>
        ,   ss_typename_param_k V = S
        ,   ss_typename_param_k T = string_tokeniser_type_traits<S, V>
        ,   ss_typename_param_k D = S
        ,   ss_typename_param_k P = charset_comparator<S>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k B
        ,   ss_typename_param_k V
        ,   ss_typename_param_k T
        ,   ss_typename_param_k D
        ,   ss_typename_param_k P
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class charset_tokeniser
    : public string_tokeniser<S, D, B, V, T, P>
{
/// \name Member Types
/// @{
private:
    typedef string_tokeniser<S, D, B, V, T, P>                          parent_class_type;
public:
    /// The current parameterisation of the type
    typedef charset_tokeniser<S, B, V, T, D, P>                         class_type;
    /// The sequence string type
    typedef ss_typename_type_k parent_class_type::string_type           string_type;
    /// The delimiter type
    typedef ss_typename_type_k parent_class_type::delimiter_type        delimiter_type;
    /// The blanks policy type
    typedef ss_typename_type_k parent_class_type::blanks_policy_type    blanks_policy_type;
    /// The value type
    typedef ss_typename_type_k parent_class_type::value_type            value_type;
    /// The traits type
    typedef ss_typename_type_k parent_class_type::traits_type           traits_type;
    /// The tokeniser comparator type
    typedef ss_typename_type_k parent_class_type::comparator_type       comparator_type;
    /// The character type
    typedef ss_typename_type_k parent_class_type::char_type             char_type;
    /// The size type
    typedef ss_typename_type_k parent_class_type::size_type             size_type;
#if 0
    /// The difference type
    typedef ss_typename_type_k parent_class_type::difference_type       difference_type;
#endif /* 0 */
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k parent_class_type::const_reference       const_reference;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k parent_class_type::const_iterator        const_iterator;
/// @}

public:
/// \name Construction
/// @{
public:
    /// Tokenise the given C-string with the given delimiter
    ///
    /// \param psz Pointer to C-string whose contents will be tokenised
    /// \param charSet The delimiter to perform the tokenisation
    ///
    /// \note The tokeniser class takes a copy of \c psz. It does not alter the contents of \c psz
    charset_tokeniser(char_type const* psz, delimiter_type const& charSet)
        : parent_class_type(psz, charSet)
    {}

// Define the string_type overload if there member template ctors are not supported, or
// they are correctly discriminated
#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    /// Tokenise the given string with the given delimiter
    ///
    /// \param str The string whose contents will be tokenised
    /// \param charSet The delimiter to perform the tokenisation
    ///
    /// \note The tokeniser class takes a copy of \c str. It does not alter the contents of \c str
    charset_tokeniser(string_type const& str, delimiter_type const& charSet)
        : parent_class_type(str, charSet)
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

// Define the template overload if member template ctors are supported
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    /// Tokenise the given string with the given delimiter
    ///
    /// \param str The string whose contents will be tokenised
    /// \param charSet The delimiter to perform the tokenisation
    ///
    /// \note The tokeniser class takes a copy of \c str. It does not alter the contents of \c str
    template <ss_typename_param_k S1>
    charset_tokeniser(S1 const& str, delimiter_type const& charSet)
        : parent_class_type(str, charSet)
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */

    /// Tokenise the specified length of the given string with the given delimiter
    ///
    /// \param psz Pointer to C-string whose contents will be tokenised
    /// \param cch The number of characters in \c psz to use
    /// \param charSet The delimiter to perform the tokenisation
    ///
    /// \note The tokeniser class takes a copy of \c psz. It does not alter the contents of \c psz
    charset_tokeniser(char_type const* psz, size_type cch, delimiter_type const& charSet)
        : parent_class_type(psz, cch, charSet)
    {}

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
    defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    /// \brief Tokenise the given range with the given delimiter
    ///
    /// \param from The start of the asymmetric range to tokenise
    /// \param to The start of the asymmetric range to tokenise
    /// \param charSet The delimiter to use
    charset_tokeniser(char_type const* from, char_type const* to, delimiter_type const& charSet)
        : parent_class_type(from, to, charSet)
    {}
#endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    /// Tokenise the given range with the given delimiter
    ///
    /// \param from The start of the asymmetric range to tokenise
    /// \param to The start of the asymmetric range to tokenise
    /// \param charSet The delimiter to use
    template <ss_typename_param_k I>
    charset_tokeniser(I from, I to, delimiter_type const& charSet)
        : parent_class_type(from, to, charSet)
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/charset_tokeniser_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHARSET_TOKENISER */

/* ///////////////////////////// end of file //////////////////////////// */
