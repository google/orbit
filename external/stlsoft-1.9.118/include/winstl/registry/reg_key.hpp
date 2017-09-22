/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/registry/reg_key.hpp
 *
 * Purpose:     Contains the basic_reg_key class template, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     19th January 2002
 * Updated:     22nd November 2013
 *
 * Thanks:      To Sam Fisher for spotting the defect in the set_value_()
 *              overload for REG_MULTI_SZ values (widestring only).
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2013, Matthew Wilson and Synesis Software
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


/** \file winstl/registry/reg_key.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_reg_key class template
 *   (\ref group__library__windows_registry "Windows Registry" Library).
 */

#ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_KEY
#define WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_KEY

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_KEY_MAJOR       3
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_KEY_MINOR       9
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_KEY_REVISION    10
# define WINSTL_VER_WINSTL_REGISTRY_HPP_REG_KEY_EDIT        137
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
#if defined(STLSOFT_COMPILER_IS_DMC)
# ifndef WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE
#  include <winstl/registry/reg_value.hpp>
# endif /* !WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_VALUE */
#endif /* compiler */
#ifndef WINSTL_INCL_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS
# include <winstl/registry/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_REGISTRY_ERROR_HPP_EXCEPTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HKEY
# include <winstl/shims/attribute/get_HKEY.hpp>
#endif /* !WINSTL_INCL_SHIMS_ATTRIBUTE_HPP_GET_HKEY */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR
# include <stlsoft/iterators/transform_iterator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_TRANSFORM_ITERATOR */

#ifndef STLSOFT_INCL_NUMERIC
# define STLSOFT_INCL_NUMERIC
# include <numeric>
#endif /* !STLSOFT_INCL_NUMERIC */
#ifndef STLSOFT_INCL_FUNCTIONAL
# define STLSOFT_INCL_FUNCTIONAL
# include <functional>
#endif /* !STLSOFT_INCL_FUNCTIONAL */

#ifdef STLSOFT_UNITTEST
# include <winstl/registry/reg_value.hpp>
#endif /* STLSOFT_UNITTEST */

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

/** Represents a registry key, and provides methods for manipulating its
 * values and sub-keys.
 *
 * \ingroup group__library__windows_registry
 *
 * This class acts as the value type of classes that manipulate registry keys
 * and encapsulates the concept of a registry key.
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
class basic_reg_key
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C                                           char_type;
    /// The traits type
    typedef T                                           traits_type;
    /// The allocator type
    typedef A                                           allocator_type;
    /// The current parameterisation of the type
    typedef basic_reg_key<C, T, A>                      class_type;
    /// The size type
    typedef ss_typename_type_k traits_type::size_type   size_type;
    /// The string type
    typedef ss_typename_type_k traits_type::string_type string_type;
    /// The key type
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1100
    /* WSCB: VC5 has an internal compiler error if use traits_type::hkey_type */
    typedef HKEY                                        hkey_type;
#else /* ? compiler */
    typedef ss_typename_type_k traits_type::hkey_type   hkey_type;
#endif /* compiler */
    /// The Boolean type
    typedef ws_bool_t                                   bool_type;
    /// The type of the key's values
    typedef basic_reg_value<C, T, A>                    key_value_type;
#if 0
    /// The type of the sub-key collection
    typedef basic_reg_key_sequence<C, T, A>             subkeys_collection_type;
    /// The type of the value collection
    typedef basic_reg_value_sequence<C, T, A>           value_collection_type;
#endif /* 0 */
private:
    /// The results type of the Registry API
    typedef ss_typename_type_k traits_type::result_type result_type;
public:
    typedef hkey_type                                   resource_type;
/// @}

/// \name Construction
/// @{
private:
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    friend class basic_reg_value_sequence<C, T, A>;
    friend class basic_reg_key_sequence<C, T, A>;
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    (   _MSC_VER == 1200 || \
        _MSC_VER == 1300)
public:
#endif /* compiler */
    basic_reg_key(hkey_type* hkey, string_type const& keyName, REGSAM accessMask);
