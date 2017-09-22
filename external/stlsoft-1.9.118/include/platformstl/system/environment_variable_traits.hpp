/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/system/environment_variable_traits.hpp
 *
 * Purpose:     Definition of the environment_variable_traits class.
 *
 * Created:     9th December 2005
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


/** \file platformstl/system/environment_variable_traits.hpp
 *
 * \brief [C++ only] Definition of the platformstl::environment_variable_traits
 *  traits class
 *   (\ref group__library__system "System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS
#define PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS_MAJOR    2
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS_MINOR    2
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS_REVISION 2
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS_EDIT     22
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */

#if defined(PLATFORMSTL_OS_IS_UNIX)
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */
#ifndef STLSOFT_INCL_H_STDLIB
# define STLSOFT_INCL_H_STDLIB
# include <stdlib.h> // for environ, getenv(), etc.
#endif /* !STLSOFT_INCL_H_STDLIB */
#if defined(STLSOFT_COMPILER_IS_MWERKS)
# include <crtl.h>  // for _environ
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Feature discrimination
 *
 * Advanced Prorgramming in the UNIX Environment, Stevens, gives the following:
 *
 *  Function        Standards                         Implementations
 *
 *              ANSI C     POSIX.1    XPG3        SVR4         4.3+BSD
 *   getenv        *          *         *           *              *
 *   putenv        -          ?         *           *              *
 *   setenv        -          -         -           -              *
 *   unsetenv      -          -         -           -              *
 */

/* set by: putenv / setenv / <not supported> */

#ifdef PLATFORMSTL_ENVVAR_SET_BY_PUTENV
# undef PLATFORMSTL_ENVVAR_SET_BY_PUTENV
#endif /* PLATFORMSTL_ENVVAR_SET_BY_PUTENV */

#ifdef PLATFORMSTL_ENVVAR_SET_BY_SETENV
# undef PLATFORMSTL_ENVVAR_SET_BY_SETENV
#endif /* PLATFORMSTL_ENVVAR_SET_BY_SETENV */

#ifdef PLATFORMSTL_ENVVAR_SET_SUPPORTED
# undef PLATFORMSTL_ENVVAR_SET_SUPPORTED
#endif /* PLATFORMSTL_ENVVAR_SET_SUPPORTED */


/* erase by: putenv / putenv (with equals) / unsetenv / <not supported> */

#ifdef PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV
# undef PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV
#endif /* PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV */

#ifdef PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS
# undef PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS
#endif /* PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS */

#ifdef PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV
# undef PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV
#endif /* PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV */

#ifdef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
# undef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
#endif /* PLATFORMSTL_ENVVAR_ERASE_SUPPORTED */


/* environ / _environ / <no environ> */

#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
# undef PLATFORMSTL_ENVVAR_HAS_ENVIRON
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

#ifdef PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE
# undef PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE
#endif /* PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE */


/* putenv has _underscore */

#ifdef PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE
# undef PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE
#endif /* PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE */


#if defined(PLATFORMSTL_ENVVAR_STDC_ONLY)

#elif defined(PLATFORMSTL_OS_IS_UNIX)

extern char **environ;

# if defined(UNIXSTL_OS_IS_LINUX)
#  define PLATFORMSTL_ENVVAR_HAS_ENVIRON
#  define PLATFORMSTL_ENVVAR_SET_BY_SETENV
#  define PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV
# elif defined(UNIXSTL_OS_IS_MACOSX)
#  define PLATFORMSTL_ENVVAR_HAS_ENVIRON
#  define PLATFORMSTL_ENVVAR_SET_BY_SETENV
#  define PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV
# elif defined(UNIXSTL_OSFAMILY_IS_BSD)
#  define PLATFORMSTL_ENVVAR_HAS_ENVIRON
#  define PLATFORMSTL_ENVVAR_SET_BY_SETENV
#  define PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV
# elif defined(UNIXSTL_OSFAMILY_IS_SVR4)
#  define PLATFORMSTL_ENVVAR_HAS_ENVIRON
#  define PLATFORMSTL_ENVVAR_SET_BY_PUTENV
#  define PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV
# elif defined(_WIN32)
#  define PLATFORMSTL_ENVVAR_SET_BY_PUTENV
#  define PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS
#  define PLATFORMSTL_ENVVAR_HAS_ENVIRON
#  define PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE
#  if defined(STLSOFT_COMPILER_IS_BORLAND)
    /* putenv() */
