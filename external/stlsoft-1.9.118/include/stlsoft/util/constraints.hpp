/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/constraints.hpp (originally MTAlgo.h, ::SynesisStl)
 *
 * Purpose:     Compile-time template constraints templates.
 *
 * Created:     19th November 1998
 * Updated:     11th August 2010
 *
 * Thanks:      To Peter Bannister for having the clear thinking to see the
 *              obvious (but only in hindsight) tactic of overloading the
 *              constraints method in must_be_derived.
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


/** \file stlsoft/util/constraints.hpp
 *
 * \brief [C++ only] Definition of compile-time template constraints
 *   templates
 *   (\ref group__library__utility__constraints "Constraints" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_CONSTRAINTS_MAJOR      5
# define STLSOFT_VER_STLSOFT_UTIL_HPP_CONSTRAINTS_MINOR      0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_CONSTRAINTS_REVISION   4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_CONSTRAINTS_EDIT       99
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_SIZE_OF
# include <stlsoft/meta/size_of.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_SIZE_OF */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MWERKS)
# define stlsoft_constraint_must_be_pod(T)              do { stlsoft_ns_qual(must_be_pod)<T>::func_ptr_type const pfn = stlsoft_ns_qual(must_be_pod)<T>::constraint(); STLSOFT_SUPPRESS_UNUSED(pfn); } while(0)
# define stlsoft_constraint_must_be_pod_or_void(T)      do { stlsoft_ns_qual(must_be_pod_or_void)<T>::func_ptr_type const pfn = stlsoft_ns_qual(must_be_pod_or_void)<T>::constraint(); STLSOFT_SUPPRESS_UNUSED(pfn); } while(0)
#elif defined(STLSOFT_COMPILER_IS_DMC)
# define stlsoft_constraint_must_be_pod(T)              do { int i = sizeof(stlsoft_ns_qual(must_be_pod)<T>::constraint()); } while(0)
# define stlsoft_constraint_must_be_pod_or_void(T)      do { int i = sizeof(stlsoft_ns_qual(must_be_pod_or_void)<T>::constraint()); } while(0)
#else /* ? compiler */
# define stlsoft_constraint_must_be_pod(T)              STLSOFT_STATIC_ASSERT(sizeof(stlsoft_ns_qual(must_be_pod)<T>::constraint()) != 0)
# define stlsoft_constraint_must_be_pod_or_void(T)      STLSOFT_STATIC_ASSERT(sizeof(stlsoft_ns_qual(must_be_pod_or_void)<T>::constraint()) != 0)
#endif /* compiler */

# define stlsoft_constraint_must_be_same_size(T1, T2)   static_cast<void>(stlsoft_ns_qual(must_be_same_size)<T1, T2>())
# define stlsoft_constraint_must_be_subscriptable(T)    static_cast<void>(stlsoft_ns_qual(must_be_subscriptable)<T>())
# define stlsoft_constraint_must_have_base(D, B)        static_cast<void>(stlsoft_ns_qual(must_have_base)<D, B>())
# define stlsoft_constraint_must_be_derived(D, B)       static_cast<void>(stlsoft_ns_qual(must_be_derived)<D, B>())

/* /////////////////////////////////////////////////////////////////////////
 * Constraints
 */

/** \brief Constraint to ensure that the one type is convertible to another via inheritance
 *
 * \ingroup group__library__utility__constraints
 *
 * \param D The derived type
 * \param B The base type
 *
 * It may be used as follows:
\code
  class Parent {};
  class Child : public Parent {};
  class Orphan {};

  stlsoft::must_have_base<Parent, Parent>();  // Ok
  stlsoft::must_have_base<Child, Parent>();   // Ok
  stlsoft::must_have_base<Child, Child>();    // Ok
  stlsoft::must_have_base<Child, Parent>();   // Compile error!

  stlsoft::must_have_base<Orphan, Parent>();  // Compile error!
  stlsoft::must_have_base<Orphan, Child>();   // Compile error!
  stlsoft::must_have_base<Parent, Orphan>();  // Compile error!
  stlsoft::must_have_base<Child, Orphan>();   // Compile error!
\endcode
 *
 * \note This is borrowed from Bjarne Stroustrup's idea as posted to comp.lang.c++.moderated
 *   17th February 2001.
 *
 * \see stlsoft::must_be_derived
 */
// [[synesis:class:constraint:must_have_base<T<D>, T<B>>]]
template<   ss_typename_param_k D
        ,   ss_typename_param_k B
        >
