/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/reg_traits.hpp
 *
 * Purpose:     Contains the reg_traits class template, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     19th January 2002
 * Updated:     10th August 2009
 *
 * Thanks to:   Sam Fisher for requesting reg_delete_tree().
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/reg_traits.hpp
 *
 * \brief [C++ only] Definition of the winstl::reg_traits class template
 *   (\ref group__library__windows_registry "Windows Registry" Library).
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS
#define WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_TRAITS_MAJOR    3
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_TRAITS_MINOR    5
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_TRAITS_REVISION 1
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_TRAITS_EDIT     77
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS
# include <winstl/registry/util/defs.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS
# include <winstl/system/system_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS */

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

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** Traits for accessing the correct registry functions for a given character type
 *
 * \ingroup group__library__windows_registry
 *
 * reg_traits is a traits class for determining the correct registry
 * structures and functions for a given character type.
 *
 * \param C The character type
 */
template <ss_typename_param_k C>
struct reg_traits
    : public system_traits<C>
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C               char_type;
    /// The size type
    typedef ws_size_t       size_type;
    /// The difference type
    typedef ws_ptrdiff_t    difference_type;
    /// The registry key type
    typedef HKEY            hkey_type;
    /// The string type
    typedef reg_string_t    string_type;        // Placeholder only
    /// The time type
    typedef FILETIME        time_type;
    /// The API result type (LONG)
    typedef LONG           result_type;
/// @}

/// \name Operations
/// @{
public:
    /// Duplicates a registry key
    ///
    /// \deprecated Use reg_dup_key() instead
    static hkey_type    key_dup(        hkey_type           hkey
                                    ,   REGSAM              samDesired  =   KEY_ALL_ACCESS
                                    ,   result_type*        result     =   NULL);
    /// Duplicates a registry key
    static hkey_type    reg_dup_key(    hkey_type           hkey
                                    ,   REGSAM              samDesired  =   KEY_ALL_ACCESS
                                    ,   result_type*        result     =   NULL);
    /// Opens a registry sub-key
    static result_type  reg_open_key(   hkey_type           hkey,
                                        char_type const*    sub_key_name,
                                        hkey_type*          hkey_result,
                                        REGSAM              samDesired = KEY_ALL_ACCESS);
    /// Opens a registry sub-key
    static result_type  reg_create_key( hkey_type           hkey,
                                        char_type const*    sub_key_name,
                                        hkey_type*          hkey_result,
                                        REGSAM              samDesired = KEY_ALL_ACCESS);
    static result_type  reg_create_key( hkey_type           hkey,
                                        char_type const*    sub_key_name,
                                        hkey_type*          hkey_result,
                                        ws_bool_t&          bCreated,
                                        REGSAM              samDesired = KEY_ALL_ACCESS);
    /// Destroys a registry sub-key
    static result_type  reg_delete_key( hkey_type           hkey,
                                        char_type const*    sub_key_name);
    /// Queries a registry key value
    static result_type  reg_query_value(hkey_type           hkey,
                                        char_type const*    valueName,
                                        ws_dword_t&         valueType,
                                        void*               data,
                                        size_type           &cbData);
    /// Sets the value of the named value.
    static result_type  reg_set_value(  hkey_type           hkey
                                    ,   char_type const*    valueName
                                    ,   ws_dword_t          valueType
                                    ,   void const*         data
                                    ,   size_type           cbData);
    /// Deletes the named value.
    static result_type  reg_delete_value(hkey_type          hkey
                                    ,   char_type const*    valueName);

    /// Deletes the key and all sub-keys, permissions allowing 
    static result_type reg_delete_tree(
        hkey_type           hkey
    ,   char_type const*    sub_key_name
    );

    /// Queries a registry key's characteristics
    static result_type  reg_query_info( hkey_type       hkey,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        ws_uint32_t*    c_sub_keys,
                                        size_type*      cch_sub_key_max,
                                        size_type*      cch_key_class_max,
                                        ws_uint32_t*    c_values,
                                        size_type*      cch_valueName_max,
                                        size_type*      cb_value_data_max,
                                        size_type*      cb_security_descriptor_max,
                                        time_type*      time_last_write);
    /// Enumerates a registry key's sub-keys
    static result_type  reg_enum_key(   hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        time_type*      time_last_write    =   NULL);
    /// [DEPRECATED] Enumerates a registry key's sub-keys
    ///
    /// \deprecated This is deprecated in favour of reg_enum_key(hkey_type, ws_dword_t, char_type*, size_type*, time_type *)
    static result_type  reg_enum_key(   hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        time_type*      time_last_write);
    /// Enumerates a registry key's values
    static result_type  reg_enum_value( hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName,
                                        ws_dword_t*     valueType,
                                        void*           data,
                                        size_type       &cbData);
    /// Enumerates a registry key's values
    static result_type  reg_enum_value( hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName);
