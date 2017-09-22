/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/filesystem/path_functions.hpp
 *
 * Purpose:     Helper functions for file handling
 *
 * Created:     13th June 2006
 * Updated:     2nd April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2010, Matthew Wilson and Synesis Software
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


/** \file unixstl/filesystem/path_functions.hpp
 *
 * \brief [C++ only] Helper functions for (text) file handling
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS
#define UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_MAJOR    1
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_MINOR    1
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_REVISION 5
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS_EDIT     12
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <unixstl/filesystem/filesystem_traits.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH
# include <unixstl/filesystem/path.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#if defined(_WIN32) || \
    defined(_WIN64)
# include <ctype.h>
#endif /* Windows */

#ifdef STLSOFT_UNITTEST
#endif // STLSOFT_UNITTEST

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
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
us_size_t path_squeeze_impl(
    C const*    path
,   us_size_t   pathLen
,   C*          buffer
,   us_size_t   cchBuffer
)
{
    typedef C                       char_t;
    typedef filesystem_traits<C>    traits_t;
    typedef us_size_t               size_t;

    if(NULL == buffer)
    {
        cchBuffer = pathLen + 1u;
    }
    else if(0 != cchBuffer)
    {
        basic_path<char_t>  p(path, pathLen);
        char_t const*       file_ptr    =   p.get_file();
        char_t const*       path_ptr    =   p.c_str();
        const size_t        fileLen     =   p.size() - (file_ptr - path_ptr);

        if(cchBuffer > pathLen)
        {
            // Room for all

            traits_t::char_copy(buffer, path_ptr, pathLen);
            buffer[pathLen] = '\0';

            cchBuffer = pathLen + 1u;
        }
        else
        {
            size_t rootLen;

            // Need to handle:
            //
            // 1. UNC
            // 2. drive
            // 3. rooted - begins with \ or /
            // 4. non-rooted

            if(p.is_rooted())
            {
                if(p.is_absolute())
                {
                    if(traits_t::is_path_UNC(path_ptr))
                    {
                        // 1. UNC

                        char_t const* p1 = traits_t::str_chr(path_ptr + 2, '\\');

                        rootLen = 1 + static_cast<size_t>(p1 - path_ptr);
                    }
#if defined(_WIN32) || \
    defined(_WIN64)
                    else if(isalpha(path_ptr[0]) &&
                            ':' == path_ptr[1])
                    {
                        // 2. drive
                        rootLen = 3;
                    }
#endif /* Windows */
                    else
                    {
                        // 3. rooted - begins with \ or /
                        rootLen = 1;
                    }
                }
                else
                {
                    // 3. rooted - begins with \ or /
                    rootLen = 1;
                }
            }
            else
            {
                // 4. non-rooted
                rootLen = 0;
            }

            if(cchBuffer < 5 + 1)
            {
                traits_t::char_copy(buffer, file_ptr, cchBuffer - 1);
                buffer[cchBuffer - 1] = '\0';

                if(cchBuffer > fileLen)
                {
                    cchBuffer = fileLen + 1;
                }
            }
            else if(cchBuffer < fileLen + 1)
            {
                // Squeezing just file+ext
                size_t  leftLen     =   (cchBuffer - 3 - 1) / 2;
                size_t  rightLen    =   (cchBuffer - 3 - 1) - leftLen;

                traits_t::char_copy(buffer, file_ptr, leftLen);
                buffer[leftLen + 0] = '.';
                buffer[leftLen + 1] = '.';
                buffer[leftLen + 2] = '.';
                traits_t::char_copy(buffer + leftLen + 3, file_ptr + (fileLen - rightLen), rightLen);
                buffer[leftLen + 3 + rightLen] = '\0';
            }
            else if(cchBuffer < rootLen + 3 + 1 + fileLen + 1)
            {
                // File (name + ext) only

                traits_t::char_copy(buffer, file_ptr, fileLen);
                buffer[fileLen] = '\0';

                if(cchBuffer > fileLen)
                {
                    cchBuffer = fileLen + 1;
                }
            }
            else
            {
                UNIXSTL_ASSERT(cchBuffer < pathLen + 1);

                // Squeezing
                size_t  rightLen    =   1 + fileLen;
                size_t  leftLen     =   (cchBuffer - 3 - 1) - rightLen;

                traits_t::char_copy(buffer, path_ptr, leftLen);
                buffer[leftLen + 0] = '.';
                buffer[leftLen + 1] = '.';
                buffer[leftLen + 2] = '.';
                traits_t::char_copy(buffer + leftLen + 3, file_ptr - 1, rightLen);
                buffer[leftLen + 3 + rightLen] = '\0';
            }
        }
    }

    return cchBuffer;
}


template<ss_typename_param_k S>
us_size_t path_squeeze_impl2(S const& path, us_char_a_t* buffer, us_size_t cchBuffer)
{
    return path_squeeze_impl(stlsoft_ns_qual(c_str_ptr_a)(path), stlsoft_ns_qual(c_str_len)(path), buffer, cchBuffer);
}

template<ss_typename_param_k S>
us_size_t path_squeeze_impl2(S const& path, us_char_w_t* buffer, us_size_t cchBuffer)
{
    return path_squeeze_impl(stlsoft_ns_qual(c_str_ptr_w)(path), stlsoft_ns_qual(c_str_len)(path), buffer, cchBuffer);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#if 0
template <ss_typename_param_k C>
us_size_t path_squeeze(C const* path, C* buffer, us_size_t cchBuffer)
{
    typedef filesystem_traits<C>    traits_t;

    return path_squeeze_impl(path, traits_t::str_len(path), buffer, cchBuffer);
}
#endif /* 0 */

template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        >
us_size_t path_squeeze(S const& path, C* buffer, us_size_t cchBuffer)
{
    return path_squeeze_impl2(path, buffer, cchBuffer);
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/path_functions_unittest_.h"
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

#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_PATH_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
