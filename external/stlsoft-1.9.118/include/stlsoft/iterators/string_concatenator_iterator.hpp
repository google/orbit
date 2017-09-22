/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/string_concatenator_iterator.hpp
 *
 * Purpose:     string_concatenator_iterator class template.
 *
 * Created:     12th May 1998
 * Updated:     31st July 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/string_concatenator_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::string_concatenator_iterator
 *   iterator adaptor class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR_MAJOR       2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR_MINOR       4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR_REVISION    1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR_EDIT        42
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS
# include <stlsoft/iterators/common/string_concatenation_flags.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_COMMON_HPP_STRING_CONCATENATION_FLAGS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS
# include <stlsoft/string/string_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS */
#ifndef _STLSOFT_STRING_FUNCTIONALS_NO_STD
# include <functional>
#else /* ? _STLSOFT_STRING_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _STLSOFT_STRING_FUNCTIONALS_NO_STD */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# include <string>
# include <vector>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

#ifdef __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE
# pragma message(_sscomp_fileline_message("TODO: Need a function that can do quoting (or anything else)"))
#endif /* __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief This output iterator adaptor template wraps a C-style string pointer as an
 * output iterator, to enable a C-style string to be built up as a result of the
 * application of an algorithm.
 *
 * \ingroup group__library__iterators
 *
 * \param S The type of the string that will be written to
 * \param D The type of the delimiter that will be used to separate the elements written to the iterator
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k D
        >
