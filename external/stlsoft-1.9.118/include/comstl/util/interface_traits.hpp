/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/interface_traits.hpp
 *
 * Purpose:     Interface traits.
 *
 * Created:     25th May 2002
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


/** \file comstl/util/interface_traits.hpp
 *
 * \brief [C++ only] Definition of the comstl::IID_traits traits class
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS
#define COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_HPP_INTERFACE_TRAITS_MAJOR      5
# define COMSTL_VER_COMSTL_UTIL_HPP_INTERFACE_TRAITS_MINOR      0
# define COMSTL_VER_COMSTL_UTIL_HPP_INTERFACE_TRAITS_REVISION   1
# define COMSTL_VER_COMSTL_UTIL_HPP_INTERFACE_TRAITS_EDIT       57
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler discrimination
 */

#ifdef COMSTL_UUIDOF_SUPPORTED
# undef COMSTL_UUIDOF_SUPPORTED
#endif /* COMSTL_UUIDOF_SUPPORTED */

#if !defined(_COMSTL_NO_UUIDOF) && \
    (   defined(STLSOFT_COMPILER_IS_BORLAND) ||  \
        defined(STLSOFT_COMPILER_IS_INTEL) ||  \
        defined(STLSOFT_COMPILER_IS_MWERKS) ||  \
        (   defined(STLSOFT_COMPILER_IS_MSVC) && \
            _MSC_VER >= 1200))
# define COMSTL_UUIDOF_SUPPORTED
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define COMSTL_IID_TRAITS_DEFINE__(I, T)   \
            STLSOFT_TEMPLATE_SPECIALISATION \
            struct IID_traits<T> \
            { \
            public: \
                static REFIID   iid() { return IID_##I; } \
            };

# define COMSTL_IID_TRAITS_DEFINE_NS_(I, T, NS)   \
            STLSOFT_TEMPLATE_SPECIALISATION \
            struct IID_traits<NS::T> \
            { \
            public: \
                static REFIID   iid() { return NS::IID_##I; } \
            };

# define _COMSTL_IID_TRAITS_DEFINE(I)       COMSTL_IID_TRAITS_DEFINE__(I, I)

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \brief Interface identifier traits
 *
 * \ingroup group__library__utility__com
 *
 * This traits class takes an interface type and provides a specialisation that
 * has a statid iid() method which returns the interface identifier (IID) for
 * that type.
 *
 * To use, simply have the expression IID_traits<I>::iid() where I is your
 * interface type.
 *
 * On compilers that support the __uuidof pseudo-operator this is used,
 * otherwise you must specify specialisations - using the
 * COMSTL_IID_TRAITS_DEFINE macro - for your interfaces, as in
 *
 *   COMSTL_IID_TRAITS_DEFINE(IMyInterface)
 *
 * All the interfaces currently defined in unknown.idl and objidl.idl are
 * so defined in the file comstl_interface_traits_std.h, which is included in
 * for non-__uuidof compilations.
 *
 * \param I The interface
 */
template <class I>
struct IID_traits
{
public:
    /// Returns a reference to the IID for the parameterising interface
    static REFIID   iid();
};
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

# ifdef COMSTL_UUIDOF_SUPPORTED

template <class I>
struct IID_traits
{
public:
    static REFIID   iid() { return __uuidof(I); }
};

#  define COMSTL_IID_TRAITS_DEFINE(I)
#  define COMSTL_IID_TRAITS_DEFINE_NS(I, NS)

# else /* ? COMSTL_UUIDOF_SUPPORTED */

template <class I>
struct IID_traits;

#  define COMSTL_IID_TRAITS_DEFINE(I)           EXTERN_C const IID IID_##I; \
                                                _COMSTL_IID_TRAITS_DEFINE(I) \
                                                COMSTL_IID_TRAITS_DEFINE__(I, I*)

#  define COMSTL_IID_TRAITS_DEFINE_NS(I, NS)    COMSTL_IID_TRAITS_DEFINE_NS_(I, I, NS) \
                                                COMSTL_IID_TRAITS_DEFINE_NS_(I, I*, NS)

/* For backwards compatibility */
#  define comstl_IID_traits_define(I)   COMSTL_IID_TRAITS_DEFINE(I)

# endif /* COMSTL_UUIDOF_SUPPORTED */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

#ifndef COMSTL_INCL_COMSTL_INTERNAL_HPP_INTERFACE_TRAITS_STD
# include <comstl/internal/interface_traits_std.hpp>
#endif /* !COMSTL_INCL_COMSTL_INTERNAL_HPP_INTERFACE_TRAITS_STD */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
