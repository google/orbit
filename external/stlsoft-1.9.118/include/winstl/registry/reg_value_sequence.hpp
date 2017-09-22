/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/reg_value_sequence.hpp
 *
 * Purpose:     Contains the basic_reg_value_sequence class template, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Notes:       The original implementation of the class had the iterator
 *              and value_type as nested classes. Unfortunately, Visual C++ 5 &
 *              6 both had either compilation or linking problems so these are
 *              regretably now implemented as independent classes.
 *
 * Thanks:      To Allan McLellan, for pointing out some inadequacies in the
 *              basic_reg_key_sequence class interface (that equally applied to.
 *              basic_reg_value_sequence).
 *
 * Created:     19th January 2002
 * Updated:     13th May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2014, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/reg_value_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_reg_value_sequence
 *   class template
 *   (\ref group__library__windows_registry "Windows Registry" Library).
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE
#define WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE_MAJOR    3
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE_MINOR    7
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE_REVISION 3
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE_EDIT     126
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REGFWD
# include <winstl/registry/regfwd.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REGFWD */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS
# include <winstl/registry/util/defs.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_DEFS */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS
# include <winstl/registry/reg_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_TRAITS */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE
# include <winstl/registry/reg_value.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES
# include <winstl/registry/util/shared_handles.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_UTIL_HPP_SHARED_HANDLES */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR
# include <stlsoft/smartptr/ref_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */

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

// class basic_reg_value_sequence
/** \brief Presents an STL-like sequence interface over the values of a given registry key
 *
 * \ingroup group__library__windows_registry
 *
 * \param C The character type
 * \param T The traits type. On translators that support default template arguments this defaults to reg_traits<C>
 * \param A The allocator type. On translators that support default template arguments this defaults to processheap_allocator<C>
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = reg_traits<C>
        ,   ss_typename_param_k A = processheap_allocator<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = reg_traits<C> */
        ,   ss_typename_param_k A /* = processheap_allocator<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_reg_value_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// \brief The character type
    typedef C                                                                   char_type;
    /// \brief The traits type
    typedef T                                                                   traits_type;
    /// \brief The allocator type
    typedef A                                                                   allocator_type;
    /// \brief The current parameterisation of the type
    typedef basic_reg_value_sequence<C, T, A>                                   class_type;
    /// \brief The value type
    typedef basic_reg_value<C, T, A>                                            value_type;
    /// \brief The size type
    typedef ss_typename_type_k traits_type::size_type                           size_type;
    /// \brief The get key type
    typedef basic_reg_key<C, T, A>                                              reg_key_type;
    /// \brief The non-mutating (const) iterator type
    typedef basic_reg_value_sequence_iterator<C, T, value_type, A>              iterator;
    /// \brief The non-mutating (const) iterator type
    ///
    /// \note This is retained for backwards compatibility
    typedef iterator                                                            const_iterator;
    /// \brief The reference type
    typedef value_type&                                                         reference;
    /// \brief The non-mutable (const) reference type
    typedef value_type const&                                                   const_reference;
    /// \brief The hkey type
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1100
    /* WSCB: VC5 has an unresolved external linker error if use traits_type::hkey_type */
    typedef HKEY                                                                hkey_type;
#else /* ? compiler */
    typedef ss_typename_type_k traits_type::hkey_type                           hkey_type;
#endif /* compiler */
    /// \brief The difference type
    typedef ws_ptrdiff_t                                                        difference_type;
    /// \brief The non-mutating (const) reverse iterator type
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef stlsoft_ns_qual(reverse_bidirectional_iterator_base)  <   iterator
                                                                  ,   value_type
                                                                  ,   value_type  // By-Value Temporary reference category
                                                                  ,   void        // By-Value Temporary reference category
                                                                  ,   difference_type
                                                                  >             reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// \brief The Boolean type
    typedef ws_bool_t                                                           bool_type;
private:
    /// \brief The results type of the Registry API
    typedef ss_typename_type_k traits_type::result_type                         result_type;
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   char_type
                                            ,   allocator_type
                                            ,   CCH_REG_API_AUTO_BUFFER
                                            >                                   buffer_type_;
