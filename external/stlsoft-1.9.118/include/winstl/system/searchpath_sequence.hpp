/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/searchpath_sequence.hpp
 *
 * Purpose:     Contains the basic_searchpath_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     12th July 2002
 * Updated:     12th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/system/searchpath_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_searchpath_sequence
 *  class template
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SEARCHPATH_SEQUENCE
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_SEARCHPATH_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_SYSTEM_HPP_SEARCHPATH_SEQUENCE_MAJOR    4
# define WINSTL_VER_SYSTEM_HPP_SEARCHPATH_SEQUENCE_MINOR    2
# define WINSTL_VER_SYSTEM_HPP_SEARCHPATH_SEQUENCE_REVISION 4
# define WINSTL_VER_SYSTEM_HPP_SEARCHPATH_SEQUENCE_EDIT     97
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <winstl/filesystem/filesystem_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_VERSION
# include <winstl/system/system_version.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_VERSION */
#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX
# include <winstl/synch/spin_mutex.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX */
#ifndef WINSTL_INCL_WINSTL_HPP_REGISTRY_FUNCTIONS
//# include <winstl/registry_functions.hpp>
#endif /* !WINSTL_INCL_WINSTL_HPP_REGISTRY_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE
# include <stlsoft/synch/lock_scope.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

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

// class basic_searchpath_sequence
/** \brief Presents an STL-like sequence interface to the search path sequence for the current process
 *
 * \ingroup group__library__system
 *
 * \param C The character type
 * \param T The traits type. On translators that support default template arguments this defaults to filesystem_traits<C>
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = filesystem_traits<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = filesystem_traits<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_searchpath_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// The character type
    typedef C                                                       char_type;
    /// The traits type
    typedef T                                                       traits_type;
    /// The current parameterisation of the type
    typedef basic_searchpath_sequence<C, T>                         class_type;
    /// The value type
    typedef char_type const*                                        value_type;
    /// The pointer type
    typedef value_type*                                             pointer;
    /// The non-mutable (const) pointer type
    typedef value_type const*                                       const_pointer;
    /// The reference type
    typedef value_type&                                             reference;
    /// The non-mutable (const) reference type
    typedef value_type const&                                       const_reference;
    /// The size type
    typedef ws_size_t                                               size_type;
    /// The difference type
    typedef ws_ptrdiff_t                                            difference_type;
    /// The non-mutating (const) iterator type
#if defined(STLSOFT_COMPILER_IS_BORLAND)
    typedef                   stlsoft_ns_qual(pointer_iterator)<
#else /* ? compiler */
    typedef ss_typename_type_k stlsoft_ns_qual(pointer_iterator)<
#endif /* compiler */
        value_type
,   const_pointer
,   const_reference
>::type                                                             const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The non-mutating (const) reverse iterator type
    typedef stlsoft_ns_qual(const_reverse_iterator_base)<
        const_iterator
    ,   value_type
    ,   const_reference
    ,   const_pointer
    ,   difference_type
    >                                                               const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// Construction
public:
    /// Determines the search path sequence for the calling process
    ///
    /// \note The process directory appears before the current directory
    basic_searchpath_sequence();
    /// Determines the search path sequence for the calling process
    ///
    /// \param bIncludeApplicationDirectory If this is \c true the application directory is included in the search path sequence
    /// \param bIncludeCurrentDirectory If this is \c true the current directory is included in the search path sequence
    /// \param bApplicationDirectoryFirst If this is \c true the process directory is placed before the current directory in the search (the normal loading sequence). If not, then the current directory comes first.
    basic_searchpath_sequence(
        ws_bool_t bIncludeApplicationDirectory
    ,   ws_bool_t bIncludeCurrentDirectory
    ,   ws_bool_t bApplicationDirectoryFirst = true
    );
    /// Releases any resources
    ~basic_searchpath_sequence() stlsoft_throw_0();

// Iteration
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator  begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// State
public:
    /// Returns the number of items in the sequence
    size_type size() const;
    /// Indicates whether the sequence is empty
    ws_bool_t empty() const;
    /// Returns the maximum number of items in the sequence
    static size_type max_size();

// Accessors
public:
    /// Returns the item at the given index
    value_type operator [](size_type index) const;

