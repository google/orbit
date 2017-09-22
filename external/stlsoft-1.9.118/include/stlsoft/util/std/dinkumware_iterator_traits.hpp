/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/std/dinkumware_iterator_traits.hpp
 *
 * Purpose:     "Old" Dinkumware library iterator capability discrimination.
 *
 * Created:     31st December 2005
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


/** \file stlsoft/util/std/dinkumware_iterator_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::Dinkumware_iterator_traits
 *   utility traits class
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS
#define STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS_MAJOR      1
# define STLSOFT_VER_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS_MINOR      0
# define STLSOFT_VER_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS_REVISION   6
# define STLSOFT_VER_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS_EDIT       17
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR
# include <stlsoft/util/std/library_discriminator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR */
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE
#  include <stlsoft/meta/is_same_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE */
# include <deque>
# include <list>
# include <vector>

# include <set> // Shouldn't really be allowed, but do so since DW libs allow it
#endif /* partial-specialisation && old-dinkumware */

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

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k I, int B_IS_DINKUMWARE_OLD_FORM>
struct Dinkumware_iterator_test
{
    enum { is_const =   1   };  // Defaults to const, for safety

protected:
    ~Dinkumware_iterator_test() stlsoft_throw_0()
    {
        // If this fires, then someone has specified a value other then 0 or 1. Very naughty!
        STLSOFT_STATIC_ASSERT(0 == B_IS_DINKUMWARE_OLD_FORM);
    }
};

template <ss_typename_param_k I>
struct Dinkumware_iterator_test<I, 1>
{
    typedef ss_typename_type_k I::value_type                                    value_type;

    typedef ss_typename_type_k std::deque<value_type>::iterator                 deque_iterator_type;
    typedef ss_typename_type_k std::deque<value_type>::const_iterator           deque_const_iterator_type;
    typedef ss_typename_type_k std::deque<value_type>::reverse_iterator         deque_reverse_iterator_type;
    typedef ss_typename_type_k std::deque<value_type>::const_reverse_iterator   deque_const_reverse_iterator_type;

    typedef ss_typename_type_k std::list<value_type>::iterator                  list_iterator_type;
    typedef ss_typename_type_k std::list<value_type>::const_iterator            list_const_iterator_type;
    typedef ss_typename_type_k std::list<value_type>::reverse_iterator          list_reverse_iterator_type;
    typedef ss_typename_type_k std::list<value_type>::const_reverse_iterator    list_const_reverse_iterator_type;

    typedef ss_typename_type_k std::vector<value_type>::reverse_iterator        vector_reverse_iterator_type;
    typedef ss_typename_type_k std::vector<value_type>::const_reverse_iterator  vector_const_reverse_iterator_type;

    typedef ss_typename_type_k std::set<value_type>::iterator                   set_iterator_type;
    typedef ss_typename_type_k std::set<value_type>::const_iterator             set_const_iterator_type;
    typedef ss_typename_type_k std::set<value_type>::reverse_iterator           set_reverse_iterator_type;
    typedef ss_typename_type_k std::set<value_type>::const_reverse_iterator     set_const_reverse_iterator_type;

    enum
    {
        is_const    =   (   stlsoft::is_same_type<deque_iterator_type, I>::value ||
                            stlsoft::is_same_type<deque_reverse_iterator_type, I>::value ||
                            stlsoft::is_same_type<list_iterator_type, I>::value ||
                            stlsoft::is_same_type<list_reverse_iterator_type, I>::value ||
                            stlsoft::is_same_type<vector_reverse_iterator_type, I>::value ||
                            stlsoft::is_same_type<set_iterator_type, I>::value ||
                            stlsoft::is_same_type<set_reverse_iterator_type, I>::value
                        )
                            ?   0
                            :   (   stlsoft::is_same_type<deque_const_iterator_type, I>::value ||
                                    stlsoft::is_same_type<deque_const_reverse_iterator_type, I>::value ||
                                    stlsoft::is_same_type<list_const_iterator_type, I>::value ||
                                    stlsoft::is_same_type<list_const_reverse_iterator_type, I>::value ||
                                    stlsoft::is_same_type<vector_const_reverse_iterator_type, I>::value ||
                                    stlsoft::is_same_type<set_const_iterator_type, I>::value ||
                                    stlsoft::is_same_type<set_const_reverse_iterator_type, I>::value
                                )
                                    ?   1
                                    :   1
    };
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \brief Determines whether the iterator is mutating or non-mutating
 *
 * \ingroup group__library__utility
 *
 * \param I The iterator type
 */
template <ss_typename_param_k I>
struct Dinkumware_iterator_traits
{
private:
    enum { IS_DINKUMWARE_OLD_FORM   =   0 == member_traits<I>::has_member_pointer &&
                                        0 == member_traits<I>::has_member_difference_type &&
                                        0 != member_traits<I>::has_member_distance_type                 };
public:
    enum { is_const                 =   Dinkumware_iterator_test<I, IS_DINKUMWARE_OLD_FORM>::is_const   };
};

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Partial specialisations for dealing with pointers

template<ss_typename_param_k T>
struct Dinkumware_iterator_traits<T*>
{
    enum { is_const = 0 };
};
template<ss_typename_param_k T>
struct Dinkumware_iterator_traits<T const*>
{
    enum { is_const = 1 };
};
template<ss_typename_param_k T>
struct Dinkumware_iterator_traits<T volatile*>
{
    enum { is_const = 0 };
};
template<ss_typename_param_k T>
struct Dinkumware_iterator_traits<T const volatile*>
{
    enum { is_const = 1 };
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#endif /* partial-specialisation && old-dinkumware */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
