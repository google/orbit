/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/error/error_functions.h (originally MWBase.h, ::SynesisWin)
 *
 * Purpose:     Error functions.
 *
 * Created:     7th May 2000
 * Updated:     7th April 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2000-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/error/error_functions.h
 *
 * \brief [C, C++] Windows error manipulation and representation
 *   functions
 *   (\ref group__library__error "Error" Library).
 */

#ifndef WINSTL_INCL_WINSTL_ERROR_H_ERROR_FUNCTIONS
#define WINSTL_INCL_WINSTL_ERROR_H_ERROR_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_ERROR_H_ERROR_FUNCTIONS_MAJOR     4
# define WINSTL_VER_WINSTL_ERROR_H_ERROR_FUNCTIONS_MINOR     4
# define WINSTL_VER_WINSTL_ERROR_H_ERROR_FUNCTIONS_REVISION  3
# define WINSTL_VER_WINSTL_ERROR_H_ERROR_FUNCTIONS_EDIT      69
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(_WINSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
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
 * Constants
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#define WINSTL_ERROR_FUNCTIONS_ELIDE_DOT                (0x0001)
#define WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY   (0x0002)

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION



/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
ws_char_a_t const*
winstl_C_fmtmsg_empty_reason_unknown_a()
{
    static ws_char_a_t const s_reason_unknown[] = "reason unknown";

    return s_reason_unknown;
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
ws_char_a_t const*
winstl_C_fmtmsg_empty_string_a()
{
    static ws_char_a_t const s_empty[1] = { '\0' };

    return s_empty;
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
ws_char_a_t*
winstl_C_fmtmsg_elide_message_a_(
    ws_char_a_t*    first
,   ws_char_a_t*    last
,   int             elisionFlags
)
{
    ws_char_a_t const* firstDot = NULL;

    if(WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY & elisionFlags)
    {
        ws_char_a_t const* s;

        for(s = first; s != last; ++s)
        {
            if('.' == *s)
            {
                firstDot = s;
                break;
            }
        }
    }

    for(; first != last; )
    {
        switch(*(last - 1))
        {
            case    '.':
                if( (WINSTL_ERROR_FUNCTIONS_ELIDE_DOT & elisionFlags) &&
                    (   0 == (WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY & elisionFlags) ||
                        NULL == firstDot ||
                        firstDot == last - 1))
                {
            case    ' ':
            case    '\t':
            case    '\r':
            case    '\n':
                *(last - 1) = '\0';
                --last;
                break;
                }
                else
                {
            default:
                first = last;
                break;
                }
        }
    }

    return last;
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
ws_char_w_t*
winstl_C_fmtmsg_elide_message_w_(
    ws_char_w_t*    first
,   ws_char_w_t*    last
,   int             elisionFlags
)
{
    ws_char_w_t const* firstDot = NULL;

    if(WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY & elisionFlags)
    {
        ws_char_w_t const* s;

        for(s = first; s != last; ++s)
        {
            if(L'.' == *s)
            {
                firstDot = s;
                break;
            }
        }
    }

    for(; first != last; )
    {
        switch(*(last - 1))
        {
            case    L'.':
                if( (WINSTL_ERROR_FUNCTIONS_ELIDE_DOT & elisionFlags) &&
                    (   0 == (WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY & elisionFlags) ||
                        NULL == firstDot ||
                        firstDot == last - 1))
                {
            case    L' ':
            case    L'\t':
            case    L'\r':
            case    L'\n':
                *(last - 1) = L'\0';
                --last;
                break;
                }
                else
                {
            default:
                first = last;
                break;
                }
        }
    }

    return last;
}

/**
 *
 *
 * \param flags. Automatically added to this are FORMAT_MESSAGE_ALLOCATE_BUFFER and, if source is NULL, FORMAT_MESSAGE_FROM_SYSTEM
 * \param source
 *
 * \note The flags are altered in the following
 *   ways: \c FORMAT_MESSAGE_ALLOCATE_BUFFER is always
 *   added; \c FORMAT_MESSAGE_FROM_SYSTEM is added if
 *   \c source is \c NULL.
 * 
 * \ingroup group__library__error
 */
STLSOFT_INLINE
DWORD
winstl_C_FormatMessageA_INVOKE_for_alloc_(
    DWORD           flags
,   LPCVOID         source
,   DWORD           code
,   DWORD           languageId
,   LPSTR*          ppBuffer
,   DWORD           maxSize
,   va_list*        arguments
)
{
    if(NULL == source)
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    flags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;

    return STLSOFT_NS_GLOBAL(FormatMessageA)(
        flags
    ,   source
    ,   code
    ,   languageId
    ,   stlsoft_reinterpret_cast(LPSTR, ppBuffer)
    ,   maxSize
    ,   arguments
    );
}

/**
 *
 * 
 * \param flags. Automatically added to this are FORMAT_MESSAGE_ALLOCATE_BUFFER and, if source is NULL, FORMAT_MESSAGE_FROM_SYSTEM
 * \param source
 * 
 * \note The flags are altered in the following
 *   ways: \c FORMAT_MESSAGE_ALLOCATE_BUFFER is always
 *   added; \c FORMAT_MESSAGE_FROM_SYSTEM is added if
 *   \c source is \c NULL.
 * 
 * \ingroup group__library__error
 */
STLSOFT_INLINE
DWORD
winstl_C_FormatMessageW_INVOKE_for_alloc_(
    DWORD           flags
,   LPCVOID         source
,   DWORD           code
,   DWORD           languageId
,   LPWSTR*         ppBuffer
,   DWORD           maxSize
,   va_list*        arguments
)
{
    if(NULL == source)
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    flags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;

    return STLSOFT_NS_GLOBAL(FormatMessageW)(
        flags
    ,   source
    ,   code
    ,   languageId
    ,   stlsoft_reinterpret_cast(LPWSTR, ppBuffer)
    ,   maxSize
    ,   arguments
    );
}


/**
 *
 * 
 * \param flags. Automatically removed from this is FORMAT_MESSAGE_ALLOCATE_BUFFER and added to this, if source is NULL, is FORMAT_MESSAGE_FROM_SYSTEM
 * \param source
 * 
 * \note The flags are altered in the following
 *   ways: \c FORMAT_MESSAGE_ALLOCATE_BUFFER is always
 *   removed; \c FORMAT_MESSAGE_FROM_SYSTEM is added if
 *   \c source is \c NULL.
 * 
 * \ingroup group__library__error
 */
STLSOFT_INLINE
DWORD
winstl_C_FormatMessageA_INVOKE_in_buffer_(
    DWORD           flags
,   LPCVOID         source
,   DWORD           code
,   DWORD           languageId
,   LPSTR           buffer
,   DWORD           cchBuffer
,   va_list*        arguments
)
{
    if(NULL == source)
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    flags &= ~(FORMAT_MESSAGE_ALLOCATE_BUFFER);

    return STLSOFT_NS_GLOBAL(FormatMessageA)(
        flags
    ,   source
    ,   code
    ,   languageId
    ,   buffer
    ,   cchBuffer
    ,   arguments
    );
}

/**
 *
 * 
 * \param flags. Automatically removed from this is FORMAT_MESSAGE_ALLOCATE_BUFFER and added to this, if source is NULL, is FORMAT_MESSAGE_FROM_SYSTEM
 * \param source
 * 
 * \note The flags are altered in the following
 *   ways: \c FORMAT_MESSAGE_ALLOCATE_BUFFER is always
 *   removed; \c FORMAT_MESSAGE_FROM_SYSTEM is added if
 *   \c source is \c NULL.
 * 
 * \ingroup group__library__error
 */
STLSOFT_INLINE
DWORD
winstl_C_FormatMessageW_INVOKE_in_buffer_(
    DWORD           flags
,   LPCVOID         source
,   DWORD           code
,   DWORD           languageId
,   LPWSTR          buffer
,   DWORD           cchBuffer
,   va_list*        arguments
)
{
    if(NULL == source)
    {
        flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    }
    flags &= ~(FORMAT_MESSAGE_ALLOCATE_BUFFER);

    return STLSOFT_NS_GLOBAL(FormatMessageW)(
        flags
    ,   source
    ,   code
    ,   languageId
    ,   buffer
    ,   cchBuffer
    ,   arguments
    );
}


/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_FormatMessageA__buff_inst)
#endif
STLSOFT_INLINE
ws_dword_t
winstl_C_FormatMessageA__buff_inst(
    int             flags
,   HINSTANCE       hinst
,   DWORD           error
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    if(NULL != hinst)
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_SYSTEM);
        flags = flags | (FORMAT_MESSAGE_FROM_HMODULE);
    }
    else
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_HMODULE);
    }

    return winstl_C_FormatMessageA_INVOKE_in_buffer_(
        stlsoft_static_cast(ws_dword_t, flags)
    ,   stlsoft_static_cast(LPCVOID, hinst)
    ,   error
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   buffer
    ,   stlsoft_static_cast(DWORD, cchBuffer)
    ,   NULL
    );
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_FormatMessageW__buff_inst)
#endif
STLSOFT_INLINE
ws_dword_t
winstl_C_FormatMessageW__buff_inst(
    int             flags
,   HINSTANCE       hinst
,   DWORD           error
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    if(NULL != hinst)
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_SYSTEM);
        flags = flags | (FORMAT_MESSAGE_FROM_HMODULE);
    }
    else
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_HMODULE);
    }

    return winstl_C_FormatMessageW_INVOKE_in_buffer_(
        stlsoft_static_cast(ws_dword_t, flags)
    ,   stlsoft_static_cast(LPCVOID, hinst)
    ,   error
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   buffer
    ,   stlsoft_static_cast(DWORD, cchBuffer)
    ,   NULL
    );
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_FormatMessageA__alloc_inst)
#endif
STLSOFT_INLINE
ws_dword_t
winstl_C_FormatMessageA__alloc_inst(
    int             flags
,   HINSTANCE       hinst
,   DWORD           error
,   ws_char_a_t**   buffer
)
{
    if(NULL != hinst)
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_SYSTEM);
        flags = flags | (FORMAT_MESSAGE_FROM_HMODULE);
    }
    else
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_HMODULE);
    }

    return winstl_C_FormatMessageA_INVOKE_for_alloc_(
        stlsoft_static_cast(ws_dword_t, flags)
    ,   stlsoft_static_cast(LPCVOID, hinst)
    ,   error
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   buffer
    ,   0
    ,   NULL
    );
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_FormatMessageW__alloc_inst)
#endif
STLSOFT_INLINE
ws_dword_t
winstl_C_FormatMessageW__alloc_inst(
    int             flags
,   HINSTANCE       hinst
,   DWORD           error
,   ws_char_w_t**   buffer
)
{
    if(NULL != hinst)
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_SYSTEM);
        flags = flags | (FORMAT_MESSAGE_FROM_HMODULE);
    }
    else
    {
        flags = flags & ~(FORMAT_MESSAGE_FROM_HMODULE);
    }

    return winstl_C_FormatMessageW_INVOKE_for_alloc_(
        stlsoft_static_cast(ws_dword_t, flags)
    ,   stlsoft_static_cast(LPCVOID, hinst)
    ,   error
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   buffer
    ,   0
    ,   NULL
    );
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
void
winstl_C_fmtmsg_LocalFree__(void *pv)
{
    STLSOFT_NS_GLOBAL(LocalFree)(pv);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C functions
 */

/** Translates the 
 *
 * \ingroup group__library__error
 *
 *
 */
STLSOFT_INLINE
ws_dword_t
winstl_C_format_message_from_module_to_allocated_buffer_a(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_a_t**   ppBuffer
,   int             elisionFlags
)
{
    ws_dword_t r = winstl_C_FormatMessageA_INVOKE_for_alloc_(
        flags
    ,   hModule
    ,   code
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   ppBuffer
    ,   0
    ,   NULL
    );

    if( 0 != r &&
        0 != elisionFlags)
    {
        ws_char_a_t* newLast =
        winstl_C_fmtmsg_elide_message_a_(
            *ppBuffer
        ,   *ppBuffer + r
        ,   elisionFlags
        );

        r = stlsoft_static_cast(ws_dword_t, newLast - *ppBuffer);
    }

    return r;
}

/** Translates the 
 *
 * \ingroup group__library__error
 *
 *
 */
STLSOFT_INLINE
ws_dword_t
winstl_C_format_message_from_module_to_allocated_buffer_w(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_w_t**   ppBuffer
,   int             elisionFlags
)
{
    ws_dword_t r = winstl_C_FormatMessageW_INVOKE_for_alloc_(
        flags
    ,   hModule
    ,   code
    ,   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
    ,   ppBuffer
    ,   0
    ,   NULL
    );

    if( 0 != r &&
        0 != elisionFlags)
    {
        ws_char_w_t* newLast =
        winstl_C_fmtmsg_elide_message_w_(
            *ppBuffer
        ,   *ppBuffer + r
        ,   elisionFlags
        );

        r = stlsoft_static_cast(ws_dword_t, newLast - *ppBuffer);
    }

    return r;
}


/** \brief Translates the given error to an error string and
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_inst_a)
#endif
STLSOFT_INLINE ws_dword_t winstl_C_format_message_buff_inst_a(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    ws_dword_t res = winstl_C_FormatMessageA__buff_inst(FORMAT_MESSAGE_MAX_WIDTH_MASK, hinst, error, buffer, cchBuffer);

    if(res != 0)
    {
        /* Now trim the trailing space */
        ws_char_a_t* last_good = winstl_C_fmtmsg_elide_message_a_(buffer, buffer + res, WINSTL_ERROR_FUNCTIONS_ELIDE_DOT);

        WINSTL_ASSERT((last_good - buffer) >= 0);

        *last_good = 0;
        res = stlsoft_static_cast(ws_dword_t, last_good - buffer);
    }

    return res;
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_inst_w)
#endif
STLSOFT_INLINE ws_dword_t winstl_C_format_message_buff_inst_w(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    ws_dword_t res = winstl_C_FormatMessageW__buff_inst(FORMAT_MESSAGE_MAX_WIDTH_MASK, hinst, error, buffer, cchBuffer);

    if(res != 0)
    {
        /* Now trim the trailing space */
        ws_char_w_t* last_good = winstl_C_fmtmsg_elide_message_w_(buffer, buffer + res, WINSTL_ERROR_FUNCTIONS_ELIDE_DOT);

        WINSTL_ASSERT((last_good - buffer) >= 0);

        *last_good = 0;
        res = stlsoft_static_cast(ws_dword_t, last_good - buffer);
    }

    return res;
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_a)
#endif
STLSOFT_INLINE ws_dword_t winstl_C_format_message_buff_a(
    DWORD           error
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_a(error, NULL, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_w)
#endif
STLSOFT_INLINE ws_dword_t winstl_C_format_message_buff_w(
    DWORD           error
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_w(error, NULL, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_alloc_a)
#endif
STLSOFT_INLINE ws_dword_t winstl_C_format_message_alloc_a(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_a_t**   buffer
)
{
    ws_dword_t res = winstl_C_FormatMessageA__alloc_inst(FORMAT_MESSAGE_MAX_WIDTH_MASK, hinst, error, buffer);

    if(res != 0)
    {
        /* Now trim the trailing space */
        ws_char_a_t* last_good = winstl_C_fmtmsg_elide_message_a_(*buffer, *buffer + res, WINSTL_ERROR_FUNCTIONS_ELIDE_DOT);

        WINSTL_ASSERT((last_good - *buffer) >= 0);

        *last_good = 0;
        res = stlsoft_static_cast(ws_dword_t, last_good - *buffer);
    }

    return res;
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE ws_dword_t winstl_C_format_message_alloc_w(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_w_t**   buffer
)
{
    ws_dword_t res = winstl_C_FormatMessageW__alloc_inst(FORMAT_MESSAGE_MAX_WIDTH_MASK, hinst, error, buffer);

    if(res != 0)
    {
        /* Now trim the trailing space */
        ws_char_w_t* last_good = winstl_C_fmtmsg_elide_message_w_(*buffer, *buffer + res, WINSTL_ERROR_FUNCTIONS_ELIDE_DOT);

        WINSTL_ASSERT((last_good - *buffer) >= 0);

        *last_good = 0;
        res = stlsoft_static_cast(ws_dword_t, last_good - *buffer);
    }

    return res;
}

/** Functional equivalent to the C standard library 
 *   function <code>strerror()</code> for the Windows API, except that the
 *   returned pointer must be freed
 *   (by <code>winstl_C_format_message_free_buff_a()</code>) to avoid
 *   a memory leak.
 *
 * 
 * \return Always a non-NULL pointer to a nul-terminated multibyte string.
 *
 * \note The returned pointer must be released by a call 
 *   to <code>winstl_C_format_message_free_buff_a()</code>
 */
STLSOFT_INLINE
ws_char_a_t*
winstl_C_format_message_strerror_a(
    DWORD           code
)
{
    ws_char_a_t*    p;
    DWORD const     n = winstl_C_format_message_from_module_to_allocated_buffer_a(0, NULL, code, &p, WINSTL_ERROR_FUNCTIONS_ELIDE_DOT | WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY);

    if(0 == n)
    {
        /* If nothing was retrieved, we try to write a number into it */

        WINSTL_ASSERT(NULL == p);

        p = stlsoft_static_cast(ws_char_a_t*, STLSOFT_NS_GLOBAL(LocalAlloc)(LMEM_FIXED, sizeof(ws_char_a_t) * 21));

        if(NULL == p)
        {
            return stlsoft_const_cast(ws_char_a_t*, winstl_C_fmtmsg_empty_reason_unknown_a());
        }
        else
        {
            wsprintfA(p, "%lu", stlsoft_static_cast(unsigned long, code));
        }
    }
    else
    {
        WINSTL_ASSERT(NULL != p);
    }

    return p;
}



/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
void
winstl_C_format_message_free_buff_a(ws_char_a_t* buffer)
{
    if(winstl_C_fmtmsg_empty_reason_unknown_a() == buffer)
    {
        return;
    }
    if(winstl_C_fmtmsg_empty_reason_unknown_a() == buffer)
    {
        return;
    }

    winstl_C_fmtmsg_LocalFree__(buffer);
}

/**
 *
 * \ingroup group__library__error
 */
STLSOFT_INLINE
void
winstl_C_format_message_free_buff_w(ws_char_w_t* buffer)
{
    winstl_C_fmtmsg_LocalFree__(buffer);
}

/* /////////////////////////////////////////////////////////////////////////
 * Character-encoding independent symbols
 */

#ifdef __cplusplus

STLSOFT_INLINE
ws_dword_t
winstl_C_format_message_from_module_to_allocated_buffer(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_a_t**   ppBuffer
,   int             elisionFlags
)
{
    return winstl_C_format_message_from_module_to_allocated_buffer_a(flags, hModule, code, ppBuffer, elisionFlags);
}

STLSOFT_INLINE
ws_dword_t
winstl_C_format_message_from_module_to_allocated_buffer(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_w_t**   ppBuffer
,   int             elisionFlags
)
{
    return winstl_C_format_message_from_module_to_allocated_buffer_w(flags, hModule, code, ppBuffer, elisionFlags);
}


/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_inst)
#endif
inline ws_dword_t winstl_C_format_message_buff_inst(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_a(error, hinst, buffer, cchBuffer);
}

/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff_inst)
#endif
inline ws_dword_t winstl_C_format_message_buff_inst(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_w(error, hinst, buffer, cchBuffer);
}

/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff)
#endif
inline ws_dword_t winstl_C_format_message_buff(
    DWORD           error
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_a(error, buffer, cchBuffer);
}

