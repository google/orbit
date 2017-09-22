/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/error/error_desc.hpp
 *
 * Purpose:     Error .
 *
 * Created:     20th March 2005
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


/** \file platformstl/error/error_desc.hpp
 *
 * \brief [C++ only] Definition of the platformstl::basic_error_desc class
 *  template
 *   (\ref group__library__error "Error" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_ERROR_DESC
#define PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_ERROR_DESC

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_ERROR_DESC_MAJOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_ERROR_DESC_MINOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_ERROR_DESC_REVISION   1
# define PLATFORMSTL_VER_PLATFORMSTL_ERROR_HPP_ERROR_DESC_EDIT       21
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef STLSOFT_INCL_STLSOFT_HPP_ERROR_UNIX_ERROR_DESC
#  include <stlsoft/error/error_desc.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_UNIX_ERROR_DESC */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_ERROR_DESC
#  include <winstl/error/error_desc.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_ERROR_DESC */
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
namespace platformstl
{
#else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)

    /** \brief Class used for composing and decomposing file-system paths.
     *
     * \ingroup group__library__error
     *
     * The class is not actually defined in the
     * \link ::platformstl platformstl\endlink namespace. Rather, it
     * resolves to the appropriate type for the given platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::basic_error_desc
     * type resolves to the unixstl::basic_error_desc class. On Windows
     * platforms it resolves to the winstl::basic_error_desc class. It
     * is not defined for other platforms.
     */
    template <ss_typename_param_k C>
    class basic_error_desc
    {};

    /// \brief Specialisation of the basic_error_desc template for the ANSI character type \c char
    typedef basic_error_desc<char>          path_a;
    /// \brief Specialisation of the basic_error_desc template for the Unicode character type \c wchar_t
    typedef basic_error_desc<wchar_t>       path_w;
    /// \brief Specialisation of the basic_error_desc template for the ANSI character type \c char on UNIX, and for the \c TCHAR type on Windows
    typedef basic_error_desc<tchar>         path;

#elif defined(STLSOFT_COMPILER_IS_MSVC) && \
      _MSC_VER < 1310

    // Because early incarnations of Visual C++ are pretty stupid, we need to
    // work around their inability to introduce a template via using by
    // defining and deriving here
    //
    // This one required an *extra* degree of trickery, by defining a derivation
    // of unixstl::basic_error_desc _within_ unixstl, in the form of
    // basic_error_desc__, and then deriving from that. If we just try and
    // derive from unixstl::basic_error_desc directly, the compiler gets
    // confused between the unixstl one and the local one, and fails and says that
    // it cannot derive from itself (which happens to be incomplete).


    template <ss_typename_param_k C>
    class basic_error_desc
#if defined(PLATFORMSTL_OS_IS_WINDOWS)
        : public winstl_ns_qual(basic_error_desc)<C>
#else /* ? OS */
        : public stlsoft_ns_qual(basic_error_desc)<C>
#endif /* OS */
    {
    private:
#if defined(PLATFORMSTL_OS_IS_WINDOWS)
        typedef winstl_ns_qual(basic_error_desc)<C>                     parent_class_type;
#else /* ? OS */
        typedef stlsoft_ns_qual(basic_error_desc)<C>                    parent_class_type;
#endif /* OS */
    public:
        typedef basic_error_desc<C>                                     class_type;
        typedef ss_typename_type_k parent_class_type::size_type         size_type;
        typedef ss_typename_type_k parent_class_type::error_type        error_type;
    public:
        basic_error_desc()
            : parent_class_type()
        {}
        ss_explicit_k basic_error_desc(error_type error)
            : parent_class_type(error)
        {}
    };

    typedef basic_error_desc<char>        error_desc_a;
#if defined(PLATFORMSTL_OS_IS_WINDOWS)
    typedef basic_error_desc<wchar_t>     error_desc_w;
#endif /* OS */
    typedef basic_error_desc<char>        error_desc;


#elif defined(PLATFORMSTL_OS_IS_UNIX)

# ifdef _STLSOFT_NO_NAMESPACE
    using ::basic_error_desc;
    using ::error_desc_a;
    using ::error_desc;
# else /* ? _STLSOFT_NO_NAMESPACE */
    using ::stlsoft::basic_error_desc;
    using ::stlsoft::error_desc_a;
    using ::stlsoft::error_desc;
# endif /* _STLSOFT_NO_NAMESPACE */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

# ifdef _WINSTL_NO_NAMESPACE
    using ::basic_error_desc;
    using ::error_desc_a;
    using ::error_desc_w;
    using ::error_desc;
# else /* ? _WINSTL_NO_NAMESPACE */
    using ::winstl::basic_error_desc;
    using ::winstl::error_desc_a;
    using ::winstl::error_desc_w;
    using ::winstl::error_desc;
# endif /* _WINSTL_NO_NAMESPACE */

#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_ERROR_DESC */

/* ///////////////////////////// end of file //////////////////////////// */