/// @}
};

#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k C>
struct reg_traits;

STLSOFT_TEMPLATE_SPECIALISATION
struct reg_traits<ws_char_a_t>
    : public system_traits<ws_char_a_t>
{
public:
    typedef ws_char_a_t         char_type;
    typedef ws_size_t           size_type;
    typedef ws_ptrdiff_t        difference_type;
    typedef HKEY                hkey_type;
    typedef reg_string_a_t      string_type;
    typedef FILETIME            time_type;
    typedef LONG                result_type;

public:
    static hkey_type key_dup(hkey_type hkey, REGSAM samDesired, result_type *result = NULL)
    {
        return reg_dup_key(hkey, samDesired, result);
    }
    static hkey_type reg_dup_key(hkey_type hkey, REGSAM samDesired, result_type *result = NULL)
    {
        hkey_type   hkeyDup;
        result_type res = ::RegOpenKeyExA(hkey, "", 0, samDesired, &hkeyDup);

        if(ERROR_SUCCESS != res)
        {
            hkeyDup = NULL;
        }

        if(NULL != result)
        {
            *result = res;
        }

        return hkeyDup;
    }

    static result_type reg_open_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        return ::RegOpenKeyExA(hkey, sub_key_name, 0, samDesired, hkey_result);
    }

    static result_type reg_create_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        return ::RegCreateKeyExA(hkey, sub_key_name, 0, NULL, 0, samDesired, NULL, hkey_result, NULL);
    }

    static result_type reg_create_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, ws_bool_t &bCreated, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        DWORD       disposition;
        result_type res =   ::RegCreateKeyExA(hkey, sub_key_name, 0, NULL, 0, samDesired, NULL, hkey_result, &disposition);

        bCreated = (ERROR_SUCCESS == res) && (REG_CREATED_NEW_KEY == disposition);

        return res;
    }

    static result_type reg_delete_key(hkey_type hkey, char_type const* sub_key_name)
    {
        return ::RegDeleteKeyA(hkey, sub_key_name);
    }

    static result_type reg_query_value(hkey_type hkey, char_type const* valueName, ws_dword_t& valueType, void* data, size_type &cbData)
    {
        return ::RegQueryValueExA(hkey, valueName, NULL, &valueType, static_cast<LPBYTE>(data), reinterpret_cast<LPDWORD>(&cbData));
    }

    static result_type reg_set_value(hkey_type hkey, char_type const* valueName, ws_dword_t valueType, void const* data, size_type cbData)
    {
        return ::RegSetValueExA(hkey, valueName, 0, valueType, static_cast<BYTE const*>(data), static_cast<DWORD>(cbData));
    }

    static result_type reg_delete_value(hkey_type hkey, char_type const* valueName)
    {
        return ::RegDeleteValueA(hkey, valueName);
    }

    static result_type reg_delete_tree(
        hkey_type           hkey
    ,   char_type const*    sub_key_name
    )
    {
        result_type res = execute_dynamic_("advapi32.dll", "RegDeleteTreeA", hkey, sub_key_name);

        if(ERROR_PROC_NOT_FOUND == res)
        {
            res = execute_dynamic_("shlwapi.dll", "SHDeleteKeyA", hkey, sub_key_name);
        }

        return res;
    }

    static result_type reg_query_info(  hkey_type       hkey,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        ws_uint32_t*    c_sub_keys,
                                        size_type*      cch_sub_key_max,
                                        size_type*      cch_key_class_max,
                                        ws_uint32_t*    c_values,
                                        size_type*      cch_valueName_max,
                                        size_type*      cb_value_data_max,
                                        size_type*      cb_security_descriptor_max,
                                        time_type*      time_last_write)
    {
        if( NULL == cch_key_class &&
            NULL != key_class)
        {
            return ERROR_INVALID_PARAMETER;
        }

        return ::RegQueryInfoKeyA(hkey, key_class, reinterpret_cast<LPDWORD>(cch_key_class), NULL, reinterpret_cast<LPDWORD>(c_sub_keys), reinterpret_cast<LPDWORD>(cch_sub_key_max), reinterpret_cast<LPDWORD>(cch_key_class_max), reinterpret_cast<LPDWORD>(c_values), reinterpret_cast<LPDWORD>(cch_valueName_max), reinterpret_cast<LPDWORD>(cb_value_data_max), reinterpret_cast<LPDWORD>(cb_security_descriptor_max), time_last_write);
    }

    static result_type reg_enum_key(    hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        time_type*      time_last_write    =   NULL)
    {
        return ::RegEnumKeyExA(hkey, index, key_name, reinterpret_cast<LPDWORD>(cch_key_name), NULL, NULL, NULL, time_last_write);
    }

    static result_type reg_enum_key(    hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        time_type*      time_last_write)
    {
        return ::RegEnumKeyExA(hkey, index, key_name, reinterpret_cast<LPDWORD>(cch_key_name), NULL, key_class, reinterpret_cast<LPDWORD>(cch_key_class), time_last_write);
    }

    static result_type reg_enum_value(  hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName,
                                        ws_dword_t*     valueType,
                                        void*           data,
                                        size_type       &cbData)
    {
        return ::RegEnumValueA(hkey, index, valueName, reinterpret_cast<LPDWORD>(cch_valueName), NULL, valueType, reinterpret_cast<LPBYTE>(data), reinterpret_cast<LPDWORD>(&cbData));
    }

    static result_type reg_enum_value(  hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName)
    {
        return ::RegEnumValueA(hkey, index, valueName, reinterpret_cast<LPDWORD>(cch_valueName), NULL, NULL, NULL, NULL);
    }

