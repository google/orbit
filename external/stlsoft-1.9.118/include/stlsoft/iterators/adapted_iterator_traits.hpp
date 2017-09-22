/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/adapted_iterator_traits.hpp
 *
 * Purpose:     Traits for detecting characteristics of adapted iterators.
 *
 * Created:     9th July 2004
 * Updated:     21st June 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2010, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/iterators/adapted_iterator_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::adapted_iterator_traits
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS_MAJOR    2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS_MINOR    5
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS_REVISION 4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS_EDIT     49
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_MSVC:       _MSC_VER < 1310
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
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
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF
# include <stlsoft/meta/select_first_type_if.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SELECT_FIRST_TYPE_IF */
#if !defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
# error This file is not compatible with compilers that do not support member type detection
#else /* ? STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#  include <stlsoft/meta/member_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE
#  include <stlsoft/meta/is_same_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS
#  include <stlsoft/meta/base_type_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS */
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_ADD_QUALIFIER
#  include <stlsoft/meta/add_qualifier.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_ADD_QUALIFIER */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE
#  include <stlsoft/meta/typefixer/difference_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_DIFFERENCE_TYPE */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_POINTER
#  include <stlsoft/meta/typefixer/pointer.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_POINTER */
# ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE
#  include <stlsoft/meta/typefixer/reference.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENCE */
#endif /* !STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
# ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS
#  include <stlsoft/util/std/dinkumware_iterator_traits.hpp>
# endif /* STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS */
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

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

/** \brief Traits for adapted_iterator_traits
 *
 * \ingroup group__library__iterators
 */
template <ss_typename_param_k I>
struct adapted_iterator_traits
{
private:
    // Because adapted_iterator_traits may need to work with By-Value Temporary References (which must be denoted by
    // having pointer and reference of type void), we need to detect this, and define pointer and iterator
    // (and const_pointer and const_reference) members appropriately

    enum { HAS_MEMBER_DIFFERENCE_TYPE   =   0 != member_traits<I>::has_member_difference_type               };
    enum { HAS_MEMBER_POINTER           =   0 != member_traits<I>::has_member_pointer                       };
#if defined(STLSOFT_COMPILER_IS_MWERKS)
    enum { HAS_MEMBER_REFERENCE         =   0 != member_traits<I>::has_member_reference                     };
#else /* ? compiler */
    enum { HAS_MEMBER_REFERENCE         =   HAS_MEMBER_POINTER                                              };
#endif /* compiler */

    public:
    enum { IS_DINKUMWARE_OLD_FORM       =   0 != member_traits<I>::has_member_distance_type &&
                                            0 == HAS_MEMBER_DIFFERENCE_TYPE                                 };
    private:

public:
    /// \brief The iterator category
    ///
    /// This is simply the member <code>iterator_category</code> of the adapted iterator type
    typedef ss_typename_type_k I::iterator_category                                     iterator_category;
    /// \brief The value type
    ///
    /// This is simply the member <code>value_type</code> of the adapted iterator type
    typedef ss_typename_type_k I::value_type                                            value_type;
    /// \brief The difference type
    ///
    /// This is the member <code>value_type</code> of the adapted iterator type, if it is defined, or <code>ptrdiff_t</code> if not.
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k typefixer::fixer_difference_type<I, HAS_MEMBER_DIFFERENCE_TYPE>::difference_type
                                                ,   ss_ptrdiff_t
                                                ,   HAS_MEMBER_DIFFERENCE_TYPE
                                                >::type                                 difference_type;
private:
# if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
     STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
    /// \brief The Dinkumware list::/set::(const_)iterator's putative pointer type
    typedef ss_typename_type_k select_first_type_if<value_type const*
                                                ,   value_type*
                                                ,   Dinkumware_iterator_test<I, IS_DINKUMWARE_OLD_FORM>::is_const
                                                >::type                                 putative_dinkumware_pointer_;
    /// \brief The Dinkumware list::/set::(const_)iterator's putative reference type
    typedef ss_typename_type_k select_first_type_if<value_type const&
                                                ,   value_type&
                                                ,   Dinkumware_iterator_test<I, IS_DINKUMWARE_OLD_FORM>::is_const
                                                >::type                                 putative_dinkumware_reference_;