// [[synesis:class:iterator: string_concatenator_iterator<T<S>, T<D>>]]
class string_concatenator_iterator
    : public stlsoft_ns_qual(iterator_base)<stlsoft_ns_qual_std(output_iterator_tag), void, void, void, void>
{
/// \name Member Types
/// @{
public:
    typedef S                                   string_type;
    typedef D                                   delimiter_type;
    typedef string_concatenator_iterator<S, D>  class_type;
/// @}

/// \name Construction
/// @{
private:
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1310
public:
#endif /* compiler */
    string_concatenator_iterator(
        string_type*            s
    ,   delimiter_type const*   delim
    ,   int                     flags
    )
        : m_s(s)
        , m_delim(delim)
        , m_flags(flags)
        , m_count(0)
    {}
public:
    static class_type create(
        string_type&            s
    ,   delimiter_type const&   delim
    ,   int                     flags
    )
    {
        return class_type(&s, &delim, flags);
    }
private:
    class deref_proxy;
    friend class deref_proxy;
/// @}

/// \name Output Iterator Operations
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

/// \name Implementation
/// @{
private:
    class deref_proxy
    {
    public:
        deref_proxy(string_concatenator_iterator* it)
            : m_it(it)
        {}

    public:
        template <ss_typename_param_k S3>
        void operator =(S3 const& value)
        {
#if defined(STLSOFT_COMPILER_IS_BORLAND)
            // NOTE: Borland has a cow with the default implementation of this
            // method (shown below), so we do an extra (and less efficient)
            // indirection. Sigh!
            m_it->invoke_(c_str_ptr(value));
#else /* ? compiler */
            // Derive the character type of the string, so can dispatch on it

            // NOTE: If you get an 'undefined' compilation error here, you
            // need to include a suitable specialisation of the stlsoft::string_traits
            // traits class. For example, if you are concatenating std::string/std::wstring
            // or
            // COM VARIANTs, then
            // you will need to include comstl/string_traits.hpp
            typedef ss_typename_type_k string_traits<string_type>::char_type    char_t;

            STLSOFT_STATIC_ASSERT(sizeof(char) == sizeof(char_t) || sizeof(wchar_t) == sizeof(char_t));

            m_it->invoke_(value, char_t());
#endif /* compiler */
        }

    private:
        string_concatenator_iterator    *const m_it;

    // Not to be implemented
    private:
        void operator =(deref_proxy const&);
    };

#if defined(STLSOFT_COMPILER_IS_BORLAND)
    template <ss_typename_param_k C2>
    void invoke_(C2 const* value)
    {
        this->invoke_(value, *value);
    }
#endif /* ? compiler */

#if 0
    template <ss_typename_param_k S3>
    void invoke_(S3 const& value)
    {
        STLSOFT_ASSERT(NULL != m_s);
        STLSOFT_ASSERT(NULL != m_delim);

        if(0 != c_str_len(*m_s))
        {
            // NOTE: Use +=, as it's the most general
            *m_s += c_str_ptr(*m_delim);
        }
        *m_s += c_str_ptr(value);
    }
#else /* ? 0 */
    template <ss_typename_param_k S3>
    void invoke_(S3 const& value, ss_char_a_t)
    {
        STLSOFT_ASSERT(NULL != m_s);
        STLSOFT_ASSERT(NULL != m_delim);

        bool const valueEmpty = (0u == c_str_len(value));

        if( valueEmpty &&
            0 == (string_concatenation_flags::AlwaysSeparate & m_flags))
        {
            ;
        }
        else
        {
            bool const stringEmpty = (0u == c_str_len(*m_s));

            if(stringEmpty)
            {
                if( 0 != m_count &&
                    0 != (string_concatenation_flags::AlwaysSeparate & m_flags))
                {
                  *m_s += c_str_ptr_a(*m_delim);
                }
            }
            else
            {
                // NOTE: Use +=, as it's the most general
                *m_s += c_str_ptr_a(*m_delim);
            }
            *m_s += c_str_ptr_a(value);
        }

        ++m_count;
    }
    template <ss_typename_param_k S3>
    void invoke_(S3 const& value, ss_char_w_t)
    {
        STLSOFT_ASSERT(NULL != m_s);
        STLSOFT_ASSERT(NULL != m_delim);

        bool const valueEmpty = (0u == c_str_len(value));

        if( valueEmpty &&
            0 == (string_concatenation_flags::AlwaysSeparate & m_flags))
        {
            ;
        }
        else
        {
            bool const stringEmpty = (0u == c_str_len(*m_s));

            if(stringEmpty)
            {
                if( 0 != m_count &&
                    0 != (string_concatenation_flags::AlwaysSeparate & m_flags))
                {
                  *m_s += c_str_ptr_w(*m_delim);
                }
            }
            else
            {
                // NOTE: Use +=, as it's the most general
                *m_s += c_str_ptr_w(*m_delim);
            }
            *m_s += c_str_ptr_w(value);
        }

        ++m_count;
    }
#endif /* 0 */
/// @}

/// \name Member Variables
/// @{
private:
    string_type*            m_s;
    delimiter_type const*   m_delim;
    int                     m_flags;
    unsigned                m_count;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Creator Functions
 */

/** \brief Creator function for string_concatenator_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param s The string instance to which elements will be concatenated
 * \param delim The delimiter applied between consecutive elements in the concatenation
 *
 * \return An instance of the specialisation string_concatenator_iterator&lt;S, D&gt;
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k D
        >
inline string_concatenator_iterator<S, D>
make_string_concatenator_iterator(
    S&          s
,   D const&    delim
,   int         flags = 0
)             
{
    return string_concatenator_iterator<S, D>::create(s, delim, flags);
}

/** \brief Creator function for string_concatenator_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param s The string instance to which elements will be concatenated
 * \param delim The delimiter applied between consecutive elements in the concatenation
 *
 * \return An instance of the specialisation string_concatenator_iterator&lt;S, D&gt;
 *
 * \note Short-hand for make_string_concatenator_iterator()
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k D
        >
inline string_concatenator_iterator<S, D>
string_concatenator(
    S&          s
,   D const&    delim
,   int         flags = 0
)
{
#if defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MSVC)
    return string_concatenator_iterator<S, D>::create(s, delim, flags);
#else /* ? compiler */
    return make_string_concatenator_iterator(s, delim, flags);
#endif /* compiler */
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/string_concatenator_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_STRING_CONCATENATOR_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