public:
    typedef hkey_type                                                           resource_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Creates an instance which provides access to the values of the named sub-key of \c hkey
    ///
    /// \param hkey A registry key handle representing the parent of \c subKeyName
    /// \param subKeyName The name of the sub-key whose values will be enumerated. If subKeyName is NULL or the empty string, then
    /// the values of \c hkey will be enumerated
    /// \param accessMask The security access mask with which the key (hkey + subKeyName) will be opened. Defaults to KEY_READ.
    ///
    /// \note If accessMask contains KEY_NOTIFY, this method will construct a sequence whose iterators monitor for external iterator
    /// invalidation. Use the alternative (four-parameter) constructor form to explicitly suppress monitoring.
    basic_reg_value_sequence(   hkey_type           hkey
                            ,   char_type const     *subKeyName
                            ,   REGSAM              accessMask                      =   KEY_READ);
    /// \brief Creates an instance which provides access to the values of the named sub-key of \c hkey
    ///
    /// \param hkey A registry key handle representing the parent of \c subKeyName
    /// \param subKeyName The name of the sub-key whose values will be enumerated. If subKeyName is NULL or the empty string, then
    /// the values of \c hkey will be enumerated
    /// \param accessMask The security access mask with which the key (hkey + subKeyName) will be opened. Defaults to KEY_READ
    /// \param bMonitorExternalInvalidation If non-zero, the iterators will monitor for external iterator invalidation, throwing
    /// an instance of registry_exception (or a derived class) when any values are added or removed
    ///
    /// \note The bMonitorExternalInvalidation parameter overrides the accessMask parameter. i.e. if bMonitorExternalInvalidation is
    /// non-zero then accessMask is combined with KEY_NOTIFY. If not, then KEY_NOTIFY is stripped from accessMask.
    basic_reg_value_sequence(   hkey_type           hkey
                            ,   char_type const     *subKeyName
                            ,   REGSAM              accessMask
                            ,   bool_type           bMonitorExternalInvalidation);
    /// \brief Creates an instance which provides access to the values of of \c key
    ///
    /// \param key A registry key handle representing the parent of \c subKeyName
    ///
    /// \note If the key's access mask contains KEY_NOTIFY, this method will construct a sequence whose iterators monitor for external iterator
    /// invalidation. Use the alternative (three-parameter) constructor form to explicitly suppress monitoring.
    ss_explicit_k basic_reg_value_sequence(reg_key_type const& key);
    /// \brief Creates an instance which provides access to the values of of \c key
    ///
    /// \param key A registry key handle representing the parent of \c subKeyName
    /// \param accessMask The security access mask with which the key will be used. Defaults to KEY_READ
    ///
    /// \note If accessMask contains KEY_NOTIFY, this method will construct a sequence whose iterators monitor for external iterator
    /// invalidation. Use the alternative (three-parameter) constructor form to explicitly suppress monitoring.
    basic_reg_value_sequence(   reg_key_type const& key
                            ,   REGSAM              accessMask);
    /// \brief Creates an instance which provides access to the values of of \c key
    ///
    /// \param key A registry key handle representing the parent of \c subKeyName
    /// \param accessMask The security access mask with which the key will be used. Defaults to KEY_READ
    /// \param bMonitorExternalInvalidation If non-zero, the iterators will monitor for external iterator invalidation, throwing
    /// an instance of registry_exception (or a derived class) when any values are added or removed
    ///
    /// \note The bMonitorExternalInvalidation parameter overrides the accessMask parameter. i.e. if bMonitorExternalInvalidation is
    /// non-zero then accessMask is combined with KEY_NOTIFY. If not, then KEY_NOTIFY is stripped from accessMask.
    basic_reg_value_sequence(   reg_key_type const& key
                            ,   REGSAM              accessMask
                            ,   bool_type           bMonitorExternalInvalidation);
    /// \brief Destructor
    ~basic_reg_value_sequence() stlsoft_throw_0();
/// @}

/// \name Iteration
/// @{
public:
    /// \brief Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator          begin();
    /// \brief Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator          end();

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// \brief Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator  rbegin();
    /// \brief Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator  rend();
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Attributes
/// @{
public:
    /// \brief Returns the number of values
    ///
    /// \note This gives a result valid only at the epoch of the call. A
    /// subsequent call may return a different result.
    size_type               current_size() const;
    /// \brief Returns the number of values
    ///
    /// \deprecated This is equivalent to current_size()
    size_type               size() const;
    /// \brief Evalulates whether there are no values
    ws_bool_t               empty() const;

    /// \brief The key handle
    hkey_type               get_key_handle() const;
    /// \brief The key handle
    hkey_type               get() const;