    /// \brief The putative pointer type
    ///
    /// This is either the putative Dinkumware pointer, or the actual member pointer
    typedef ss_typename_type_k select_first_type_if<putative_dinkumware_pointer_
                                                ,   ss_typename_type_k typefixer::fixer_pointer<I, HAS_MEMBER_POINTER>::pointer
                                                ,   IS_DINKUMWARE_OLD_FORM
                                                >::type                                 putative_pointer_;
    /// \brief The putative reference type
    ///
    /// This is either the putative Dinkumware reference, or the actual member reference
    typedef ss_typename_type_k select_first_type_if<putative_dinkumware_reference_
                                                ,   ss_typename_type_k typefixer::fixer_reference<I, HAS_MEMBER_POINTER>::reference
                                                ,   IS_DINKUMWARE_OLD_FORM
                                                >::type                                 putative_reference_;
# else /* ? Dinkumware */
    typedef ss_typename_type_k typefixer::fixer_pointer<I, HAS_MEMBER_POINTER>::pointer         putative_pointer_;
    typedef ss_typename_type_k typefixer::fixer_reference<I, HAS_MEMBER_REFERENCE>::reference   putative_reference_;
# endif /* Dinkumware */
public:
    /// \brief The pointer type
    ///
    /// This is the member <code>pointer</code> of the adapted iterator type, if it is defined. If not, it is defined to
    /// be <code>void</code>, so as to prevent any dangerous use of it. This will likely act as a compile-time constraint
    /// to prevent the base iterator type being adapted (which is nice).
    typedef ss_typename_type_k select_first_type_if<putative_pointer_
                                                ,   void
                                                ,   HAS_MEMBER_POINTER || IS_DINKUMWARE_OLD_FORM
                                                >::type                                 pointer;

//private:
//    enum { is_const           =   0 != member_traits<pointer>::is_const                                   };

public:
    /// \brief The reference type
    ///
    /// This is the member <code>reference</code> of the adapted iterator type, if it is defined. If not, it is defined to
    /// be <code>void</code>, so as to prevent any dangerous use of it. This will likely act as a compile-time constraint
    /// to prevent the base iterator type being adapted (which is nice).
    typedef ss_typename_type_k select_first_type_if<putative_reference_
                                                ,   void
                                                ,   HAS_MEMBER_REFERENCE || IS_DINKUMWARE_OLD_FORM
                                                >::type                                 reference;
private:
    // Here's where it gets super-tricky, since we need to work out the const_pointer and const_reference types,
    // and define them in the cases that:
    //
    // value_type is void     - Reference Category: Void
    // pointer is void        - Reference Category: By-Value Temporary
    // pointer is a pointer   - Reference Category: > By-Value Temporary

    public:
    enum { REF_CAT_IS_VOID          =   0 != is_same_type<value_type, void>::value && (0 == IS_DINKUMWARE_OLD_FORM)                         };
    enum { REF_CAT_IS_BVT           =   0 == REF_CAT_IS_VOID && 0 != is_same_type<pointer, void>::value && (0 == IS_DINKUMWARE_OLD_FORM)    };
    private:

    ~adapted_iterator_traits()
    {
        void    (*p)()  =   constraints;

        STLSOFT_SUPPRESS_UNUSED(p);
    }

// This has to be non-private, otherwise GCC cries me a river
protected:
    static void constraints()
    {
        // If this fires, then the adapted iterator defines one, but not both, of pointer & reference, or defines
        // them inconsistenly (i.e. one is void, but not both)
        STLSOFT_STATIC_ASSERT((is_same_type<pointer, void>::value == is_same_type<reference, void>::value));
    }
private:

    // The base type
    typedef ss_typename_type_k base_type_traits<pointer>::base_type                     pointer_base_type_;
    typedef ss_typename_type_k base_type_traits<reference>::base_type                   reference_base_type_;

    // Work out what const_pointer will be:
    //
    // - if REF_CAT_IS_VOID, then it's void
    // - if REF_CAT_IS_BVT, then it's void
    // - if higher ref cat, then it's base_type_traits<pointer>::base_type const*


public:
    /// \brief The const_pointer type
    ///
    /// This is either void, if the iterator element reference category is <b>Void</b> or <b>BVT</b>, otherwise
    /// it's <code>"<i>value_type</i>" const*</code>, where <code>"<i>value_type</i>"</code> is the
    /// deduced value type obtained from <code>pointer</code>.
    typedef ss_typename_type_k select_first_type_if<void
                                                ,   pointer_base_type_ const*
                                                ,   REF_CAT_IS_VOID || REF_CAT_IS_BVT
                                                >::type                                 const_pointer;