struct must_have_base
{
public:
    ~must_have_base() stlsoft_throw_0()
    {
        void(*p)(D*, B*) = constraints;

        STLSOFT_SUPPRESS_UNUSED(p);
    }

private:
    static void constraints(D* pd, B* pb)
    {
        pb = pd;

        STLSOFT_SUPPRESS_UNUSED(pb);
    }
};


/** \brief Constraint to ensure that the one type is convertible to another via inheritance,
 * but is not the same type
 *
 * \ingroup group__library__utility__constraints
 *
 * \param D The derived type
 * \param B The base type
 *
 * It may be used as follows:
\code
  class Parent {};
  class Child : public Parent {};
  class Orphan {};

  stlsoft::must_be_derived<Parent, Parent>(); // Compile error!
  stlsoft::must_be_derived<Child, Parent>();  // Ok
  stlsoft::must_be_derived<Child, Child>();   // Compile error!
  stlsoft::must_have_base<Child, Parent>();   // Compile error!

  stlsoft::must_be_derived<Orphan, Parent>(); // Compile error!
  stlsoft::must_be_derived<Orphan, Child>();  // Compile error!
  stlsoft::must_be_derived<Parent, Orphan>(); // Compile error!
  stlsoft::must_be_derived<Child, Orphan>();  // Compile error!
\endcode
 *
 * \note This extension to the must_have_base constraint was proposed by
 * Peter Bannister after reading Chapter 1 of \ref section__publishing__books__imperfectcplusplus.
 *
 * \see stlsoft::must_have_base
 */
// [[synesis:class:constraint:must_be_derived<T<D>, T<B>>]]
template<   ss_typename_param_k D
        ,   ss_typename_param_k B
        >
struct must_be_derived
{
public:
    ~must_be_derived() stlsoft_throw_0()
    {
# if defined(STLSOFT_COMPILER_IS_BORLAND)
        cant_be_overloaded_if_same_type(static_cast<D*>(0)
                                    ,   static_cast<B*>(0));
# else /* ? compiler */
        void(*p)(D*, B*) = cant_be_overloaded_if_same_type;

        STLSOFT_SUPPRESS_UNUSED(p);
# endif /* compiler */
    }

private:
    static void cant_be_overloaded_if_same_type(D* pd, B* pb)
    {
        pb = pd;

        STLSOFT_SUPPRESS_UNUSED(pb);
    }
    static void cant_be_overloaded_if_same_type(B* pb, D* pd)
    {
        pb = pd;

        STLSOFT_SUPPRESS_UNUSED(pb);
    }
};



/** \brief Constrains two types to be of the same size
 *
 * \ingroup group__library__utility__constraints
 *
 * \param T1 The first type
 * \param T2 The second type
 *
 * It may be used as follows:
\code
  stlsoft::must_be_same_size<long, long>(); // Ok
  stlsoft::must_be_same_size<char, long>(); // Compile error!
  stlsoft::must_be_same_size<long, char>(); // Compile error!
\endcode
 *
 */
// [[synesis:class:constraint:must_be_same_size<T<T1>, T<T2>>]]
template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
struct must_be_same_size
{
    ~must_be_same_size() stlsoft_throw_0()
    {
        void    (*pfn)(void) = constraints;

        STLSOFT_SUPPRESS_UNUSED(pfn);
    }
private:
    static void constraints()
    {
        // The compiler will bring you here if T1 and T2 are not the same
        // size.
        struct must_be_same_size_
        {
            int T1_must_be_same_size_as_T2 : (static_cast<int>(size_of<T1>::value) == static_cast<int>(size_of<T2>::value));
        };

        const int   T1_must_be_same_size_as_T2    = (static_cast<int>(size_of<T1>::value) == static_cast<int>(size_of<T2>::value));
        int         i[T1_must_be_same_size_as_T2];

        STLSOFT_SUPPRESS_UNUSED(i);
    }
};


/** \brief Constraint to enforce that a given type is an array, or pointer, or user defined type
 * which is amenable to subsripting (i.e. defines <code>operator[]</code> or <code>operator X*()</code>)
 *
 * \ingroup group__library__utility__constraints
 *
 * \param T The type to be constrained
 *
 * It may be used as follows:
\code
  typedef std::vector<int>  vec_t;

  stlsoft::must_be_subscriptable<int*>();   // Ok
  stlsoft::must_be_subscriptable<vec_t>();  // Ok
  stlsoft::must_be_subscriptable<double>(); // Compile error!
\endcode
 *
 * \see stlsoft::must_subscript_as_decayable_pointer
 */
