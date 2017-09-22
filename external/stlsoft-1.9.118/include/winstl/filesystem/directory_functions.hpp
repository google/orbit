/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/directory_functions.hpp (originally MLFlMan.h, ::SynesisStd)
 *
 * Purpose:     Functions for manipulating directories.
 *
 * Created:     7th February 2002
 * Updated:     1st February 2013
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2013, Matthew Wilson and Synesis Software
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


/** \file winstl/filesystem/directory_functions.hpp
 *
 * \brief [C++ only] Functions for manipulating directories
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_MAJOR     5
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_MINOR     0
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_REVISION  6
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_EDIT      51
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <winstl/filesystem/filesystem_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
# include <winstl/filesystem/file_path_buffer.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
#ifdef _ATL_MIN_CRT
# ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
#  include <stlsoft/memory/allocator_selector.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#endif /* _ATL_MIN_CRT */

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
 * Helper functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline C* find_last_path_name_separator_(C const* s)
{
    typedef filesystem_traits<C>    traits_t;

    ss_typename_type_k traits_t::char_type const*   slash  =   traits_t::str_rchr(s, '/');
    ss_typename_type_k traits_t::char_type const*   bslash =   traits_t::str_rchr(s, '\\');

    if(NULL == slash)
    {
        slash = bslash;
    }
    else if(NULL != bslash)
    {
        if(slash < bslash)
        {
            slash = bslash;
        }
    }

    return const_cast<C*>(slash);
}

template <ss_typename_param_k C>
inline
ws_bool_t
create_directory_recurse_impl(
    C const*                dir
,   LPSECURITY_ATTRIBUTES   lpsa
)
{
    typedef C                                   char_type;
    typedef filesystem_traits<C>                traits_t;
#ifdef _ATL_MIN_CRT
    typedef ss_typename_type_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type   allocator_t;
    typedef basic_file_path_buffer< char_type
                                ,   allocator_t
                                >               file_path_buffer_t;
#else /* ? _ATL_MIN_CRT */
    typedef basic_file_path_buffer<char_type>   file_path_buffer_t;
#endif /* _ATL_MIN_CRT */

    ws_bool_t    bRet;

    if( NULL == dir ||
        '\0' == *dir)
    {
        traits_t::set_last_error(ERROR_DIRECTORY);

        bRet = false;
    }
    else
    {
        if(traits_t::file_exists(dir))
        {
            if(traits_t::is_directory(dir))
            {
                traits_t::set_last_error(ERROR_ALREADY_EXISTS);

                bRet = true;
            }
            else
            {
                traits_t::set_last_error(ERROR_FILE_EXISTS);

                bRet = false;
            }
        }
        else
        {
            file_path_buffer_t  sz;
            file_path_buffer_t  szParent;

            // May be being compiled absent exception support, so need to check the
            // file path buffers. (This _could_ be done with a compile-time #ifdef,
            // but it's best not, since some translators support exceptions but yet
            // don't throw on mem exhaustion, and in any case a user could change
            // ::new)
            if( 0 == sz.size() ||
                0 == szParent.size())
            {
                bRet = false;
            }
            else
            {
                ws_size_t dirLen = traits_t::str_len(dir);
                traits_t::char_copy(&sz[0], dir, dirLen);
                sz[dirLen] = '\0';
                traits_t::remove_dir_end(&sz[0]);

                if( traits_t::create_directory(sz.c_str(), lpsa) ||
                    ERROR_ALREADY_EXISTS == traits_t::get_last_error())
                {
                    traits_t::set_last_error(ERROR_SUCCESS);

                    bRet = true;
                }
                else
                {
                    // Trim previous directory
                    ws_size_t szLen = traits_t::str_len(dir);
                    traits_t::char_copy(&szParent[0], sz.c_str(), szLen);
                    szParent[szLen] = '\0';

                    char_type* pszSlash = find_last_path_name_separator_<C>(szParent.c_str());
                    if(pszSlash == NULL)
                    {
                        traits_t::set_last_error(ERROR_DIRECTORY);

                        bRet = false;
                    }
                    else
                    {
                        *pszSlash = '\0';                   // Will always have enough room for two bytes

                        // If second character is ':', and total lengths is less than four,
                        // or the recurse create fails, then return false;
                        if( (   szParent[1] == ':' &&
                                (traits_t::set_last_error(ERROR_CANNOT_MAKE), traits_t::str_len(szParent.c_str()) < 4)) ||
                            !create_directory_recurse_impl(szParent.c_str(), lpsa))
                        {
                            bRet = false;
                        }
                        else
                        {
                            bRet = traits_t::create_directory(sz.c_str(), lpsa) || ERROR_ALREADY_EXISTS == traits_t::get_last_error();
                        }
                    }
                }
            }
        }
    }

    return bRet;
}

