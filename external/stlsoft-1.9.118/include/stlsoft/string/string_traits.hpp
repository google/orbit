/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/string_traits.hpp
 *
 * Purpose:     string_traits traits class.
 *
 * Created:     16th January 2002
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/string/string_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::string_traits traits class
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS
#define STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STRING_HPP_STRING_TRAITS_MAJOR     4
# define STLSOFT_VER_STLSOFT_STRING_HPP_STRING_TRAITS_MINOR     0
# define STLSOFT_VER_STLSOFT_STRING_HPP_STRING_TRAITS_REVISION  4
# define STLSOFT_VER_STLSOFT_STRING_HPP_STRING_TRAITS_EDIT      78
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
# include <stlsoft/string/string_traits_fwd.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */
#ifndef _STLSOFT_STRING_TRAITS_NO_STD_STRING
# include <string>
#endif /* _STLSOFT_STRING_TRAITS_NO_STD_STRING */

#ifdef STLSOFT_UNITTEST
# include <string>
# include <stlsoft/meta/base_type_traits.hpp>
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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

/* C-style ANSI string */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_a_t *>
{
    typedef ss_char_a_t             value_type;
    typedef ss_char_a_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
    enum {  is_pointer          =   true                };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   int(sizeof(char_type))   };

    static string_type empty_string()
    {
        // This character array is initialised to 0, which conveniently happens to
        // be the empty string, by the module/application load, so it is
        // guaranteed to be valid, and there are no threading/race conditions
        static char_type    s_empty[1];

        STLSOFT_ASSERT(s_empty[0] == '\0'); // Paranoid check

        return s_empty;
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_a_t const*>
{
    typedef ss_char_a_t             value_type;
    typedef ss_char_a_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
    enum {  is_pointer          =   true                };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   int(sizeof(char_type))   };

    static string_type empty_string()
    {
        // This character array is initialised to 0, which conveniently happens to
        // be the empty string, by the module/application load, so it is
        // guaranteed to be valid, and there are no threading/race conditions
        static char_type    s_empty[1];

        STLSOFT_ASSERT(s_empty[0] == '\0'); // Paranoid check

        return s_empty;
    }
};


# if 0
#  ifdef STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT
#   if 1

template <ss_size_t N>
struct string_traits<ss_char_a_t const (&)[N]>
    : public string_traits<ss_char_a_t const*>
{};

template <ss_size_t N>
struct string_traits<ss_char_a_t (&)[N]>
    : public string_traits<ss_char_a_t *>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_a_t const []>
    : public string_traits<ss_char_a_t const*>
{};

STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_a_t []>
    : public string_traits<ss_char_a_t *>
{};

#   else /* ? 0 */

template <ss_typename_param_k C, ss_size_t N>
struct string_traits<C (&)[N]>
    : public string_traits<C *>
{
    typedef ss_char_a_t             value_type;
    typedef ss_char_a_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
};

template <ss_typename_param_k C, ss_size_t N>
struct string_traits<C const (&)[N]>
    : public string_traits<C const*>
{
    typedef ss_char_a_t             value_type;
    typedef ss_char_a_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
};

#   endif /* 0 */
#  endif /* STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
# endif /* 0 */


/* C-style Unicode string */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_w_t *>
{
    typedef ss_char_w_t             value_type;
    typedef ss_char_w_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
    enum {  is_pointer          =   true                };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   int(sizeof(char_type))   };

    static string_type empty_string()
    {
        // This character array is initialised to 0, which conveniently happens to
        // be the empty string, by the module/application load, so it is
        // guaranteed to be valid, and there are no threading/race conditions
        static char_type    s_empty[1];

        STLSOFT_ASSERT(s_empty[0] == '\0'); // Paranoid check

        return s_empty;
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<ss_char_w_t const*>
{
    typedef ss_char_w_t             value_type;
    typedef ss_char_w_t             char_type;
    typedef ss_size_t               size_type;
    typedef char_type const         const_char_type;
    typedef value_type const*       string_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
    enum {  is_pointer          =   true                };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   int(sizeof(char_type))   };

    static string_type empty_string()
    {
        // This character array is initialised to 0, which conveniently happens to
        // be the empty string, by the module/application load, so it is
        // guaranteed to be valid, and there are no threading/race conditions
        static char_type    s_empty[1];

        STLSOFT_ASSERT(s_empty[0] == '\0'); // Paranoid check

        return s_empty;
    }
};