    /// \brief The const_reference type
    ///
    /// This is either void, if the iterator element reference category is <b>Void</b> or <b>BVT</b>, otherwise
    /// it's <code>"<i>value_type</i>" const*</code>, where <code>"<i>value_type</i>"</code> is the
    /// deduced value type obtained from <code>reference</code>.
    typedef ss_typename_type_k select_first_type_if<void
                                                ,   ss_typename_type_k add_const_ref<reference_base_type_>::type
                                                ,   REF_CAT_IS_VOID || REF_CAT_IS_BVT
                                                >::type                                 const_reference;

public:
    /// \brief The effective reference type
    ///
    /// If the base iterator element reference category is <b>Transient</b> or higher, then this is
    /// the reference member of the base iterator. Otherwise, if the reference category is <b>BVT</b>
    /// then this is value_type. Otherwise (if the reference category is <b>Void</b>), then this is void
    typedef ss_typename_type_k select_first_type_if<value_type
                                                ,   reference
                                                ,   REF_CAT_IS_BVT
                                                >::type                                 effective_reference;
    /// \brief The effective const_reference type
    ///
    /// If the base iterator element reference category is <b>Transient</b> or higher, then this is
    /// const_reference. Otherwise, if the reference category is <b>BVT</b> then this is value_type. Otherwise
    /// (if the reference category is <b>Void</b>), then this is void
    typedef ss_typename_type_k select_first_type_if<ss_typename_type_k add_const<value_type>::type
                                                ,   const_reference
                                                ,   REF_CAT_IS_BVT
                                                >::type                                 effective_const_reference;

    /// \brief The effective pointer type
    ///
    /// If the base iterator element pointer category is <b>Transient</b> or higher, then this is
    /// the pointer member of the base iterator. Otherwise, if the pointer category is <b>BVT</b>
    /// then this is void. Otherwise (if the pointer category is <b>Void</b>), then this is void
    typedef ss_typename_type_k select_first_type_if<void
                                                ,   pointer
                                                ,   REF_CAT_IS_BVT
                                                >::type                                 effective_pointer;
    /// \brief The effective const_pointer type
    ///
    /// If the base iterator element pointer category is <b>Transient</b> or higher, then this is
    /// const_pointer. Otherwise, if the pointer category is <b>BVT</b> then this is void. Otherwise
    /// (if the pointer category is <b>Void</b>), then this is void
    typedef ss_typename_type_k select_first_type_if<void
                                                ,   const_pointer
                                                ,   REF_CAT_IS_BVT
                                                >::type                                 effective_const_pointer;
};

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
struct adapted_iterator_traits<T*>
{
    typedef stlsoft_ns_qual_std(random_access_iterator_tag) iterator_category;
    typedef T                                               value_type;
    typedef ss_ptrdiff_t                                    difference_type;
    typedef value_type*                                     pointer;
    typedef value_type const*                               const_pointer;
    typedef value_type&                                     reference;
    typedef value_type const&                               const_reference;
    typedef reference                                       effective_reference;
    typedef const_reference                                 effective_const_reference;
    typedef pointer                                         effective_pointer;
    typedef const_pointer                                   effective_const_pointer;
};

template <ss_typename_param_k T>
struct adapted_iterator_traits<T const*>
{
    typedef stlsoft_ns_qual_std(random_access_iterator_tag) iterator_category;
    typedef T                                               value_type;
    typedef ss_ptrdiff_t                                    difference_type;
    typedef value_type const*                               pointer;
    typedef value_type const*                               const_pointer;
    typedef value_type const&                               reference;
    typedef value_type const&                               const_reference;
    typedef reference                                       effective_reference;
    typedef const_reference                                 effective_const_reference;
    typedef pointer                                         effective_pointer;
    typedef const_pointer                                   effective_const_pointer;
};

template <ss_typename_param_k T>
struct adapted_iterator_traits<T volatile*>
{
    typedef stlsoft_ns_qual_std(random_access_iterator_tag) iterator_category;
    typedef T                                               value_type;
    typedef ss_ptrdiff_t                                    difference_type;
    typedef value_type volatile*                            pointer;
    typedef value_type volatile const*                      const_pointer;
    typedef value_type volatile&                            reference;
    typedef value_type volatile const&                      const_reference;
    typedef reference                                       effective_reference;
    typedef const_reference                                 effective_const_reference;
    typedef pointer                                         effective_pointer;
    typedef const_pointer                                   effective_const_pointer;
};

template <ss_typename_param_k T>
struct adapted_iterator_traits<T const volatile*>
{
    typedef stlsoft_ns_qual_std(random_access_iterator_tag) iterator_category;
    typedef T                                               value_type;
    typedef ss_ptrdiff_t                                    difference_type;
    typedef value_type volatile const*                      pointer;
    typedef value_type volatile const*                      const_pointer;
    typedef value_type volatile const&                      reference;
    typedef value_type volatile const&                      const_reference;
    typedef reference                                       effective_reference;
    typedef const_reference                                 effective_const_reference;
    typedef pointer                                         effective_pointer;
    typedef const_pointer                                   effective_const_pointer;
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_ADAPTED_ITERATOR_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
