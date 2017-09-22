/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/dl/module.hpp (originally MXModule.h, ::SynesisUnix)
 *
 * Purpose:     Contains the module class.
 *
 * Created:     30th October 1997
 * Updated:     12th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1997-2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/dl/module.hpp
 *
 * \brief [C++ only] Definition of the unixstl::module class
 *   (\ref group__library__dl "DL" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_DL_HPP_MODULE
#define UNIXSTL_INCL_UNIXSTL_DL_HPP_MODULE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_DL_HPP_MODULE_MAJOR    6
# define UNIXSTL_VER_UNIXSTL_DL_HPP_MODULE_MINOR    3
# define UNIXSTL_VER_UNIXSTL_DL_HPP_MODULE_REVISION 1
# define UNIXSTL_VER_UNIXSTL_DL_HPP_MODULE_EDIT     220
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_HPP_ERROR_UNIX_EXCEPTIONS
# include <unixstl/error/exceptions.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_UNIX_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING
# include <unixstl/shims/access/string.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_H_DLFCN
# define STLSOFT_INCL_H_DLFCN
# include <dlfcn.h>
#endif /* !STLSOFT_INCL_H_DLFCN */
#ifndef STLSOFT_INCL_H_ERRNO
# define STLSOFT_INCL_H_ERRNO
# include <errno.h>
#endif /* !STLSOFT_INCL_H_ERRNO */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Class for manipulating dynamically loaded libraries
 *
 * \ingroup group__library__dl
 */
class module
{
public:
    /// \brief The handle type
    typedef void*       module_handle_type;
    /// \brief The handle type
    ///
    /// \note This member type is required to make it compatible with
    ///  the STLSoft get_module_handle access shim
    typedef void*       handle_type;
    /// \brief The class type
    typedef module      class_type;
    /// \brief The entry point type
    typedef void*       proc_pointer_type;
public:
    typedef handle_type resource_type;

/// \name Construction
/// @{
public:
    /// \brief Constructs by loading the named module
    ///
    /// \param moduleName The file name of the executable module to be loaded.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \note If exception-handling is being used, then this throws a
    ///  \link unixstl::unix_exception unix_exception\endlink
    ///  if the module cannot be loaded
    ss_explicit_k module(us_char_a_t const* moduleName, int mode = RTLD_NOW);
    /// \brief Constructs by loading the named module
    ///
    /// \param moduleName The file name of the executable module to be loaded.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \note If exception-handling is being used, then this throws a
    ///  \link unixstl::unix_exception unix_exception\endlink
    ///  if the module cannot be loaded
    ss_explicit_k module(us_char_w_t const* moduleName, int mode = RTLD_NOW);
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    /// \brief Constructs by loading the named module
    ///
    /// \param moduleName The file name of the executable module to be
    ///   loaded. The argument may be of any type for which the
    ///   \ref group__concept__shim__string_access "string access shim"
    ///   stlsoft::c_str_ptr is defined.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \note If exception-handling is being used, then this throws a
    ///  \link unixstl::unix_exception unix_exception\endlink
    ///  if the module cannot be loaded
    template <ss_typename_param_k S>
    ss_explicit_k module(S const& moduleName, int mode = RTLD_NOW)
        : m_hmodule(load(moduleName, mode))
    {
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        if(NULL == m_hmodule)
        {
            STLSOFT_THROW_X(unix_exception("Cannot load module", errno));
        }
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    /// \brief Constructs by taking ownership of the given handle
    ///
    /// \note If exception-handling is being used, then this throws a
    ///  \link unixstl::unix_exception unix_exception\endlink
    ///  if the handle is NULL.
    ss_explicit_k module(module_handle_type hmodule);
    /// \brief Closes the module handle
    ~module() stlsoft_throw_0();
/// @}

/// \name Static operations
/// @{
public:
    /// \brief Loads the named module, returning its handle, which the
    ///   caller must close with unload().
    ///
    /// \param moduleName The file name of the executable module to be loaded.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \return The module handle, or NULL if no matching module found.
    static module_handle_type   load(us_char_a_t const* moduleName, int mode = RTLD_NOW);
    /// \brief Loads the named module, returning its handle, which the
    ///   caller must close with unload().
    ///
    /// \param moduleName The file name of the executable module to be loaded.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \return The module handle, or NULL if no matching module found.
    static module_handle_type   load(us_char_w_t const* moduleName, int mode = RTLD_NOW);
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT)
    /// \brief Loads the named module, returning its handle, which the
    ///   caller must close with unload().
    ///
    /// \param moduleName The file name of the executable module to be
    ///   loaded. The argument may be of any type for which the
    ///   \ref group__concept__shim__string_access "string access shim"
    ///   stlsoft::c_str_ptr is defined.
    /// \param mode The loading mode (as used by <code>::dlopen()</code>).
    ///
    /// \return The module handle, or NULL if no matching module found.
    template <ss_typename_param_k S>
    static module_handle_type   load(S const& moduleName, int mode = RTLD_NOW)
    {
        return class_type::load(stlsoft_ns_qual(c_str_ptr)(moduleName), mode);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    /// \brief Closes the given module handle
    static void                 unload(module_handle_type hmodule) stlsoft_throw_0();
    /// \brief Looks up the named symbol from the given module
    ///
    /// \return A pointer to the named symbol, or NULL if not found
    static proc_pointer_type    get_symbol(module_handle_type hmodule, us_char_a_t const* symbolName);
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT)
    /// \brief Looks up a named symbol from the given module into a typed function pointer variable.
    ///
    /// \return A pointer to the named symbol, or NULL if not found.
    template <ss_typename_param_k F>
    static proc_pointer_type    get_symbol(module_handle_type hmodule, us_char_a_t const* symbolName, F &f)
    {
        proc_pointer_type proc = class_type::get_symbol(hmodule, symbolName);

        f = reinterpret_cast<F>(proc);

        return proc;
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
/// @}

/// \name Operations
/// @{
public:
    /// \brief Closes the module handle
    void unload() stlsoft_throw_0();

    /// \brief Yields the module handle to the caller
    module_handle_type detach();
/// @}

/// \name Lookup Operations
/// @{
public:
    /// \brief Looks up the named symbol.
    ///
    /// \return A pointer to the named symbol, or NULL if not found
    proc_pointer_type   get_symbol(us_char_a_t const* symbolName);
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT)
    /// \brief Looks up a named symbol into a typed function pointer variable.
    ///
    /// \return A pointer to the named symbol, or NULL if not found.
    template <ss_typename_param_k F>
    proc_pointer_type   get_symbol(us_char_a_t const* symbolName, F &f)
    {
        return class_type::get_symbol(m_hmodule, symbolName, f);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Provides access to the underlying module handle
    module_handle_type  get_module_handle() const;
/// @}

/// \name Implementation
/// @{
private:
/// @}

/// \name Member Variables
/// @{
private:
    module_handle_type  m_hmodule;
/// @}

/// \name Not to be implemented
/// @{
private:
    module(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Access shims
 */

/** \brief Returns the module handle for the given module
 *
 * \ingroup group__concept__shim__module_attribute
 */
inline void *get_module_handle(unixstl_ns_qual(module) const& m)
{
    return m.get_module_handle();
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/module_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline module::module(us_char_a_t const* moduleName, int mode)
    : m_hmodule(load(moduleName, mode))
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hmodule)
    {
        STLSOFT_THROW_X(unix_exception("Cannot load module", errno));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline module::module(us_char_w_t const* moduleName, int mode)
    : m_hmodule(load(moduleName, mode))
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hmodule)
    {
        STLSOFT_THROW_X(unix_exception("Cannot load module", errno));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline module::module(module::module_handle_type hmodule)
    : m_hmodule(hmodule)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hmodule)
    {
        STLSOFT_THROW_X(unix_exception("Cannot load module", errno));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

inline module::~module() stlsoft_throw_0()
{
    unload(m_hmodule);
}

inline void module::unload() stlsoft_throw_0()
{
    if(NULL != m_hmodule)
    {
        unload(m_hmodule);
        m_hmodule = NULL;
    }
}

inline module::module_handle_type module::detach()
{
    module_handle_type  h;

    h = m_hmodule;
    m_hmodule = NULL;

    return h;
}

inline /* static */ module::module_handle_type module::load(us_char_a_t const* moduleName, int mode)
{
    return ::dlopen(moduleName, mode);
}

inline /* static */ void module::unload(module::module_handle_type hmodule) stlsoft_throw_0()
{
    if(NULL != hmodule)
    {
        ::dlclose(hmodule);
    }
}

inline /* static */ module::proc_pointer_type module::get_symbol(module::module_handle_type hmodule, us_char_a_t const* symbolName)
{
    return ::dlsym(hmodule, symbolName);
}

inline module::proc_pointer_type module::get_symbol(us_char_a_t const* symbolName)
{
    return get_symbol(m_hmodule, symbolName);
}

inline module::module_handle_type module::get_module_handle() const
{
    return m_hmodule;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* UNIXSTL_INCL_UNIXSTL_DL_HPP_MODULE */

/* ///////////////////////////// end of file //////////////////////////// */