/* std::basic_string */
# ifndef _STLSOFT_STRING_TRAITS_NO_STD_STRING
#  ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
template <ss_typename_param_k C>
struct string_traits<stlsoft_ns_qual_std(basic_string)<C> >
{
    // NOTE: Originally, what is string_type_ was defined as value_type, but
    // Borland objects to value_type::value_type.
    typedef stlsoft_ns_qual_std(basic_string)<C>                    string_type_;
    typedef ss_typename_type_k string_type_::value_type             char_type;
    typedef ss_typename_type_k string_type_::size_type              size_type;
    typedef char_type const                                         const_char_type;
    typedef string_type_                                            string_type;
    typedef string_type_                                            value_type;
    typedef ss_typename_type_k string_type::pointer                 pointer;
    typedef ss_typename_type_k string_type::const_pointer           const_pointer;
    typedef ss_typename_type_k string_type::iterator                iterator;
    typedef ss_typename_type_k string_type::const_iterator          const_iterator;
    typedef ss_typename_type_k string_type::reverse_iterator        reverse_iterator;
    typedef ss_typename_type_k string_type::const_reverse_iterator  const_reverse_iterator;
    enum {  is_pointer          =   false               };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   sizeof(char_type)   };

    static string_type empty_string()
    {
        return string_type();
    }
    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(src, pos, len);
    }
#  ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k I>
    static string_type &assign_inplace(string_type& str, I first, I last)
#  else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type &assign_inplace(string_type& str, const_iterator first, const_iterator last)
#  endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        // std::basic_string cannot assign in-place (or rather not all implementations do so)
        return (str = string_type(first, last), str);
    }
};
#  else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
#   if defined(STLSOFT_CF_std_NAMESPACE)
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<stlsoft_ns_qual_std(basic_string)<ss_char_a_t> >
{
    typedef stlsoft_ns_qual_std(basic_string)<ss_char_a_t>  value_type;
    typedef value_type::value_type                          char_type;
    typedef value_type::size_type                           size_type;
    typedef char_type const                                 const_char_type;
    typedef value_type                                      string_type;
    typedef string_type::pointer                            pointer;
    typedef string_type::const_pointer                      const_pointer;
    typedef string_type::iterator                           iterator;
    typedef string_type::const_iterator                     const_iterator;
    typedef string_type::reverse_iterator                   reverse_iterator;
    typedef string_type::const_reverse_iterator             const_reverse_iterator;
    enum {  is_pointer          =   false               };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   sizeof(char_type)   };

    static string_type empty_string()
    {
        return string_type();
    }
    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(src, pos, len);
    }
#  ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k I>
    static string_type &assign_inplace(string_type& str, I first, I last)
#  else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type &assign_inplace(string_type& str, const_iterator first, const_iterator last)
#  endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        // std::basic_string cannot assign in-place (or rather not all implementations do so)
        return (str = string_type(first, last), str);
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<stlsoft_ns_qual_std(basic_string)<ss_char_w_t> >
{
    typedef stlsoft_ns_qual_std(basic_string)<ss_char_w_t>  value_type;
    typedef value_type::value_type                          char_type;
    typedef value_type::size_type                           size_type;
    typedef char_type const                                 const_char_type;
    typedef value_type                                      string_type;
    typedef string_type::pointer                            pointer;
    typedef string_type::const_pointer                      const_pointer;
    typedef string_type::iterator                           iterator;
    typedef string_type::const_iterator                     const_iterator;
    typedef string_type::reverse_iterator                   reverse_iterator;
    typedef string_type::const_reverse_iterator             const_reverse_iterator;
    enum {  is_pointer          =   false               };
    enum {  is_pointer_to_const =   false               };
    enum {  char_type_size      =   sizeof(char_type)   };

    static string_type empty_string()
    {
        return string_type();
    }
    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(src, pos, len);
    }
#  ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k I>
    static string_type &assign_inplace(string_type& str, I first, I last)
#  else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type &assign_inplace(string_type& str, const_iterator first, const_iterator last)
#  endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    {
        // std::basic_string cannot assign in-place (or rather not all implementations do so)
        return (str = string_type(first, last), str);
    }
};
#   endif /* STLSOFT_CF_std_NAMESPACE */
#  endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
# endif /* _STLSOFT_STRING_TRAITS_NO_STD_STRING */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/string_traits_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
