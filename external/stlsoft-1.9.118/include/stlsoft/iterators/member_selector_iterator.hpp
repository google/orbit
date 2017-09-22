/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/member_selector_iterator.hpp
 *
 * Purpose:     member_selector_iterator class.
 *
 * Created:     7th April 2005
 * Updated:     10th August 2009
 *
 * Thanks to:   Felix Gartsman for spotting a bug in (lack of) operator <()
 *              when building Pantheios.
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


/** \file stlsoft/iterators/member_selector_iterator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::member_selector_iterator
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR_MAJOR       2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR_MINOR       4
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR_REVISION    6
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR_EDIT        56
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE
# include <stlsoft/meta/is_const_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_CONST_TYPE */
#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS
#  include <stlsoft/meta/base_type_traits.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_BASE_TYPE_TRAITS */
# if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
     STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
#  ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS
#   include <stlsoft/util/std/dinkumware_iterator_traits.hpp>
#  endif /* STLSOFT_INCL_STLSOFT_UTIL_STD_DINKUMWARE_ITERATOR_TRAITS */
# endif /* old-dinkumware */
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
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
template<   ss_typename_param_k I
        ,   ss_typename_param_k C
        ,   ss_typename_param_k M
        >
struct msi_parent_type
// Unfortunately, we can't just go with what iterator form the library supports,
// because we have to deal with:
//
// - VC 7.0, whose Dinkumware library thinks its iterator form is form-1, but
//    the compiler doesn't do partial template specialiation
// - Intel 6/7/7.1/8, which may be used with Visual C++ 6 or 7.0 Dinkumware
//    libraries
//
# if defined(STLSOFT_ITERATOR_ITERATOR_FORM1_SUPPORT) && \
     defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
     (   !defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) || \
         STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION >= STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1) && \
     !defined(STLSOFT_COMPILER_IS_BORLAND) && \
     !defined(STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW)
    : public iterator_base< ss_typename_type_k stlsoft_ns_qual_std(iterator_traits)<I>::iterator_category
                        ,   M
                        ,   ss_ptrdiff_t
                        ,   M*
                        ,   M&
                        >
# elif defined(STLSOFT_ITERATOR_ITERATOR_FORM2_SUPPORT) || \
       (   defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
           STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0) || \
       (   defined(STLSOFT_COMPILER_IS_MSVC) && \
           _MSC_VER == 1300) || \
     defined(STLSOFT_COMPILER_IS_BORLAND) || \
     defined(STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW)
    : public iterator_base< stlsoft_ns_qual_std(input_iterator_tag)
                        ,   M
                        ,   ss_ptrdiff_t
                        ,   M*
                        ,   M&
                        >
# elif defined(STLSOFT_ITERATOR_ITERATOR_FORM3_SUPPORT)
  // do not derive