/// @}

/// \name Implementation
/// @{
private:
    registry_util::shared_handle    *create_shared_handle_(result_type& res);
    static REGSAM                   validate_access_mask_(REGSAM accessMask, bool_type bMonitorExternalInvalidation);
    static hkey_type                dup_key_(hkey_type hkey, REGSAM accessMask/* , result_type *result */);
/// @}

/// \name Members
/// @{
private:
    hkey_type       m_hkey;
    const REGSAM    m_accessMask;
    const bool_type m_bMonitorExternalInvalidation;
/// @}

/// \name Not to be implemented
/// @{
private:
    basic_reg_value_sequence(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

/* Typedefs to commonly encountered types. */
/** \brief Specialisation of the basic_reg_value_sequence template for the ANSI character type \c char
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_value_sequence<ws_char_a_t, reg_traits<ws_char_a_t>, processheap_allocator<ws_char_a_t> >     reg_value_sequence_a;
/** \brief Specialisation of the basic_reg_value_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_value_sequence<ws_char_w_t, reg_traits<ws_char_w_t>, processheap_allocator<ws_char_w_t> >     reg_value_sequence_w;
/** \brief Specialisation of the basic_reg_value_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_value_sequence<TCHAR, reg_traits<TCHAR>, processheap_allocator<TCHAR> >                       reg_value_sequence;

// class basic_reg_value_sequence_iterator
/** \brief Iterator for the basic_reg_value_sequence class
 *
 * \ingroup group__library__windows_registry
 *
 * \param C The character type
 * \param T The traits type
 * \param V The value type
 * \param A The allocator type
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k V
        ,   ss_typename_param_k A
        >
class basic_reg_value_sequence_iterator
    : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(bidirectional_iterator_tag)
                                        ,   V
                                        ,   ws_ptrdiff_t
                                        ,   void    // By-Value Temporary reference
                                        ,   V       // By-Value Temporary reference
                                        >
{
/// \name Member Types
/// @{
public:
    /// \brief The character type
    typedef C                                                           char_type;
    /// \brief The traits type
    typedef T                                                           traits_type;
    /// \brief The value type
    typedef V                                                           value_type;
    /// \brief The allocator type
    typedef A                                                           allocator_type;
    /// \brief The current parameterisation of the type
    typedef basic_reg_value_sequence_iterator<C, T, V, A>               class_type;
    /// \brief The size type
    typedef ss_typename_type_k traits_type::size_type                   size_type;
    /// \brief The difference type
    typedef ss_typename_type_k traits_type::difference_type             difference_type;
    /// \brief The string type
    typedef ss_typename_type_k traits_type::string_type                 string_type;
    /// \brief The index type
    typedef ws_sint32_t                                                 index_type;
    /// \brief The hkey type
    typedef ss_typename_type_k traits_type::hkey_type                   hkey_type;
private:
    /// \brief The results type of the Registry API
    typedef ss_typename_type_k traits_type::result_type                 result_type;
    /// \brief The Boolean type
    typedef ws_bool_t                                                   bool_type;
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   char_type
                                            ,   allocator_type
                                            ,   CCH_REG_API_AUTO_BUFFER
                                            >                           buffer_type_;
/// @}

/// \name Construction
/// @{
private:
    friend class basic_reg_value_sequence<C, T, A>;

    /// \note Eats the key, rather than taking a copy
    basic_reg_value_sequence_iterator(registry_util::shared_handle *handle, char_type const* name, size_type cchName, index_type index, REGSAM accessMask)
        : m_handle(handle)
        , m_index(index)
        , m_name(name, cchName)
        , m_accessMask(accessMask)
    {
        WINSTL_ASSERT(NULL != m_handle);
        m_handle->test_reset_and_throw();
        m_handle->AddRef();
    }
public:
    /// \brief Default constructor
    basic_reg_value_sequence_iterator();
    /// \brief Copy constructor
    basic_reg_value_sequence_iterator(class_type const& rhs);
    /// \brief Destructor
    ~basic_reg_value_sequence_iterator() stlsoft_throw_0();

    /// \brief Copy assignment operator
    class_type& operator =(class_type const& rhs);
/// @}

/// \name Accessors
/// @{
public:
    string_type const&  get_key_name() const;
/// @}

/// \name Operators
/// @{
public:
    /// \brief Pre-increment operator
    class_type& operator ++();
    /// \brief Pre-decrement operator
    class_type& operator --();
    /// \brief Post-increment operator
    const class_type operator ++(int);
    /// \brief Post-decrement operator
    const class_type operator --(int);
    /// \brief Dereference to return the value representing the current position
    const value_type operator *() const;
    /// \brief Evaluates whether \c this and \c rhs are equivalent
    ws_bool_t equal(class_type const& rhs) const;
    /// \brief Evaluates whether \c this and \c rhs are equivalent
    ws_bool_t operator ==(class_type const& rhs) const;
    /// \brief Evaluates whether \c this and \c rhs are not equivalent
    ws_bool_t operator !=(class_type const& rhs) const;
/// @}

/// \name Implementation
/// @{
private:
    static index_type sentinel_() stlsoft_throw_0();
/// @}

/// \name Members
/// @{
private:
    registry_util::shared_handle    *m_handle;      // Shared context for registry key and event object
    index_type                      m_index;        // Current iteration index
    string_type                     m_name;         // The value name
    REGSAM                          m_accessMask;   // Security access mask
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/reg_value_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// basic_reg_value_sequence

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline registry_util::shared_handle* basic_reg_value_sequence<C, T, A>::create_shared_handle_(result_type& res)
{
    // 1. Duplicate the registry handle
    //
    // 2. create the shared_handle

    registry_util::shared_handle*   handle  =   NULL;
    hkey_type                       hkey2   =   traits_type::key_dup(m_hkey, m_accessMask, &res);

    if(NULL == hkey2)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not duplicate key";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(key_not_duplicated_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        handle = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        // Pop it in a scoped handle for RAII
        scoped_handle<HKEY> sh(hkey2, ::RegCloseKey);

        handle = registry_util::create_shared_handle(hkey2, m_bMonitorExternalInvalidation, REG_NOTIFY_CHANGE_LAST_SET);

        if(NULL == handle)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            static const char   message[]   =   "could not create shared enumeration context";
            DWORD               err         =   ::GetLastError();

            if(ERROR_ACCESS_DENIED == err)
            {
                STLSOFT_THROW_X(access_denied_exception(message, err));
            }
            else
            {
                STLSOFT_THROW_X(registry_exception(message, err));
            }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            sh.detach();
        }
    }

    return handle;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ REGSAM basic_reg_value_sequence<C, T, A>::validate_access_mask_(REGSAM accessMask, ss_typename_type_k basic_reg_value_sequence<C, T, A>::bool_type bMonitorExternalInvalidation)
{
    if(bMonitorExternalInvalidation)
    {
        return accessMask | KEY_NOTIFY;
    }
    else
    {
        return accessMask & ~(KEY_NOTIFY);
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::hkey_type basic_reg_value_sequence<C, T, A>::dup_key_(ss_typename_type_k basic_reg_value_sequence<C, T, A>::hkey_type hkey, REGSAM accessMask/* , ss_typename_type_k basic_reg_value_sequence<C, T, A>::result_type *result */)
{
    result_type res;
    HKEY        hkeyDup =   traits_type::key_dup(hkey, accessMask, &res);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not duplicate key";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(key_not_duplicated_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        hkeyDup = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return hkeyDup;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::basic_reg_value_sequence( ss_typename_type_k basic_reg_value_sequence<C, T, A>::hkey_type         hkey
                                                                ,   ss_typename_type_k basic_reg_value_sequence<C, T, A>::char_type const   *subKeyName
                                                                ,   REGSAM                                                                  accessMask /* = KEY_READ */)
    : m_hkey(NULL)
    , m_accessMask(accessMask)
    , m_bMonitorExternalInvalidation(0 != (KEY_NOTIFY & accessMask))
{
    result_type res;

    if(ERROR_SUCCESS != (res = traits_type::reg_open_key(hkey, subKeyName, &m_hkey, accessMask)))
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not open key";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        m_hkey = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::basic_reg_value_sequence( ss_typename_type_k basic_reg_value_sequence<C, T, A>::hkey_type         hkey
                                                                ,   ss_typename_type_k basic_reg_value_sequence<C, T, A>::char_type const   *subKeyName
                                                                ,   REGSAM                                                                  accessMask
                                                                ,   ss_typename_type_k basic_reg_value_sequence<C, T, A>::bool_type         bMonitorExternalInvalidation)
    : m_hkey(NULL)
    , m_accessMask(validate_access_mask_(accessMask, bMonitorExternalInvalidation))
    , m_bMonitorExternalInvalidation(bMonitorExternalInvalidation)
{
    result_type res;

    if(ERROR_SUCCESS != (res = traits_type::reg_open_key(hkey, subKeyName, &m_hkey, accessMask)))
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not open key";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        m_hkey = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::basic_reg_value_sequence(ss_typename_type_k basic_reg_value_sequence<C, T, A>::reg_key_type const& key)
    : m_hkey(dup_key_(key.m_hkey, key.get_access_mask()))
    , m_accessMask(key.get_access_mask())
    , m_bMonitorExternalInvalidation(0 != (KEY_NOTIFY & key.get_access_mask()))
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hkey)
    {
        STLSOFT_THROW_X(registry_exception("failed to take duplicate of key", ::GetLastError()));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::basic_reg_value_sequence( ss_typename_type_k basic_reg_value_sequence<C, T, A>::reg_key_type const&   key
                                                                ,   REGSAM                                                                      accessMask)
    : m_hkey(dup_key_(key.m_hkey, accessMask))
    , m_accessMask(accessMask)
    , m_bMonitorExternalInvalidation(0 != (KEY_NOTIFY & accessMask))
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hkey)
    {
        STLSOFT_THROW_X(registry_exception("failed to take duplicate of key", ::GetLastError()));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::basic_reg_value_sequence( ss_typename_type_k basic_reg_value_sequence<C, T, A>::reg_key_type const&   key
                                                                ,   REGSAM                                                                      accessMask
                                                                ,   bool_type                                                                   bMonitorExternalInvalidation)
    : m_hkey(dup_key_(key.m_hkey, validate_access_mask_(accessMask, bMonitorExternalInvalidation)))
    , m_accessMask(validate_access_mask_(accessMask, bMonitorExternalInvalidation))
    , m_bMonitorExternalInvalidation(bMonitorExternalInvalidation)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(NULL == m_hkey)
    {
        STLSOFT_THROW_X(registry_exception("failed to take duplicate of key", ::GetLastError()));
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value_sequence<C, T, A>::~basic_reg_value_sequence() stlsoft_throw_0()
{
    if(m_hkey != NULL)
    {
        ::RegCloseKey(m_hkey);
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::iterator basic_reg_value_sequence<C, T, A>::begin()
{
    // 1. Check that there are some items
    //
    // 2. Duplicate the registry key handle & Create the shared handle
    //
    // 4. Loop to get the full name
    //
    // 5. Create the iterator and return


    // 1. Check that there are some items

    // Grab enough for the first item
    size_type   cchName     =   0;
    ws_uint32_t numEntries  =   0;
    result_type res         =   traits_type::reg_query_info(m_hkey, NULL, NULL, NULL, NULL, NULL, &numEntries, &cchName, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not elicit value information";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ; // This will fall through to the end() call at the end of the function
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        if(0 != numEntries)
        {
            // 2. Duplicate the registry key handle & create the shared handle
            registry_util::shared_handle    *handle =   create_shared_handle_(res);
            ws_sint32_t                     index   =   0;

            if(NULL == handle)
            {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                static const char message[] = "could not create shared enumeration context";

                if(ERROR_ACCESS_DENIED == res)
                {
                    STLSOFT_THROW_X(access_denied_exception(message, res));
                }
                else
                {
                    STLSOFT_THROW_X(registry_exception(message, res));
                }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                ; // This will fall through to the end() call at the end of the function
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
            else
            {
                ref_ptr<registry_util::shared_handle>   ref(handle, false); // Eat the reference here. The iterator will take another

                // 4. Loop to get the full name
                buffer_type_ buffer(++cchName); // This is increased so that the call to reg_enum_value is likely to succeed

                for(; !buffer.empty(); )    // Need to loop because sub-keys can change, when we're not monitoring
                {
                    cchName = buffer.size();

                    res = traits_type::reg_enum_value(m_hkey, 0, &buffer[0], &cchName);

                    if(ERROR_MORE_DATA == res)
                    {
                        if(!buffer.resize(2 * buffer.size())) // Throws, or returns false
                        {
                            cchName =   0;
                            index   =   const_iterator::sentinel_();
                            break;
                        }

                        continue; // "Let's go round again"
                    }
                    else if(ERROR_SUCCESS != res)
                    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                        static const char message[] = "could not enumerate values";

                        if(ERROR_ACCESS_DENIED == res)
                        {
                            STLSOFT_THROW_X(access_denied_exception(message, res));
                        }
                        else
                        {
                            STLSOFT_THROW_X(registry_exception(message, res));
                        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                        cchName = 0;
                        index   =   const_iterator::sentinel_();
                        break; // This will fall through to the end() call at the end of the function
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
                    }
                    else
                    {
                        break;
                    }
                }

                // 5. Create the iterator and return
                return iterator(handle, buffer.data(), cchName, index, m_accessMask);
            }
        }
    }

    return end();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::iterator basic_reg_value_sequence<C, T, A>::end()
{
    result_type                             res;
    registry_util::shared_handle            *handle =   create_shared_handle_(res);
    ref_ptr<registry_util::shared_handle>   ref(handle, false); // Eat the reference here. The iterator will take another
    ws_sint32_t                             index   =   const_iterator::sentinel_();

    if(NULL == handle)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(registry_exception("failed to take duplicate of key", static_cast<DWORD>(res)));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        index = 0; // This will fall through to the constructor at the end of the function
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return iterator(handle, NULL, 0, index, m_accessMask);
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::reverse_iterator basic_reg_value_sequence<C, T, A>::rbegin()
{
    return reverse_iterator(end());
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::reverse_iterator basic_reg_value_sequence<C, T, A>::rend()
{
    return reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::size_type basic_reg_value_sequence<C, T, A>::size() const
{
    return current_size();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::size_type basic_reg_value_sequence<C, T, A>::current_size() const
{
    ws_uint32_t numEntries;
    result_type res         =   traits_type::reg_query_info(m_hkey, NULL, NULL, NULL, NULL, NULL, &numEntries, NULL, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not elicit number of values";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        numEntries = 0;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return static_cast<size_type>(numEntries);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ws_bool_t basic_reg_value_sequence<C, T, A>::empty() const
{
    return 0 == size();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::hkey_type basic_reg_value_sequence<C, T, A>::get_key_handle() const
{
    return m_hkey;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence<C, T, A>::hkey_type basic_reg_value_sequence<C, T, A>::get() const
{
    return get_key_handle();
}

// basic_reg_value_sequence_iterator

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline /* static */ ss_typename_type_ret_k basic_reg_value_sequence_iterator<C, T, V, A>::index_type basic_reg_value_sequence_iterator<C, T, V, A>::sentinel_() stlsoft_throw_0()
{
    return 0x7fffffff;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline basic_reg_value_sequence_iterator<C, T, V, A>::basic_reg_value_sequence_iterator()
    : m_handle(NULL)
    , m_index(sentinel_())
    , m_name()
    , m_accessMask(KEY_READ)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline basic_reg_value_sequence_iterator<C, T, V, A>::basic_reg_value_sequence_iterator(class_type const& rhs)
    : m_handle(rhs.m_handle)
    , m_index(rhs.m_index)
    , m_name(rhs.m_name)
    , m_accessMask(rhs.m_accessMask)
{
    if(NULL != m_handle)
    {
        m_handle->AddRef();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type& basic_reg_value_sequence_iterator<C, T, V, A>::operator =(ss_typename_type_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type const& rhs)
{
    registry_util::shared_handle   *this_handle;

    m_index         =   rhs.m_index;
    m_name          =   rhs.m_name;

    this_handle     =   m_handle;
    m_handle        =   rhs.m_handle;
    m_accessMask    =   rhs.m_accessMask;

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

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline basic_reg_value_sequence_iterator<C, T, V, A>::~basic_reg_value_sequence_iterator() stlsoft_throw_0()
{
    if(NULL != m_handle)
    {
        m_handle->Release();
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline const ss_typename_type_k basic_reg_value_sequence_iterator<C, T, V, A>::string_type& basic_reg_value_sequence_iterator<C, T, V, A>::get_key_name() const
{
    return m_name;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type& basic_reg_value_sequence_iterator<C, T, V, A>::operator ++()
{
    WINSTL_MESSAGE_ASSERT("Attempting to increment an invalid iterator!", NULL != m_handle);
    WINSTL_MESSAGE_ASSERT("Attempting to increment an invalid iterator!", sentinel_() != m_index);

    // Grab enough for the first item
    size_type   cchName =   0;
    result_type res     =   traits_type::reg_query_info(m_handle->m_hkey, NULL, NULL, NULL, NULL, NULL, NULL, &cchName, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not elicit value information";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        m_index = sentinel_();
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        buffer_type_    buffer(++cchName);   // This is increased so that the call to reg_enum_value is likely to succeed

        for(; !buffer.empty(); buffer.resize(2 * buffer.size()))    // Need to loop because values can change, when we're not monitoring
        {
            cchName  =   buffer.size();

            res = traits_type::reg_enum_value(m_handle->m_hkey, static_cast<ws_dword_t>(1 + m_index), &buffer[0], &cchName);

            if(ERROR_MORE_DATA == res)
            {
                continue; // "Let's go round again"
            }
            else if(ERROR_NO_MORE_ITEMS == res)
            {
                m_index = sentinel_();
                break;
            }
            else if(ERROR_SUCCESS != res)
            {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                static const char message[] = "could not enumerate values";

                if(ERROR_ACCESS_DENIED == res)
                {
                    STLSOFT_THROW_X(access_denied_exception(message, res));
                }
                else
                {
                    STLSOFT_THROW_X(registry_exception(message, res));
                }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                m_index = sentinel_();
                break;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
            else
            {
                m_name.assign(buffer.data(), cchName);

                ++m_index;

                break;
            }
        }
    }

    m_handle->test_reset_and_throw();

    return *this;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type& basic_reg_value_sequence_iterator<C, T, V, A>::operator --()
{
    WINSTL_MESSAGE_ASSERT("Attempting to decrement an invalid iterator", NULL != m_handle);

    // Grab enough for the first item
    size_type   cchName         =   0;
    ws_uint32_t numEntries      =   0;
    result_type res             =   traits_type::reg_query_info(m_handle->m_hkey, NULL, NULL, NULL, NULL, NULL, &numEntries, &cchName, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not elicit value information";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        m_index = sentinel_();
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        buffer_type_    buffer(++cchName);   // This is increased so that the call to reg_enum_value is likely to succeed
        ws_dword_t      index;

        // If the iterator is currently at the "end()", ...
        if(m_index == sentinel_())
        {
            // ... then set the index to be one past the end
            index = numEntries - 1;
        }
        else
        {
            // ... otherwise just go back one from current
            index = m_index - 1;
        }

        for(; !buffer.empty(); buffer.resize(2 * buffer.size()))    // Need to loop because values can change, when we're not monitoring
        {
            cchName = buffer.size();

            res = traits_type::reg_enum_value(m_handle->m_hkey, index, &buffer[0], &cchName);

            if(ERROR_MORE_DATA == res)
            {
                continue; // "Let's go round again"
            }
            else if(ERROR_SUCCESS != res)
            {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                static const char message[] = "could not elicit value information";

                if(ERROR_ACCESS_DENIED == res)
                {
                    STLSOFT_THROW_X(access_denied_exception(message, res));
                }
                else
                {
                    STLSOFT_THROW_X(registry_exception(message, res));
                }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
                m_index = sentinel_();
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
            else
            {
                m_name.assign(buffer.data(), cchName);

                m_index = index;

                break;
            }
        }
    }

    m_handle->test_reset_and_throw();

    return *this;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline const ss_typename_type_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type basic_reg_value_sequence_iterator<C, T, V, A>::operator ++(int)
{
    class_type  ret(*this);

    operator ++();

    return ret;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline const ss_typename_type_k basic_reg_value_sequence_iterator<C, T, V, A>::class_type basic_reg_value_sequence_iterator<C, T, V, A>::operator --(int)
{
    class_type  ret(*this);

    operator --();

    return ret;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline const ss_typename_type_k basic_reg_value_sequence_iterator<C, T, V, A>::value_type basic_reg_value_sequence_iterator<C, T, V, A>::operator *() const
{
    WINSTL_MESSAGE_ASSERT("Attempting to dereference an invalid iterator", NULL != m_handle);

    m_handle->test_reset_and_throw();

    return value_type(m_handle->m_hkey, m_name);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ws_bool_t basic_reg_value_sequence_iterator<C, T, V, A>::equal(class_type const& rhs) const
{
    return m_index == rhs.m_index;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ws_bool_t basic_reg_value_sequence_iterator<C, T, V, A>::operator ==(class_type const& rhs) const
{
    return equal(rhs);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k V, ss_typename_param_k A>
inline ws_bool_t basic_reg_value_sequence_iterator<C, T, V, A>::operator !=(class_type const& rhs) const
{
    return !equal(rhs);
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

#endif /* WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
