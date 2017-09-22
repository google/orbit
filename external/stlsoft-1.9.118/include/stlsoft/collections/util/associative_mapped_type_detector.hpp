/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/collections/util/associative_mapped_type_detector.hpp
 *
 * Purpose:     Definition of the associative_mapped_type_detector type.
 *
 * Created:     26th February 2005
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


/** \file stlsoft/collections/util/associative_mapped_type_detector.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::associative_mapped_type_detector
 *   class template
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR
#define STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR_MAJOR    3
# define STLSOFT_VER_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR_MINOR    0
# define STLSOFT_VER_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR_REVISION 1
# define STLSOFT_VER_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR_EDIT     25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1310
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
#ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_MAPPED_TYPE
# include <stlsoft/meta/typefixer/mapped_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_MAPPED_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENT_TYPE
# include <stlsoft/meta/typefixer/referent_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_TYPEFIXER_HPP_REFERENT_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS
# include <stlsoft/meta/member_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_MEMBER_TRAITS */

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

/** \brief Detector traits class template for determining the member types of
 * associative containers
 *
 * \ingroup group__library__collections
 */
template <ss_typename_param_k T>
struct associative_mapped_type_detector
{
private:
    enum { HAS_REFERENT_TYPE    =   0 != member_traits<T>::has_member_referent_type   };

    typedef ss_typename_type_k ::stlsoft::typefixer::fixer_mapped_type<T, 0 == HAS_REFERENT_TYPE>::mapped_type    mapped_type_;   // The container's mapped_type
    typedef ss_typename_type_k ::stlsoft::typefixer::fixer_referent_type<T, HAS_REFERENT_TYPE>::referent_type     referent_type_; // The container's referent_type

public:
    typedef ss_typename_type_k select_first_type_if<referent_type_
                                                ,   mapped_type_
                                                ,   HAS_REFERENT_TYPE
                                                >::type                 mapped_type;
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_ASSOCIATIVE_MAPPED_TYPE_DETECTOR */

/* ///////////////////////////// end of file //////////////////////////// */
