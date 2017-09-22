/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shell/browse_for_folder.hpp
 *
 * Purpose:     Shell browsing functions.
 *
 * Created:     2nd March 2002
 * Updated:     15th February 2010
 *
 * Thanks:      To Pablo Aguilar for default folder enhancements.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
 * Copyright (c) 2005, Pablo Aguilar
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


/** \file winstl/shell/browse_for_folder.hpp
 *
 * \brief [C++ only] Definition of Windows Shell folder browsing functions
 *   (\ref group__library__windows_shell "Windows Shell" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER
#define WINSTL_INCL_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER_MAJOR    4
# define WINSTL_VER_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER_MINOR    2
# define WINSTL_VER_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER_REVISION 5
# define WINSTL_VER_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER_EDIT     62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes.
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR
# include <winstl/memory/shell_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */

#ifdef STLSOFT_UNITTEST
# include <string>
#endif /* STLSOFT_UNITTEST */

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

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// struct shell_browse_traits

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \brief Traits for accessing the correct browse information functions for a given character type
 *
 * \ingroup group__library__windows_shell
 *
 * shell_browse_traits is a traits class for determining the correct browse
 * information structures and functions for a given character type.
 */
template <ss_typename_param_k C>
struct shell_browse_traits
{
    /// The browse-info type
    typedef BROWSEINFO     browseinfo_t;

    /// Browses for the folder according to the given information
    static LPITEMIDLIST browseforfolder(browseinfo_t *bi);
    /// \brief Translates am ITEMIDLIST pointer to a path.
    ///
    /// \param pidl The item identifier list from which to elicit the path
    /// \param pszPath A non-null pointer to a buffer of at least _MAX_PATH (aka WINSTL_CONST_MAX_PATH) length
    static BOOL getpathfromidlist(LPCITEMIDLIST pidl, ws_char_a_t *pszPath);
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

// class shell_browse_traits
template <ss_typename_param_k C>
struct shell_browse_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct shell_browse_traits<ws_char_a_t>
{
public:
    typedef BROWSEINFOA     browseinfo_t;

    static LPITEMIDLIST browseforfolder(browseinfo_t *bi)
    {
        return ::SHBrowseForFolderA(bi);
    }

