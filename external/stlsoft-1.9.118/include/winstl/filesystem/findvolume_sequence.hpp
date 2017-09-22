/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/findvolume_sequence.hpp
 *
 * Purpose:     Contains the basic_findvolume_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Notes:       The original implementation of the class had the const_iterator
 *              and value_type as nested classes. Unfortunately, Visual C++ 5 &
 *              6 both had either compilation or linking problems so these are
 *              regretably now implemented as independent classes.
 *
 * Created:     15th January 2002
 * Updated:     21st June 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file winstl/filesystem/findvolume_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_findvolume_sequence
 *  class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE_MAJOR     4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE_MINOR     3
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE_REVISION  7
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE_EDIT      116
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
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifdef STLSOFT_MINIMUM_SAS_INCLUDES
# ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING
#  include <stlsoft/shims/access/string/std/c_string.h>
# endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_STD_H_C_STRING */
#else /* ? STLSOFT_MINIMUM_SAS_INCLUDES */
# ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
#  include <stlsoft/shims/access/string.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#endif /* STLSOFT_MINIMUM_SAS_INCLUDES */

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
 * Enumerations
 */

// The FindNextVolume API is not well documented so assume _MAX_PATH
// (aka WINSTL_CONST_MAX_PATH)
// is sufficient for volume names
enum
{
    MAX_VOL_NAME = WINSTL_CONST_MAX_PATH   //!< The maximum number of characters in a volume name
};

/* /////////////////////////////////////////////////////////////////////////
 * Forward declarations
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C, ss_typename_param_k T>
class basic_findvolume_sequence_value_type;

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
class basic_findvolume_sequence_const_iterator;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class basic_findvolume_sequence
/** \brief Presents an STL-like sequence interface over the volumes on a system
 *
 * \ingroup group__library__filesystem
 *
 * \note This class functions only on Windows 2000 and later
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
class basic_findvolume_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// The character type
    typedef C                                                           char_type;
    /// The traits type
    typedef T                                                           traits_type;
    /// The current parameterisation of the type
    typedef basic_findvolume_sequence<C, T>                             class_type;
    /// The value type
    typedef basic_findvolume_sequence_value_type<C, T>                  value_type;
    /// The non-mutating (const) iterator type
    typedef basic_findvolume_sequence_const_iterator<C, T, value_type>  const_iterator;
    /// The reference type
    typedef value_type&                                                 reference;
    /// The non-mutable (const) reference type
    typedef value_type const&                                           const_reference;

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

// State
public:
    /// Evalulates whether the sequence is empty
    ws_bool_t       empty() const;
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_findvolume_sequence template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findvolume_sequence<ws_char_a_t, filesystem_traits<ws_char_a_t> >     findvolume_sequence_a;
/** \brief Specialisation of the basic_findvolume_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findvolume_sequence<ws_char_w_t, filesystem_traits<ws_char_w_t> >     findvolume_sequence_w;
/** \brief Specialisation of the basic_findvolume_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_findvolume_sequence<TCHAR, filesystem_traits<TCHAR> >                 findvolume_sequence;

/* ////////////////////////////////////////////////////////////////////// */