// Members
private:
    // 1. Application directory - GetModuleFileName(NULL, ...);
    // 2. Current directory - GetCurrentDirectory
    // 3. System directory
    // 4. NT-only: 16-bit system directory
    // 5. Windows directory
    // 6 - n. Path directories

    typedef processheap_allocator<char_type>    main_allocator_type_;
    typedef processheap_allocator<value_type>   value_allocator_type_;
    typedef stlsoft_ns_qual(auto_buffer_old)<
        char_type
    ,   main_allocator_type_
    ,   1024
    >                                           main_buffer_type_;
    typedef stlsoft_ns_qual(auto_buffer_old)<
        value_type
    ,   value_allocator_type_
    ,   24
    >                                           value_buffer_type_;

    main_buffer_type_   m_buffer;
    value_buffer_type_  m_values;
    const_iterator      m_end;

// Implementation
private:
    void construct_(
        ws_bool_t   bIncludeApplicationDirectory
    ,   ws_bool_t   bIncludeCurrentDirectory
    ,   ws_bool_t   bApplicationDirectoryFirst
    );

    /* WSCB: Borland has an internal compiler error if use ws_bool_t */
#ifdef STLSOFT_COMPILER_IS_BORLAND
    typedef ws_int_t        init_type;
#else /* ? compiler */
    typedef ws_bool_t       init_type;
#endif /* compiler */

    static char_type const* get_application_directory()
    {
        static char_type                        s_application_directory[WINSTL_CONST_MAX_PATH + 1];
        static atomic_int_t                     s_mx;
        spin_mutex                              mx(&s_mx);
        stlsoft_ns_qual(lock_scope)<spin_mutex> lock(mx);
        static init_type                        s_init = ws_false_v;

        if(!s_init)
        {
            char_type   dummy[WINSTL_CONST_MAX_PATH + 1];
            char_type*  file_part;

            traits_type::get_module_filename(NULL, s_application_directory, STLSOFT_NUM_ELEMENTS(s_application_directory));
            traits_type::get_full_path_name(s_application_directory, STLSOFT_NUM_ELEMENTS(dummy), dummy, &file_part);
            s_application_directory[file_part - &dummy[0] - 1] = '\0';
            s_init = ws_true_v;
        }

        return s_application_directory;
    }

    static char_type const* get_system_directory()
    {
        static char_type                        s_system_directory[WINSTL_CONST_MAX_PATH + 1];
        static atomic_int_t                     s_mx;
        spin_mutex                              mx(&s_mx);
        stlsoft_ns_qual(lock_scope)<spin_mutex> lock(mx);
        static init_type                        s_init = (traits_type::get_system_directory(s_system_directory, STLSOFT_NUM_ELEMENTS(s_system_directory)), ws_true_v);

        return s_system_directory;
    }

    static char_type const* get_windows_directory()
    {
        static char_type                        s_windows_directory[WINSTL_CONST_MAX_PATH + 1];
        static atomic_int_t                     s_mx;
        spin_mutex                              mx(&s_mx);
        stlsoft_ns_qual(lock_scope)<spin_mutex> lock(mx);
        static init_type                        s_init = (traits_type::get_windows_directory(s_windows_directory, STLSOFT_NUM_ELEMENTS(s_windows_directory)), ws_true_v);

        return s_windows_directory;
    }

    static char_type const* get_system16_directory()
    {
        static char_type                        s_system16_directory[WINSTL_CONST_MAX_PATH + 1];
        static atomic_int_t                     s_mx;
        spin_mutex                              mx(&s_mx);
        stlsoft_ns_qual(lock_scope)<spin_mutex> lock(mx);
        static init_type                        s_init = ws_false_v;

        if(!s_init)
        {
            if(system_version::winnt())
            {
                char_type* file_part;

                traits_type::get_full_path_name(get_system_directory(), STLSOFT_NUM_ELEMENTS(s_system16_directory), s_system16_directory, &file_part);
                traits_type::char_copy(file_part, disgusting_hack_("SYSTEM", L"SYSTEM"), 7);
            }
            else
            {
                s_system16_directory[0] = '\0';
            }

            s_init = ws_true_v;
        }

        return s_system16_directory;
    }

    static ws_size_t    directories_total()
    {
        ws_size_t   cch =   0;

        cch += 1 + traits_type::str_len(get_application_directory());               // Application directory
        cch += 1 + traits_type::get_current_directory(static_cast<char*>(NULL), 0); // Current directory
        cch += 1 + traits_type::str_len(get_system_directory());                    // System directory
        cch += 1 + traits_type::str_len(get_system16_directory());                  // 16-bit System directory
        cch += 1 + traits_type::str_len(get_windows_directory());                   // Windows directory
        cch += 1 + traits_type::get_environment_variable(disgusting_hack_("PATH", L"PATH"), NULL, 0);  // PATH

        return cch;
    }

    static ws_size_t    num_paths()
    {
        ws_size_t           cPaths  =   0;
        ws_size_t           cch     =   traits_type::get_environment_variable(disgusting_hack_("PATH", L"PATH"), NULL, 0);
        main_buffer_type_   buffer(1 + cch);
        char_type const*    begin  =   &buffer[0];
        char_type const*    end    =   begin + cch;
        char_type const*    last;

        traits_type::get_environment_variable(disgusting_hack_("PATH", L"PATH"), &buffer[0], buffer.size());

        for(; begin != end; ++begin)
        {
            if(*begin != ';')
            {
                break;
            }
        }

        for(last = begin; begin != end; ++begin)
        {
            if(*begin == ';')
            {
                if(1 < begin - last)
                {
                    ++cPaths;
                }

                last = begin + 1;
            }
        }

        if(1 < begin - last)
        {
            ++cPaths;
        }

        return cPaths;
    }

    static ws_bool_t is_curr_dir_last_()
    {
        if( system_version::winnt() &&
            system_version::major() >= 5 &&
            system_version::minor() == 1)
        {
            ws_bool_t   res     =   false;
            HKEY        hkey;
            LRESULT     lRes    =   ::RegOpenKeyExW(HKEY_LOCAL_MACHINE
                                                ,   L"SYSTEM\\CurrentControlSet\\Control\\Session Manager"
                                                ,   0
                                                ,   KEY_QUERY_VALUE
                                                ,   &hkey);

            if(ERROR_SUCCESS == lRes)
            {
                DWORD   type;
                DWORD   data;
                DWORD   cbData  =   sizeof(data);

                lRes = ::RegQueryValueExW(hkey, L"SafeDllSearchMode", NULL, &type, reinterpret_cast<LPBYTE>(&data), &cbData);
                if(ERROR_SUCCESS == lRes)
                {
                    if(1 == data)
                    {
                        res = true;
                    }
                }

                ::RegCloseKey(hkey);
            }

            return res;
        }
        else
        {
            return false;
        }
    }

    // One to be ashamed of. This will be replaced in the next version of the libraries
    static char_type const* disgusting_hack_(ws_char_a_t* literal_a, ws_char_w_t* literal_w)
    {
#if defined(STLSOFT_COMPILER_IS_DMC)
        if(sizeof(char_type) == sizeof(ws_char_w_t))
        {
            return static_cast<char_type*>(static_cast<void*>(literal_w));
        }
        else
        {
            return static_cast<char_type*>(static_cast<void*>(literal_a));
        }
#else /* ? compiler */
        return static_cast<char_type*>((sizeof(char_type) == sizeof(ws_char_w_t)) ? static_cast<void*>(literal_w) : static_cast<void*>(literal_a));
#endif /* compiler */
    }