    static BOOL getpathfromidlist(LPCITEMIDLIST pidl, ws_char_a_t *pszPath)
    {
        return ::SHGetPathFromIDListA(pidl, pszPath);
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct shell_browse_traits<ws_char_w_t>
{
public:
    typedef BROWSEINFOW     browseinfo_t;

    static LPITEMIDLIST browseforfolder(browseinfo_t *bi)
    {
        return ::SHBrowseForFolderW(bi);
    }

    static BOOL getpathfromidlist(LPCITEMIDLIST pidl, ws_char_w_t *pszPath)
    {
        return ::SHGetPathFromIDListW(pidl, pszPath);
    }
};

template <ss_typename_param_k C>
struct shell_browse_callback_holder
{
    static int CALLBACK proc(   HWND    hwnd
                            ,   UINT    uMsg
                            ,   LPARAM  /* lParam */
                            ,   LPARAM  lpData)
    {
        if(BFFM_INITIALIZED == uMsg)
        {
            C const* path = reinterpret_cast<C const*>(lpData);

            ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(path));
        }

        return 0;
    }
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
inline ws_bool_t shell_browse_impl( C const*        title
                                ,   C*              displayName
                                ,   UINT            flags
                                ,   HWND            hwndOwner
                                ,   LPCITEMIDLIST   pidlRoot
                                ,   C const*        defaultFolder)
{
    typedef shell_browse_traits<C>                  traits_type;
    ss_typename_type_k traits_type::browseinfo_t    browseinfo;
    LPITEMIDLIST                                    lpiidl;
    ws_bool_t                                       bRet    =   false;

    browseinfo.hwndOwner        =   hwndOwner;
    browseinfo.pidlRoot         =   pidlRoot;
    browseinfo.pszDisplayName   =   displayName;
    browseinfo.lpszTitle        =   title;
    browseinfo.ulFlags          =   flags;

    if( NULL != defaultFolder &&
        '\0' != *defaultFolder)
    {
        browseinfo.lpfn         =   &shell_browse_callback_holder<C>::proc;
        browseinfo.lParam       =   reinterpret_cast<LPARAM>(defaultFolder);
    }
    else
    {
        browseinfo.lpfn         =   0;
        browseinfo.lParam       =   0;
    }

    lpiidl                      =   traits_type::browseforfolder(&browseinfo);

    if(lpiidl != 0)
    {
        if(traits_type::getpathfromidlist(lpiidl, displayName))
        {
            bRet = true;
        }

        shell_allocator<ITEMIDLIST>().deallocate(lpiidl);
    }

    if(!bRet)
    {
        displayName[0] = '\0';
    }

    return bRet;
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


// function browse_for_folder
/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 * \param hwndOwner The parent of the browse dialog. May be null
 * \param pidlRoot Pointer to an ITEMIDLIST structure (PIDL) specifying the location of the root folder from which to start browsing. May be null
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName, UINT flags, HWND hwndOwner, LPCITEMIDLIST pidlRoot)
{
    return shell_browse_impl(stlsoft_ns_qual(c_str_ptr)(title), displayName, flags, hwndOwner, pidlRoot, static_cast<C const*>(NULL));
}

/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 * \param hwndOwner The parent of the browse dialog. May be null
 * \param pidlRoot Pointer to an ITEMIDLIST structure (PIDL) specifying the location of the root folder from which to start browsing. May be null
 * \param defaultFolder The default folder to select when the browse window opens
 */
template<   ss_typename_param_k S0
        ,   ss_typename_param_k C
        ,   ss_typename_param_k S1
        >
inline ws_bool_t browse_for_folder(S0 const& title, C* displayName, UINT flags, HWND hwndOwner, LPCITEMIDLIST pidlRoot, S1 const& defaultFolder)
{
    return shell_browse_impl(stlsoft_ns_qual(c_str_ptr)(title), displayName, flags, hwndOwner, pidlRoot, stlsoft_ns_qual(c_str_ptr)(defaultFolder));
}

/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 * \param hwndOwner The parent of the browse dialog. May be null
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName, UINT flags, HWND hwndOwner)
{
    return browse_for_folder(title, displayName, flags, hwndOwner, static_cast<LPCITEMIDLIST>(0));
}

/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 * \param pidlRoot Pointer to an ITEMIDLIST structure (PIDL) specifying the location of the root folder from which to start browsing. May be null
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName, UINT flags, LPCITEMIDLIST pidlRoot)
{
    return browse_for_folder(title, displayName, flags, 0, pidlRoot);
}

/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName, UINT flags)
{
    return browse_for_folder(title, displayName, flags, 0, 0);
}

#if !defined(STLSOFT_COMPILER_IS_DMC) && \
    !defined(STLSOFT_COMPILER_IS_MWERKS) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER != 1300)
/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 * \param flags Combination of the <b>BIF_*</b> flags for the Win32 \c SHBrowseForFolder() function
 * \param defaultFolder The default folder to select when the browse window opens
 */
template<   ss_typename_param_k S0
        ,   ss_typename_param_k C
        ,   ss_typename_param_k S1
        >
inline ws_bool_t browse_for_folder( S0 const    &title
                                ,   C           *displayName
                                ,   UINT        flags
                                ,   S1 const    &defaultFolder
                                )
{
    return browse_for_folder(title, displayName, flags, 0, 0, defaultFolder);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
// Disambiguates between the two four parameter overloads
//  browse_for_folder(S const& , C *, UINT , LPCITEMIDLIST )
// and
// template < . . . >
//  browse_for_folder(S0 const&, C*, UINT, S1 const&)
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName, UINT flags, LPITEMIDLIST pidlRoot)
{
    return browse_for_folder(title, displayName, flags, const_cast<LPCITEMIDLIST>(pidlRoot));
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#endif /* compiler */

/** \brief Browses the shell namespace according to the given parameters
 *
 * \ingroup group__library__windows_shell
 *
 * \param title The title for the browse dialog
 * \param displayName Buffer to receive the display name
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
inline ws_bool_t browse_for_folder(S const& title, C* displayName)
{
    return browse_for_folder(title, displayName, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/browse_for_folder_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* WINSTL_INCL_WINSTL_SHELL_HPP_BROWSE_FOR_FOLDER */

/* ///////////////////////////// end of file //////////////////////////// */
