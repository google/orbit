/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/must_init.hpp
 *
 * Purpose:     Simple class that wraps a fundamental type and forces its
 *              explicit initialisation.
 *
 * Thanks:      To Josh Kelley, whose blog prompted me to fix docs, and
 *              newsgroup request prompted me to put it under test and
 *              code coverage.
 *
 * Created:     18th June 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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
 * ////////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/util/must_init.hpp
 *
 * \brief [C++ only] Definition of stlsoft::must_init class template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_MUST_INIT
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_MUST_INIT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_MUST_INIT_MAJOR       1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_MUST_INIT_MINOR       1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_MUST_INIT_REVISION    4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_MUST_INIT_EDIT        17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifdef STLSOFT_UNITTEST
# include <string>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Algorithms
 */

/** \brief Wraps a variable and forces its explicit initialisation by the
 *   user.
 *
 * \param T The base type, e.g. \c int, \c std::wstring
 *
 * \ingroup group__library__utility
 *
 * <b>Problem:</b>
 *
\code
  int             i1; // Not initialised. Compiler doesn't care!

  int             res = 2 * i1; // Result is undefined!
\endcode
 *
 * <b>Solution:</b>
 *
\code
  must_init<int>  i1; // Not initialised. Compiler error
\endcode
 *
 * The user is required to explicitly initialise <code>i1</code>:
 *
\code
  must_init<int>  i1(0); // Initialised. Everybody's happy

  int             res = 2 * i1.get(); // Result is defined
\endcode
 */

//STLSOFT_PRAGMA_PACK_PUSH(1)

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
class must_init_builtin
{
/// \name Member Types
/// @{
public:
    /// \brief The wrapped type
    typedef T                       value_type;
    /// \brief The current instantiation of the type
    typedef must_init_builtin<T>    class_type;
    /// The reference type
    typedef T&                      reference;
    /// The non-mutating (const) reference type
    typedef T const&                const_reference;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructor
    ss_explicit_k must_init_builtin(T t)
        : value(t)
    {}

    /// \brief Implicit conversion to the wrapped type
    operator value_type () const
    {
        return value;
    }

    class_type& operator =(const value_type rhs)
    {
        value = rhs;

        return *this;
    }
/// @}

/// \name Members
/// @{
public:
    /// Provides non-mutating (const) access to the base type value
    const_reference base_type_value() const
    {
        return value;
    }
    /// Provides mutating access to the base type value
    reference       base_type_value()
    {
        return value;
    }

    /// Provides non-mutating (const) access to the base type value
    const_reference get() const
    {
        return base_type_value();
    }
    /// Provides mutating access to the base type value
    reference       get()
    {
        return base_type_value();
    }
/// @}

/// \name Members
/// @{
public:
    /// \brief The underlying value
    ///
    ///  \remarks Since the purpose of must_init is to guard against a
    ///   forgotten initialisation in composition involving fundamental
    ///   types, rather than encapsulation in any wider sense, the member
    ///   value is public, to simplify manipulation of the actual value by
    ///   its encapsulating class, thereby avoiding all the
    ///   (compiler-dependent) hassles attendant with implicit conversion to
    ///   reference types.
    value_type  value;
/// @}
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k T>
class must_init
{
/// \name Member Types
/// @{
public:
    /// \brief The wrapped type
    typedef T               value_type;
    /// \brief The current instantiation of the type
    typedef must_init<T>    class_type;
    /// The reference type
    typedef T&              reference;
    /// The non-mutating (const) reference type
    typedef T const&        const_reference;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructor
    ss_explicit_k must_init(T t)
        : value(t)
    {}

    /// \brief Implicit conversion to the wrapped type
    operator value_type const& () const
    {
        return value;
    }

    class_type& operator =(value_type const &rhs)
    {
        value = rhs;

        return *this;
    }
/// @}

/// \name Members
/// @{
public:
    /// Provides non-mutating (const) access to the base type value
    const_reference base_type_value() const
    {
        return value;
    }
    /// Provides mutating access to the base type value
    reference       base_type_value()
    {
        return value;
    }