private:
    static result_type execute_dynamic_(
        ws_char_a_t const*  module
    ,   ws_char_a_t const*  function
    ,   hkey_type           a1
    ,   char_type const*    a2
    )
    {
        result_type r       =   ERROR_SUCCESS;
        HINSTANCE   hinst   =   ::LoadLibraryA(module);

        if(NULL == hinst)
        {
            r = static_cast<result_type>(::GetLastError());
        }
        else
        {
            union
            {
                FARPROC                 fp;
                DWORD (STLSOFT_STDCALL* pfn)(HKEY, LPCSTR);
            } u;

            u.fp = ::GetProcAddress(hinst, function);

            if(NULL == u.fp)
            {
                r = static_cast<result_type>(::GetLastError());
            }
            else
            {
                r = static_cast<result_type>((*u.pfn)(a1, a2));
            }

            ::FreeLibrary(hinst);
        }

        return r;
    }
};

STLSOFT_TEMPLATE_SPECIALISATION
struct reg_traits<ws_char_w_t>
    : public system_traits<ws_char_w_t>
{
public:
    typedef ws_char_w_t         char_type;
    typedef ws_size_t           size_type;
    typedef ws_ptrdiff_t        difference_type;
    typedef HKEY                hkey_type;
    typedef reg_string_w_t      string_type;
    typedef FILETIME            time_type;
    typedef LONG                result_type;

public:
    static hkey_type key_dup(hkey_type hkey, REGSAM samDesired, result_type *result = NULL)
    {
        return reg_dup_key(hkey, samDesired, result);
    }
    static hkey_type reg_dup_key(hkey_type hkey, REGSAM samDesired, result_type *result = NULL)
    {
        hkey_type   hkeyDup;
        result_type res = ::RegOpenKeyExW(hkey, L"", 0, samDesired, &hkeyDup);

        if(ERROR_SUCCESS != res)
        {
            hkeyDup = NULL;
        }

        if(NULL != result)
        {
            *result = res;
        }

        return hkeyDup;
    }

    static result_type reg_open_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        return ::RegOpenKeyExW(hkey, sub_key_name, 0, samDesired, hkey_result);
    }

    static result_type reg_create_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        return ::RegCreateKeyExW(hkey, sub_key_name, 0, NULL, 0, samDesired, NULL, hkey_result, NULL);
    }

    static result_type reg_create_key(hkey_type hkey, char_type const* sub_key_name, hkey_type *hkey_result, ws_bool_t &bCreated, REGSAM samDesired = KEY_ALL_ACCESS)
    {
        DWORD       disposition;
        result_type res =   ::RegCreateKeyExW(hkey, sub_key_name, 0, NULL, 0, samDesired, NULL, hkey_result, &disposition);

        bCreated = (ERROR_SUCCESS == res) && (REG_CREATED_NEW_KEY == disposition);

        return res;
    }

    static result_type reg_delete_key(hkey_type hkey, char_type const* sub_key_name)
    {
        return ::RegDeleteKeyW(hkey, sub_key_name);
    }

    static result_type reg_query_value(hkey_type hkey, char_type const* valueName, ws_dword_t& valueType, void* data, size_type &cbData)
    {
        return ::RegQueryValueExW(hkey, valueName, NULL, &valueType, static_cast<LPBYTE>(data), reinterpret_cast<LPDWORD>(&cbData));
    }

    static result_type reg_set_value(hkey_type hkey, char_type const* valueName, ws_dword_t valueType, void const* data, size_type cbData)
    {
        return ::RegSetValueExW(hkey, valueName, 0, valueType, static_cast<BYTE const*>(data), static_cast<DWORD>(cbData));
    }

    static result_type reg_delete_value(hkey_type hkey, char_type const* valueName)
    {
        return ::RegDeleteValueW(hkey, valueName);
    }

    static result_type reg_delete_tree(
        hkey_type           hkey
    ,   char_type const*    sub_key_name
    )
    {
        result_type res = execute_dynamic_("advapi32.dll", "RegDeleteTreeW", hkey, sub_key_name);

        if(ERROR_PROC_NOT_FOUND == res)
        {
            res = execute_dynamic_("shlwapi.dll", "SHDeleteKeyW", hkey, sub_key_name);
        }

        return res;
    }

    static result_type reg_query_info(  hkey_type       hkey,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        ws_uint32_t*    c_sub_keys,
                                        size_type*      cch_sub_key_max,
                                        size_type*      cch_key_class_max,
                                        ws_uint32_t*    c_values,
                                        size_type*      cch_valueName_max,
                                        size_type*      cb_value_data_max,
                                        size_type*      cb_security_descriptor_max,
                                        time_type*      time_last_write)
    {
        if( NULL == cch_key_class &&
            NULL != key_class)
        {
            return ERROR_INVALID_PARAMETER;
        }

        return ::RegQueryInfoKeyW(hkey, key_class, reinterpret_cast<LPDWORD>(cch_key_class), NULL, reinterpret_cast<LPDWORD>(c_sub_keys), reinterpret_cast<LPDWORD>(cch_sub_key_max), reinterpret_cast<LPDWORD>(cch_key_class_max), reinterpret_cast<LPDWORD>(c_values), reinterpret_cast<LPDWORD>(cch_valueName_max), reinterpret_cast<LPDWORD>(cb_value_data_max), reinterpret_cast<LPDWORD>(cb_security_descriptor_max), time_last_write);
    }

    static result_type reg_enum_key(    hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        time_type*      time_last_write    =   NULL)
    {
        return ::RegEnumKeyExW(hkey, index, key_name, reinterpret_cast<LPDWORD>(cch_key_name), NULL, NULL, NULL, time_last_write);
    }

    static result_type reg_enum_key(    hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      key_name,
                                        size_type*      cch_key_name,
                                        char_type*      key_class,
                                        size_type*      cch_key_class,
                                        time_type*      time_last_write)
    {
        return ::RegEnumKeyExW(hkey, index, key_name, reinterpret_cast<LPDWORD>(cch_key_name), NULL, key_class, reinterpret_cast<LPDWORD>(cch_key_class), time_last_write);
    }

    static result_type reg_enum_value(  hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName,
                                        ws_dword_t*     valueType,
                                        void*           data,
                                        size_type       &cbData)
    {
        return ::RegEnumValueW(hkey, index, valueName, reinterpret_cast<LPDWORD>(cch_valueName), NULL, valueType, reinterpret_cast<LPBYTE>(data), reinterpret_cast<LPDWORD>(&cbData));
    }

    static result_type reg_enum_value(  hkey_type       hkey,
                                        ws_dword_t      index,
                                        char_type*      valueName,
                                        size_type*      cch_valueName)
    {
        return ::RegEnumValueW(hkey, index, valueName, reinterpret_cast<LPDWORD>(cch_valueName), NULL, NULL, NULL, NULL);
    }

private:
    static result_type execute_dynamic_(
        ws_char_a_t const*  module
    ,   ws_char_a_t const*  function
    ,   hkey_type           a1
    ,   char_type const*    a2
    )
    {
        result_type r       =   ERROR_SUCCESS;
        HINSTANCE   hinst   =   ::LoadLibraryA(module);

        if(NULL == hinst)
        {
            r = static_cast<result_type>(::GetLastError());
        }
        else
        {
            union
            {
                FARPROC                 fp;
                DWORD (STLSOFT_STDCALL* pfn)(HKEY, LPCWSTR);
            } u;

            u.fp = ::GetProcAddress(hinst, function);

            if(NULL == u.fp)
            {
                r = static_cast<result_type>(::GetLastError());
            }
            else
            {
                r = static_cast<result_type>((*u.pfn)(a1, a2));
            }

            ::FreeLibrary(hinst);
        }

        return r;
    }
    
};

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS */

/* ///////////////////////////// end of file //////////////////////////// */
