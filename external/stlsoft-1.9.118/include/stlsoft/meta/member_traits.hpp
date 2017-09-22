/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/member_traits.hpp
 *
 * Purpose:     An amalgamation of meta traits that's been pending reification
 *              for too long.
 *
 * Created:     11th October 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/member_traits.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::member_traits component
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

// An amalgamation of meta traits that's been pending reification for too long.
//
// It defines the member_traits template, which is used to detect the members
// supported by a given type.

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
#define STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_MEMBER_TRAITS_MAJOR       2
# define STLSOFT_VER_STLSOFT_META_HPP_MEMBER_TRAITS_MINOR       1
# define STLSOFT_VER_STLSOFT_META_HPP_MEMBER_TRAITS_REVISION    1
# define STLSOFT_VER_STLSOFT_META_HPP_MEMBER_TRAITS_EDIT        37
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_DMC:        __DMC__ < 0x0845
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

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */

#ifndef STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED
# error This file is not compatible with compilers that do not support member type detection
#endif /* !STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_DETECTORS
# include <stlsoft/meta/detectors.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_DETECTORS */

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

/** \brief A structure that can be used to report whether various well-known
 * facets of a type are supported
 *
 * \ingroup group__library__meta
 *
 *
 * member_traits may be used in the compile-time evaluation of whether a given
 * type has certain members, as follows:
 *
\code





\endcode
 */

template<ss_typename_param_k T>
struct member_traits
{
public:
    typedef T                   value_type;
    typedef member_traits<T>    class_type;

/// \name Member Constants
/// @{
public:

#if defined(STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED)
    enum {  has_member_iterator_category    =   0 != has_iterator_category<T>::value    /*!< Indicates whether the type has an \c iterator_category member  */  };
    enum {  has_member_value_type           =   0 != has_value_type<T>::value           /*!< Indicates whether the type has a \c value_type member          */  };
    enum {  has_member_distance_type        =   0 != has_distance_type<T>::value        /*!< Indicates whether the type has a \c distance_type member       */  };
    enum {  has_member_pointer              =   0 != has_pointer<T>::value              /*!< Indicates whether the type has a \c pointer member             */  };
    enum {  has_member_pointer_type         =   0 != has_pointer_type<T>::value         /*!< Indicates whether the type has a \c pointer_type member        */  };
    enum {  has_member_iterator             =   0 != has_iterator<T>::value             /*!< Indicates whether the type has an \c iterator member           */  };
# if !defined(STLSOFT_COMPILER_IS_DMC) || \
     __DMC__ >= 0x0845
    enum {  has_member_const_iterator       =   0 != has_const_iterator<T>::value       /*!< Indicates whether the type has a \c const_iterator member      */  };
# endif /* compiler */
    enum {  has_member_const_pointer        =   0 != has_const_pointer<T>::value        /*!< Indicates whether the type has a \c const_pointer member       */  };
# if defined(STLSOFT_COMPILER_IS_MWERKS)
    enum {  has_member_reference            =   0 != has_reference<T>::value            /*!< Indicates whether the type has a \c reference member           */  };
    enum {  has_member_reference_type       =   0 != has_reference_type<T>::value       /*!< Indicates whether the type has a \c reference_type member      */  };
    enum {  has_member_const_reference      =   0 != has_const_reference<T>::value      /*!< Indicates whether the type has a \c const_reference member     */  };
# endif /* compiler */
    enum {  has_member_difference_type      =   0 != has_difference_type<T>::value      /*!< Indicates whether the type has a \c difference_type member     */  };
    enum {  has_member_key_type             =   0 != has_key_type<T>::value             /*!< Indicates whether the type has a \c key_type member            */  };
    enum {  has_member_mapped_type          =   0 != has_mapped_type<T>::value          /*!< Indicates whether the type has a \c mapped_type member         */  };
    enum {  has_member_referent_type        =   0 != has_referent_type<T>::value        /*!< Indicates whether the type has a \c referent_type member       */  };
#endif /* STLSOFT_CF_HAS_MEMBER_TYPE_SUPPORTED */
/// @}

// Not to be implemented
private:
    member_traits();
#ifdef STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR
protected:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */

    member_traits(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