// class basic_findvolume_sequence_value_type
/** \brief Value type for the basic_findvolume_sequence
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        >
class basic_findvolume_sequence_value_type
{
public:
    /// The character type
    typedef C                                           char_type;
    /// The traits type
    typedef T                                           traits_type;
    /// The current parameterisation of the type
    typedef basic_findvolume_sequence_value_type<C, T>  class_type;

public:
    /// Default constructor
    basic_findvolume_sequence_value_type();
    /// Copy constructor
    basic_findvolume_sequence_value_type(class_type const& rhs);
private:
    basic_findvolume_sequence_value_type(char_type const* vol_name);
public:

    /// Copy assignment operator
    class_type& operator =(class_type const& rhs);

// Attributes
public:
    /// Implicit conversion to non-mutable pointer-to-const of the volume name
    operator char_type const* () const
    {
        return m_name;
    }

private:
    friend class basic_findvolume_sequence_const_iterator<C, T, class_type>;

    char_type   m_name[MAX_VOL_NAME + 1];
};

// class basic_findvolume_sequence_const_iterator
/** \brief Iterator type for the basic_findvolume_sequence, supporting the Input Iterator concept
 *
 * \ingroup group__library__filesystem
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k V
        >
class basic_findvolume_sequence_const_iterator
    : public stlsoft_ns_qual(iterator_base)<
        winstl_ns_qual_std(input_iterator_tag)
    ,   V
    ,   ws_ptrdiff_t
    ,   void    // By-Value Temporary reference
    ,   V       // By-Value Temporary reference
    >
{
public:
    /// The character type
    typedef C                                                   char_type;
    /// The traits type
    typedef T                                                   traits_type;
    /// The traits type
    typedef V                                                   value_type;
    /// The current parameterisation of the type
    typedef basic_findvolume_sequence_const_iterator<C, T, V>   class_type;

/// \name Utility classes
/// @{
private:
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    struct shared_handle
    {
    public:
        typedef shared_handle       class_type;

    // Members
    public:
        HANDLE      hSrch;
    private:
        ss_sint32_t cRefs;

    public:
        ss_explicit_k shared_handle(HANDLE h)
            : hSrch(h)
            , cRefs(1)
        {}
        void AddRef()
        {
            ++cRefs;
        }
        void Release()
        {
            if(0 == --cRefs)
            {
                delete this;
            }
        }
    #if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
    protected:
    #else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
    private:
    #endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
        ~shared_handle() stlsoft_throw_0()
        {
            WINSTL_MESSAGE_ASSERT("Shared search handle being destroyed with outstanding references!", 0 == cRefs);

            if(hSrch != INVALID_HANDLE_VALUE)
            {
                traits_type::find_volume_close(hSrch);
            }
        }

    private:
        shared_handle(class_type const&);
        class_type& operator =(class_type const&);
    };
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Construction
/// @{
private:
    basic_findvolume_sequence_const_iterator(basic_findvolume_sequence<C, T> const& l, HANDLE hSrch, char_type const* vol_name)
        : m_list(&l)
        , m_handle(new shared_handle(hSrch))
    {
        WINSTL_ASSERT(INVALID_HANDLE_VALUE != hSrch);

        if(NULL == m_handle)
        {
            traits_type::find_volume_close(hSrch);
            m_name[0] = '\0';
        }
        else
        {
            traits_type::char_copy(m_name, vol_name, STLSOFT_NUM_ELEMENTS(m_name));
        }
    }
    basic_findvolume_sequence_const_iterator(basic_findvolume_sequence<C, T> const& l);
public:
    /// Default constructor
    basic_findvolume_sequence_const_iterator();
    /// Copy constructor
    basic_findvolume_sequence_const_iterator(class_type const& rhs);
    /// Destructor
    ~basic_findvolume_sequence_const_iterator() stlsoft_throw_0();

    /// Copy assignment operator
    class_type& operator =(class_type const& rhs);

public:
    /// Pre-increment operator
    class_type& operator ++();
    /// Post-increment operator
    class_type operator ++(int);
    /// Dereference to access the value at the current position
    const value_type operator *() const;
    /// Evaluates whether \c this and \c rhs are equivalent
    ws_bool_t operator ==(class_type const& rhs) const;
    /// Evaluates whether \c this and \c rhs are not equivalent
    ws_bool_t operator !=(class_type const& rhs) const;

// Members
private:
    friend class basic_findvolume_sequence<C, T>;

    typedef basic_findvolume_sequence<C, T>     list_type;

    list_type const*    m_list;
    shared_handle*      m_handle;
    char_type           m_name[MAX_VOL_NAME + 1];
};

////////////////////////////////////////////////////////////////////////////
// Shims

// c_str_data

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ws_char_a_t const* c_str_data_a(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_a_t, T> const& v)
{
    return v;
}
template <ss_typename_param_k T>
inline ws_char_w_t const* c_str_data_w(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_w_t, T> const& v)
{
    return v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for winstl::basic_findvolume_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T>
inline C const* c_str_data(winstl_ns_qual(basic_findvolume_sequence_value_type)<C, T> const& v)
{
    return v;
}


// c_str_len

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ws_size_t c_str_len_a(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_a_t, T> const& v)
{
    return stlsoft_ns_qual(c_str_len_a(stlsoft_ns_qual(c_str_ptr_a(v))));
}
template <ss_typename_param_k T>
inline ws_size_t c_str_len_w(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_w_t, T> const& v)
{
    return stlsoft_ns_qual(c_str_len_w(stlsoft_ns_qual(c_str_ptr_w(v))));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for winstl::basic_findvolume_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T>
inline ws_size_t c_str_len(winstl_ns_qual(basic_findvolume_sequence_value_type)<C, T> const& v)
{
    return stlsoft_ns_qual(c_str_len(stlsoft_ns_qual(c_str_ptr(v))));
}


// c_str_ptr

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ws_char_a_t const* c_str_ptr_a(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_a_t, T> const& v)
{
    return v;
}
template <ss_typename_param_k T>
inline ws_char_w_t const* c_str_ptr_w(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_w_t, T> const& v)
{
    return v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for winstl::basic_findvolume_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T>
inline C const* c_str_ptr(winstl_ns_qual(basic_findvolume_sequence_value_type)<C, T> const& v)
{
    return v;
}


// c_str_ptr_null

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ws_char_a_t const* c_str_ptr_null_a(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_a_t, T> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null_a(v));
}
template <ss_typename_param_k T>
inline ws_char_w_t const* c_str_ptr_null_w(winstl_ns_qual(basic_findvolume_sequence_value_type)<ws_char_w_t, T> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null_w(v));
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for winstl::basic_findvolume_sequence_value_type
 *
 * \ingroup group__concept__shim__string_access
 */