#  elif defined(STLSOFT_COMPILER_IS_DMC) || \
        defined(STLSOFT_COMPILER_IS_GCC) || \
        defined(STLSOFT_COMPILER_IS_INTEL) || \
        defined(STLSOFT_COMPILER_IS_MSVC) || \
        defined(STLSOFT_COMPILER_IS_MWERKS)
#   define PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE
#  else /* ? compiler */
#  endif /* compiler */
# else /* ? UNIX OS family */
#  error UNIX family flavours other than BSD and SVR4 are not currently supported.
# endif /* UNIX OS family */
#elif defined(PLATFORMSTL_OS_IS_WINDOWS)
# define PLATFORMSTL_ENVVAR_SET_BY_PUTENV
# define PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS
# define PLATFORMSTL_ENVVAR_HAS_ENVIRON
# define PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE
# if defined(STLSOFT_COMPILER_IS_BORLAND)
   /* putenv() */
# elif defined(STLSOFT_COMPILER_IS_DMC) || \
       defined(STLSOFT_COMPILER_IS_GCC) || \
       defined(STLSOFT_COMPILER_IS_INTEL) || \
       defined(STLSOFT_COMPILER_IS_MSVC) || \
       defined(STLSOFT_COMPILER_IS_MWERKS)
#  define PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE
# else /* ? compiler */
# endif /* compiler */
#else /* ? operating system */
# error Operating systems other than UNIX and Win32 are not currently supported.
#endif /* operating system */


#if defined(PLATFORMSTL_ENVVAR_SET_BY_PUTENV) || \
    defined(PLATFORMSTL_ENVVAR_SET_BY_SETENV)
# define PLATFORMSTL_ENVVAR_SET_SUPPORTED
#endif /* set ? */

#if defined(PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV) || \
    defined(PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS) || \
    defined(PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV)
# define PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
#endif /* set ? */

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

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Abstraction of the platform-dependent environment variable
 *   handling.
 *
 * \ingroup group__library__system
 */
struct environment_variable_traits
{
/// \name Member Types
/// @{
public:
    /// \brief The character type
    typedef char        char_type;
/// @}

/// \name Operations
/// @{
public:
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    /// \brief Returns a pointer to the environment block pointer.
    ///
    /// \note The returned pointer must be passed back to release_environ().
    static char_type const** get_environ();
    /// \brief Releases any allocation performed by get_environ().
    ///
    /// \param env The pointer returned in a previous call to get_environ().
    static void release_environ(char_type const** env) throw();

#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

