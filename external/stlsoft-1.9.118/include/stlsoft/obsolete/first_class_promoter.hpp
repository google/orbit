/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/first_class_promoter.hpp
 *
 * Purpose:     Class template that allows built-in & aggregate types to be treated as 1st-class types.
 *
 * Created:     8th September 2002
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


/// \file stlsoft/obsolete/first_class_promoter.hpp
///
/// Class template that allows built-in & aggregate types to be treated as 1st-class types.

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER_MAJOR     4
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER_MINOR     0
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER_REVISION  4
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER_EDIT      51
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>      // for yes_type, no_type
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE
# include <stlsoft/meta/is_fundamental_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>                    // for memset()
#endif /* !STLSOFT_INCL_H_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * TMP worker classes / functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifndef _STLSOFT_NO_NAMESPACE
namespace first_class_promotion
{
#endif /* _STLSOFT_NO_NAMESPACE */


template<ss_bool_t INIT>
struct first_class_promoter_init_traits
{
    typedef yes_type    type;
};

STLSOFT_TEMPLATE_SPECIALISATION
struct first_class_promoter_init_traits<false>
{
    typedef no_type     type;
};


template<ss_typename_param_k T>
inline void first_class_promotion_do_init(T *t, yes_type, yes_type)
{
    *t = T();
}

template<ss_typename_param_k T>
inline void first_class_promotion_do_init(T *t, yes_type, no_type)
{
    ::memset(t, 0, sizeof(T));
}

template<ss_typename_param_k T>
inline void first_class_promotion_do_init(T *, no_type, yes_type)
{}

template<ss_typename_param_k T>
inline void first_class_promotion_do_init(T *, no_type, no_type)
{}

template<   ss_typename_param_k T
        ,   ss_bool_t           INIT
        >
struct first_class_promoter_traits
{
    typedef ss_typename_type_k first_class_promoter_init_traits<INIT>::type                             do_init_yesno_type;
#if defined(STLSOFT_COMPILER_IS_BORLAND) && \
    __BORLANDC__ < 0x0564
    enum { val = is_fundamental_type<T>::value };
    typedef first_class_promoter_init_traits<val>::type    is_fundamental_yesno_type;
#else /* ? compiler */
    typedef ss_typename_type_k first_class_promoter_init_traits<is_fundamental_type<T>::value>::type    is_fundamental_yesno_type;
#endif /* compiler */

    static void initialise(T *value)
    {
        first_class_promotion_do_init(value, do_init_yesno_type(), is_fundamental_yesno_type());
    }
};

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace first_class_promotion
#endif /* _STLSOFT_NO_NAMESPACE */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class first_class_promoter

/** \brief Promotes a basic type to a first-class type
 *
 * \ingroup group__library__obsolete
 *
 * This type can be used to promote a basic type (e.g. <code>int</code>) or a type that
 * cannot be used as the base of an inheritance relationship (such as a union)
 * to first class status.
 *
 * \param T The basic type
 *
 */
template<   ss_typename_param_k T
        ,   ss_bool_t           INIT    =   false
        >
class first_class_promoter
{
public:
    /// The value type
    typedef T                       value_type;
    /// The type of the current parameterisation
    typedef first_class_promoter<T> class_type;
    /// The pointer type
    typedef T*                      pointer;
    /// The non-mutating (const) pointer type
    typedef T const*                const_pointer;
    /// The reference type
    typedef T&                      reference;
    /// The non-mutating (const) reference type
    typedef T const&                const_reference;

// Construction
public:
    /// Default constructor
    ///
    /// \note The internal member of the \c value_type is <i><b>not</b></i>
    /// initialised, for efficiency, if INIT is false (the default)
    first_class_promoter()
    {
#ifndef _STLSOFT_NO_NAMESPACE
        using namespace first_class_promotion;
#endif /* _STLSOFT_NO_NAMESPACE */

        first_class_promoter_traits<T, INIT>::initialise(&m_value);
    }

    /// Copy constructor
    first_class_promoter(class_type const& rhs)
        : m_value(rhs.m_value)
    {}

    /// Initialise an instance from an instance of the promoted type
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k U>
    ss_explicit_k first_class_promoter(U& value)
        : m_value(value)
    {}
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    ss_explicit_k first_class_promoter(value_type const& value)
        : m_value(value)
    {}
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

    /// Destructor
    ~first_class_promoter() stlsoft_throw_0()
    {
        // This class shouldn't be used for 1st class types, so constrain to
        // non-class types, or trivial class types.
        //
        // It is put in a static assert merely to persuade recalcitrant
        // compilers to desist
        stlsoft_constraint_must_be_pod(value_type);

        // Check the assumption that this veneer is of zero size. The runtime
        // assert is included for those compilers that do not implement
        // compile-time asserts.
        STLSOFT_STATIC_ASSERT(sizeof(class_type) == sizeof(value_type));
#if defined(STLSOFT_COMPILER_IS_WATCOM)
        STLSOFT_ASSERT(sizeof(class_type) == sizeof(value_type));
#else /* ? compiler */
        STLSOFT_MESSAGE_ASSERT("first_class_promoter used for inappropriate type", sizeof(class_type) == sizeof(value_type));
#endif /* compiler */
    }

    /// Copy assignment operator
    class_type& operator =(class_type const& rhs)
    {
        m_value = rhs.m_value;

        return *this;
    }

    /// Assignment operator, taking an instance of the promoted type
    class_type& operator =(value_type const& value)
    {
        m_value = value;

        return *this;
    }

// Accessors
public:
    /// Provides a non-mutable (const) reference to the promoted type member
    const_reference base_type_value() const
    {
        return m_value;
    }
    /// Provides a mutable reference to the promoted type member
    reference base_type_value()
    {
        return m_value;
    }

// Operators
public:
    /// Implicit conversion operator to a reference to the promoted type member
    operator reference()
    {
        return m_value;
    }

    /// Implicit conversion operator to a non-mutable (const) reference to the promoted type member
    operator const_reference() const
    {
        return m_value;
    }

    /// Address-of operator, providing pointer access to the promoted type member
    pointer operator &()
    {
        return &m_value;
    }

    /// Address-of operator, providing non-mutable (const) pointer access to the promoted type member
    const_pointer operator &() const
    {
        return &m_value;
    }

// Members
private:
    T   m_value;
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FIRST_CLASS_PROMOTER */

/* ///////////////////////////// end of file //////////////////////////// */