// [[synesis:class:constraint:must_be_subscriptable<T<T>>]]
template <ss_typename_param_k T>
struct must_be_subscriptable
{
public:
    ~must_be_subscriptable() stlsoft_throw_0()
    {
        int (*pfn)(T const&) = constraints;

        STLSOFT_SUPPRESS_UNUSED(pfn);
    }
private:
    static int constraints(T const& T_is_not_subscriptable)
    {
        // The compiler will bring you here if T is not subscriptable
        return sizeof(T_is_not_subscriptable[0]);
    }
};

/** \brief Constraint to enforce that a given type is an actual array or pointer, rather than
 * a user-defined type with a subscript operator.
 *
 * \ingroup group__library__utility__constraints
 *
 * \param T The type to be constrained
 *
 * It may be used as follows:
\code
  typedef std::vector<int>  vec_t;

  stlsoft::must_subscript_as_decayable_pointer<int*>();   // Ok
  stlsoft::must_subscript_as_decayable_pointer<vec_t>();  // Compile error!
  stlsoft::must_subscript_as_decayable_pointer<double>(); // Compile error!
\endcode
 *
 * \see stlsoft::must_be_subscriptable
 */
// [[synesis:class:constraint:must_subscript_as_decayable_pointer<T<T>>]]
template <ss_typename_param_k T>
struct must_subscript_as_decayable_pointer
{
public:
    ~must_subscript_as_decayable_pointer() stlsoft_throw_0()
    {
        ss_size_t   (*pfn)(T const&) = constraints;

        STLSOFT_SUPPRESS_UNUSED(pfn);
    }
private:
    static ss_size_t constraints(T const& T_is_not_decay_subscriptable)
    {
        // The compiler will bring you here if T has a user-defined
        // subscript operator.
        return sizeof(0[T_is_not_decay_subscriptable]);
    }
};


/** \brief Constraint to ensure that a type is a built-in or trivial type.
 *
 * \ingroup group__library__utility__constraints
 *
 * \param T The type to be constrained
 *
 * This class can be used to constrain a type to be of either built-in, e.g.
 * int, or of a trivial type, i.e. aggregate types or types with publicly
 * accessible default contructors and assignment operators.
 *
 * It may be used as follows:
\code
  typedef std::vector<int>  vec_t;

  stlsoft::must_be_pod<int*>();   // Ok
  stlsoft::must_be_pod<vec_t>();  // Compile error!
  stlsoft::must_be_pod<double>(); // Ok
  stlsoft::must_be_pod<void>();   // Compile error!
\endcode
 *
 * \see stlsoft::must_be_pod_or_void
 */
// [[synesis:class:constraint:must_be_pod<T<T>>]]
template <ss_typename_param_k T>
union must_be_pod
{
private:
    typedef must_be_pod<T>  class_type;

public:
    T   t;
    int i;

    typedef int (*func_ptr_type)();

    static func_ptr_type constraint()
    {
        return constraints;
    }

    // Required by CodeWarrior
    must_be_pod()
    {}

    // Required by CodeWarrior
    ~must_be_pod() stlsoft_throw_0()
    {
        int   (*pfn)(void) = constraints;

        STLSOFT_SUPPRESS_UNUSED(pfn);
    }

private:
#if !defined(STLSOFT_COMPILER_IS_MWERKS)
    must_be_pod(class_type const&);
    class_type& operator =(class_type const&);
#endif

private:
    static int constraints()
    {
#if defined(STLSOFT_COMPILER_IS_MWERKS)
# if ((__MWERKS__ & 0xFF00) < 0x3000)
        class_type  u;
# else /* ? compiler */
        class_type  u;

        u = *static_cast<class_type*>(0);
# endif /* compiler */
#elif defined(STLSOFT_COMPILER_IS_GCC) && \
      __GNUC__ < 3
        class_type  u = *static_cast<class_type*>(0);
#else /* ? compiler */
        class_type  u = class_type();
#endif /* compiler */

        STLSOFT_SUPPRESS_UNUSED(u);

        return sizeof(u);
    }
};


/** \brief Constraint to ensure that a type is a built-in or trivial type,
 *    or is \c void.
 *
 * \ingroup group__library__utility__constraints
 *
 * \param T The type to be constrained
 *
 * This class can be used to constrain a type to be of either built-in, e.g.
 * int, or of a trivial type, i.e. aggregate types or types with publicly
 * accessible default contructors and assignment operators, or \c void.
 *
 * It may be used as follows:
\code
  typedef std::vector<int>  vec_t;

  stlsoft::must_be_pod_or_void<int*>();   // Ok
  stlsoft::must_be_pod_or_void<vec_t>();  // Compile error!
  stlsoft::must_be_pod_or_void<double>(); // Ok
  stlsoft::must_be_pod_or_void<void>();   // Ok
\endcode
 *
 * \see stlsoft::must_be_pod
 */