/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_buff)
#endif
inline ws_dword_t winstl_C_format_message_buff(
    DWORD           error
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_w(error, buffer, cchBuffer);
}

/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_alloc)
#endif
inline ws_dword_t winstl_C_format_message_alloc(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_a_t**   buffer
)
{
    return winstl_C_format_message_alloc_a(error, hinst, buffer);
}

/**
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
#if _STLSOFT_VER >= 0x010a0000
STLSOFT_DECLARE_FUNCTION_DEPRECATION(winstl_C_format_message_alloc)
#endif
inline ws_dword_t winstl_C_format_message_alloc(
    DWORD           error
,   HINSTANCE       hinst
,   ws_char_w_t**   buffer
)
{
    return winstl_C_format_message_alloc_w(error, hinst, buffer);
}

inline void winstl_C_format_message_free_buff(ws_char_a_t* buffer)
{
    winstl_C_format_message_free_buff_a(buffer);
}

inline void winstl_C_format_message_free_buff(ws_char_w_t* buffer)
{
    winstl_C_format_message_free_buff_w(buffer);
}

#else /* ? __cplusplus */

# ifdef UNICODE
#  define winstl_C_format_message_buff_inst     winstl_C_format_message_buff_inst_w
#  define winstl_C_format_message_buff          winstl_C_format_message_buff_w
#  define winstl_C_format_message_alloc         winstl_C_format_message_alloc_w
#  define winstl_C_format_message_free_buff     winstl_C_format_message_free_buff_w
# else /* ? UNICODE */
#  define winstl_C_format_message_buff_inst     winstl_C_format_message_buff_inst_a
#  define winstl_C_format_message_buff          winstl_C_format_message_buff_a
#  define winstl_C_format_message_alloc         winstl_C_format_message_alloc_a
#  define winstl_C_format_message_free_buff     winstl_C_format_message_free_buff_a
# endif /* UNICODE */

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete symbols
 *
 * NOTE: these are only defined if:
 *
 * - we're generating documentation, or
 * - STLSOFT_OBSOLETE is specified, or
 * - it's STLSoft 1.9 (or earlier)
 */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
    defined(STLSOFT_OBSOLETE) || \
    _STLSOFT_VER < 0x010a0000