# else /* ? STLSOFT_ITERATOR_ITERATOR_FORM??????_SUPPORT */
#  error iterator support type not discriminated
# endif /* !STLSOFT_ITERATOR_ITERATOR_FORM1_SUPPORT */
{
/// \name Member Types
/// @{
public:
# if defined(STLSOFT_ITERATOR_ITERATOR_FORM3_SUPPORT)
    typedef stlsoft_ns_qual_std(input_iterator_tag) iterator_category;
# endif /* form ? */
    typedef I                                       base_iterator_type;
    typedef C                                       selected_class_type;
    typedef M                                       value_type;
    typedef value_type*                             pointer;
    typedef value_type const*                       const_pointer;
    typedef value_type&                             reference;
    typedef value_type const&                       const_reference;
    typedef ss_size_t                               size_type;
    typedef ss_ptrdiff_t                            difference_type;
/// @}
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

// class member_selector_iterator
/** \brief An iterator adaptor class template that presents a member of the underlying
 * iterator's value type as the apparent value type.
 *
 * \ingroup group__library__iterators
 *
 * \param I The type of the base iterator, whose value type should be C, or convertible to C
 * \param C The class of the member pointer
 * \param M The type of the member pointer
 */
template<   ss_typename_param_k I
        ,   ss_typename_param_k C
        ,   ss_typename_param_k M
        >
// [[synesis:class:iterator: member_selector_iterator<T<I>, T<C>, T<M>>]]
class member_selector_iterator
    : public msi_parent_type<I, C, M>
{
/// \name Member Types
/// @{
private:
    typedef msi_parent_type<I, C, M>                                    parent_class_type;
public:
    typedef ss_typename_type_k parent_class_type::base_iterator_type    base_iterator_type;
    typedef ss_typename_type_k parent_class_type::selected_class_type   selected_class_type;
    typedef ss_typename_type_k parent_class_type::value_type            value_type;
    typedef ss_typename_type_k parent_class_type::pointer               pointer;
    typedef ss_typename_type_k parent_class_type::const_pointer         const_pointer;
    typedef ss_typename_type_k parent_class_type::reference             reference;
    typedef ss_typename_type_k parent_class_type::const_reference       const_reference;
    typedef ss_typename_type_k parent_class_type::size_type             size_type;
    typedef ss_typename_type_k parent_class_type::difference_type       difference_type;
    typedef member_selector_iterator<I, C, M>                           class_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance from an iterator and a member pointer
    member_selector_iterator(base_iterator_type it, M C::*member)
        : m_it(it)
        , m_member(member)
    {}

    /// \brief Copy constructor
    member_selector_iterator(class_type const& rhs)
        : m_it(rhs.m_it)
        , m_member(rhs.m_member)
    {}

    /// \brief Destructor
    ~member_selector_iterator() stlsoft_throw_0()
    {
        void    (*p)()  =   constraints;

        STLSOFT_SUPPRESS_UNUSED(p);
    }
private:
    static void constraints()
    {
        C       *p      =   0;          // C* is a proper pointer
        M       C::*pm  =   0;          // M C:: are meaningful class/member types
#if defined(STLSOFT_COMPILER_IS_BORLAND)
        M       &m      =   p->*pm;     // Pointer to member can be dereferenced
#else /* ? compiler */
        M const& m      =   p->*pm;     // Pointer to member can be dereferenced
#endif /* compiler */

        STLSOFT_SUPPRESS_UNUSED(m);
    }
public:
    /// \brief A copy of the base iterator
    base_iterator_type base() const
    {
        return m_it;
    }
    /// \brief A copy of the base iterator
    ///
    /// \deprecated This is replaced by base()
    base_iterator_type current() const
    {
        return m_it;
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Constructs an instance from an iterator and a member pointer
    value_type C::* member() const // NOTE: Has to be C, not selected_class_type, or VC 7.0+ spits
    {
        return m_member;
    }
/// @}

/// \name Input Iterator methods
/// @{
public:
    /// \brief Pre-increment iterator
    ///
    /// \note Increments the base iterator
    class_type& operator ++()
    {
        ++m_it;

        return *this;
    }
    /// \brief Post-increment iterator
    ///
    /// \note Increments the base iterator
    class_type operator ++(int)
    {
        class_type  r(*this);

        operator ++();

        return r;
    }
# if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
     !defined(STLSOFT_COMPILER_IS_BORLAND)
    /// \brief Returns a mutating (non-const) reference to the selected member of the base iterators current value
    reference operator *()
    {
        return (*m_it).*m_member;
    }
# endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT && !STLSOFT_COMPILER_IS_BORLAND */
    /// \brief Returns a non-mutating (const) reference to the selected member of the base iterators current value
    const_reference operator *() const
    {
        return (*m_it).*m_member;
    }

    /// Evaluates two instances for equality
    bool equal(class_type const& rhs) const
    {
        STLSOFT_ASSERT(m_member == rhs.m_member);

        return m_it == rhs.m_it;
    }
/// @}

/// \name Bidirectional Iterator methods
/// @{
public:
    /// \brief Pre-decrement iterator
    ///
    /// \note Decrements the base iterator
    class_type& operator --()
    {
        --m_it;

        return *this;
    }
    /// \brief Post-decrement iterator
    ///
    /// \note Decrements the base iterator
    class_type operator --(int)
    {
        class_type  r(*this);

        operator --();

        return r;
    }
/// @}

/// \name Random-Access Iterator methods
/// @{
public:
    /// \brief Moves the iterator forward
    ///
    /// \param n The amount by which to increment the iterator's current position
    class_type& operator +=(difference_type n)
    {
        m_it += n;

        return *this;
    }

    /// \brief Moves the iterator backward
    ///
    /// \param n The amount by which to decrement the iterator's current position
    class_type& operator -=(difference_type n)
    {
        m_it -= n;

        return *this;
    }

    /// \brief Access the element at the given index
    ///
    /// \param index The required offset from the iterator's position
    value_type& operator [](difference_type index)
    {
        return m_it[index];
    }

    /// \brief Access the element at the given index
    ///
    /// \param index The required offset from the iterator's position
    value_type const& operator [](difference_type index) const
    {
        return m_it[index];
    }

    /// \brief Calculate the distance between \c this and \c rhs
    difference_type distance(class_type const& rhs) const
    {
        return m_it - rhs.m_it;
    }
/// @}

/// \name Members
/// @{
private:
    base_iterator_type  m_it;
    M               C:: *m_member;
/// @}
};

#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW)

# if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
     STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1

template <ss_typename_param_k I>
struct msi_iterator_traits
{
public:
    enum
    {
//      is_const    =   base_type_traits<typename I::pointer>::is_const
        // This doesn't work, because some libs (Dinkumware) don't have member 'pointer'

//      is_const    =   base_type_traits<typename I::reference>::is_const
        // This doesn't work because the nested class list::iterator does not have a 'reference'
        // type, even though op *() is declared as 'reference operator*() const'. The 'reference'
        // here is of the outer class. This is a good example of why you should:

        // NOTE: == BEST PRACTICE: Always define the types you're using as member types of the type where
        // they're being used! ==

//      is_const    =   base_type_traits<(I::operator*())>::is_const

//      is_const    =   base_type_traits<typename I::pointer_type>::is_const
        // This doesn't work because std::list<>::iterator derives from std::list<>::const_iterator derives
        // from std::_Bidit<> derives from std::iterator<>
        //
        // Neither std::list<>::const_iterator nor std::_Bitit<> define any members of their own, so we're
        // left only with std::iterator<>, which contains only iterator_category, value_type, and
        // distance_type.

        is_const    =   Dinkumware_iterator_traits<I>::is_const
    };
};

template<ss_typename_param_k T>
struct msi_iterator_traits<T*>
{
    enum { is_const = 0 };
};
template<ss_typename_param_k T>
struct msi_iterator_traits<T const*>
{
    enum { is_const = 1 };
};
template<ss_typename_param_k T>
struct msi_iterator_traits<T volatile*>
{
    enum { is_const = 0 };
};
template<ss_typename_param_k T>
struct msi_iterator_traits<T const volatile*>
{
    enum { is_const = 1 };
};
#endif /* dinkumware */


/** \brief Traits class used for specifying sub-types for the member_selector()
 * creator function(s)
 *
 * \ingroup group__library__iterators
 */
template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
struct msi_traits
{
private:
    typedef member_selector_iterator<I, C, const M>             const_msi_type;
    typedef member_selector_iterator<I, C, M>                   non_const_msi_type;
#if !defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) || \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION >= STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
    typedef ss_typename_type_k std::iterator_traits<I>::pointer tested_member_type;

    enum
    {
        is_const    =   base_type_traits<tested_member_type>::is_const
    };
#endif /* dinkumware */
public:

#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION < STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1
    typedef ss_typename_type_k select_first_type_if<    const_msi_type
                                                    ,   non_const_msi_type
                                                    ,   msi_iterator_traits<I>::is_const
                                                    >::type             type;
#else /* ? dinkumware */
    typedef ss_typename_type_k select_first_type_if<    const_msi_type
                                                    ,   non_const_msi_type
#if 0
                                                    ,   /* base_type_traits<ss_typename_type_k std::iterator_traits<I>::value_type>::is_const ||
                                                         */base_type_traits<tested_member_type>::is_const/*  ||
                                                        base_type_traits<M>::is_const */
#else /* ? 0 */
                                                    ,   is_const
#endif /* 0 */

                                                    >::type             type;
#endif /* dinkumware */
};


// Need to identify the conditions under which the iterator needs to define the
// value type to be const:
//
// - M is const
// - I's value_type is const
//     - alas

// Solution 1:
//
//  + is_const_type<iterator_traits<I>::value_type>
//
//  - some libraries do not make define, for iterator_traits<T const*>, value_type as const T, but just T
//  - doesn't cater for case where the member M is constant, even if iterator is mutable (e.g. C* not C const*)
//
// Solution 2:
//
//  + is_const_type<iterator_traits<I>::value_type> ||
//    base_type_traits<typename std::iterator_traits<I>::pointer>::is_const ||
//    is_const_type<M>
//
//  - at this point we move into msi_traits
//
//  - doesn't work with good compilers with crufty libraries (e.g. Intel 8 with VC++ Dinkumware libs)
//
// Solution 3:
//
//   use msi_iterator_traits
//
//
// Solution final:
//
//  + use member types within msi_traits to simplify for readers
//  + use msi_iterator_traits for member-detecting compilers with Dinkumware (pre 7.1)
//  + select the general version for DMC++
//  + proscribe particular functionality for Borland and Visual C++ (pre 7.1)

/** \brief Creator function for member_selector_iterator
 *
 * \ingroup group__library__iterators
 *
 * \param it The iterator whose values will be subject to member selection
 * \param member The member pointer which will be used in the selection
 *
 * \return An instance of a suitable specialisation of member_selector_iterator
 */
template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline ss_typename_type_ret_k msi_traits<I, C, M>::type member_selector(I it, M C::*member)
{
    typedef ss_typename_type_k msi_traits<I, C, M>::type    iterator_t;

#if !defined(STLSOFT_COMPILER_IS_DMC) && \
    (   !defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) || \
        STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION >= STLSOFT_CF_DINKUMWARE_VC_VERSION_7_1)
    STLSOFT_STATIC_ASSERT((int)is_const_type<ss_typename_type_k std::iterator_traits<I>::value_type>::value == (int)base_type_traits<ss_typename_type_k std::iterator_traits<I>::value_type>::is_const);
#endif /* !compiler */

//    STLSOFT_STATIC_ASSERT(is_const_type<iterator_t::value_type>::value == 1);

    return iterator_t(it, member);
}

#else /* ? STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

# if 0 || \
     defined(STLSOFT_COMPILER_IS_DMC)
template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<I, C, M> member_selector(I it, M C::*member)
{
    return member_selector_iterator<I, C, M>(it, member);
}
# endif /* 0 */

#if 0
/** \brief This one needed by const Struct1 (cw8)
 *
 * \ingroup group__library__iterators
 */
template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<I, C, const M> member_selector(I it, const M C::*member)
{
    return member_selector_iterator<I, C, const M>(it, member);
}
#endif /* 0 */

#if 0 || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1310) || \
    defined(STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW)
template<   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<C*, C, M> member_selector(C *it, M C::*member)
{
    return member_selector_iterator<C*, C, M>(it, member);
}
#endif /* 0 */

# if 0 /* || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1310) */ || \
    defined(STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW)
template<   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<C const*, C, const M> member_selector(C const* it, M C::*member)
{
    return member_selector_iterator<C const*, C, const M>(it, member);
}
#endif /* 0 */

#if 0
//template<   class               C
//        ,   ss_typename_param_k M
//        >
//inline member_selector_const_iterator<C const*, C, const M> member_selector(C const* it, M C::*member)
//{
//    return member_selector_const_iterator<C const*, C, const M>(it, member);
//}
#endif /* 0 */

#if 0
template<   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_const_iterator<C const*, C, const M> member_selector(C const* it, const M C::*member)
{
    return member_selector_const_iterator<C const*, C, const M>(it, member);
}
#endif /* 0 */

# endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator ==(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator !=(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return !lhs.equal(rhs);
}


template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<I, C, M> operator +(member_selector_iterator<I, C, M> const& lhs, ss_ptrdiff_t delta)
{
    return member_selector_iterator<I, C, M>(lhs.current() + delta, lhs.member());
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline member_selector_iterator<I, C, M> operator -(member_selector_iterator<I, C, M> const& lhs, ss_ptrdiff_t delta)
{
    return member_selector_iterator<I, C, M>(lhs.current() - delta, lhs.member());
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline ss_ptrdiff_t operator -(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.distance(rhs);
}


template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator <(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.distance(rhs) < 0;
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator <=(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.distance(rhs) <= 0;
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator >(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.distance(rhs) > 0;
}

template<   ss_typename_param_k I
        ,   class               C
        ,   ss_typename_param_k M
        >
inline bool operator >=(member_selector_iterator<I, C, M> const& lhs, member_selector_iterator<I, C, M> const& rhs)
{
    return lhs.distance(rhs) >= 0;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/member_selector_iterator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_MEMBER_SELECTOR_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
