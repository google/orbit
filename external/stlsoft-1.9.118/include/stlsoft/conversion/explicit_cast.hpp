/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/explicit_cast.hpp (originally MTExCast.h, ::SynesisStl)
 *
 * Purpose:     Class to provide explicit cast operators.
 *
 * Created:     20th September 2002
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


/** \file stlsoft/conversion/explicit_cast.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::explicit_cast cast class
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST
#define STLSOFT_INCL_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST_MAJOR     4
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST_MINOR     0
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST_REVISION  1
# define STLSOFT_VER_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST_EDIT      36
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

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class explicit_cast

/** \brief This class is used to provide explicit casting operators.
 *
 * \ingroup group__library__conversion
 *
 * \param T The underlying type of the cast
 */
template <ss_typename_param_k T>
class explicit_cast
{
public:
    typedef T                   value_type;
    typedef explicit_cast<T>    class_type;

// Construction
public:
    explicit_cast(T t)
        : m_t(t)
    {}

#ifndef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
    // For compilers that do not support partial specialisation we need to
    // enforce the constraint that only basic types may be used.
    ~explicit_cast() stlsoft_throw_0()
    {
        union must_be_basic_type
        {
            int i;
            T   t;
        };
    }
#endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */

// Conversions
public:
    operator value_type () const
    {
        return m_t;
    }

// Members
private:
    T   m_t;
};

#ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
/** \brief Being able to cast to non-const references does not work, since
 * the compilers will refuse to apply such to a temporary.
 *
 * \ingroup group__library__conversion
 *
 * Furthermore, it is too evil to contemplate (see Effective C++ #30)
 * so we simply hide the constructor and conversion operator.
 */
template <ss_typename_param_k T>
class explicit_cast<T &>
{
public:
    typedef T                   value_type;
    typedef explicit_cast<T>    class_type;

// Construction
private:
    explicit_cast(T &);

// Conversions
private:
    operator T & ();

# if defined(STLSOFT_COMPILER_IS_GCC)
public: static void f() {}
# endif /* compiler */
};

# ifdef _STLSOFT_EXPLICIT_CAST_FORCE_ALLOW_REFERENCE_TO_CONST_UDT
/* Compiler check */
#  if defined(STLSOFT_COMPILER_IS_BORLAND) || \
      defined(STLSOFT_COMPILER_IS_DMC) || \
      defined(STLSOFT_COMPILER_IS_GCC) || \
      (  defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER == 1310) || \
      defined(STLSOFT_COMPILER_IS_WATCOM)
  // These compilers are ok
#  elif defined(STLSOFT_COMPILER_IS_INTEL) || \
      (  defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER != 1310)
  // These are not
#   error The current compiler does not properly implement it
#  else /* ? compiler */
  // These are unknown
#   error The current compiler has not been assessed as to whether it correctly translates explicit_cast for references to non-const
#  endif /* compiler */

template <ss_typename_param_k T>
class explicit_cast<T const&>
{
public:
    typedef T                   value_type;
    typedef explicit_cast<T>    class_type;

// Construction
public:
    explicit_cast(T const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator T const& () const
    {
        return m_t;
    }

// Members
private:
    T const& m_t;
};
# else /* ? _STLSOFT_EXPLICIT_CAST_FORCE_ALLOW_REFERENCE_TO_CONST_UDT */

#  ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
#   if defined(STLSOFT_COMPILER_IS_MSVC)
#    if _MSC_VER >= 1200
#     pragma warning(push)
#    endif /* _MSC_VER >= 1200 */
#    pragma warning(disable : 4512)
#   endif /* compiler */
#   ifndef STLSOFT_INCL_STLSOFT_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS
#    include <stlsoft/conversion/internal/explicit_cast_specialisations.hpp>
#   endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS */
#   if defined(STLSOFT_COMPILER_IS_MSVC)
#    if _MSC_VER >= 1200
#     pragma warning(pop)
#    else /* _MSC_VER */
#     pragma warning(disable : 4512)
#    endif /* _MSC_VER >= 1200 */
#   endif /* compiler */
#  endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

# endif /* _STLSOFT_EXPLICIT_CAST_FORCE_ALLOW_REFERENCE_TO_CONST_UDT */

/** \brief Not sure I really like this one, and reserve the right to remove it
 * but for now it stays.
 *
 * \ingroup group__library__conversion
 */
template <ss_typename_param_k T>
class explicit_cast<T *>
{
public:
    typedef T                   value_type;
    typedef explicit_cast<T>    class_type;

// Construction
public:
    explicit_cast(T *t)
        : m_t(t)
    {}

// Conversions
public:
    operator T * ()
    {
        return m_t;
    }

// Members
private:
    T   *m_t;
};

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
class explicit_cast<T const*>
{
public:
    typedef T                   value_type;
    typedef explicit_cast<T>    class_type;

// Construction
public:
    explicit_cast(T const* t)
        : m_t(t)
    {}

// Conversions
public:
    operator T const* () const
    {
        return m_t;
    }

// Members
private:
    T const* m_t;
};

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#endif // STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
