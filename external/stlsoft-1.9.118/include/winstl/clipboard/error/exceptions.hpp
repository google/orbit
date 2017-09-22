/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/clipboard/error/exceptions.hpp
 *
 * Purpose:     Exceptions used by the Clipboard library.
 *
 * Created:     6th June 2006
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


/** \file winstl/clipboard/error/exceptions.hpp
 *
 * \brief [C++ only] Exceptions used by the Windows Clipboard Library
 *   (\ref group__library__windows_clipboard "Windows Clipboard" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS
#define WINSTL_INCL_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS_MAJOR     2
# define WINSTL_VER_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS_MINOR     0
# define WINSTL_VER_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS_REVISION  1
# define WINSTL_VER_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS_EDIT      9
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
 *   the \ref group__library__windows_clipboard "Windows Clipboard" Library.
 *
 * \ingroup group__library__windows_clipboard
 */
class clipboard_exception
    : public windows_exception
{
/// \name Member Types
/// @{
public:
    typedef windows_exception           parent_class_type;
    typedef clipboard_exception         class_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance from a reason and an error code
    clipboard_exception(char const* reason, error_code_type err)
        : windows_exception(reason, err)
    {}
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

#endif /* WINSTL_INCL_WINSTL_CLIPBOARD_ERROR_HPP_EXCEPTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
