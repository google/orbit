/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/error/exceptions.hpp
 *
 * Purpose:     Exceptions used by the Registry library.
 *
 * Created:     8th February 2006
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
 * ////////////////////////////////////////////////////////////////////// */


/** \file winstl/registry/error/exceptions.hpp
 *
 * \brief [C++ only] Exceptions used by
 *   the \ref group__library__windows_registry "Windows Registry" Library.
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS
#define WINSTL_INCL_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS_MAJOR      2
# define WINSTL_VER_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS_MINOR      1
# define WINSTL_VER_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS_REVISION   1
# define WINSTL_VER_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS_EDIT       17
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

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
# include <winstl/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS
# include <stlsoft/util/sign_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_SIGN_TRAITS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

/** \brief Root exception thrown by
 *   the \ref group__library__windows_registry "Windows Registry" Library.
 *
 * \ingroup group__library__windows_registry
 */
class registry_exception
    : public windows_exception
{
/// \name Member Types
/// @{
public:
    typedef windows_exception                                               parent_class_type;
    typedef registry_exception                                              class_type;
    typedef parent_class_type::error_code_type                              error_code_type;
    typedef stlsoft_ns_qual(sign_traits)<error_code_type>::alt_sign_type    error_code_alt_type;

/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance from a reason and an error code
    registry_exception(char const* reason, error_code_type err)
        : windows_exception(reason, err)
    {}
    /// \brief Constructs an instance from a reason and an error code
    ///
    /// \note Since the Windows Registry API error code type is LONG
    ///  this provides a suitable overload that does not incur compiler
    ///  warnings. The casting to an unsigned type within the body is
    ///  entirely benign.
    registry_exception(char const* reason, error_code_alt_type err)
        : windows_exception(reason, static_cast<error_code_type>(err))
    {}
/// @}
};

/** \brief Indicates that a registry key could not be duplicated.
 *
 * \ingroup group__library__windows_registry
 */
class key_not_duplicated_exception
    : public registry_exception
{
/// \name Member Types
/// @{
public:
    typedef registry_exception              parent_class_type;
    typedef key_not_duplicated_exception    class_type;
/// @}

/// \name Construction
/// @{
public:
    key_not_duplicated_exception(char const* reason, error_code_type err)
        : parent_class_type(reason, err)
    {}
    key_not_duplicated_exception(char const* reason, error_code_alt_type err)
        : parent_class_type(reason, err)
    {}
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** \brief Indicates a registry value type mismatch.
 *
 * \ingroup group__library__windows_registry
 */
class wrong_value_type_exception
    : public registry_exception
{
/// \name Member Types
/// @{
public:
    typedef registry_exception          parent_class_type;
    typedef wrong_value_type_exception  class_type;
/// @}

/// \name Construction
/// @{
public:
    wrong_value_type_exception(char const* reason, error_code_type err, ws_dword_t type)
        : parent_class_type(reason, err)
        , m_valueType(type)
    {}
    wrong_value_type_exception(char const* reason, error_code_alt_type err, ws_dword_t type)
        : parent_class_type(reason, err)
        , m_valueType(type)
    {}
/// @}

/// \name Accessors
/// @{
public:
    /// \brief The actual type of the value
    ws_dword_t actual_value_type() const
    {
        return m_valueType;
    }
/// @}

/// \name Members
/// @{
private:
    const ws_dword_t    m_valueType;
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** \brief Indicates insufficient rights to access a registry key.
 *
 * \ingroup group__library__windows_registry
 */
class access_denied_exception
    : public registry_exception
{
/// \name Member Types
/// @{
public:
    typedef registry_exception          parent_class_type;
    typedef access_denied_exception     class_type;
/// @}

/// \name Construction
/// @{
public:
    access_denied_exception(char const* reason, error_code_type err)
        : parent_class_type(reason, err)
    {}
    access_denied_exception(char const* reason, error_code_alt_type err)
        : parent_class_type(reason, err)
    {}
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
