/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/filesystem/file_path_buffer.hpp
 *
 * Purpose:     Platform header for the file_path_buffer components.
 *
 * Created:     20th March 2005
 * Updated:     1st April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file platformstl/filesystem/file_path_buffer.hpp
 *
 * \brief [C++ only] Definition of the platformstl::basic_file_path_buffer
 *  type
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
#define PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_MAJOR      2
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_MINOR      1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_REVISION   2
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_EDIT       24
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
# ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
#  include <unixstl/filesystem/file_path_buffer.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
#  include <winstl/filesystem/file_path_buffer.hpp>
# endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
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
     * \ingroup group__library__filesystem
     *
     * The class is not actually defined in the
     * \link ::platformstl platformstl\endlink namespace. Rather, it
     * resolves to the appropriate type for the given platform, relying on
     * \ref section__principle__conformance__intersecting_conformance "Intersecting Conformance"
     * of the resolved platform-specific types.
     *
     * When compiling on UNIX platforms, the platformstl::basic_file_path_buffer
     * type resolves to the unixstl::basic_file_path_buffer class. On Windows
     * platforms it resolves to the winstl::basic_file_path_buffer class. It
     * is not defined for other platforms.
     */
    template<   ss_typename_param_k C
            ,   ss_typename_param_k T = filesystem_traits<C>
            ,   ss_typename_param_k A = ss_typename_type_def_k stlsoft::allocator_selector<C>::allocator_type
            >
    class basic_file_path_buffer
    {};

    /// \brief Specialisation of the basic_file_path_buffer template for the ANSI character type \c char
    typedef basic_file_path_buffer<char, filesystem_traits<char> >          path_a;
    /// \brief Specialisation of the basic_file_path_buffer template for the Unicode character type \c wchar_t
    typedef basic_file_path_buffer<wchar_t, filesystem_traits<wchar_t> >    path_w;
    /// \brief Specialisation of the basic_file_path_buffer template for the ANSI character type \c char on UNIX, and for the \c TCHAR type on Windows
    typedef basic_file_path_buffer<tchar, filesystem_traits<tchar> >        path;

#elif defined(PLATFORMSTL_OS_IS_UNIX)

    // Because early incarnations of Visual C++ are pretty stupid, we need to
    // work around their inability to introduce a template via using by
    // defining and deriving here
    //
    // This one required an *extra* degree of trickery, by defining a derivation
    // of unixstl::basic_file_path_buffer _within_ unixstl, in the form of
    // basic_file_path_buffer__, and then deriving from that. If we just try and
    // derive from unixstl::basic_file_path_buffer directly, the compiler gets
    // confused between the unixstl one and the local one, and fails and says that
    // it cannot derive from itself (which happens to be incomplete).

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1310

    template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
            ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<C>::allocator_type
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            ,   ss_typename_param_k A /* = ss_typename_type_def_k allocator_selector<C>::allocator_type */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            >
    class basic_file_path_buffer
        : public unixstl_ns_qual(basic_file_path_buffer__)<C, A>
    {
    private:
        typedef unixstl_ns_qual(basic_file_path_buffer__)<C, A>          parent_class_type;
    public:
        typedef basic_file_path_buffer<C, A>                            class_type;
        typedef ss_typename_type_k parent_class_type::value_type        value_type;
        typedef ss_typename_type_k parent_class_type::reference         reference;
        typedef ss_typename_type_k parent_class_type::const_reference   const_reference;
        typedef ss_typename_type_k parent_class_type::size_type         size_type;
    };

    typedef basic_file_path_buffer<char>        file_path_buffer_a;
    typedef basic_file_path_buffer<wchar_t>     file_path_buffer_w;
    typedef basic_file_path_buffer<char>        file_path_buffer;

# else /* ? compiler */

#  ifdef _UNIXSTL_NO_NAMESPACE
    using ::basic_file_path_buffer;
    using ::file_path_buffer_a;
    using ::file_path_buffer_w;
    using ::file_path_buffer;
#  else /* ? _UNIXSTL_NO_NAMESPACE */
    using ::unixstl::basic_file_path_buffer;
    using ::unixstl::file_path_buffer_a;
    using ::unixstl::file_path_buffer_w;
    using ::unixstl::file_path_buffer;
#  endif /* _UNIXSTL_NO_NAMESPACE */

# endif /* compiler */

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

    // Because early incarnations of Visual C++ are pretty stupid, we need to
    // work around their inability to introduce a template via using by
    // defining and deriving here
    //
    // This one required an *extra* degree of trickery, by defining a derivation
    // of winstl::basic_file_path_buffer _within_ winstl, in the form of
    // basic_file_path_buffer__, and then deriving from that. If we just try and
    // derive from winstl::basic_file_path_buffer directly, the compiler gets
    // confused between the winstl one and the local one, and fails and says that
    // it cannot derive from itself (which happens to be incomplete).

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER < 1310

    template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
            ,   ss_typename_param_k A = winstl_ns_qual(processheap_allocator)<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            ,   ss_typename_param_k A /* = processheap_allocator<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            >
    class basic_file_path_buffer
        : public winstl_ns_qual(basic_file_path_buffer__)<C, A>
    {
    private:
        typedef winstl_ns_qual(basic_file_path_buffer__)<C, A>          parent_class_type;
    public:
        typedef basic_file_path_buffer<C, A>                            class_type;
        typedef ss_typename_type_k parent_class_type::value_type        value_type;
        typedef ss_typename_type_k parent_class_type::reference         reference;
        typedef ss_typename_type_k parent_class_type::const_reference   const_reference;
        typedef ss_typename_type_k parent_class_type::size_type         size_type;
    };

    typedef basic_file_path_buffer<char>        file_path_buffer_a;
    typedef basic_file_path_buffer<wchar_t>     file_path_buffer_w;
    typedef basic_file_path_buffer<TCHAR>       file_path_buffer;

# else /* ? compiler */

#  ifdef _WINSTL_NO_NAMESPACE
    using ::basic_file_path_buffer;
    using ::file_path_buffer_a;
    using ::file_path_buffer_w;
    using ::file_path_buffer;
#  else /* ? _WINSTL_NO_NAMESPACE */
    using ::winstl::basic_file_path_buffer;
    using ::winstl::file_path_buffer_a;
    using ::winstl::file_path_buffer_w;
    using ::winstl::file_path_buffer;
#  endif /* _WINSTL_NO_NAMESPACE */

# endif /* compiler */

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

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */

/* ///////////////////////////// end of file //////////////////////////// */