    /// \brief Returns a pointer to the value of the given variable, or NULL if
    /// the variable does not exist
    ///
    /// \param name The name of the variable whose value will be retrieved
    static char_type const* get_variable(char_type const* name) throw();

#ifdef PLATFORMSTL_ENVVAR_SET_SUPPORTED
    /// \brief Creates or updates the given variable to the given value
    ///
    /// \param name The name of the variable to create or update
    /// \param value The new value of the variable
    ///
    /// \return A status code indicating success
    /// \retval 0 The operation completed successfully
    /// \retval !0 The operation failed
    static int set_variable(char_type const* name, char_type const* value) throw();
#endif /* PLATFORMSTL_ENVVAR_SET_SUPPORTED */
#ifdef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
    /// \brief Removed the given variable
    ///
    /// \param name The name of the variable to remove
    ///
    /// \note Erasing a variable that does not exist is a succesful operation
    ///
    /// \return A status code indicating success
    /// \retval 0 The operation completed successfully
    /// \retval !0 The operation failed
    static int erase_variable(char_type const* name) throw();
#endif /* PLATFORMSTL_ENVVAR_ERASE_SUPPORTED */
/// @}

/// \name Implementation
/// @{
private:
#if defined(PLATFORMSTL_ENVVAR_SET_BY_PUTENV) || \
    defined(PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV)
    static int          call_putenv_(char_type const* str) throw();
    static int          call_putenv_(char_type const* name, char_type const* value) throw();
#endif /* putenv ? */
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
inline /* static */ environment_variable_traits::char_type const** environment_variable_traits::get_environ()
{
# ifdef PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE
    return const_cast<char_type const**>(_environ);
# else /* ? PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE */
    return const_cast<char_type const**>(environ);
# endif /* PLATFORMSTL_ENVVAR_ENVIRON_HAS_UNDERSCORE */
}

inline /* static */ void environment_variable_traits::release_environ(environment_variable_traits::char_type const**) throw()
{}
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

inline /* static */ environment_variable_traits::char_type const* environment_variable_traits::get_variable(environment_variable_traits::char_type const* name) throw()
{
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", NULL == ::strchr(name, '='));

    return const_cast<char_type const*>(::getenv(name));
}

#ifdef PLATFORMSTL_ENVVAR_SET_SUPPORTED
inline /* static */ int environment_variable_traits::set_variable(environment_variable_traits::char_type const* name, environment_variable_traits::char_type const* value) throw()
{
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_ASSERT(NULL != value);
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", NULL == ::strchr(name, '='));

#if defined(PLATFORMSTL_ENVVAR_SET_BY_PUTENV)
    return call_putenv_(name, value);
#elif defined(PLATFORMSTL_ENVVAR_SET_BY_SETENV)
    return ::setenv(name, value, 1);
#endif /* unsetenv */
}
#endif /* PLATFORMSTL_ENVVAR_SET_SUPPORTED */

#ifdef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
inline /* static */ int environment_variable_traits::erase_variable(environment_variable_traits::char_type const* name) throw()
{
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", NULL == ::strchr(name, '='));

#if defined(PLATFORMSTL_ENVVAR_ERASE_BY_UNSETENV)
    static_cast<void>(::unsetenv(name));    // Some definitions of unsetenv() return void

    return 0;
#else /* ? unsetenv */
    return call_putenv_(name, NULL);
#endif /* unsetenv */
}
#endif /* PLATFORMSTL_ENVVAR_ERASE_SUPPORTED */

#if defined(PLATFORMSTL_ENVVAR_SET_BY_PUTENV) || \
    defined(PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV)

inline /* static */ int environment_variable_traits::call_putenv_(environment_variable_traits::char_type const* str) throw()
{
#ifdef PLATFORMSTL_ENVVAR_PUTENV_HAS_UNDERSCORE
    return ::_putenv(str);
#else /* ? compiler */
    return ::putenv(str);
#endif /* compiler */
}

inline /* static */ int environment_variable_traits::call_putenv_(environment_variable_traits::char_type const* name, environment_variable_traits::char_type const* value) throw()
{
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_ASSERT(NULL == ::strchr(name, '='));

#ifndef PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS
    if(NULL == value)
    {
        return call_putenv_(name);
    }
    else
#endif /* !PLATFORMSTL_ENVVAR_ERASE_BY_PUTENV_EQUALS */
    {
#ifdef STLSOFT_CF_THROW_BAD_ALLOC
        try
#endif /* STLSOFT_CF_THROW_BAD_ALLOC */
        {
            const ss_size_t                 cchName     =   ::strlen(name);
            const ss_size_t                 cchValue    =   stlsoft::c_str_len(value);
            stlsoft::auto_buffer<char_type> buff(cchName + 1 + cchValue + 1);

#ifdef STLSOFT_CF_THROW_BAD_ALLOC
            STLSOFT_ASSERT(!buff.empty());
#else /* ? STLSOFT_CF_THROW_BAD_ALLOC */
            if(buff.empty()) // This check worthwhile since implementation of ator may not support bad_alloc
            {
                errno = ENOMEM;
                return -1;
            }
            else
#endif /* STLSOFT_CF_THROW_BAD_ALLOC */
            {
                ::strncpy(&buff[0], name, cchName);
                buff[cchName] = '=';
                ::strncpy(&buff[cchName + 1], value, cchValue);
                buff[cchName + 1 + cchValue] = '\0';
                STLSOFT_ASSERT(::strlen(buff.data()) == buff.size() - 1);

                return call_putenv_(buff.data());
            }
        }
#ifdef STLSOFT_CF_THROW_BAD_ALLOC
        catch(std::bad_alloc &)
        {
            errno = ENOMEM;
            return -1;
        }
#endif /* STLSOFT_CF_THROW_BAD_ALLOC */
    }
}
#endif /* putenv ? */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
