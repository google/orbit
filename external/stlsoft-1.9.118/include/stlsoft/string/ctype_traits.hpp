/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/ctype_traits.hpp
 *
 * Purpose:     Traits for ctype functions.
 *
 * Created:     1st April 2005
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


/** \file stlsoft/string/ctype_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::ctype_traits traits class
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS
#define STLSOFT_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS_MAJOR     2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS_MINOR     0
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS_REVISION  2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS_EDIT      18
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_H_CTYPE
# define STLSOFT_INCL_H_CTYPE
# include <ctype.h>
#endif /* !STLSOFT_INCL_H_CTYPE */
#if defined(STLSOFT_COMPILER_IS_GCC) || \
    defined(STLSOFT_COMPILER_IS_MWERKS)
# ifndef STLSOFT_INCL_H_WCTYPE
#  define STLSOFT_INCL_H_WCTYPE
#  include <wctype.h>
# endif /* !STLSOFT_INCL_H_WCTYPE */
#elif defined(STLSOFT_COMPILER_IS_WATCOM)
# ifndef STLSOFT_INCL_H_WCHAR
#  define STLSOFT_INCL_H_WCHAR
#  include <wchar.h>
# endif /* !STLSOFT_INCL_H_WCHAR */
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

/** \brief Traits class that
 *
 * \ingroup group__library__string
 *
 */
template <ss_typename_param_k C>
struct ctype_traits;

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
struct ctype_traits<ss_char_a_t>
{
    typedef ss_char_a_t     char_type;

    static ss_bool_t    is_alpha(char_type ch)      {   return 0 != ::isalpha(ch);      }
    static ss_bool_t    is_upper(char_type ch)      {   return 0 != ::isupper(ch);      }
    static ss_bool_t    is_lower(char_type ch)      {   return 0 != ::islower(ch);      }
    static ss_bool_t    is_digit(char_type ch)      {   return 0 != ::isdigit(ch);      }
    static ss_bool_t    is_xdigit(char_type ch)     {   return 0 != ::isxdigit(ch);     }
    static ss_bool_t    is_space(char_type ch)      {   return 0 != ::isspace(ch);      }
    static ss_bool_t    is_punct(char_type ch)      {   return 0 != ::ispunct(ch);      }
    static ss_bool_t    is_alnum(char_type ch)      {   return 0 != ::isalnum(ch);      }
    static ss_bool_t    is_print(char_type ch)      {   return 0 != ::isprint(ch);      }
    static ss_bool_t    is_graph(char_type ch)      {   return 0 != ::isgraph(ch);      }
    static ss_bool_t    is_cntrl(char_type ch)      {   return 0 != ::iscntrl(ch);      }
    static char_type    to_upper(char_type ch)      {   return static_cast<char_type>(::toupper(ch));       }
    static char_type    to_lower(char_type ch)      {   return static_cast<char_type>(::tolower(ch));       }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct ctype_traits<ss_char_w_t>
{
    typedef ss_char_w_t     char_type;

    static ss_bool_t    is_alpha(char_type ch)      {   return 0 != ::iswalpha(ch);     }
    static ss_bool_t    is_upper(char_type ch)      {   return 0 != ::iswupper(ch);     }
    static ss_bool_t    is_lower(char_type ch)      {   return 0 != ::iswlower(ch);     }
    static ss_bool_t    is_digit(char_type ch)      {   return 0 != ::iswdigit(ch);     }
    static ss_bool_t    is_xdigit(char_type ch)     {   return 0 != ::iswxdigit(ch);    }
    static ss_bool_t    is_space(char_type ch)      {   return 0 != ::iswspace(ch);     }
    static ss_bool_t    is_punct(char_type ch)      {   return 0 != ::iswpunct(ch);     }
    static ss_bool_t    is_alnum(char_type ch)      {   return 0 != ::iswalnum(ch);     }
    static ss_bool_t    is_print(char_type ch)      {   return 0 != ::iswprint(ch);     }
    static ss_bool_t    is_graph(char_type ch)      {   return 0 != ::iswgraph(ch);     }
    static ss_bool_t    is_cntrl(char_type ch)      {   return 0 != ::iswcntrl(ch);     }
    static char_type    to_upper(char_type ch)      {   return static_cast<char_type>(::toupper(ch));       }
    static char_type    to_lower(char_type ch)      {   return static_cast<char_type>(::tolower(ch));       }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CTYPE_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
