/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/toolhelp/sequence_value_traits.hpp
 *
 * Purpose:     Instantiations of th_sequence_value_traits<>.
 *
 * Created:     21st May 2005
 * Updated:     13th January 2011
 *
 * Thanks:      To Pablo for contributing this great library.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2011, Pablo Aguilar
 * Copyright (c) 2006-2011, Matthew Wilson
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
 * - Neither the name(s) of Matthew Wilson and Synesis Software, nor Pablo
 *   Aguilar, nor the names of any contributors may be used to endorse or
 *   promote products derived from this software without specific prior written
 *   permission.
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


/** \file winstl/toolhelp/sequence_value_traits.hpp
 *
 * \brief [C++ only] Instantiations of th_sequence_value_traits<>
 *   (\ref group__library__windows_toolhelp "Windows ToolHelp" Library).
 */

// NO INCLUDE GUARDS
// This file is meant to be included multiple times

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_VALUE_TRAITS_MAJOR     1
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_VALUE_TRAITS_MINOR     1
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_VALUE_TRAITS_REVISION  4
# define WINSTL_VER_WINSTL_TOOLHELP_HPP_SEQUENCE_VALUE_TRAITS_EDIT      9
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* Compatibility
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

#ifndef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE
# error This file cannot be included independently, but only within one of the toolhelp sequence specialisation headers
#endif /* !WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE */
#ifndef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FIRST
# error This file cannot be included independently, but only within one of the toolhelp sequence specialisation headers
#endif /* !WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FIRST */
#ifndef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_NEXT
# error This file cannot be included independently, but only within one of the toolhelp sequence specialisation headers
#endif /* !WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_NEXT */
#ifndef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FLAG
# error This file cannot be included independently, but only within one of the toolhelp sequence specialisation headers
#endif /* !WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FLAG */

STLSOFT_TEMPLATE_SPECIALISATION
struct th_sequence_value_traits<WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE>
{
    typedef HANDLE                                      handle_type;
    typedef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE   value_type;

    static handle_type null_handle()
    {
        return NULL;
    }

    static handle_type invalid_handle()
    {
        return INVALID_HANDLE_VALUE;
    }

    static DWORD flag()
    {
        return WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FLAG;
    }

    static bool first(handle_type snapshot, value_type& value)
    {
        return (WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FIRST(snapshot, &value) != FALSE);
    }

    static bool next(handle_type snapshot, value_type& value)
    {
        value.dwSize = sizeof(value);
        return (WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_NEXT(snapshot, &value) != FALSE);
    }

    static LPCTSTR create_snapshot_fail_message()
    {
        return WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_CREATE_SNAPSHOT_FAIL_MESSAGE;
    }
};

// #undef here so we don't have to repeat this each time we include this file
#undef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_VALUE
#undef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FIRST
#undef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_NEXT
#undef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_FLAG
#undef WINSTL_TH_API_SEQUENCE_VALUE_TRAITS_CREATE_SNAPSHOT_FAIL_MESSAGE

/* ///////////////////////////// end of file //////////////////////////// */