// [[synesis:class:constraint:must_be_pod_or_void<T<T>>]]
template <ss_typename_param_k T>
union must_be_pod_or_void
{
private:
    typedef must_be_pod_or_void<T>  class_type;

public:
    T   t;
    int i;

    typedef int (*func_ptr_type)();

    static func_ptr_type constraint()
    {
        return constraints;
    }

#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
private:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
    static int constraints()
    {
        class_type   u;

        u.i = 1;    // CodeWarrior requires this

        STLSOFT_SUPPRESS_UNUSED(u);

        return sizeof(u);
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

STLSOFT_TEMPLATE_SPECIALISATION
union must_be_pod_or_void<void>
{
    typedef int (*func_ptr_type)();

    static func_ptr_type constraint()
    {
        return static_cast<func_ptr_type>(0);   // Can't use NULL here, as Intel C++ gets snippy with NULL_v
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#if 0

/** \brief This type is used as a tag parent to hide implicit comparison of types that
 * provide implicit converion operators.
 *
 * \ingroup group__library__utility__constraints
 *
 */

#ifdef NIC_TEMPLATE_VERSION
//struct nic_null_
//{};

typedef void    nic_null_;

template <ss_typename_param_k T = nic_null_>
struct not_implicitly_comparable
    : public T
{
public:
    typedef not_implicitly_comparable<T>    class_type;

#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
private:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
#if 0
    template <ss_typename_param_k T2>
    ss_bool_t operator ==(T2 const&) const;
    template <ss_typename_param_k T2>
    ss_bool_t operator !=(T2 const&) const;
#endif /* 0 */
};

/** \brief This specialisation allows it to
 *
 * \ingroup group__library__utility__constraints
 *
 */
STLSOFT_TEMPLATE_SPECIALISATION
struct not_implicitly_comparable<nic_null_>
{
public:
    typedef not_implicitly_comparable<nic_null_>    class_type;

#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
private:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
#if 0
    template <ss_typename_param_k T2>
    ss_bool_t operator ==(T2 const&) const;
    template <ss_typename_param_k T2>
    ss_bool_t operator !=(T2 const&) const;
#endif /* 0 */
};

template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator ==(not_implicitly_comparable<T> const& lhs, T2 const& rhs)
{
//  return lhs.operator ==(rhs);

  lhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator ==(T2 const& lhs, not_implicitly_comparable<T> const& rhs)
{
//  return rhs.operator ==(lhs);

  rhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

#if 0
template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator ==(not_implicitly_comparable<T1> const& lhs, not_implicitly_comparable<T2> const& rhs)
{
//  return rhs.operator ==(lhs);

  rhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}
#endif /* 0 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator !=(T2 const& lhs, not_implicitly_comparable<T> const& rhs)
{
//  return rhs.operator !=(lhs);

  rhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator !=(not_implicitly_comparable<T> const& lhs, T2 const& rhs)
{
//  return lhs.operator !=(rhs);

  lhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

template<   ss_typename_param_k T1
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator !=(not_implicitly_comparable<T1> const& lhs, not_implicitly_comparable<T2> const& rhs)
{
//  return lhs.operator !=(rhs);

  lhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

#else /* ? 0 */

struct not_implicitly_comparable
{
public:
    typedef not_implicitly_comparable   class_type;

#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
private:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */

#ifndef NIC_EXTERNAL_OPERATORS
    ss_bool_t operator ==(class_type const&) const;
    ss_bool_t operator !=(class_type const&) const;

    template <ss_typename_param_k T2>
    ss_bool_t operator ==(T2 const&) const;
    template <ss_typename_param_k T2>
    ss_bool_t operator !=(T2 const&) const;
#endif /* !NIC_EXTERNAL_OPERATORS */
};


#ifdef NIC_EXTERNAL_OPERATORS

template<   ss_typename_param_k T2
        >
inline ss_bool_t operator ==(not_implicitly_comparable const& lhs, T2 const& rhs)
{
//  return lhs.operator ==(rhs);

  lhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}

template<   ss_typename_param_k T2
        >
inline ss_bool_t operator ==(T2 const& lhs, not_implicitly_comparable const& rhs)
{
//  return rhs.operator ==(lhs);

  rhs.this_type_does_not_support_comparisons();

  return false; // Placate the eager beavers
}
#endif /* NIC_EXTERNAL_OPERATORS */

#endif /* 0 */
#endif /* 0 */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */

/* ///////////////////////////// end of file //////////////////////////// */