public:
    /// Default constructor
    basic_reg_key();
    /** Construct from the named sub-key of the given parent
     *
     * \param hkeyParent A handle to the parent key, whose named subkey is
     *  to be opened.
     * \param keyName The name of the subkey to open. If <code>NULL</code>
     *  or the empty string, then a copy of <code>hkeyParent</code> will be
     *  opened.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    basic_reg_key(hkey_type hkeyParent, char_type const* keyName, REGSAM accessMask = KEY_ALL_ACCESS)
        : m_name(keyName)
        , m_hkey(open_key_(hkeyParent, keyName, accessMask))
        , m_accessMask(accessMask)
    {} // Implementation is within class, otherwise VC5 will not link
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Construct from the named sub-key of the given parent
     *
     * \param hkeyParent A handle to the parent key, whose named subkey is
     *  to be opened.
     * \param keyName The name of the subkey to open. If <code>NULL</code>
     *  or the empty string, then a copy of <code>hkeyParent</code> will be
     *  opened.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    template <ss_typename_param_k S>
    basic_reg_key(hkey_type hkeyParent, S const& keyName, REGSAM accessMask = KEY_ALL_ACCESS)
        : m_name(keyName)
        , m_hkey(open_key_(hkeyParent, stlsoft_ns_qual(c_str_ptr)(keyName), accessMask))
        , m_accessMask(accessMask)
    {} // Implementation is within class, otherwise VC5 will not link
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    /** Construct from the named sub-key of the given parent
     *
     * \param keyParent A handle to the parent key, whose named subkey is
     *  to be opened.
     * \param keyName The name of the subkey to open. If <code>NULL</code>
     *  or the empty string, then a copy of <code>keyParent</code> will be
     *  opened.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    basic_reg_key(class_type const& keyParent, char_type const* keyName, REGSAM accessMask = KEY_ALL_ACCESS)
        : m_name(keyName)
        , m_hkey(open_key_(keyParent.get_key_handle(), keyName, accessMask))
        , m_accessMask(accessMask)
    {} // Implementation is within class, otherwise VC5 will not link
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Construct from the named sub-key of the given parent
     *
     * \param keyParent A handle to the parent key, whose named subkey is
     *  to be opened.
     * \param keyName The name of the subkey to open. If <code>NULL</code>
     *  or the empty string, then a copy of <code>keyParent</code> will be
     *  opened.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    template <ss_typename_param_k S>
    basic_reg_key(class_type const& keyParent, S const& keyName, REGSAM accessMask = KEY_ALL_ACCESS)
        : m_name(keyName)
        , m_hkey(open_key_(keyParent.get_key_handle(), stlsoft_ns_qual(c_str_ptr)(keyName), accessMask))
        , m_accessMask(accessMask)
    {} // Implementation is within class, otherwise VC5 will not link
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    /** Constructs an instance as a (logical) copy of another.
     *
     * \param rhs Instance whose key will be opened by the new instance.
     *
     * \note The instance will hold a <i>different</i> handle to the
     *  <i>same</i> registry key.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    basic_reg_key(class_type const& rhs);
    /** Constructs an instance as a (logical) copy of another, but with different permissions.
     *
     * \param rhs Instance whose key will be opened by the new instance.
     * \param accessMask The permissions for the new instance.
     *
     * \note The instance will hold a <i>different</i> handle to the
     *  <i>same</i> registry key.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be opened. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    basic_reg_key(class_type const& rhs, REGSAM accessMask);

    /// Destructor
    ///
    /// Releases any resources allocated by the instance, including closing the
    /// underlying registry key.
    ~basic_reg_key() stlsoft_throw_0();

    /// Copy assignment operator
    class_type& operator =(class_type const& rhs);
/// @}

/// \name Attributes
/// @{
public:
    /// The name of the key
    string_type const       &name() const;
    /// The registry class of the key
    string_type             reg_class() const;
    /// The number of sub-keys
    ///
    /// \note This is not a constant-time operation
    size_type               num_sub_keys() const;
    /// The number of values
    ///
    /// \note This is not a constant-time operation
    size_type               num_values() const;
    /// Indicates whether the sub-key exists
    ///
    /// \param subKeyName The name of the sub-key to test
    template <ss_typename_param_k S>
    bool_type               has_sub_key(S const& subKeyName)
    {
        return this->has_sub_key_(stlsoft_ns_qual(c_str_ptr)(subKeyName));
    }
    /// Indicates whether the value exists
    ///
    /// \param valueName The name of the value to test
    template <ss_typename_param_k S>
    bool_type               has_value(S const& valueName)
    {
        return this->has_value_(stlsoft_ns_qual(c_str_ptr)(valueName));
    }

    /** The handle to the underlying Registry API key.
     *
     * \note If \ref page__exception_agnostic "exception handling is not enabled",
     *  then this method will return <code>NULL</code> in the case where an
     *  instance constructor failed to open the key with the requested permissions.
     */
    hkey_type               get_key_handle() const;
    /// The handle to the underlying Registry API key.
    ///
    /// \note Equivalent to get_key_handle()
    hkey_type               get() const;

#if 0
    subkeys_collection_type subkeys() const;
    value_collection_type   values() const;
#endif /* 0 */

    /// The access mask associated with the key
    REGSAM                  get_access_mask() const;
