/* /////////////////////////////////////////////////////////////////////////
 * File:        inetstl/shims/access/string/std/in_addr.hpp
 *
 * Purpose:     String access shims for Internet types
 *
 * Created:     21st October 2006
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


/** \file inetstl/shims/access/string/std/in_addr.hpp
 *
 * \brief [C++] Definition of the string access shims for
 *   <code>in_addr</code>
 *   (\ref group__concept__shim__string_access "String Access Shims" Concept).
 */

#ifndef INETSTL_INCL_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR
#define INETSTL_INCL_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define INETSTL_VER_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR_MAJOR      1
# define INETSTL_VER_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR_MINOR      0
# define INETSTL_VER_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR_REVISION   6
# define INETSTL_VER_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR_EDIT       10
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef INETSTL_INCL_INETSTL_H_INETSTL
# include <inetstl/inetstl.h>
#endif /* !INETSTL_INCL_INETSTL_H_INETSTL */
#ifndef INETSTL_INCL_INETSTL_INCLUDES_STD_H_IN_ADDR
# include <inetstl/includes/std/in_addr.h>
#endif /* !INETSTL_INCL_INETSTL_INCLUDES_STD_H_IN_ADDR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */
#ifndef STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR
# include <stlsoft/internal/safestr.h>
#endif /* !STLSOFT_INCL_STLSOFT_INTERNAL_H_SAFESTR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::inetstl */
namespace inetstl
{
# else
/* Define stlsoft::inetstl_project */

namespace stlsoft
{

namespace inetstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

inline stlsoft::basic_shim_string<is_char_a_t, 16> c_str_data_a(struct in_addr const& addr)
{
    stlsoft::basic_shim_string<is_char_a_t, 16>     s(15);

    is_uint32_t laddr = ntohl(addr.s_addr);
    unsigned    b3  =   (laddr & 0x000000ff) >>  0;
    unsigned    b2  =   (laddr & 0x0000ff00) >>  8;
    unsigned    b1  =   (laddr & 0x00ff0000) >> 16;
    unsigned    b0  =   (laddr & 0xff000000) >> 24;
# ifdef STLSOFT_USING_SAFE_STR_FUNCTIONS
    int         cch =   ::_snprintf_s(
                                s.data()
                            ,   s.size() + 1u
                            ,   _TRUNCATE
# else /* ? STLSOFT_USING_SAFE_STR_FUNCTIONS */
    int         cch =   ::sprintf(s.data()
# endif /* STLSOFT_USING_SAFE_STR_FUNCTIONS */
                            ,   "%u.%u.%u.%u"
                            ,   b0, b1, b2, b3
                            );

    if(cch < 0)
    {
        cch = 0;
    }

    s.truncate(static_cast<is_size_t>(cch));

    return s;
}

inline is_size_t c_str_len_a(struct in_addr const& addr)
{
    // Don't care about the byte order of the address, as we're only
    // calculating length
    is_uint32_t laddr = addr.s_addr;

    unsigned    b0  =   (laddr & 0x000000ff) >>  0;
    unsigned    b1  =   (laddr & 0x0000ff00) >>  8;
    unsigned    b2  =   (laddr & 0x00ff0000) >> 16;
    unsigned    b3  =   (laddr & 0xff000000) >> 24;

    int         cch =   3   // The dot separators
                    +   1 + (b0 > 9) + (b0 > 99)
                    +   1 + (b1 > 9) + (b1 > 99)
                    +   1 + (b2 > 9) + (b2 > 99)
                    +   1 + (b3 > 9) + (b3 > 99)
                    ;

    return static_cast<is_size_t>(cch);
}

inline stlsoft::basic_shim_string<is_char_a_t, 16> c_str_ptr_a(struct in_addr const& addr)
{
    return c_str_data_a(addr);
}

inline stlsoft::basic_shim_string<is_char_a_t, 16> c_str_data_a(struct in_addr const* addr)
{
    typedef stlsoft::basic_shim_string<is_char_a_t, 16>     shim_string_t;

    if(NULL != addr)
    {
        return c_str_data_a(*addr);
    }
    else
    {
        return shim_string_t(is_size_t(0));
    }
}

inline is_size_t c_str_len_a(struct in_addr const* addr)
{
    typedef stlsoft::basic_shim_string<is_char_a_t, 16>     shim_string_t;

    if(NULL != addr)
    {
        return c_str_len_a(*addr);
    }
    else
    {
        return shim_string_t(is_size_t(0));
    }
}

inline stlsoft::basic_shim_string<is_char_a_t, 16> c_str_ptr_a(struct in_addr const* addr)
{
    return c_str_data_a(addr);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _INETSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace inetstl
# else
} // namespace inetstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _INETSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::inetstl::c_str_data_a;
using ::inetstl::c_str_len_a;
using ::inetstl::c_str_ptr_a;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_INETSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* INETSTL_INCL_INETSTL_SHIMS_ACCESS_STRING_STD_HPP_IN_ADDR */

/* ///////////////////////////// end of file //////////////////////////// */