template<
    ss_typename_param_k C
,   ss_typename_param_k FD  // This is need because VC++6 cannot deduce filesystem_traits<C>::find_data_type
>
inline
ws_dword_t
remove_directory_recurse_impl(
    C const*    dir
,   ws_int_t  (*pfn)(void* param, C const* subDir, FD const* st, DWORD err)
,   void*       param
)
{
    typedef C                                   char_type;
    typedef filesystem_traits<C>                traits_t;
#ifdef _ATL_MIN_CRT
    typedef ss_typename_type_k stlsoft_ns_qual(allocator_selector)<C>::allocator_type   allocator_t;
    typedef basic_file_path_buffer< char_type
                                ,   allocator_t
                                >   file_path_buffer_t;
#else /* ? _ATL_MIN_CRT */
    typedef basic_file_path_buffer<char_type>                                           file_path_buffer_t;
#endif /* _ATL_MIN_CRT */

    ws_dword_t dwRet = static_cast<ws_dword_t>(E_FAIL);

    if(NULL != pfn)
    {
        // starting: { param, dir, NULL, ~0 }
        (void)(*pfn)(param, dir, NULL, ~static_cast<ws_dword_t>(0)); // Entering
    }

    if( NULL == dir ||
        '\0' == *dir)
    {
        dwRet = ERROR_DIRECTORY;

        if(NULL != pfn)
        {
            // failed: { param, dir, NULL, error-code }
            (void)(*pfn)(param, dir, NULL, dwRet);
        }
    }
    else
    {
        if(!traits_t::file_exists(dir))
        {
            // The given path does not exist, so this is treated as success, but
            // reporting ERROR_PATH_NOT_FOUND

            dwRet = ERROR_PATH_NOT_FOUND;

            if(NULL != pfn)
            {
                // failed: { param, dir, NULL, error-code }
                (void)(*pfn)(param, dir, NULL, dwRet);
            }
        }
        else
        {
            if(traits_t::is_file(dir))
            {
                // The given path exists as a file. This is failure
                dwRet = ERROR_FILE_EXISTS;

                if(NULL != pfn)
                {
                    // failed: { param, dir, NULL, error-code }
                    (void)(*pfn)(param, dir, NULL, dwRet);
                }
            }
            else
            {
                // Otherwise, we attempt to remove it
                if(traits_t::remove_directory(dir))
                {
                    dwRet = ERROR_SUCCESS;

                    if(NULL != pfn)
                    {
                        // succeeded: { param, dir, NULL, ERROR_SUCCESS }
                        (void)(*pfn)(param, dir, NULL, dwRet); // Deleted
                    }
                }
                else
                {
                    const DWORD removeError = traits_t::get_last_error();

                    if( ERROR_DIR_NOT_EMPTY != removeError &&
                        ERROR_SHARING_VIOLATION != removeError)
                    {
                        dwRet = removeError;

                        if(NULL != pfn)
                        {
                            // failed: { param, dir, NULL, error-code }
                            (void)(*pfn)(param, dir, NULL, dwRet);
                        }
                    }
                    else
                    {
                        // It has some contents, so we need to remove them

                        ss_typename_type_k traits_t::stat_data_type st;
                        file_path_buffer_t                          sz;
                        HANDLE                                      hSrch;
                        ws_size_t                                   n;
                        ws_size_t                                   dirLen = traits_t::str_len(dir);
                        ws_size_t                                   allLen = traits_t::str_len(traits_t::pattern_all());

                        traits_t::char_copy(&sz[0], dir, dirLen);
                        sz[dirLen] = '\0';
                        traits_t::ensure_dir_end(&sz[0]);
                        n = traits_t::str_len(sz.c_str());
                        WINSTL_ASSERT(n + traits_t::str_len(traits_t::pattern_all()) <= file_path_buffer_t::max_size());
                        traits_t::char_copy(&sz[n], traits_t::pattern_all(), allLen);
                        sz[n + allLen] = '\0';

                        hSrch = traits_t::find_first_file(sz.c_str(), &st);
                        if(INVALID_HANDLE_VALUE == hSrch)
                        {
                            dwRet = traits_t::get_last_error();
                        }
                        else
                        {
                            dwRet = ERROR_SUCCESS;

                            do
                            {
                                if(!traits_t::is_dots(st.cFileName))
                                {
                                    ws_size_t filenameLen = traits_t::str_len(st.cFileName);
                                    traits_t::char_copy(&sz[n], st.cFileName, filenameLen);
                                    sz[n + filenameLen] = '\0';

                                    if(traits_t::is_file(sz.c_str()))
                                    {
                                        // If it's a file, the pfn must be consulted, otherwise
                                        // it's an automatic failure

                                        ws_int_t r = 0;

                                        if( NULL == pfn ||
                                            0 == (r = (*pfn)(param, dir, &st, ERROR_SUCCESS)))
                                        {
                                            dwRet = ERROR_DIR_NOT_EMPTY;

                                            if(NULL != pfn)
                                            {
                                                // failed: { param, dir, &entry, error-code }
                                                (void)(*pfn)(param, dir, &st, dwRet);
                                            }

                                            break;
                                        }
                                        else
                                        {
                                            if(r > 0)
                                            {
                                                if(!traits_t::delete_file(sz.c_str()))
                                                {
                                                    dwRet = traits_t::get_last_error();

                                                    if(NULL != pfn)
                                                    {
                                                        // failed: { param, dir, &entry, error-code }
                                                        (void)(*pfn)(param, dir, &st, dwRet);
                                                    }

                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        ws_int_t r = 1;

                                        // If it's a directory, then pfn is consulted, otherwise
                                        // it's an automatic attempt to recursively delete
                                        if( NULL != pfn &&
                                            0 == (r = (*pfn)(param, dir, &st, ERROR_SUCCESS)))
                                        {
                                            dwRet = ERROR_DIR_NOT_EMPTY;

                                            if(NULL != pfn)
                                            {
                                                // failed: { param, dir, &entry, error-code }
                                                (void)(*pfn)(param, dir, &st, dwRet);
                                            }

                                            break;
                                        }
                                        else
                                        {
                                            if(r > 0)
                                            {
                                                dwRet = remove_directory_recurse_impl(sz.c_str(), pfn, param);

                                                if(ERROR_SUCCESS != dwRet)
                                                {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                            } while(traits_t::find_next_file(hSrch, &st));

                            traits_t::find_file_close(hSrch);

                            if(ERROR_SUCCESS == dwRet)
                            {
                                if(traits_t::remove_directory(dir))
                                {
                                    if(NULL != pfn)
                                    {
                                        // succeeded: { param, dir, NULL, ERROR_SUCCESS }
                                        (void)(*pfn)(param, dir, NULL, ERROR_SUCCESS); // Deleted
                                    }
                                }
                                else
                                {
                                    dwRet = traits_t::get_last_error();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return dwRet;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param lpsa The security attributes with which each directory is to be created
 */
inline ws_bool_t create_directory_recurse(ws_char_a_t const* dir, LPSECURITY_ATTRIBUTES lpsa = NULL)
{
    return create_directory_recurse_impl(dir, lpsa);
}

/** Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param lpsa The security attributes with which each directory is to be created
 */
inline ws_bool_t create_directory_recurse(ws_char_w_t const* dir, LPSECURITY_ATTRIBUTES lpsa = NULL)
{
    return create_directory_recurse_impl(dir, lpsa);
}

/** Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param lpsa The security attributes with which each directory is to be created
 */
template <ss_typename_param_k S>
inline ws_bool_t create_directory_recurse(S const& dir, LPSECURITY_ATTRIBUTES lpsa = NULL)
{
    return create_directory_recurse(stlsoft_ns_qual(c_str_ptr)(dir), lpsa);
}

/** Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to remove.
 * \param pfn Pointer to a callback function, which will receive
 *   notifications and requests for file/directory deletion. The semantics
 *   of the parameters are specified in the note below.
 * \param param Caller-supplied parameter, always passed through to the
 *   callback function \c pfn.
 *
 * \note If no callback function is specified, then the function will remove
 *   only empty subdirectories, i.e. no files will be removed. To remove
 *   files, a function must be supplied, and may take additional measures
 *   (such as changing file attributes) before the deletion is attempted by
 *   <code>remove_directory_recurse()</code>. Do not delete the file in the
 *   callback, otherwise the attempt within
 *   <code>remove_directory_recurse()</code> will fail, and the function
 *   will report overall failure of the operation.
 *
 * \note The semantics of the callback function's parameters are as follows:
 * \li If the err param is ~0 (-1 on UNIX), then the \c dir param specifies
 *   the name of the current directory being traversed. All other params are
 *   unspecified (except \c param). The return value is ignored.
 * \li If the err param is 0 and the \c st param is NULL, then \c dir
 *   specifies the name of a directory that has been successfully removed.
 *   All other params are unspecified (except \c param). The return value is
 *   ignored.
 * \li If the err param is 0 and the \c st param is not NULL, then \c dir
 *   specifies the name of the currently traversing directory, and \c st
 *   specifies the stat information for the entry to be deleted. Return true
 *   to enable removal of this entry, or false to prevent removal (and
 *   cancel the overall operation). All other params are unspecified (except
 *   \c param).
 * \li If the err param is any other value, and the \c st param is NULL,
 *   then the \c dir param specifies the name of a directory that could not
 *   be deleted and err specifies the errno value associated with the
 *   failure. All other params are unspecified (except \c param). The return
 *   value is ignored.
 * \li If the err param is any other value, and the \c st param is not NULL,
 *   then the \c dir param specifies the name of a directory within which an
 *   entry could not be deleted, \c st specifies the stat information of the
 *   entry that could not be deleted, and err specifies the errno value
 *   associated with the failure. All other params are unspecified (except
 *   \c param). The return value is ignored.
 */
inline
ws_bool_t
remove_directory_recurse(
    ws_char_a_t const*  dir
,   ws_int_t          (*pfn)(void* param, ws_char_a_t const* subDir, WIN32_FIND_DATAA const* st, DWORD err)
,   void*               param
)
{
    typedef filesystem_traits<ws_char_a_t>  traits_t;

    ws_dword_t dwRet = remove_directory_recurse_impl<ws_char_a_t, WIN32_FIND_DATAA>(dir, pfn, param);

    traits_t::set_last_error(dwRet);

    return ERROR_SUCCESS == dwRet;
}

/** Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline ws_bool_t remove_directory_recurse(ws_char_a_t const* dir)
{
    return remove_directory_recurse(dir, NULL, NULL);
}

/** Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline ws_bool_t remove_directory_recurse(
    ws_char_w_t const*  dir
,   ws_int_t          (*pfn)(void* param, ws_char_w_t const* subDir, WIN32_FIND_DATAW const* st, DWORD err)
,   void*               param
)
{
    typedef filesystem_traits<ws_char_w_t>  traits_t;

    ws_dword_t dwRet = remove_directory_recurse_impl<ws_char_w_t, WIN32_FIND_DATAW>(dir, pfn, param);

    traits_t::set_last_error(dwRet);

    return ERROR_SUCCESS == dwRet;
}

/** Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline ws_bool_t remove_directory_recurse(ws_char_w_t const* dir)
{
    return remove_directory_recurse(dir, NULL, NULL);
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
template <ss_typename_param_k S>
inline ws_bool_t remove_directory_recurse(S const& dir)
{
    typedef filesystem_traits<ws_char_w_t>  traits_t;

    ws_dword_t dwRet = remove_directory_recurse(stlsoft_ns_qual(c_str_ptr)(dir), NULL, NULL);

    traits_t::set_last_error(dwRet);

    return ERROR_SUCCESS == dwRet;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/directory_functions_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
