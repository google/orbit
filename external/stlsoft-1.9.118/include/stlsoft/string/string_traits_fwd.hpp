/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/string_traits_fwd.hpp
 *
 * Purpose:     Forward declaration of the string_traits traits class.
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


/** \file stlsoft/string/string_traits_fwd.hpp
 *
 * \brief [C++ only] Forward definition of the stlsoft::string_traits traits
 *  class
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
#define STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD_MAJOR       2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD_MINOR       0
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD_REVISION    2
# define STLSOFT_VER_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD_EDIT        18
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

/* ////////////////////////////////////////////////////////////////////// */


#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \brief Traits for determining the characteristics of strings
 *
 * string_traits is a traits class for determining various characteristics of
 * strings.
 *
 * \param C The char type
 *
 * \ingroup group__library__string
 */
template <ss_typename_param_k C>
struct string_traits
{
    typedef C               value_type;     //!< The value type
    typedef C               char_type;      //!< The char type
    typedef ss_size_t       size_type;      //!< The size type
    typedef ???             string_type;    //!< The string type
    typedef ???             iterator;       //!< The iterator type. Not defined for non-mutating type
    typedef ???             const_iterator; //!< The const_iterator type
    typedef ???             pointer;        //!< The pointer type. Not defined for non-mutating type
    typedef ???             const_pointer;  //!< The const_pointer type
    enum
    {
            is_pointer                      //!< non-zero if C is a pointer type
    };
    enum
    {
        ,   is_pointer_to_const             //!< non-zero if C is a pointer-to-const type
    };
    enum
    {
        ,   char_type_size                  //!< The size of \c char_type
    };
    enum
    {
        ,   is_mutating                     //!< non-zero if C is a mutating type
    };

    /// \brief Returns an instance of the empty string form of the string type
    static string_type empty_string();

    /// \brief Constructs an instance of the string type
    static string_type construct(string_type const& src, size_type pos, size_type len);

# ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /// \brief Assigns a new value to the string, based on the range [first, last)
    ///
    /// \note The range [first, last) may be contained within the string instance
    template <ss_typename_param_k I>
    static string_type &assign_inplace(string_type &str, I first, I last);
# else /* ? STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    static string_type &assign_inplace(string_type &str, const_iterator first, const_iterator last);
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k T>
struct string_traits;

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */

/* ///////////////////////////// end of file //////////////////////////// */
