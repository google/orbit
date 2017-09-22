/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/conversion_veneer.hpp
 *
 * Purpose:     Raw conversion veneer class.
 *
 * Created:     30th July 2002
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


/// \file stlsoft/obsolete/conversion_veneer.hpp
///
/// Raw conversion veneer class.

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER_MAJOR      3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER_MINOR      2
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER_REVISION   2
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER_EDIT       47
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

// class invalid_conversion
/** \brief Prevents any conversion
 *
 * \ingroup group__library__obsolete
 *
 * \param T The value type
 * \param C The conversion type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
        >
struct invalid_conversion
{
protected:
    /// The invalid type
    typedef void    invalid_type;
public:
    /// The value type
    typedef T       value_type;
    /// The conversion type
    typedef C       conversion_type;

public:
    /// Converts a pointer to the \c value_type to a pointer to the \c conversion_type
    static invalid_type convert_pointer(value_type * /* pv */)
    {}

    /// Converts a pointer-to-const to the \c value_type to a pointer-to-const to the \c conversion_type
    static invalid_type convert_const_pointer(value_type const* /* pv */)
    {}

    /// Converts a reference to the \c value_type to a reference to the \c conversion_type
    static invalid_type convert_reference(value_type &/* v */)
    {}

    /// Converts a reference-to-const to the \c value_type to a reference-to-const to the \c conversion_type
    static invalid_type convert_const_reference(value_type const& /* v */)
    {}

    /// Pointer conversion type
    struct pointer_conversion
    {
        invalid_type operator ()(value_type * /* pv */)
        {}
    };

    /// Pointer-to-const conversion type
    struct pointer_const_conversion
    {
        invalid_type operator ()(value_type const* /* pv */)
        {}
    };

    /// Reference conversion type
    struct reference_conversion
    {
        invalid_type operator ()(value_type &/* v */)
        {}
    };

    /// Reference-to-const conversion type
    struct reference_const_conversion
    {
        invalid_type operator ()(value_type const& /* v */)
        {}
    };

};

