/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/time/comparison_functions.h
 *
 * Purpose:     Comparison functions for Windows time structures.
 *
 * Created:     21st November 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/time/comparison_functions.h
 *
 * \brief [C, C++] Comparison functions for Windows time types
 *   (\ref group__library__time "Time" Library).
 */

#ifndef WINSTL_INCL_WINSTL_TIME_H_COMPARISON_FUNCTIONS
#define WINSTL_INCL_WINSTL_TIME_H_COMPARISON_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_TIME_H_COMPARISON_FUNCTIONS_MAJOR    4
# define WINSTL_VER_WINSTL_TIME_H_COMPARISON_FUNCTIONS_MINOR    0
# define WINSTL_VER_WINSTL_TIME_H_COMPARISON_FUNCTIONS_REVISION 5
# define WINSTL_VER_WINSTL_TIME_H_COMPARISON_FUNCTIONS_EDIT     47
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
 * C functions
 */

/**
 *
 * \ingroup group__library__time
 */
STLSOFT_INLINE ws_sint_t winstl__compare_FILETIMEs(FILETIME const* lhs, FILETIME const* rhs)
{
    WINSTL_ASSERT(NULL != lhs);
    WINSTL_ASSERT(NULL != rhs);

    if(lhs->dwHighDateTime < rhs->dwHighDateTime)
    {
        return -1;
    }
    else if(rhs->dwHighDateTime < lhs->dwHighDateTime)
    {
        return +1;
    }
    else
    {
        if(lhs->dwLowDateTime < rhs->dwLowDateTime)
        {
            return -1;
        }
        else if(rhs->dwLowDateTime < lhs->dwLowDateTime)
        {
            return +1;
        }
        else
        {
            return 0;
        }
    }
}

/**
 *
 * \ingroup group__library__time
 */
STLSOFT_INLINE ws_sint_t winstl__compare_FILETIME_with_SYSTEMTIME(FILETIME const* lhs, SYSTEMTIME const* rhs)
{
    FILETIME    ft2;

    WINSTL_ASSERT(NULL != lhs);
    WINSTL_ASSERT(NULL != rhs);

    if(!STLSOFT_NS_GLOBAL(SystemTimeToFileTime)(rhs, &ft2))
    {
        return -1;
    }

    return winstl__compare_FILETIMEs(lhs, &ft2);
}

/**
 *
 * \ingroup group__library__time
 */
STLSOFT_INLINE ws_sint_t winstl__compare_SYSTEMTIME_with_FILETIME(SYSTEMTIME const* lhs, FILETIME const* rhs)
{
    FILETIME    ft1;

    WINSTL_ASSERT(NULL != lhs);
    WINSTL_ASSERT(NULL != rhs);

    if(!STLSOFT_NS_GLOBAL(SystemTimeToFileTime)(lhs, &ft1))
    {
        return +1;
    }

    return winstl__compare_FILETIMEs(&ft1, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
STLSOFT_INLINE ws_sint_t winstl__compare_SYSTEMTIMEs(SYSTEMTIME const* lhs, SYSTEMTIME const* rhs)
{
    FILETIME    ft1;
    FILETIME    ft2;

    WINSTL_ASSERT(NULL != lhs);
    WINSTL_ASSERT(NULL != rhs);

    if(!STLSOFT_NS_GLOBAL(SystemTimeToFileTime)(lhs, &ft1))
    {
        return +1;
    }
    if(!STLSOFT_NS_GLOBAL(SystemTimeToFileTime)(rhs, &ft2))
    {
        return -1;
    }

    return winstl__compare_FILETIMEs(&ft1, &ft2);
}

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

#ifdef __cplusplus

/**
 *
 * \ingroup group__library__time
 */
inline ws_sint_t compare(FILETIME const& lhs, FILETIME const& rhs)
{
    return winstl__compare_FILETIMEs(&lhs, &rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_sint_t compare(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return winstl__compare_FILETIME_with_SYSTEMTIME(&lhs, &rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_sint_t compare(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return winstl__compare_SYSTEMTIME_with_FILETIME(&lhs, &rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_sint_t compare(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return winstl__compare_SYSTEMTIMEs(&lhs, &rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

/* operator == */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator ==(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 == compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator ==(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 == compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator ==(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 == compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator ==(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 == compare(lhs, rhs);
}

/* operator != */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator !=(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 != compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator !=(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 != compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator !=(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 != compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator !=(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 != compare(lhs, rhs);
}

/* operator < */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 > compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 > compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 > compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 > compare(lhs, rhs);
}

/* operator > */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 < compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 < compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 < compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 < compare(lhs, rhs);
}

/* operator <= */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <=(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 >= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <=(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 >= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <=(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 >= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator <=(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 >= compare(lhs, rhs);
}

/* operator >= */

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >=(FILETIME const& lhs, FILETIME const& rhs)
{
    return 0 <= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >=(FILETIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 <= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >=(SYSTEMTIME const& lhs, FILETIME const& rhs)
{
    return 0 <= compare(lhs, rhs);
}

/**
 *
 * \ingroup group__library__time
 */
inline ws_bool_t operator >=(SYSTEMTIME const& lhs, SYSTEMTIME const& rhs)
{
    return 0 <= compare(lhs, rhs);
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/comparison_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* !WINSTL_INCL_WINSTL_TIME_H_COMPARISON_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
