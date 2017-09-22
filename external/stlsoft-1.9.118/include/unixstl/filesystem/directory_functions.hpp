/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/filesystem/directory_functions.hpp
 *
 * Purpose:     Functions for manipulating directories.
 *
 * Created:     7th February 2002
 * Updated:     13th February 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2011, Matthew Wilson and Synesis Software
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


/** \file unixstl/filesystem/directory_functions.hpp
 *
 * \brief [C++ only] Functions for manipulating directories
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS
#define UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_MAJOR       3
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_MINOR       0
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_REVISION    6
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS_EDIT        43
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

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <unixstl/filesystem/filesystem_traits.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
# include <unixstl/filesystem/file_path_buffer.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */

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
 * Helper functions
 */

template <ss_typename_param_k C>
inline C* find_last_path_name_separator_(C const* s)
{
    typedef filesystem_traits<C>    traits_t;

    ss_typename_type_k traits_t::char_type const*   slash   =   traits_t::str_rchr(s, '/');
#ifdef _WIN32
    ss_typename_type_k traits_t::char_type const*   bslash  =   traits_t::str_rchr(s, '\\');

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
#endif /* _WIN32 */

    return const_cast<C*>(slash);
}

template <ss_typename_param_k C>
inline
us_bool_t
create_directory_recurse_impl(
    C const*        dir
,   unsigned short  mode
)
{
    typedef C                                   char_type;
    typedef filesystem_traits<C>                traits_t;
    typedef basic_file_path_buffer<char_type>   file_path_buffer_t;

    us_bool_t bRet;

    if( NULL == dir ||
        '\0' == *dir)
    {
        traits_t::set_last_error(ENOTDIR);

        bRet = false;
    }
    else
    {
        if(traits_t::file_exists(dir))
        {
            if(traits_t::is_directory(dir))
            {
                traits_t::set_last_error(EISDIR);

                bRet = true;
            }
            else
            {
                traits_t::set_last_error(EEXIST);

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
                us_size_t const dirLen = traits_t::str_len(dir);

                if((dirLen + 1) > sz.size())
                {
                    traits_t::set_last_error(EINVAL);

                    bRet = false;
                }
                else
                {
                    traits_t::char_copy(&sz[0], dir, dirLen + 1);
                    traits_t::remove_dir_end(&sz[0]);

                    if( traits_t::create_directory(sz.c_str(), mode) ||
                        EEXIST == traits_t::get_last_error())
                    {
                        traits_t::set_last_error(0);

                        bRet = true;
                    }
                    else
                    {
                        // Trim previous directory
                        traits_t::char_copy(&szParent[0], sz.c_str(), dirLen + 1);

                        char_type* pszSlash = find_last_path_name_separator_(szParent.c_str());
                        if(pszSlash == NULL)
                        {
                            traits_t::set_last_error(ENOTDIR);

                            bRet = false;
                        }
                        else
                        {
                            *pszSlash = '\0';   // Will always have enough room for two bytes

                            // If second character is ':', and total lengths is less than four,
                            // or the recurse create fails, then return false;
                            if( (   szParent[1] == ':' &&
                                    (traits_t::set_last_error(EACCES), traits_t::str_len(szParent.c_str()) < 4)) ||
                                !create_directory_recurse_impl(szParent.c_str(), mode))
                            {
                                bRet = false;
                            }
                            else
                            {
                                bRet = traits_t::create_directory(sz.c_str(), mode) || EEXIST == traits_t::get_last_error();
                            }
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
us_int_t
remove_directory_recurse_impl(
    C const*    dir
,   us_int_t (*pfn)(void* param, C const* subDir, FD const* st, struct dirent const* de, int err)
,   void*       param
)
{
    typedef C                                   char_type;
    typedef filesystem_traits<C>                traits_t;
    typedef basic_file_path_buffer<char_type>   file_path_buffer_t;
    us_int_t                                    dwRet;

    if(NULL != pfn)
    {
        (void)(*pfn)(param, dir, NULL, NULL, ~0);   // Entering
    }

    if( NULL == dir ||
        '\0' == *dir)
    {
        dwRet = ENOTDIR;

        if(NULL != pfn)
        {
            (void)(*pfn)(param, dir, NULL, NULL, dwRet);
        }
    }
    else
    {
        if(!traits_t::file_exists(dir))
        {
            // The given path does not exist, so this is treated as success, but
            // reporting ENOENT

            dwRet = ENOENT;

            if(NULL != pfn)
            {
                (void)(*pfn)(param, dir, NULL, NULL, dwRet);
            }
        }
        else
        {
            if(traits_t::is_file(dir))
            {
                // The given path exists as a file. This is failure
                dwRet = EEXIST;

                if(NULL != pfn)
                {
                    (void)(*pfn)(param, dir, NULL, NULL, dwRet);
                }
            }
            else
            {
                // Otherwise, we attempt to remove it
                if(traits_t::remove_directory(dir))
                {
                    dwRet = 0;

                    if(NULL != pfn)
                    {
                        (void)(*pfn)(param, dir, NULL, NULL, dwRet); // Deleted
                    }
                }
                else
                {
                    const int removeError = traits_t::get_last_error();

                    if(ENOTEMPTY != removeError)
                    {
                        dwRet = removeError;

                        if(NULL != pfn)
                        {
                            (void)(*pfn)(param, dir, NULL, NULL, dwRet);
                        }
                    }
                    else
                    {
                        // It has some contents, so we need to remove them

                        file_path_buffer_t  sz;
                        DIR*                hSrch;
                        us_size_t           n;
                        us_size_t const     dirLen = traits_t::str_len(dir);

                        traits_t::char_copy(&sz[0], dir, dirLen + 1);
                        traits_t::ensure_dir_end(&sz[0]);
                        n = traits_t::str_len(sz.c_str());

                        hSrch = traits_t::open_dir(sz.c_str());
                        if(NULL == hSrch)
                        {
                            dwRet = traits_t::get_last_error();
                        }
                        else
                        {
                            dwRet = 0;

                            for(struct dirent const* de; 0 == dwRet && NULL != (de = traits_t::read_dir(hSrch)); )
                            {
                                if(!traits_t::is_dots(de->d_name))
                                {
                                    ss_typename_type_k traits_t::stat_data_type st;

                                    us_size_t const denameLen = traits_t::str_len(de->d_name);

                                    traits_t::char_copy(&sz[0] + n, de->d_name, denameLen + 1);
                                    if(!traits_t::stat(sz.c_str(), &st))
                                    {
                                        dwRet = traits_t::get_last_error();

                                        if(NULL != pfn)
                                        {
                                            (void)(*pfn)(param, dir, NULL, de, dwRet);
                                        }
                                    }
                                    else
                                    {
                                        if(traits_t::is_file(&st))
                                        {
                                            // If it's a file, the pfn must be consulted, otherwise
                                            // it's an automatic failure

                                            if( NULL == pfn ||
                                                !(*pfn)(param, dir, &st, de, 0))
                                            {
                                                dwRet = ENOTEMPTY;

                                                if(NULL != pfn)
                                                {
                                                    (void)(*pfn)(param, dir, &st, de, dwRet);
                                                }

                                                break;
                                            }
                                            else
                                            {
                                                if(!traits_t::delete_file(sz.c_str()))
                                                {
                                                    dwRet = traits_t::get_last_error();

                                                    if(NULL != pfn)
                                                    {
                                                        (void)(*pfn)(param, dir, &st, de, dwRet);
                                                    }

                                                    break;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // If it's a directory, then pfn is consulted, otherwise
                                            // it's an automatic attempt to recursively delete
                                            if( NULL != pfn &&
                                                !(*pfn)(param, dir, &st, de, 0))
                                            {
                                                dwRet = ENOTEMPTY;

                                                if(NULL != pfn)
                                                {
                                                    (void)(*pfn)(param, dir, &st, de, dwRet);
                                                }

                                                break;
                                            }
                                            else
                                            {
                                                dwRet = remove_directory_recurse_impl(sz.c_str(), pfn, param);
                                            }
                                        }
                                    }
                                }
                            }

                            traits_t::close_dir(hSrch);

                            if(0 == dwRet)
                            {
                                if(traits_t::remove_directory(dir))
                                {
                                    if(NULL != pfn)
                                    {
                                        (void)(*pfn)(param, dir, NULL, NULL, 0); // Deleted
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

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

/** \brief Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param mode The permissions with which each directory is to be created
 */
inline us_bool_t create_directory_recurse(us_char_a_t const* dir, unsigned short mode = 0755)
{
    return create_directory_recurse_impl(dir, mode);
}

#if 0
/** \brief Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param mode The permissions with which each directory is to be created
 */
inline us_bool_t create_directory_recurse(us_char_w_t const* dir, unsigned short mode = 0755)
{
    return create_directory_recurse_impl(dir, mode);
}
#endif /* 0 */

/** \brief Creates the given directory, including all its parent directories, applying
 * the given mode.
 *
 * \ingroup group__library__filesystem
 *
 * \param dir The path of the directory to create
 * \param mode The permissions with which each directory is to be created
 */
template <ss_typename_param_k S>
inline us_bool_t create_directory_recurse(S const& dir, unsigned short mode = 0755)
{
    return create_directory_recurse(stlsoft_ns_qual(c_str_ptr)(dir), mode);
}

/** \brief Removes the given directory, and all its subdirectories.
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
 *   specifies the name of the currently traversing directory, \c st
 *   specifies the stat information for the entry to be deleted, and \c de
 *   specifies the name of the entry within directory \c dir that is a
 *   candidate for removal. Return true to enable removal of this entry, or
 *   false to prevent removal (and cancel the overall operation). All other
 *   params are unspecified (except \c param). The return value is ignored.
 * \li If the err param is any other value, and the \c st param is NULL,
 *   then the \c dir param specifies the name of a directory that could not
 *   be deleted and err specifies the errno value associated with the
 *   failure. All other params are unspecified (except \c param). The return
 *   value is ignored.
 * \li If the err param is any other value, and the \c st param is not NULL,
 *   then the \c dir param specifies the name of a directory within which an
 *   entry could not be deleted, \c st specifies the stat information of the
 *   entry that could not be deleted, \c de specifies the name of the entry
 *   that could not be deleted, and err specifies the errno value associated
 *   with the failure. All other params are unspecified (except \c param).
 *   The return value is ignored.
 */
inline us_bool_t remove_directory_recurse(
    us_char_a_t const*  dir
,   us_int_t            (*pfn)(void* param, us_char_a_t const* subDir, struct stat const* st, struct dirent const* de, int err)
,   void*               param
)
{
    typedef filesystem_traits<us_char_a_t> traits_t;

    us_int_t dwRet = remove_directory_recurse_impl<us_char_a_t, struct stat>(dir, pfn, param);

    traits_t::set_last_error(dwRet);

    return 0 == dwRet;
}

/** \brief Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline us_bool_t remove_directory_recurse(us_char_a_t const* dir)
{
    return remove_directory_recurse(dir, NULL, NULL);
}

#if 0
/** \brief Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline us_bool_t remove_directory_recurse(
    us_char_w_t const*  dir
,   us_int_t            (*pfn)(void* param, us_char_w_t const* subDir, struct stat const* st, struct dirent const* de, int err)
,   void*               param
)
{
    typedef filesystem_traits<us_char_w_t>  traits_t;

    us_int_t dwRet = remove_directory_recurse_impl<us_char_w_t, struct stat>(dir, pfn, param);

    traits_t::set_last_error(dwRet);

    return 0 == dwRet;
}

/** \brief Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
inline us_bool_t remove_directory_recurse(us_char_w_t const* dir)
{
    return remove_directory_recurse(dir, NULL, NULL);
}
#endif /* 0 */

/** \brief Removes the given directory, and all its subdirectories.
 *
 * \ingroup group__library__filesystem
 */
template <ss_typename_param_k S>
inline us_bool_t remove_directory_recurse(S const& dir)
{
    typedef filesystem_traits<us_char_a_t>  traits_t;

    us_int_t dwRet = remove_directory_recurse(stlsoft_ns_qual(c_str_ptr)(dir), NULL, NULL);

    traits_t::set_last_error(dwRet);

    return 0 == dwRet;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/directory_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_DIRECTORY_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