template <ss_typename_param_k C, ss_typename_param_k T>
inline C const* c_str_ptr_null(winstl_ns_qual(basic_findvolume_sequence_value_type)<C, T> const& v)
{
    return stlsoft_ns_qual(c_str_ptr_null(v));
}


/* /////////////////////////////////////////////////////////////////////////
 * Deprecated Shims
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C, ss_typename_param_k T>
inline ws_bool_t is_empty(winstl_ns_qual(basic_findvolume_sequence_value_type)<C, T> const& v)
{
    return '\0' == v[0];
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/findvolume_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// basic_findvolume_sequence

template <ss_typename_param_k C, ss_typename_param_k T>
inline ss_typename_type_ret_k basic_findvolume_sequence<C, T>::const_iterator basic_findvolume_sequence<C, T>::begin() const
{
    char_type   vol_name[MAX_VOL_NAME + 1];
    HANDLE      hSrch   =   traits_type::find_first_volume(vol_name, STLSOFT_NUM_ELEMENTS(vol_name));

    if(hSrch != INVALID_HANDLE_VALUE)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        try
        {
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            return const_iterator(*this, hSrch, vol_name);
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        }
        catch(...)
        {
            traits_type::find_volume_close(hSrch);

            throw;
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return const_iterator(*this);
}

template <ss_typename_param_k C, ss_typename_param_k T>
inline ss_typename_type_ret_k basic_findvolume_sequence<C, T>::const_iterator basic_findvolume_sequence<C, T>::end() const
{
    return const_iterator(*this);
}

template <ss_typename_param_k C, ss_typename_param_k T>
inline ws_bool_t basic_findvolume_sequence<C, T>::empty() const
{
    return begin() == end();
}

// basic_findvolume_sequence_value_type

template <ss_typename_param_k C, ss_typename_param_k T>
inline basic_findvolume_sequence_value_type<C, T>::basic_findvolume_sequence_value_type()
{
    m_name[0] = '\0';
}

template <ss_typename_param_k C, ss_typename_param_k T>
inline basic_findvolume_sequence_value_type<C, T>::basic_findvolume_sequence_value_type(class_type const& rhs)
{
    traits_type::char_copy(m_name, rhs.m_name, STLSOFT_NUM_ELEMENTS(m_name));
}

template <ss_typename_param_k C, ss_typename_param_k T>
inline basic_findvolume_sequence_value_type<C, T>::basic_findvolume_sequence_value_type(char_type const* vol_name)
{
    traits_type::char_copy(m_name, vol_name, STLSOFT_NUM_ELEMENTS(m_name));
}

template <ss_typename_param_k C, ss_typename_param_k T>
inline ss_typename_type_ret_k basic_findvolume_sequence_value_type<C, T>::class_type& basic_findvolume_sequence_value_type<C, T>::operator =(class_type const& rhs)
{
    traits_type::char_copy(m_name, rhs.m_name, STLSOFT_NUM_ELEMENTS(m_name));

    return *this;
}


// basic_findvolume_sequence_const_iterator

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline basic_findvolume_sequence_const_iterator<C, T, V>::basic_findvolume_sequence_const_iterator()
    : m_list(NULL)
    , m_handle(NULL)
{
    m_name[0] = '\0';
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline basic_findvolume_sequence_const_iterator<C, T, V>::basic_findvolume_sequence_const_iterator(basic_findvolume_sequence<C, T> const& l)
    : m_list(&l)
    , m_handle(NULL)
{
    m_name[0] = '\0';
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline basic_findvolume_sequence_const_iterator<C, T, V>::basic_findvolume_sequence_const_iterator(class_type const& rhs)
    : m_list(rhs.m_list)
    , m_handle(rhs.m_handle)
{
    if(NULL != m_handle)
    {
        m_handle->AddRef();
    }
}

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline ss_typename_type_ret_k basic_findvolume_sequence_const_iterator<C, T, V>::class_type& basic_findvolume_sequence_const_iterator<C, T, V>::operator =(ss_typename_type_k basic_findvolume_sequence_const_iterator<C, T, V>::class_type const& rhs)
{
    shared_handle   *this_handle    =   m_handle;

    m_list      =   rhs.m_list;
    m_handle    =   rhs.m_handle;

    if(NULL != m_handle)
    {
        m_handle->AddRef();
    }

    if(NULL != this_handle)
    {
        this_handle->Release();
    }

    return *this;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline basic_findvolume_sequence_const_iterator<C, T, V>::~basic_findvolume_sequence_const_iterator() stlsoft_throw_0()
{
    if(NULL != m_handle)
    {
        m_handle->Release();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline ss_typename_type_ret_k basic_findvolume_sequence_const_iterator<C, T, V>::class_type& basic_findvolume_sequence_const_iterator<C, T, V>::operator ++()
{
    WINSTL_MESSAGE_ASSERT("Attempting to increment an invalid iterator!", NULL != m_handle);

    if(!traits_type::find_next_volume(m_handle->hSrch, m_name, STLSOFT_NUM_ELEMENTS(m_name)))
    {
        m_handle->Release();

        m_handle = NULL;
    }

    return *this;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline ss_typename_type_ret_k basic_findvolume_sequence_const_iterator<C, T, V>::class_type basic_findvolume_sequence_const_iterator<C, T, V>::operator ++(int)
{
    class_type  ret(*this);

    operator ++();

    return ret;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline const ss_typename_type_k basic_findvolume_sequence_const_iterator<C, T, V>::value_type basic_findvolume_sequence_const_iterator<C, T, V>::operator *() const
{
    if(NULL != m_handle)
    {
        return value_type(m_name);
    }
    else
    {
        return value_type();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline ws_bool_t basic_findvolume_sequence_const_iterator<C, T, V>::operator ==(class_type const& rhs) const
{
    WINSTL_MESSAGE_ASSERT("Comparing iterators from separate sequences", (m_list == rhs.m_list || NULL == m_list || NULL == rhs.m_list));

    return m_handle == rhs.m_handle;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V>
inline ws_bool_t basic_findvolume_sequence_const_iterator<C, T, V>::operator !=(class_type const& rhs) const
{
    return ! operator ==(rhs);
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

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _WINSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::winstl::c_str_len;
using ::winstl::c_str_len_a;
using ::winstl::c_str_len_w;

using ::winstl::c_str_data;
using ::winstl::c_str_data_a;
using ::winstl::c_str_data_w;

using ::winstl::c_str_ptr;
using ::winstl::c_str_ptr_a;
using ::winstl::c_str_ptr_w;

using ::winstl::c_str_ptr_null;
//using ::winstl::c_str_ptr_null_a;
//using ::winstl::c_str_ptr_null_w;

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

using ::winstl::is_empty;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FINDVOLUME_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