/** \def winstl__format_message_buff_inst_a
 *
 * \deprecated Use winstl_C_format_message_buff_inst_a
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_buff_inst_a, winstl_C_format_message_buff_inst_a)
# define winstl__format_message_buff_inst_a  winstl_C_format_message_buff_inst_a

/** \def winstl__format_message_buff_inst_w
 *
 * \deprecated Use winstl_C_format_message_buff_inst_w
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_buff_inst_w, winstl_C_format_message_buff_inst_w)
# define winstl__format_message_buff_inst_w  winstl_C_format_message_buff_inst_w

/** \def winstl__format_message_buff_a
 *
 * \deprecated Use winstl_C_format_message_buff_a
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_buff_a, winstl_C_format_message_buff_a)
# define winstl__format_message_buff_a       winstl_C_format_message_buff_a

/** \def winstl__format_message_buff_w
 *
 * \deprecated Use winstl_C_format_message_buff_w
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_buff_w, winstl_C_format_message_buff_w)
# define winstl__format_message_buff_w       winstl_C_format_message_buff_w

/** \def winstl__format_message_alloc_a
 *
 * \deprecated Use winstl_C_format_message_alloc_a
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_alloc_a, winstl_C_format_message_alloc_a)
# define winstl__format_message_alloc_a      winstl_C_format_message_alloc_a

/** \def winstl__format_message_alloc_w
 *
 * \deprecated Use winstl_C_format_message_alloc_w
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_alloc_w, winstl_C_format_message_alloc_w)
# define winstl__format_message_alloc_w      winstl_C_format_message_alloc_w

/** \def winstl__format_message_free_buff_a
 *
 * \deprecated Use winstl_C_format_message_free_buff_a
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_free_buff_a, winstl_C_format_message_free_buff_a)
# define winstl__format_message_free_buff_a  winstl_C_format_message_free_buff_a

/** \def winstl__format_message_free_buff_w
 *
 * \deprecated Use winstl_C_format_message_free_buff_w
 */
STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(winstl__format_message_free_buff_w, winstl_C_format_message_free_buff_w)
# define winstl__format_message_free_buff_w  winstl_C_format_message_free_buff_w

#endif /* obsolete || 1.9 */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace winstl
{
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C++ functions
 */

#if defined(__cplusplus)

inline
ws_dword_t
format_message(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_a_t**   ppBuffer
,   int             elisionFlags = WINSTL_ERROR_FUNCTIONS_ELIDE_DOT | WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY
)
{
    return winstl_C_format_message_from_module_to_allocated_buffer(flags, hModule, code, ppBuffer, elisionFlags);
}

inline
ws_dword_t
format_message(
    DWORD           flags
,   HMODULE         hModule
,   DWORD           code
,   ws_char_w_t**   ppBuffer
,   int             elisionFlags = WINSTL_ERROR_FUNCTIONS_ELIDE_DOT | WINSTL_ERROR_FUNCTIONS_ELIDE_DOT_IF_LAST_ONLY
)
{
    return winstl_C_format_message_from_module_to_allocated_buffer(flags, hModule, code, ppBuffer, elisionFlags);
}


/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   HINSTANCE       hinst
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_a(error, hinst, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   HINSTANCE       hinst
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_inst_w(error, hinst, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   ws_char_a_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_a(error, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   ws_char_w_t*    buffer
,   ws_uint_t       cchBuffer
)
{
    return winstl_C_format_message_buff_w(error, buffer, cchBuffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   HINSTANCE       hinst
,   ws_char_a_t**   buffer
)
{
    return winstl_C_format_message_alloc_a(error, hinst, buffer);
}

/**
 *
 * \ingroup group__library__error
 *
 * \deprecated This function is deprecated and will be removed in a future release.
 */
STLSOFT_DECLARE_FUNCTION_DEPRECATION(format_message)
inline
ws_dword_t
format_message(
    ws_dword_t      error
,   HINSTANCE       hinst
,   ws_char_w_t**   buffer
)
{
    return winstl_C_format_message_alloc_w(error, hinst, buffer);
}

/**
 *
 * \ingroup group__library__error
 */
inline
void
format_message_free_buff(
    ws_char_a_t* buffer
)
{
    winstl_C_format_message_free_buff_a(buffer);
}

/**
 *
 * \ingroup group__library__error
 */
inline
void
format_message_free_buff(
    ws_char_w_t* buffer
)
{
    winstl_C_format_message_free_buff_w(buffer);
}

#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_ERROR_H_ERROR_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
