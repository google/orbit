/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/view_slice_functions.hpp
 *
 * Purpose:     String view slice functions.
 *
 * Created:     25th April 2005
 * Updated:     4th July 2012
 *
 * Thanks:      To Pablo Aguilar for inspiration for these functions, and
 *              collaboration on their implementation.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2012, Matthew Wilson and Synesis Software
 * Copyright (c) 2005, Pablo Aguilar
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


/** \file stlsoft/string/view_slice_functions.hpp
 *
 * \brief [C++ only] String view slice functions
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS
#define STLSOFT_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS_MAJOR     2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS_MINOR     1
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS_REVISION  6
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS_EDIT      26
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MWERKS: __MWERKS__<0x3000
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if defined(STLSOFT_COMPILER_IS_MWERKS) && \
    ((__MWERKS__ & 0xff00) < 0x3000)
# error stlsoft/string/view_slice_functions.hpp not compatible with Metrowerks 7.x (v2.4)
#endif /* compiler */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#if defined(STLSOFT_COMPILER_IS_GCC)
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
#  include <stlsoft/string/simple_string.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#endif
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW
# include <stlsoft/string/string_view.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW */

#ifdef STLSOFT_UNITTEST
# include <string>
# ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
#  include <stlsoft/string/simple_string.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k S>
struct string_view_helper_traits
{
    typedef S                                           string_type;
    typedef ss_typename_type_k string_type::value_type  char_type;
    typedef basic_string_view<char_type>                view_type;
};

# if !defined(STLSOFT_COMPILER_IS_MSVC) || \
     _MSC_VER >= 1310
STLSOFT_TEMPLATE_SPECIALISATION
struct string_view_helper_traits<ss_char_a_t*>
{
    typedef ss_char_a_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
# endif /* compiler */

STLSOFT_TEMPLATE_SPECIALISATION
struct string_view_helper_traits<ss_char_a_t const*>
{
    typedef ss_char_a_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};

# ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
template <size_t N>
struct string_view_helper_traits<ss_char_a_t [N]>
{
    typedef ss_char_a_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
template <size_t N>
struct string_view_helper_traits<ss_char_a_t const [N]>
{
    typedef ss_char_a_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
# endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

# if !defined(STLSOFT_COMPILER_IS_MSVC) || \
     _MSC_VER >= 1310
STLSOFT_TEMPLATE_SPECIALISATION
struct string_view_helper_traits<ss_char_w_t*>
{
    typedef ss_char_w_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
# endif /* compiler */

STLSOFT_TEMPLATE_SPECIALISATION
struct string_view_helper_traits<ss_char_w_t const*>
{
    typedef ss_char_w_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};

# ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
template <size_t N>
struct string_view_helper_traits<ss_char_w_t [N]>
{
    typedef ss_char_w_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
template <size_t N>
struct string_view_helper_traits<ss_char_w_t const [N]>
{
    typedef ss_char_w_t                                 char_type;
    typedef basic_string_view<char_type>                view_type;
};
# endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Slice functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
inline basic_string_view<C> left_view_helper(C const* s, ss_size_t n)
{
    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);

    if(n > len)
    {
        // Want more than is available, so get all
        n = len;
    }

    return basic_string_view<C>(s, n);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_a_t> left_view(ss_char_a_t const* s, ss_size_t n)
{
    return left_view_helper(s, n);
}

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_w_t> left_view(ss_char_w_t const* s, ss_size_t n)
{
    return left_view_helper(s, n);
}

template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_view_helper_traits<S>::view_type left_view(S const& s, ss_size_t n)
{
    typedef string_view_helper_traits<S>            traits_t;
    typedef ss_typename_type_k traits_t::view_type  view_t;

    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);

    if(n > len)
    {
        // Want more than is available, so get all
        n = len;
    }

    return view_t(s.data(), n);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
inline basic_string_view<C> right_view_helper(C const* s, ss_size_t n)
{
    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);

    if(n > len)
    {
        // Want more than is available, so get all, from start
        n = len;
    }
    else
    {
        // Want less than is available, so get n from end
        s += (len - n);
    }

    return basic_string_view<C>(s, n);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_a_t> right_view(ss_char_a_t const* s, ss_size_t n)
{
    return right_view_helper(s, n);
}

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_w_t> right_view(ss_char_w_t const* s, ss_size_t n)
{
    return right_view_helper(s, n);
}

template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_view_helper_traits<S>::view_type right_view(S const& s, ss_size_t n)
{
    typedef string_view_helper_traits<S>            traits_t;
    typedef ss_typename_type_k traits_t::view_type  view_t;

    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);
    ss_size_t       off =   0;

    if(n > len)
    {
        // Want more than is available, so get all, from start
        n = len;
    }
    else
    {
        off = len - n;
    }

    return view_t(s.data() + off, n);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
inline basic_string_view<C> mid_view_helper(C const* s, ss_size_t start, ss_size_t n)
{
    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);
    ss_size_t       off =   0;

    if(start > len)
    {
        // Want more than is available, so we start at the end
        off = len;
    }
    else
    {
        off = start;
    }

    if(off + n > len)
    {
        // Want more than is available starting at off, so we just get what is available
        n = len - off;
    }

    return basic_string_view<C>(s + off, n);
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_a_t> mid_view(ss_char_a_t const* s, ss_size_t start, ss_size_t n)
{
    return mid_view_helper(s, start, n);
}

/** \brief
 *
 * \ingroup group__library__string
 */
inline basic_string_view<ss_char_w_t> mid_view(ss_char_w_t const* s, ss_size_t start, ss_size_t n)
{
    return mid_view_helper(s, start, n);
}

template <ss_typename_param_k S>
inline ss_typename_type_ret_k string_view_helper_traits<S>::view_type mid_view(S const& s, ss_size_t start, ss_size_t n)
{
    typedef string_view_helper_traits<S>            traits_t;
    typedef ss_typename_type_k traits_t::view_type  view_t;

    const ss_size_t len = stlsoft_ns_qual(c_str_len)(s);
    ss_size_t       off =   0;

    if(start > len)
    {
        // Want more than is available, so we start at the end
        off = len;
    }
    else
    {
        off = start;
    }

    if(off + n > len)
    {
        // Want more than is available starting at off, so we just get what is available
        n = len - off;
    }

    return view_t(s.data() + off, n);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/view_slice_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_VIEW_SLICE_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