// Not to be implemented
private:
    basic_searchpath_sequence(class_type const&);
    basic_searchpath_sequence const& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_searchpath_sequence template for the ANSI character type \c char
 *
 * \ingroup group__library__system
 */
typedef basic_searchpath_sequence<ws_char_a_t, filesystem_traits<ws_char_a_t> > searchpath_sequence_a;
/** \brief Specialisation of the basic_searchpath_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__system
 */
typedef basic_searchpath_sequence<ws_char_w_t, filesystem_traits<ws_char_w_t> > searchpath_sequence_w;
/** \brief Specialisation of the basic_searchpath_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__system
 */
typedef basic_searchpath_sequence<TCHAR, filesystem_traits<TCHAR> >             searchpath_sequence;

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
#include "./unittest/searchpath_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// basic_searchpath_sequence

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline basic_searchpath_sequence<C, T>::basic_searchpath_sequence()
    : m_buffer(directories_total())
    , m_values(num_paths() + (system_version::winnt() ? 5 : 4))
{
    construct_(true, true, true);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline basic_searchpath_sequence<C, T>::basic_searchpath_sequence(
    ws_bool_t bIncludeApplicationDirectory
,   ws_bool_t bIncludeCurrentDirectory
,   ws_bool_t bApplicationDirectoryFirst /* = true */
)

    : m_buffer(directories_total())
    , m_values(num_paths() + (system_version::winnt() ? 5 : 4) - (!bIncludeApplicationDirectory + !bIncludeCurrentDirectory))
{
    construct_(bIncludeApplicationDirectory, bIncludeCurrentDirectory, bApplicationDirectoryFirst);
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline void basic_searchpath_sequence<C, T>::construct_(
    ws_bool_t   bIncludeApplicationDirectory
,   ws_bool_t   bIncludeCurrentDirectory
,   ws_bool_t   bApplicationDirectoryFirst
)
{
    // Determine whether the current directory must be relegated
    ws_bool_t   bIncludeCurrentDirectoryLast    =   bIncludeCurrentDirectory && is_curr_dir_last_();

    // 6 - n. Path directories

    char_type*          psz =   &m_buffer[0];
    char_type const**   it  =   &m_values[0];
    char_type const*    cwd =   NULL;

    psz[0] = '\0';

    { for(int i = 0; i < 2; ++i) {

        if((i & 1) != static_cast<int>(bApplicationDirectoryFirst))
        {
            if(bIncludeApplicationDirectory)
            {
                *it++ = psz;

                // 1. Application directory - GetModuleFileName(NULL, ...);
                size_t n = traits_type::str_len(get_application_directory());
                traits_type::char_copy(psz, get_application_directory(), n + 1);
                psz += n;
            }
        }
        else
        {
            if(bIncludeCurrentDirectory)
            {
                cwd =   psz;

                // 2. Current directory - GetCurrentDirectory
                psz += traits_type::get_current_directory(WINSTL_CONST_MAX_PATH + 1, psz);

                if(!bIncludeCurrentDirectoryLast)
                {
                    *it++ = cwd;
                }
            }
        }

        ++psz;
    }}

    size_t n;

    // 3. System directory
    *it++ = psz;
    n = traits_type::str_len(get_system_directory());
    traits_type::char_copy(psz, get_system_directory(), n + 1);
    psz += n;
    ++psz;

    // 4. NT-only: 16-bit system directory
    if(system_version::winnt())
    {
        *it++ = psz;
        n = traits_type::str_len(get_system16_directory());
        traits_type::char_copy(psz, get_system16_directory(), n + 1);
        psz += n;
        ++psz;
    }

    // 5. Windows directory
    *it++ = psz;
    n = traits_type::str_len(get_windows_directory());
    traits_type::char_copy(psz, get_windows_directory(), n + 1);
    psz += n;
    ++psz;

    // 2.b. Current directory last?
    if( bIncludeCurrentDirectory &&
        bIncludeCurrentDirectoryLast)
    {
        *it++ = cwd;
    }

    // 6. Paths
    char_type const*        begin  =   psz;
    char_type const* const  end     =   begin + traits_type::get_environment_variable(disgusting_hack_("PATH", L"PATH"), psz, static_cast<DWORD>(m_buffer.end() - psz));
    char_type const*        last;

    // Move along to the first valid item
    for(; begin != end; ++begin)
    {
        if(*begin != ';')
        {
            break;
        }
    }

    for(last = begin; begin != end; ++begin)
    {
        if(*begin == ';')
        {
            if(1 < begin - last)
            {
                *it++ = last;
            }

            *const_cast<char_type*>(begin) = '\0';

            last = begin + 1;
        }
    }

    if(1 < begin - last)
    {
        *it++ = last;
    }

    m_end   =   it;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline basic_searchpath_sequence<C, T>::~basic_searchpath_sequence() stlsoft_throw_0()
{}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::const_iterator basic_searchpath_sequence<C, T>::begin() const
{
    return &m_values[0];
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::const_iterator basic_searchpath_sequence<C, T>::end() const
{
    return m_end;
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::const_reverse_iterator basic_searchpath_sequence<C, T>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::const_reverse_iterator basic_searchpath_sequence<C, T>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::size_type basic_searchpath_sequence<C, T>::size() const
{
    return static_cast<size_type>(end() - begin());
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ws_bool_t basic_searchpath_sequence<C, T>::empty() const
{
    return begin() == end();
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline /* static */ ss_typename_type_ret_k basic_searchpath_sequence<C, T>::size_type basic_searchpath_sequence<C, T>::max_size()
{
    // Kind of kludgy, sigh.
    return static_cast<size_type>(-1) / WINSTL_CONST_MAX_PATH;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_searchpath_sequence<C, T>::value_type basic_searchpath_sequence<C, T>::operator [](ss_typename_type_k basic_searchpath_sequence<C, T>::size_type index) const
{
    WINSTL_MESSAGE_ASSERT("Invalid index in search path sequence", !(size() < index));

    return begin()[index];
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_SEARCHPATH_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