/// @}

/// \name Sub-key operations
/// @{
public:
    /// Opens the named sub-key of this key
    class_type  open_sub_key(char_type const* subKeyName, REGSAM accessMask = KEY_ALL_ACCESS);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    template <ss_typename_param_k S>
    class_type  open_sub_key(S const& subKeyName, REGSAM accessMask = KEY_ALL_ACCESS)
    {
        return open_sub_key_(stlsoft_ns_qual(c_str_ptr)(subKeyName), accessMask);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

    /** Creates a named sub-key of this key
     *
     * \param subKeyName Name of the subkey to created. If <code>NULL</code> or the
     *  empty string, then the function returns a copy of the callee.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be created. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    class_type  create_sub_key(char_type const* subKeyName, REGSAM accessMask = KEY_ALL_ACCESS);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Creates a named sub-key of this key
     *
     * \param subKeyName Name of the subkey to created. If <code>NULL</code> or the
     *  empty string, then the function returns a copy of the callee.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be created. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    template <ss_typename_param_k S>
    class_type create_sub_key(S const& subKeyName, REGSAM accessMask = KEY_ALL_ACCESS)
    {
        return create_sub_key_(stlsoft_ns_qual(c_str_ptr)(subKeyName), accessMask);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

    /** Creates a named sub-key of a given key
     *
     * \param hkey The parent registry key.
     * \param subKeyName Name of the subkey to created. If <code>NULL</code> or the
     *  empty string, then the function returns a copy of the callee.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be created. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    static class_type create_key(HKEY hkey, char_type const* subKeyName, REGSAM accessMask = KEY_ALL_ACCESS);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Creates a named sub-key of this key
     *
     * \param key The parent registry key.
     * \param subKeyName Name of the subkey to created. If <code>NULL</code> or the
     *  empty string, then the function returns a copy of the callee.
     * \param accessMask A mask of <code>KEY_*</code> flags that define the
     *  required access to the key. Defaults to KEY_ALL_ACCESS.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown indicating why the given key could not be created. If not,
     *  then the instance constructed will be empty, as denoted by a <code>NULL</code>
     *  value returned from the get_key_handle() method.
     */
    template <ss_typename_param_k H, ss_typename_param_k S>
    static class_type create_key(H const& key, S const& subKeyName, REGSAM accessMask = KEY_ALL_ACCESS)
    {
        return create_key_(get_HKEY(key), stlsoft_ns_qual(c_str_ptr)(subKeyName), accessMask);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */


    /** Deletes the named sub-key of this key
     *
     * \param subKeyName The name of the sub-key to be deleted.
     *
     * \return Indicates whether the sub-key was deleted.
     * \retval true The sub-key existed and was successfully deleted.
     * \retval false The sub-key does not exist. (<b>Note</b>:
     *  if \ref page__exception_agnostic "exception handling is not enabled",
     *  then false will also be returned for any other reason, and the
     *  reason will be available via <code>::GetLastError()</code>.)
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the sub-key exists but cannot be deleted.
     */
    bool_type   delete_sub_key(char_type const* subKeyName);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Deletes the named sub-key of this key
     *
     * \param subKeyName The name of the sub-key to be deleted.
     *
     * \return Indicates whether the sub-key was deleted.
     * \retval true The sub-key existed and was successfully deleted.
     * \retval false The sub-key does not exist. (<b>Note</b>:
     *  if \ref page__exception_agnostic "exception handling is not enabled",
     *  then false will also be returned for any other reason, and the
     *  reason will be available via <code>::GetLastError()</code>.)
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the sub-key exists but cannot be deleted.
     */
    template <ss_typename_param_k S>
    class_type  delete_sub_key(S const& subKeyName)
    {
        return delete_sub_key_(stlsoft_ns_qual(c_str_ptr)(subKeyName));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
/// @}

    /// Returns a duplicate of the key's handle, if any.
    ///
    /// \param accessMask The access mask for the
    /// \param res A pointer to a variable (of type result_type) into
    ///   which will be written the result of the underlying registry API
    ///   call.
    ///
    /// \return
    ///
    /// \note The handle returned from this method <b>must</b> be closed with RegCloseKey()
    hkey_type   dup_key_handle( REGSAM      accessMask  =   KEY_ALL_ACCESS
                            ,   result_type *res        =   NULL);

/// \name Value operations
/// @{
public:
    /** Sets the named value to the value of the given 32-bit integer.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, DWORD value);
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    /** Sets the named value to the value of the given 64-bit integer.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, ws_uint64_t value);
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
    /** Sets the named value to the value of the given string.
    ///
    /// \param valueName The name of the value.
    /// \param value The value of the value
    /// \param type The type of the value. Must be one of REG_SZ, REG_EXPAND_SZ or REG_MULTI_SZ.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, char_type const* value, ws_uint_t type = REG_SZ);
    /** Sets the named value to the values of the given string array.
    ///
    /// \param valueName The name of the value.
    /// \param values The string array.
    /// \param numValues Number of elements in the string array.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, char_type const* const* values, size_type numValues);
    /** Sets the named value to the given binary value.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, void const* value, size_type cbValue);

#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the value of the given 32-bit integer.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, DWORD value)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), value);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
# ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the value of the given 64-bit integer.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, ws_uint64_t value)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), value);
    }
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the value of the given string.
    ///
    /// \param valueName The name of the value.
    /// \param value The value of the value
    /// \param type The type of the value. Must be one of REG_SZ, REG_EXPAND_SZ or REG_MULTI_SZ.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, char_type const* value, ws_uint_t type = REG_SZ)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), value, type);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the values of the given string array.
    ///
    /// \param valueName The name of the value.
    /// \param values The string array.
    /// \param numValues Number of elements in the string array.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, char_type const* const* values, size_type numValues)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), values, numValues);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the given binary value.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, void const* value, size_type cbValue)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), value, cbValue);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    /** Sets the named value to the given integer (stored as an unsigned value).
    ///
    /// \note This method is provided solely to disambiguate between the DWORD and ws_uint64_t overloads
    /// when using integer literals.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    bool_type   set_value(char_type const* valueName, int value);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Sets the named value to the given integer (stored as an unsigned value).
    ///
    /// \note This method is provided solely to disambiguate between the DWORD and ws_uint64_t overloads
    /// when using integer literals.
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value cannot be set.
     */
    template <ss_typename_param_k S>
    bool_type   set_value(S const& valueName, int value)
    {
        return set_value_(stlsoft_ns_qual(c_str_ptr)(valueName), value);
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
    /** Deletes the named value.
     *
     * \param valueName The name of the value to be deleted.
     *
     * \return Indicates whether the value was deleted.
     * \retval true The value existed and was successfully deleted.
     * \retval false The value does not exist. (<b>Note</b>:
     *  if \ref page__exception_agnostic "exception handling is not enabled",
     *  then false will also be returned for any other reason, and the
     *  reason will be available via <code>::GetLastError()</code>.)
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value exists but cannot be deleted.
     */
    bool_type   delete_value(char_type const* valueName);
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Deletes the named value.
     *
     * \param valueName The name of the value to be deleted.
     *
     * \return Indicates whether the value was deleted.
     * \retval true The value existed and was successfully deleted.
     * \retval false The value does not exist. (<b>Note</b>:
     *  if \ref page__exception_agnostic "exception handling is not enabled",
     *  then false will also be returned for any other reason, and the
     *  reason will be available via <code>::GetLastError()</code>.)
     *
     * \exception winstl::registry_exception If \ref page__exception_agnostic "exception handling is enabled",
     *  an instance of \link winstl::registry_exception registry_exception\endlink
     *  will be thrown if the value exists but cannot be deleted.
     */
    template <ss_typename_param_k S>
    bool_type   delete_value(S const& valueName)
    {
        return this->delete_value_(stlsoft_ns_qual(c_str_ptr)(valueName));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */

#if 0
    ws_dword_t      get_value_type(char_type const* valueName) const;
    size_type       get_value_data_size(char_type const* valueName) const;
#endif /* 0 */

    /** Returns the named value.
    ///
    /// \return An instance of basic_reg_value.
     */
    key_value_type              get_value(char_type const* valueName) const;
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT
    /** Returns the named value.
    ///
    /// \return An instance of basic_reg_value.
     */
    template <ss_typename_param_k S>
    key_value_type              get_value(S const& valueName) const
    {
        return this->get_value(stlsoft_ns_qual(c_str_ptr)(stlsoft_ns_qual(c_str_ptr)(valueName)));
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT */
//  std::list<key_value_type>   get_values(char_type const* valueNames) const;
/// @}

/// \name Implementation
/// @{
private:
    static hkey_type    open_key_(  hkey_type           hkeyParent
                                ,   char_type const*    keyName
                                ,   REGSAM              accessMask);
    class_type  open_sub_key_(      char_type const*    subKeyName
                                ,   REGSAM              accessMask);
    static class_type create_key_(  HKEY                hkey
                                ,   char_type const*    subKeyName
                                ,   REGSAM              accessMask);
    class_type  create_sub_key_(    char_type const*    subKeyName
                                ,   REGSAM              accessMask);
    bool_type   delete_sub_key_(    char_type const*    subKeyName);
    static result_type  set_value_( hkey_type           hkey
                                ,   char_type const*    valueName
                                ,   ws_uint_t           type
                                ,   void const*         value
                                ,   size_type           cbValue);

    bool_type   set_value_int_(char_type const* valueName, int value, stlsoft_ns_qual(yes_type));
    bool_type   set_value_int_(char_type const* valueName, int value, stlsoft_ns_qual(no_type));

    bool_type   set_value_(char_type const* valueName, DWORD value);
#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    bool_type   set_value_(char_type const* valueName, ws_uint64_t value);
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
    bool_type   set_value_(char_type const* valueName, char_type const* value, ws_uint_t type);
    bool_type   set_value_(char_type const* valueName, char_type const* const* values, size_type numValues);
    bool_type   set_value_(char_type const* valueName, void const* value, size_type cbValue);
    bool_type   set_value_(char_type const* valueName, int value);

    bool_type   delete_value_(char_type const* valueName);

    bool_type   has_sub_key_(char_type const* subKeyName);
    bool_type   has_value_(char_type const* valueName);

    static result_type  get_value_(hkey_type hkey, char_type const* valueName, ws_uint_t type, void *value, size_type *pcbValue);

    static hkey_type    dup_key_(   hkey_type       hkey
                                ,   REGSAM          accessMask  =   KEY_ALL_ACCESS);
/// @}

/// \name Operations
/// @{
public:
    /** Efficiently swaps the contents between two instances
    ///
    /// \param rhs The parameter whose contents will be swapped.
     */
    void    swap(class_type& rhs) stlsoft_throw_0();
/// @}

/// \name Members
/// @{
private:
    string_type m_name;         // The key name
    hkey_type   m_hkey;         // The key handle
    REGSAM      m_accessMask;   // The security access mask
/// @}
};

/* Typedefs to commonly encountered types. */
/** Specialisation of the basic_reg_key template for the ANSI character type \c char
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_key<ws_char_a_t, reg_traits<ws_char_a_t>, processheap_allocator<ws_char_a_t> >    reg_key_a;
/** Specialisation of the basic_reg_key template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_key<ws_char_w_t, reg_traits<ws_char_w_t>, processheap_allocator<ws_char_w_t> >    reg_key_w;
/** Specialisation of the basic_reg_key template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__windows_registry
 */
typedef basic_reg_key<TCHAR, reg_traits<TCHAR>, processheap_allocator<TCHAR> >                      reg_key;

/* /////////////////////////////////////////////////////////////////////////
 * Handle access shims
 */

/* get_handle */

/** [\ref group__concept__shims "Shim" function] Returns the corresponding registry handle of an instance of winstl::basic_reg_key basic_reg_key.
 *
 * \ingroup group__library__windows_registry
 *
 * \param key The \link winstl::basic_reg_key basic_reg_key\endlink instance.
 *
 * \return The HKEY handle of the given \link winstl::basic_reg_key basic_reg_key\endlink instance.
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline HKEY get_handle(basic_reg_key<C, T, A> const& key)
{
    return key.get_key_handle();
}

/** [\ref group__concept__shims "Shim" function] Returns the corresponding registry handle of an instance of winstl::basic_reg_key basic_reg_key.
 *
 * \ingroup group__library__shims__hkey_attribute
 *
 * This access <a href = "http://stlsoft.org/white_papers.html#shims">shim</a>
 * retrieves the HKEY registry handle for the given HKEY handle.
 *
 * \param key The winstl::basic_reg_key basic_reg_key instance whose corresponding HKEY will be retrieved
 *
 * \return The HKEY handle of the given winstl::basic_reg_key basic_reg_key instance
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline HKEY get_HKEY(basic_reg_key<C, T, A> const& key)
{
    return key.get_key_handle();
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/reg_key_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
#if (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1100) || \
    defined(STLSOFT_COMPILER_IS_VECTORC)
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::open_key_(hkey_type hkeyParent, char_type const* keyName, REGSAM accessMask)
#else /* ? compiler */
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::open_key_(ss_typename_param_k basic_reg_key<C, T, A>::hkey_type hkeyParent, ss_typename_param_k basic_reg_key<C, T, A>::char_type const* keyName, REGSAM accessMask)
#endif /* compiler */
{
    hkey_type   hkey;
    result_type res = traits_type::reg_open_key(hkeyParent, keyName, &hkey, accessMask);

    if(ERROR_SUCCESS != res)
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
        ::SetLastError(res);
        hkey = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return hkey;
}

template<   ss_typename_param_k C
        ,   ss_typename_param_k T
        ,   ss_typename_param_k A
        >
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::dup_key_(  ss_typename_type_k basic_reg_key<C, T, A>::hkey_type    hkey
                                                                                                            ,   REGSAM                                                  accessMask /* = KEY_ALL_ACCESS */)
{
    if(NULL == hkey)
    {
        return NULL;
    }
    else
    {
        result_type res;
        HKEY        hkeyDup = traits_type::key_dup(hkey, accessMask, &res);

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
            ::SetLastError(res);
            hkeyDup = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return hkeyDup;
    }
}

// Construction
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_key<C, T, A>::basic_reg_key()
    : m_name()
    , m_hkey(NULL)
    , m_accessMask(KEY_READ)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_key<C, T, A>::basic_reg_key(ss_typename_type_k basic_reg_key<C, T, A>::hkey_type* hkey, ss_typename_type_k basic_reg_key<C, T, A>::string_type const& keyName, REGSAM accessMask)
    : m_name(keyName)
    , m_hkey(*hkey)
    , m_accessMask(accessMask)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_key<C, T, A>::basic_reg_key(class_type const& rhs)
    : m_name(rhs.m_name)
    , m_hkey(dup_key_(rhs.m_hkey, rhs.get_access_mask()))
    , m_accessMask(rhs.m_accessMask)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_key<C, T, A>::basic_reg_key(class_type const& rhs, REGSAM accessMask)
    : m_name(rhs.m_name)
    , m_hkey(dup_key_(rhs.m_hkey, accessMask))
    , m_accessMask(accessMask)
{}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_key<C, T, A>::~basic_reg_key() stlsoft_throw_0()
{
    if(m_hkey != NULL)
    {
        ::RegCloseKey(m_hkey);
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline void basic_reg_key<C, T, A>::swap(ss_typename_type_k basic_reg_key<C, T, A>::class_type& rhs) stlsoft_throw_0()
{
    std_swap(m_name,        rhs.m_name);
    std_swap(m_hkey,        rhs.m_hkey);
    std_swap(m_accessMask,  rhs.m_accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type& basic_reg_key<C, T, A>::operator =(ss_typename_type_k basic_reg_key<C, T, A>::class_type const& rhs)
{
    class_type t(rhs);

    swap(t);

    return *this;
}

// Attributes
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::string_type const& basic_reg_key<C, T, A>::name() const
{
    return m_name;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::string_type basic_reg_key<C, T, A>::reg_class() const
{
    size_type   cch_key_class   =   0;
    ws_long_t   res             =   traits_type::reg_query_info(m_hkey, NULL, &cch_key_class, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
query_fail:

        static const char message[] = "could not determine the key registry class";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        stlsoft_ns_qual(auto_buffer_old)<char_type, allocator_type, CCH_REG_API_AUTO_BUFFER>  p(++cch_key_class);

        res = traits_type::reg_query_info(m_hkey, &p[0], &cch_key_class, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        if(ERROR_SUCCESS != res)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            goto query_fail;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            return string_type(&p[0], cch_key_class);
        }
    }

    return string_type();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::size_type basic_reg_key<C, T, A>::num_sub_keys() const
{
    ws_uint32_t c_sub_keys;
    ws_long_t   res         =   traits_type::reg_query_info(m_hkey, NULL, NULL, &c_sub_keys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not determine the number of sub-keys";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        c_sub_keys = 0;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return c_sub_keys;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::size_type basic_reg_key<C, T, A>::num_values() const
{
    ws_uint32_t c_values;
    ws_long_t   res         =   traits_type::reg_query_info(m_hkey, NULL, NULL, NULL, NULL, NULL, &c_values, NULL, NULL, NULL, NULL);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not determine the number of values";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        c_values = 0;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return c_values;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::get_key_handle() const
{
    return m_hkey;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::get() const
{
    return get_key_handle();
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline REGSAM basic_reg_key<C, T, A>::get_access_mask() const
{
    return m_accessMask;
}

// Operations
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::open_sub_key(char_type const* subKeyName, REGSAM accessMask /* = KEY_ALL_ACCESS */)
{
    return this->open_sub_key_(subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::create_sub_key(char_type const* subKeyName, REGSAM accessMask /* = KEY_ALL_ACCESS */)
{
    return this->create_sub_key_(subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::create_key(HKEY hkey, char_type const* subKeyName, REGSAM accessMask /* = KEY_ALL_ACCESS */)
{
    return create_key_(hkey, subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::open_sub_key_(char_type const* subKeyName, REGSAM accessMask /* = KEY_ALL_ACCESS */)
{
    return class_type(m_hkey, subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::create_key_(
    HKEY                hkey
,   char_type const*    subKeyName
,   REGSAM              accessMask
)
{
    static const char_type  s_emptyString[] = { '\0' };

    return class_type(hkey, s_emptyString, KEY_CREATE_SUB_KEY).create_sub_key(subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::class_type basic_reg_key<C, T, A>::create_sub_key_(
    char_type const*    subKeyName
,   REGSAM              accessMask
)
{
    hkey_type   hkey;
    result_type res =   traits_type::reg_create_key(m_hkey, subKeyName, &hkey, accessMask);

    if(ERROR_SUCCESS != res)
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        static const char message[] = "could not create sub-key";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ::SetLastError(res);
        return class_type();
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return class_type(&hkey, subKeyName, accessMask);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::delete_sub_key(char_type const* subKeyName)
{
    return this->delete_sub_key_(subKeyName);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::delete_sub_key_(char_type const* subKeyName)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    static const char message[] = "could not delete sub-key";
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    result_type res = traits_type::reg_delete_key(m_hkey, subKeyName);

    switch(res)
    {
        case    ERROR_SUCCESS:
            return true;
        default:
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            if(ERROR_ACCESS_DENIED == res)
            {
                STLSOFT_THROW_X(access_denied_exception(message, res));
            }
            else
            {
                STLSOFT_THROW_X(registry_exception(message, res));
            }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            ::SetLastError(res);
            // Fall through
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        case    ERROR_FILE_NOT_FOUND:
            return false;
    }
}


/* The handle returned from this method MUST be closed with RegCloseKey() */
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::hkey_type basic_reg_key<C, T, A>::dup_key_handle(REGSAM accessMask /* = KEY_ALL_ACCESS */, result_type *res /* = NULL */)
{
    return traits_type::key_dup(m_hkey, accessMask, res);
}

// Values

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, DWORD value)
{
    return set_value_(valueName, value);
}

#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, ws_uint64_t value)
{
    return set_value_(valueName, value);
}
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, int value)
{
    return set_value_(valueName, value);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, char_type const* value, ws_uint_t type /* = REG_SZ */)
{
    return set_value_(valueName, value, type);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, ss_typename_type_k basic_reg_key<C, T, A>::char_type const* const* values, ss_typename_type_k basic_reg_key<C, T, A>::size_type numValues)
{
    return set_value_(valueName, values, numValues);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, void const* value, size_type cbValue)
{
    return set_value_(valueName, value, cbValue);
}



template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline /* static */ ss_typename_type_ret_k basic_reg_key<C, T, A>::result_type basic_reg_key<C, T, A>::set_value_(  ss_typename_type_k basic_reg_key<C, T, A>::hkey_type        hkey
                                                                                                                ,   ss_typename_type_k basic_reg_key<C, T, A>::char_type const  *valueName
                                                                                                                ,   ws_uint_t                                                   type
                                                                                                                ,   void const                                                  *value
                                                                                                                ,   ss_typename_type_k basic_reg_key<C, T, A>::size_type        cbValue)
{
    result_type res = traits_type::reg_set_value(hkey, valueName, type, value, cbValue);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    if(ERROR_SUCCESS != res)
    {
        static const char message[] = "could not create value";

        if(ERROR_ACCESS_DENIED == res)
        {
            STLSOFT_THROW_X(access_denied_exception(message, res));
        }
        else
        {
            STLSOFT_THROW_X(registry_exception(message, res));
        }
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    return res;
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, DWORD value)
{
    return ERROR_SUCCESS == class_type::set_value_(m_hkey, valueName, REG_DWORD, &value, sizeof(value));
}

#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, ws_uint64_t value)
{
#ifndef REG_QWORD
    const DWORD REG_QWORD   =   11;
#endif /* !REG_QWORD */

    return ERROR_SUCCESS == class_type::set_value_(m_hkey, valueName, REG_QWORD, &value, sizeof(value));
}
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, char_type const* value, ws_uint_t type /* = REG_SZ */)
{
    WINSTL_ASSERT(REG_SZ == type || REG_EXPAND_SZ == type || REG_MULTI_SZ == type);

    return ERROR_SUCCESS == class_type::set_value_(m_hkey, valueName, type, value, traits_type::str_len(value) * sizeof(char_type));
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, ss_typename_type_k basic_reg_key<C, T, A>::char_type const* const* values, ss_typename_type_k basic_reg_key<C, T, A>::size_type numValues)
{
    // Evaluate the total length of the source values
    const size_type totalLen = winstl_ns_qual_std(accumulate)(  stlsoft_ns_qual(transformer)(values, std::ptr_fun(traits_type::str_len))
                                                            ,   stlsoft_ns_qual(transformer)(values + numValues, std::ptr_fun(traits_type::str_len))
                                                            ,   size_type(0));

    // Create a buffer of sufficient size: total length + a nul-terminator for each value + a double nul-terminator
    stlsoft_ns_qual(auto_buffer)<char_type> buff(totalLen + numValues * 1 + 2);

    if(buff.empty())
    {
        ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return false;
    }

    // Now synthesise all the data
    char_type *p = &buff[0];

    { for(size_type i = 0; i != numValues; ++i)
    {
        char_type const* const  s   =   values[i];
        const size_type         len =   traits_type::str_len(s);

        ::memcpy(p, s, sizeof(char_type) * len);
        p += len;
        *p++ = '\0';
    }}
    *p++ = '\0';
    *p++ = '\0';

    return ERROR_SUCCESS == class_type::set_value_(m_hkey, valueName, REG_MULTI_SZ, buff.data(), static_cast<size_type>(p - &buff[0]) * sizeof(char_type));
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, void const* value, size_type cbValue)
{
    return ERROR_SUCCESS == class_type::set_value_(m_hkey, valueName, REG_BINARY, value, cbValue);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_int_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, int value, stlsoft_ns_qual(yes_type))
{
    return this->set_value(valueName, static_cast<DWORD>(value));
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_int_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, int value, stlsoft_ns_qual(no_type))
{
    return this->set_value(valueName, static_cast<ws_uint64_t>(value));
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::set_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName, int value)
{
    // Because Borland is quite dense, we must use two typedefs here,
    // rather than just one

#if 0
    typedef ss_typename_type_k stlsoft_ns_qual(value_to_yesno_type)<sizeof(int) <= sizeof(DWORD)>::type yesno_t;
#else /* ? 0 */
    typedef stlsoft_ns_qual(value_to_yesno_type)<sizeof(int) <= sizeof(DWORD)>  value_to_yesno_t;
    typedef ss_typename_type_k value_to_yesno_t::type                           yesno_t;
#endif /* 0 */

    return ERROR_SUCCESS == set_value_int_(valueName, value, yesno_t());
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::delete_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName)
{
    return this->delete_value_(valueName);
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::delete_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    static const char message[] = "could not delete value";
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

    result_type res = traits_type::reg_delete_value(m_hkey, valueName);

    switch(res)
    {
        case    ERROR_SUCCESS:
            return true;
        default:
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            if(ERROR_ACCESS_DENIED == res)
            {
                STLSOFT_THROW_X(access_denied_exception(message, res));
            }
            else
            {
                STLSOFT_THROW_X(registry_exception(message, res));
            }
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            ::SetLastError(res);
            // Fall through
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        case    ERROR_FILE_NOT_FOUND:
            return false;
    }
}


template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::has_sub_key_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* subKeyName)
{
    hkey_type   hkey;
    result_type res = traits_type::reg_open_key(m_hkey, subKeyName, &hkey, KEY_READ);

    switch(res)
    {
        case    ERROR_SUCCESS:
            ::RegCloseKey(hkey);
        case    ERROR_ACCESS_DENIED:
            return true;
        default:
            return false;
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline ss_typename_type_ret_k basic_reg_key<C, T, A>::bool_type basic_reg_key<C, T, A>::has_value_(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName)
{
    ws_dword_t  valueType;
    ss_byte_t   data[1];
    size_type   cbData = sizeof(data);
    result_type res = traits_type::reg_query_value(m_hkey, valueName, valueType, &data[0], cbData);

    switch(res)
    {
        case    ERROR_SUCCESS:
        case    ERROR_MORE_DATA:
            return true;
        default:
            return false;
    }
}

template <ss_typename_param_k C, ss_typename_param_k T, ss_typename_param_k A>
inline basic_reg_value<C, T, A> basic_reg_key<C, T, A>::get_value(ss_typename_type_k basic_reg_key<C, T, A>::char_type const* valueName) const
{
    return basic_reg_value<C, T, A>(m_hkey, valueName);
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

#if defined(STLSOFT_CF_std_NAMESPACE)

namespace std
{
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
    inline void swap(winstl_ns_qual(reg_key_a)& lhs, winstl_ns_qual(reg_key_a)& rhs)
    {
        lhs.swap(rhs);
    }
    inline void swap(winstl_ns_qual(reg_key_w)& lhs, winstl_ns_qual(reg_key_w)& rhs)
    {
        lhs.swap(rhs);
    }
#endif /* compiler */

} // anonymous namespace

#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_REGISTRY_HPP_REG_KEY */

/* ///////////////////////////// end of file //////////////////////////// */