    /// Provides non-mutating (const) access to the base type value
    const_reference get() const
    {
        return base_type_value();
    }
    /// Provides mutating access to the base type value
    reference       get()
    {
        return base_type_value();
    }
/// @}

/// \name Members
/// @{
public:
    /// \brief The underlying value
    ///
    ///  \remarks Since the purpose of must_init is to guard against a
    ///   forgotten initialisation in composition involving fundamental
    ///   types, rather than encapsulation in any wider sense, the member
    ///   value is public, to simplify manipulation of the actual value by
    ///   its encapsulating class, thereby avoiding all the
    ///   (compiler-dependent) hassles attendant with implicit conversion to
    ///   reference types.
    value_type  value;
/// @}
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(type)       \
                                                            \
    STLSOFT_TEMPLATE_SPECIALISATION                         \
    class must_init<type>                                   \
        : public must_init_builtin<type>                    \
    {                                                       \
    public:                                                 \
        typedef must_init<type> class_type;                 \
    public:                                                 \
        explicit must_init(type value)                      \
            : must_init_builtin<type>(value)                \
        {}                                                  \
        class_type& operator =(const value_type rhs)        \
        {                                                   \
            value = rhs;                                    \
            return *this;                                   \
        }                                                   \
    }

# ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(bool);
# endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(char);
# ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(wchar_t);
# endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(signed char);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(unsigned char);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(short);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(unsigned short);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(int);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(unsigned int);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(long);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(unsigned long);
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(ss_sint64_t);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(ss_uint64_t);
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(float);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(double);
STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_(long double);

# undef STLSOFT_UTIL_MUST_INIT_DEFINE_BUILTIN_

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

//STLSOFT_PRAGMA_PACK_POP()

////////////////////////////////////////////////////////////////////////////
// Specialisations

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>bool</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<bool>                 bool_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>char</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<char>                 char_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>wchar_t</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<wchar_t>              wchar_t_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>signed char</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<signed char>          signed_char_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>unsigned char</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<unsigned char>        unsigned_char_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>short</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<short>                short_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>unsigned short</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<unsigned short>       unsigned_short_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>int</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<int>                  int_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>unsigned int</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<unsigned int>         unsigned_int_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>long</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<long>                 long_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>unsigned long</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<unsigned long>        unsigned_long_init_t;

#ifdef STLSOFT_CF_64BIT_INT_SUPPORT

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>ss_sint64_t</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<ss_sint64_t>          sint64_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>ss_uint64_t</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<ss_uint64_t>          uint64_init_t;

#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>float</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<float>                float_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>double</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<double>               double_init_t;

/** \brief Specialisation of \link stlsoft::must_init must_init\endlink for <code>long double</code>.
 *
 * \ingroup group__library__utility
 */
typedef must_init<long double>          long_double_init_t;

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

inline bool operator !(bool_init_t const& b)
{
    return !b.value;
}

// Pre-increment

template<   ss_typename_param_k T
        >
inline must_init<T>& operator ++(must_init<T> &v)
{
    ++v.base_type_value();

    return v;
}

// Post-increment

template<   ss_typename_param_k T
        >
inline must_init<T> const operator ++(must_init<T> &v, int)
{
    must_init<T>  r(v);

    v.base_type_value()++;

    return r;
}

// Pre-decrement

template<   ss_typename_param_k T
        >
inline must_init<T>& operator --(must_init<T> &v)
{
    --v.base_type_value();

    return v;
}

// Post-decrement

template<   ss_typename_param_k T
        >
inline must_init<T> const operator --(must_init<T> &v, int)
{
    must_init<T>  r(v);

    v.base_type_value()--;

    return r;
}

// operator ==

template<   ss_typename_param_k T
        >
inline ss_bool_t operator ==(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return lhs.base_type_value() == rhs.base_type_value();
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator ==(must_init<T> const& lhs, T const& rhs)
{
    return lhs.base_type_value() == rhs;
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator ==(T const& lhs, must_init<T> const& rhs)
{
    return lhs == rhs.base_type_value();
}


// operator !=

template<   ss_typename_param_k T
        >
inline ss_bool_t operator !=(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return ! operator ==(lhs, rhs);
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator !=(must_init<T> const& lhs, T const& rhs)
{
    return ! operator ==(lhs, rhs);
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator !=(T const& lhs, must_init<T> const& rhs)
{
    return ! operator ==(lhs, rhs);
}

// operator <

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return lhs.base_type_value() < rhs.base_type_value();
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <(must_init<T> const& lhs, T const& rhs)
{
    return lhs.base_type_value() < rhs;
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <(T const& lhs, must_init<T> const& rhs)
{
    return lhs < rhs.base_type_value();
}

// operator <=

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <=(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return lhs.base_type_value() <= rhs.base_type_value();
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <=(must_init<T> const& lhs, T const& rhs)
{
    return lhs.base_type_value() <= rhs;
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator <=(T const& lhs, must_init<T> const& rhs)
{
    return lhs <= rhs.base_type_value();
}

// operator >

template<   ss_typename_param_k T
        >
inline ss_bool_t operator >(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return lhs.base_type_value() > rhs.base_type_value();
}


#ifdef __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE
# pragma message(_sscomp_fileline_message("This had to be changed to T + T2, so as to allow comparison between different integral types"))
#endif /* __SYNSOFT_DBS_COMPILER_SUPPORTS_PRAGMA_MESSAGE */

template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline ss_bool_t operator >(must_init<T> const& lhs, T2 const& rhs)
{
    return lhs.base_type_value() > rhs;
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator >(T const& lhs, must_init<T> const& rhs)
{
    return lhs > rhs.base_type_value();
}

// operator >=

template<   ss_typename_param_k T
        >
inline ss_bool_t operator >=(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return lhs.base_type_value() >= rhs.base_type_value();
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator >=(must_init<T> const& lhs, T const& rhs)
{
    return lhs.base_type_value() >= rhs;
}

template<   ss_typename_param_k T
        >
inline ss_bool_t operator >=(T const& lhs, must_init<T> const& rhs)
{
    return lhs >= rhs.base_type_value();
}

// operator +

#if 0
template<   ss_typename_param_k T
        >
inline must_init<T> operator +(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() + rhs.base_type_value());
}
#endif /* 0 */

#if 0
template<   ss_typename_param_k T
        >
inline must_init<T> operator +(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() + rhs);
}
#else /* ? 0 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline must_init<T> operator +(must_init<T> const& lhs, T2 const& rhs)
{
    return must_init<T>(lhs.base_type_value() + rhs);
}

#endif /* 0 */

#if 0
template<   ss_typename_param_k T
        >
inline must_init<T> operator +(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs + rhs.base_type_value());
}
#else /* ? 0 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k T2
        >
inline must_init<T> operator +(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs + rhs.base_type_value());
}
#endif /* 0 */

// operator -

template<   ss_typename_param_k T
        >
inline must_init<T> operator -(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() - rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator -(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() - rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator -(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs - rhs.base_type_value());
}

// operator *

template<   ss_typename_param_k T
        >
inline must_init<T> operator *(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() * rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator *(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() * rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator *(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs * rhs.base_type_value());
}

// operator /

template<   ss_typename_param_k T
        >
inline must_init<T> operator /(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() / rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator /(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() / rhs);
}


template<   ss_typename_param_k T
        >
inline must_init<T> operator /(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs / rhs.base_type_value());
}

// operator %

template<   ss_typename_param_k T
        >
inline must_init<T> operator %(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() % rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator %(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() % rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator %(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs % rhs.base_type_value());
}

// operator ^

template<   ss_typename_param_k T
        >
inline must_init<T> operator ^(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() ^ rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator ^(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() ^ rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator ^(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs ^ rhs.base_type_value());
}

// operator ~

template<   ss_typename_param_k T
        >
inline must_init<T> operator ~(must_init<T> const& v)
{
    return must_init<T>(~v.base_type_value());
}

// operator <<

template<   ss_typename_param_k T
        >
inline must_init<T> operator <<(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() << rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator <<(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() << rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator <<(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs << rhs.base_type_value());
}

// operator >>

template<   ss_typename_param_k T
        >
inline must_init<T> operator >>(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() >> rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator >>(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() >> rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator >>(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs >> rhs.base_type_value());
}

// operator &

template<   ss_typename_param_k T
        >
inline must_init<T> operator &(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() & rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator &(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() & rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator &(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs & rhs.base_type_value());
}

// operator |

template<   ss_typename_param_k T
        >
inline must_init<T> operator |(must_init<T> const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs.base_type_value() | rhs.base_type_value());
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator |(must_init<T> const& lhs, T const& rhs)
{
    return must_init<T>(lhs.base_type_value() | rhs);
}

template<   ss_typename_param_k T
        >
inline must_init<T> operator |(T const& lhs, must_init<T> const& rhs)
{
    return must_init<T>(lhs | rhs.base_type_value());
}

// operator +=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator +=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() += rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator +=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() += rhs.base_type_value();

    return v;
}

// operator -=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator -=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() -= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator -=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() -= rhs.base_type_value();

    return v;
}

// operator *=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator *=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() *= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator *=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() *= rhs.base_type_value();

    return v;
}

// operator /=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator /=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() /= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator /=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() /= rhs.base_type_value();

    return v;
}

// operator %=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator %=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() %= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator %=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() %= rhs.base_type_value();

    return v;
}

// operator ^=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator ^=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() ^= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator ^=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() ^= rhs.base_type_value();

    return v;
}

// operator <<=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator <<=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() <<= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator <<=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() <<= rhs.base_type_value();

    return v;
}

// operator >>=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator >>=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() >>= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator >>=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() >>= rhs.base_type_value();

    return v;
}

// operator &=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator &=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() &= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator &=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() &= rhs.base_type_value();

    return v;
}

// operator |=

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator |=(must_init<T> &v, T const& rhs)
{
    v.base_type_value() |= rhs;

    return v;
}

template<   ss_typename_param_k T
        >
inline must_init<T> const& operator |=(must_init<T> &v, must_init<T> const& rhs)
{
    v.base_type_value() |= rhs.base_type_value();

    return v;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/must_init_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_MUST_INIT */

/* ///////////////////////////// end of file //////////////////////////// */