// class static_conversion
/** \brief Implements conversion via C++'s <code>static_cast</code>
 *
 * \ingroup group__library__obsolete
 *
 * \param T The value type
 * \param C The conversion type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
        >
struct static_conversion
{
public:
    /// The value type
    typedef T       value_type;
    /// The conversion type
    typedef C       conversion_type;

public:
    /// Converts a pointer to the \c value_type to a pointer to the \c conversion_type
    static conversion_type *convert_pointer(value_type *pv)
    {
        return static_cast<conversion_type*>(pv);
    }

    /// Converts a pointer-to-const to the \c value_type to a pointer-to-const to the \c conversion_type
    static conversion_type const* convert_const_pointer(value_type const* pv)
    {
        return static_cast<conversion_type const*>(pv);
    }

    /// Converts a reference to the \c value_type to a reference to the \c conversion_type
    static conversion_type &convert_reference(value_type &v)
    {
        return static_cast<conversion_type&>(v);
    }

    /// Converts a reference-to-const to the \c value_type to a reference-to-const to the \c conversion_type
    static conversion_type const& convert_const_reference(value_type const& v)
    {
        return static_cast<conversion_type const&>(v);
    }

    /// Pointer conversion type
    struct pointer_conversion
    {
        conversion_type *operator ()(value_type *pv)
        {
            return convert_pointer(pv);
        }
    };

    /// Pointer-to-const conversion type
    struct pointer_const_conversion
    {
        conversion_type const* operator ()(value_type const* pv)
        {
            return convert_const_pointer(pv);
        }
    };

    /// Reference conversion type
    struct reference_conversion
    {
        conversion_type& operator ()(value_type &v)
        {
            return convert_reference(v);
        }
    };

    /// Reference-to-const conversion type
    struct reference_const_conversion
    {
        conversion_type const& operator ()(value_type const& v)
        {
            return convert_const_reference(v);
        }
    };

};

// class dynamic_conversion
/** \brief Implements conversion via C++'s <code>dynamic_cast</code>
 *
 * \ingroup group__library__obsolete
 *
 * \param T The value type
 * \param C The conversion type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
        >
struct dynamic_conversion
{
public:
    /// The value type
    typedef T       value_type;
    /// The conversion type
    typedef C       conversion_type;

public:
    /// Converts a pointer to the \c value_type to a pointer to the \c conversion_type
    static conversion_type *convert_pointer(value_type *pv)
    {
        return dynamic_cast<conversion_type*>(pv);
    }

    /// Converts a pointer-to-const to the \c value_type to a pointer-to-const to the \c conversion_type
    static conversion_type const* convert_const_pointer(value_type const* pv)
    {
        return dynamic_cast<conversion_type const*>(pv);
    }

    /// Converts a reference to the \c value_type to a reference to the \c conversion_type
    static conversion_type &convert_reference(value_type &v)
    {
        return dynamic_cast<conversion_type&>(v);
    }

    /// Converts a reference-to-const to the \c value_type to a reference-to-const to the \c conversion_type
    static conversion_type const& convert_const_reference(value_type const& v)
    {
        return dynamic_cast<conversion_type const&>(v);
    }

    /// Pointer conversion type
    struct pointer_conversion
    {
        conversion_type *operator ()(value_type *pv)
        {
            return convert_pointer(pv);
        }
    };

    /// Pointer-to-const conversion type
    struct pointer_const_conversion
    {
        conversion_type const* operator ()(value_type const* pv)
        {
            return convert_const_pointer(pv);
        }
    };

    /// Reference conversion type
    struct reference_conversion
    {
        conversion_type& operator ()(value_type &v)
        {
            return convert_reference(v);
        }
    };

    /// Reference-to-const conversion type
    struct reference_const_conversion
    {
        conversion_type const& operator ()(value_type const& v)
        {
            return convert_const_reference(v);
        }
    };

};

// class reinterpret_conversion
/** \brief Implements conversion via C++'s <code>reinterpret_cast</code>
 *
 * \ingroup group__library__obsolete
 *
 * \param T The value type
 * \param C The conversion type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
        >
struct reinterpret_conversion
{
public:
    /// The value type
    typedef T       value_type;
    /// The conversion type
    typedef C       conversion_type;

public:
    /// Converts a pointer to the \c value_type to a pointer to the \c conversion_type
    static conversion_type *convert_pointer(value_type *pv)
    {
        return reinterpret_cast<conversion_type*>(pv);
    }

    /// Converts a pointer-to-const to the \c value_type to a pointer-to-const to the \c conversion_type
    static conversion_type const* convert_const_pointer(value_type const* pv)
    {
        return reinterpret_cast<conversion_type const*>(pv);
    }

    /// Converts a reference to the \c value_type to a reference to the \c conversion_type
    static conversion_type &convert_reference(value_type &v)
    {
        return reinterpret_cast<conversion_type&>(v);
    }

    /// Converts a reference-to-const to the \c value_type to a reference-to-const to the \c conversion_type
    static conversion_type const& convert_const_reference(value_type const& v)
    {
        return reinterpret_cast<conversion_type const&>(v);
    }

    /// Pointer conversion type
    struct pointer_conversion
    {
        conversion_type *operator ()(value_type *pv)
        {
            return convert_pointer(pv);
        }
    };

    /// Pointer-to-const conversion type
    struct pointer_const_conversion
    {
        conversion_type const* operator ()(value_type const* pv)
        {
            return convert_const_pointer(pv);
        }
    };

    /// Reference conversion type
    struct reference_conversion
    {
        conversion_type& operator ()(value_type &v)
        {
            return convert_reference(v);
        }
    };

    /// Reference-to-const conversion type
    struct reference_const_conversion
    {
        conversion_type const& operator ()(value_type const& v)
        {
            return convert_const_reference(v);
        }
    };

};

// class c_conversion
/** \brief Implements conversion via C-style casts
 *
 * \ingroup group__library__obsolete
 *
 * \param T The value type
 * \param C The conversion type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
        >
struct c_conversion
{
public:
    /// The value type
    typedef T       value_type;
    /// The conversion type
    typedef C       conversion_type;

public:
    /// Converts a pointer to the \c value_type to a pointer to the \c conversion_type
    static conversion_type *convert_pointer(value_type *pv)
    {
        return (conversion_type*)(pv);
    }

    /// Converts a pointer-to-const to the \c value_type to a pointer-to-const to the \c conversion_type
    static conversion_type const* convert_const_pointer(value_type const* pv)
    {
        return (conversion_type const*)(pv);
    }

    /// Converts a reference to the \c value_type to a reference to the \c conversion_type
    static conversion_type &convert_reference(value_type &v)
    {
        return (conversion_type&)(v);
    }

    /// Converts a reference-to-const to the \c value_type to a reference-to-const to the \c conversion_type
    static conversion_type const& convert_const_reference(value_type const& v)
    {
        return (conversion_type const&)(v);
    }

    /// Pointer conversion type
    struct pointer_conversion
    {
        conversion_type *operator ()(value_type *pv)
        {
            return convert_pointer(pv);
        }
    };

    /// Pointer-to-const conversion type
    struct pointer_const_conversion
    {
        conversion_type const* operator ()(value_type const* pv)
        {
            return convert_const_pointer(pv);
        }
    };

    /// Reference conversion type
    struct reference_conversion
    {
        conversion_type& operator ()(value_type &v)
        {
            return convert_reference(v);
        }
    };

    /// Reference-to-const conversion type
    struct reference_const_conversion
    {
        conversion_type const& operator ()(value_type const& v)
        {
            return convert_const_reference(v);
        }
    };

};

// class conversion_veneer
/** \brief This class allows policy-based control of the four conversions: pointer, non-mutable pointer, reference, non-mutable reference
 *
 * \param T The type that will be subjected to the <a href = "http://synesis.com.au/resources/articles/cpp/veneers.pdf">veneer</a>
 * \param C The type that T will be converted to
 * \param V The value type. On translators that support default template arguments this defaults to T.
 * \param P The type that controls the pointer conversion
 * \param R The type that controls the reference conversion
 * \param PC The type that controls the pointer-to-const conversion
 * \param RC The type that controls the reference-to-const conversion
 *
 * \ingroup concepts_veneer
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
        ,   ss_typename_param_k V = T
        ,   ss_typename_param_k P = invalid_conversion<T, C>
        ,   ss_typename_param_k R = invalid_conversion<T, C>
        ,   ss_typename_param_k PC = P
        ,   ss_typename_param_k RC = R
#else
        ,   ss_typename_param_k V
        ,   ss_typename_param_k P
        ,   ss_typename_param_k R
        ,   ss_typename_param_k PC
        ,   ss_typename_param_k RC
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        >
class conversion_veneer
    : public T
{
public:
    /// The parent class type
    typedef T                                                   parent_class_type;
    /// The conversion type
    typedef C                                                   conversion_type;
    /// The value type
    typedef V                                                   value_type;
    /// The pointer conversion type
    typedef ss_typename_type_k P::pointer_conversion            pointer_conversion_type;
    /// The reference conversion type
    typedef ss_typename_type_k R::reference_conversion          reference_conversion_type;
    /// The pointer-to-const conversion type
    typedef ss_typename_type_k PC::pointer_const_conversion     pointer_const_conversion_type;
    /// The reference-to-const conversion type
    typedef ss_typename_type_k RC::reference_const_conversion   reference_const_conversion_type;
    /// The current parameterisation of the type
    typedef conversion_veneer<T, C, V, P, R, PC, RC>            class_type;

// Construction
public:
    /// The default constructor
    conversion_veneer()
    {
        stlsoft_constraint_must_be_same_size(T, class_type);
    }

    /// The copy constructor
    conversion_veneer(class_type const& rhs)
        : parent_class_type(rhs)
    {
        stlsoft_constraint_must_be_same_size(T, class_type);
    }

    /// Initialise from a value
    conversion_veneer(value_type const& rhs)
        : parent_class_type(rhs)
    {
        stlsoft_constraint_must_be_same_size(T, class_type);
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    // For compilers that support member templates, the following constructors
    // are provided.

    /// Single parameter constructor
    template <ss_typename_param_k N1>
    ss_explicit_k conversion_veneer(N1 &n1)
        : parent_class_type(n1)
    {}
    /// Single parameter constructor
    template <ss_typename_param_k N1>
    ss_explicit_k conversion_veneer(N1 *n1)
        : parent_class_type(n1)
    {}

    /// Two parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2>
    conversion_veneer(N1 n1, N2 n2)
        : parent_class_type(n1, n2)
    {}

    /// Three parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3>
    conversion_veneer(N1 n1, N2 n2, N3 n3)
        : parent_class_type(n1, n2, n3)
    {}

    /// Four parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4>
    conversion_veneer(N1 n1, N2 n2, N3 n3, N4 n4)
        : parent_class_type(n1, n2, n3, n4)
    {}

    /// Five parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5>
    conversion_veneer(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5)
        : parent_class_type(n1, n2, n3, n4, n5)
    {}

    /// Six parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6>
    conversion_veneer(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6)
        : parent_class_type(n1, n2, n3, n4, n5, n6)
    {}

    /// Seven parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6, ss_typename_param_k N7>
    conversion_veneer(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6, N7 n7)
        : parent_class_type(n1, n2, n3, n4, n5, n6, n7)
    {}

    /// Eight parameter constructor
    template <ss_typename_param_k N1, ss_typename_param_k N2, ss_typename_param_k N3, ss_typename_param_k N4, ss_typename_param_k N5, ss_typename_param_k N6, ss_typename_param_k N7, ss_typename_param_k N8>
    conversion_veneer(N1 n1, N2 n2, N3 n3, N4 n4, N5 n5, N6 n6, N7 n7, N8 n8)
        : parent_class_type(n1, n2, n3, n4, n5, n6, n7, n8)
    {}
#endif // STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT

    /// Copy assignment operator
    class_type& operator =(class_type const& rhs)
    {
        static_cast<parent_class_type&>(*this) = rhs;

        return *this;
    }

    /// Copy from a value
    class_type& operator =(value_type const& rhs)
    {
        static_cast<parent_class_type&>(*this) = rhs;

        return *this;
    }

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /// Copy from a value
    template <ss_typename_param_k T1>
    class_type& operator =(T1 rhs)
    {
        static_cast<parent_class_type&>(*this) = rhs;

        return *this;
    }
#endif // STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT

    // Note that the copy constructor is not defined, and will NOT be defined copy ctor/operator not made

// Conversions
public:
    /// Implicit conversion to a reference to the conversion_type
    operator conversion_type &()
    {
        return reference_conversion_type()(*this);
    }
    /// Implicit conversion to a reference-to-const to the conversion_type
    operator conversion_type const& () const
    {
        return reference_const_conversion_type()(*this);
    }

    /// Address-of operator, returning a pointer to the conversion type
    conversion_type * operator &()
    {
        // Take a local reference, such that the application of the address-of
        // operator will allow user-defined conversions of the parent_class_type
        // to be applied.
        parent_class_type   &_this  =   *this;

        return pointer_conversion_type()(&_this);
    }
    /// Address-of operator, returning a pointer-to-const to the conversion type
    conversion_type const*  operator &() const
    {
        // Take a local reference, such that the application of the address-of
        // operator will allow user-defined conversions of the parent_class_type
        // to be applied.
        parent_class_type const& _this  =   *this;

        return pointer_const_conversion_type()(&_this);
    }
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_CONVERSION_VENEER */

/* ///////////////////////////// end of file //////////////////////////// */
