/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/memory/auto_buffer.hpp (originally MTLocBfr.h, ::SynesisStl)
 *
 * Purpose:     Contains the auto_buffer template class.
 *
 * Created:     19th January 2002
 * Updated:     21st June 2010
 *
 * Thanks:      To Thorsten Ottosen for pointing out that allocators were
 *              not swapped.
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


/** \file stlsoft/memory/auto_buffer.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::auto_buffer class template
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
#define STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_AUTO_BUFFER_MAJOR       5
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_AUTO_BUFFER_MINOR       2
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_AUTO_BUFFER_REVISION    4
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_AUTO_BUFFER_EDIT        163
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:     __GNUC__ < 3
[Incompatibilies-end]
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES
# include <stlsoft/memory/allocator_features.hpp>   // for STLSOFT_LF_ALLOCATOR_ALLOCATE_HAS_HINT
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_ALGORITHMS_HPP_POD
# include <stlsoft/algorithms/pod.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_HPP_ALGORITHMS_POD */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifdef _STLSOFT_AUTO_BUFFER_ALLOW_UDT
# define _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD
# ifdef STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT
#  pragma message("_STLSOFT_AUTO_BUFFER_ALLOW_UDT is deprecated. Use _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD instead")
# endif /* STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT */
#endif /* _STLSOFT_AUTO_BUFFER_ALLOW_UDT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD
# ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
#  include <stlsoft/util/constraints.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#endif /* _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
# if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
#  include <numeric>
# endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# if !defined(STLSOFT_COMPILER_IS_BORLAND) && \
     !defined(STLSOFT_COMPILER_IS_DMC)

struct auto_buffer_internal_default
{
    enum { min_value        =   32      };
    enum { max_value        =   256     };
    enum { division_factor  =   2       };
};

template <ss_typename_param_k T>
struct auto_buffer_internal_size_calculator
    : private auto_buffer_internal_default
{
private:
    // Stupid, stupid, stupid GCC requires them all to share the same
    // enum, which totally sucks. It whinges about comparisons between
    // enumerals (sic.) of different types. Thankfully it's irrelevant
    // because they're private
    enum
    {
            min_value        =   auto_buffer_internal_default::min_value
        ,   max_value        =   auto_buffer_internal_default::max_value
        ,   division_factor  =   auto_buffer_internal_default::division_factor
        ,   divided_value_   =   static_cast<int>((division_factor * max_value) / sizeof(T))
        ,   divided_value    =   (max_value < divided_value_)
                                    ?   max_value
                                    :   divided_value_
    };
public:
    enum { value            =   1 == sizeof(T)
                                    ?   max_value
                                    :   divided_value < min_value
                                        ?   min_value
                                        :   divided_value           };
};

STLSOFT_TEMPLATE_SPECIALISATION
struct auto_buffer_internal_size_calculator<ss_char_a_t>
{
    enum { value    =   auto_buffer_internal_default::max_value     };
};
#  if defined(STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT) || \
      defined(STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT)
STLSOFT_TEMPLATE_SPECIALISATION
struct auto_buffer_internal_size_calculator<ss_char_w_t>
{
    enum { value    =   auto_buffer_internal_default::max_value     };
};
#  endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

# endif /* compiler */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */



// class auto_buffer
//
/** \brief This class provides an efficient variable automatic buffer
 *
 * \ingroup group__library__memory
 *
 * \param T The type of the elements in the array
 * \param SPACE The number of elements in the array. For translators that
 *   support default template arguments, this is defaulted to <b>256</b>
 * \param A The allocator type. Defaults to
 *   \link stlsoft::allocator_selector allocator_selector<T>::allocator_type\endlink
 *   for translators that support default template arguments.
 *
 * This class provides an efficient replacement for dynamic memory block
 * allocation when the block size generally falls under a predictable limit. In
 * such cases, significant performance benefits can be achieved by using an
 * instance of a parameterisation of auto_buffer, whose size parameter SPACE
 * is set to a level to cater for most of the requested sizes. Only where
 * the size of the buffer needs to be larger than this limit does an
 * allocation occur from the heap/free-store via the given allocator.
 *
 * Using <code>auto_buffer</code> means one can avoid use of heap memory in
 * circumstances where stack memory is unsuitable, i.e. where there is no
 * maximum size to a memory requirement, or the maximum size is potentially
 * very large (and considerably larger than the median size). Consider the
 * following code extract from the core of the
 * <a href = "http://pantheios.org/">Pantheios</a> logging library:
\code
  int pantheios_log_n(  pan_sev_t           severity
                     ,  size_t              numSlices
                     ,  pan_slice_t const*  slices)
  {
    typedef stlsoft::auto_buffer<char, 2048>  buffer_t;

    // Calculate the total size of the log statement, by summation of the slice array
    const size_t  n = std::accumulate(stlsoft::member_selector(slices, &pan_slice_t::len)
                                    , stlsoft::member_selector(slices + numSlices, &pan_slice_t::len)
                                    , size_t(0));
    buffer_t      buffer(1 + n);

    . . .
\endcode
 *
 * This use of auto_buffer illustrates two important features:
 * - there is no (compile-time) limit on the maximum size of a log statement
 * - memory is only allocated from the heap in the case where the total statement length >= 2047 bytes.
 *
 * Without auto_buffer, we would have three choices, all bad:
 *
 * 1. We could go to the heap in all cases:
\code
  int pantheios_log_n(  pan_sev_t           severity
                     ,  size_t              numSlices
                     ,  pan_slice_t const*  slices)
  {
    typedef stlsoft::vector<char>   buffer_t;

    // Calculate the total size of the log statement, by summation of the slice array
    const size_t  n = std::accumulate(stlsoft::member_selector(slices, &pan_slice_t::len)
                                    , stlsoft::member_selector(slices + numSlices, &pan_slice_t::len)
                                    , size_t(0));
    buffer_t      buffer(1 + n);

    . . .
\endcode
 * But this would have an unacceptable performance hit (since the vast
 * majority of log statements are less than 2K in extent).
 *
 * 2. We could use a stack buffer, and truncate any log statement exceeding
 *     the limit:
\code
  int pantheios_log_n(  pan_sev_t           severity
                     ,  size_t              numSlices
                     ,  pan_slice_t const*  slices)
  {
    // Calculate the total size of the log statement, by summation of the slice array
    const size_t  n = std::accumulate(stlsoft::member_selector(slices, &pan_slice_t::len)
                                    , stlsoft::member_selector(slices + numSlices, &pan_slice_t::len)
                                    , size_t(0));
    char          buffer[2048];

    . . . // make sure to truncate the statement to a max 2047 characters

\endcode
 * But this would unnecessarily constrain users of the Pantheios logging
 * functionality.
 *
 * 3. Finally, we could synthesise the functionality of auto_buffer
 *     manually, as in:
\code
  int pantheios_log_n(  pan_sev_t           severity
                     ,  size_t              numSlices
                     ,  pan_slice_t const*  slices)
  {
    // Calculate the total size of the log statement, by summation of the slice array
    const size_t  n = std::accumulate(stlsoft::member_selector(slices, &pan_slice_t::len)
                                    , stlsoft::member_selector(slices + numSlices, &pan_slice_t::len)
                                    , size_t(0));
    char    buff[2048];
    char    *buffer = (n < 2048) ? &buff[0] : new char[1 + n];

    . . .

    if(buffer != &buff[0])
    {
      delete [] buffer;
    }
\endcode
 * But this is onerous manual fiddling, and exception-unsafe. What would be
 * the point, when auto_buffer already does this (safely) for us?
 *
 * As a consequence of its blending of the best features of stack and heap
 * memory, auto_buffer is an invaluable component in the implementation of
 * many components within the STLSoft libraries, and in several other
 * open-source projects, including:
 * <a href = "http://synesis.com.au/software/b64.html">b64</a>,
 * <a href = "http://openrj.org/">Open-RJ</a>,
 * <a href = "http://pantheios.org/">Pantheios</a>,
 * <a href = "http://recls.org/">recls</a>,
 * and
 * <a href = "http://shwild.org/">shwild</a>.
 *
 * \remarks auto_buffer works correctly whether the given allocator throws an
 *   exception on allocation failure, or returns <code>NULL</code>. In the
 *   latter case, construction failure to allocate is reflected by the size()
 *   method returning 0.
 *
 * \remarks The design of auto_buffer is described in Chapter 32 of
 *   <a href = "http://imperfectcplusplus.com">Imperfect C++</a>, and its
 *   interface is discussed in detail in Section 16.2 of
 *   <a href = "http://extendedstl.com">Extended STL, volume 1</a>.
 *
 * \note With version 1.9 of STLSoft, the order of the space and allocator
 *   arguments were reversed. Further, the allocator default changed from
 *   stlsoft::new_allocator to <code>std::allocator</code> for translators that support
 *   the standard library. If you need the old characteristics, you can
 *   <code>\#define</code> the symbol <b>STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS</b>.
 */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# define STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
#endif /* compiler */


#if defined(STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS)

# ifdef STLSOFT_AUTO_BUFFER_NEW_FORM
#  undef STLSOFT_AUTO_BUFFER_NEW_FORM
# endif /* STLSOFT_AUTO_BUFFER_NEW_FORM */

 // //////////////////////////////////////////////
 // This is the pre-1.9 template parameter list

template<   ss_typename_param_k T
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
#  if defined(STLSOFT_COMPILER_IS_BORLAND)
        ,   ss_size_t           space   =   256
#  elif defined(STLSOFT_COMPILER_IS_DMC)
        ,   ss_size_t           SPACE   =   256
#  else /* ? compiler */
        ,   ss_size_t           SPACE   =   auto_buffer_internal_size_calculator<T>::value
#  endif /* compiler */
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
#  if !defined(STLSOFT_COMPILER_IS_BORLAND)
        ,   ss_size_t           SPACE   /* =   auto_buffer_internal_size_calculator<T>::value */
#  else /* ? compiler */
        ,   ss_size_t           space   /* =   256 */
#  endif /* compiler */
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        >

 // End of pre-1.9 template parameter list
 // //////////////////////////////////////////////

#else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */

 // //////////////////////////////////////////////
 // This is the 1.9+ template parameter list

# ifndef STLSOFT_AUTO_BUFFER_NEW_FORM
#  define STLSOFT_AUTO_BUFFER_NEW_FORM
# endif /* !STLSOFT_AUTO_BUFFER_NEW_FORM */

template<   ss_typename_param_k T
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
#  if defined(STLSOFT_COMPILER_IS_BORLAND)
        ,   ss_size_t           space   =   256
#  elif defined(STLSOFT_COMPILER_IS_DMC)
        ,   ss_size_t           SPACE   =   256
#  else /* ? compiler */
        ,   ss_size_t           SPACE   =   auto_buffer_internal_size_calculator<T>::value
#  endif /* compiler */
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
#  if !defined(STLSOFT_COMPILER_IS_BORLAND)
        ,   ss_size_t           SPACE   /* =   auto_buffer_internal_size_calculator<T>::value */
#  else /* ? compiler */
        ,   ss_size_t           space   /* =   256 */
#  endif /* compiler */
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >

 // End of 1.9+ template parameter list
 // //////////////////////////////////////////////

#endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */

class auto_buffer
#if !defined(STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE)
    : protected A
    , public stl_collection_tag
#else /* ? STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
    : public stl_collection_tag
#endif /* !STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
{
/// \name Member Types
/// @{
public:
    /// The value type
    typedef T                                                   value_type;
    /// The allocator type
    typedef A                                                   allocator_type;
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
    enum
    {
        /// The number of items in the internal buffer
        space = int(SPACE) // int() required for 64-bit compatibility
    };
#endif /* compiler */
    /// The type of the current parameterisation
#ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
    typedef auto_buffer<T, A, space>                            class_type;
#else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    typedef auto_buffer<T, space, A>                            class_type;
#endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    /// The reference type
    typedef ss_typename_type_k allocator_type::reference        reference;
    /// The non-mutable (const) reference type
    typedef ss_typename_type_k allocator_type::const_reference  const_reference;
    /// The pointer type
    typedef ss_typename_type_k allocator_type::pointer          pointer;
    /// The non-mutable (const) pointer type
    typedef ss_typename_type_k allocator_type::const_pointer    const_pointer;
    /// The size type
    typedef ss_size_t                                           size_type;
    /// The difference type
    typedef ss_ptrdiff_t                                        difference_type;
#if !defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The iterator type
    typedef value_type*                                         iterator;
    /// The non-mutable (const) iterator type
    typedef value_type const*                                   const_iterator;
#else /* ? !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    /// The non-mutating (const) iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

    /// The mutating (non-const) reverse iterator type
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    /// The non-mutating (const) reverse iterator type
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Implementation
/// @{
private:
    pointer allocate_(size_type cItems, void const* hint)
    {
#ifdef STLSOFT_LF_ALLOCATOR_ALLOCATE_HAS_HINT
# ifdef STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW
        return static_cast<pointer>(get_allocator().allocate(cItems, const_cast<void*>(hint)));
# else /* ? STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW */
        return get_allocator().allocate(cItems, hint);
# endif /* STLSOFT_CF_STD_LIBRARY_IS_SUNPRO_RW */
#else /* ? STLSOFT_LF_ALLOCATOR_ALLOCATE_HAS_HINT */
        STLSOFT_SUPPRESS_UNUSED(hint);

        return get_allocator().allocate(cItems);
#endif /* STLSOFT_LF_ALLOCATOR_ALLOCATE_HAS_HINT */
    }

    pointer allocate_(size_type cItems)
    {
        return allocate_(cItems, NULL);
    }

    void deallocate_(pointer p, size_type cItems)
    {
        STLSOFT_ASSERT(NULL != p);

#ifdef STLSOFT_LF_ALLOCATOR_DEALLOCATE_HAS_COUNT
        get_allocator().deallocate(p, cItems);
#else /* ? STLSOFT_LF_ALLOCATOR_DEALLOCATE_HAS_COUNT */
        STLSOFT_SUPPRESS_UNUSED(cItems);

        get_allocator().deallocate(p);
#endif /* STLSOFT_LF_ALLOCATOR_DEALLOCATE_HAS_COUNT */
    }

    pointer reallocate_(pointer p, size_type cItems, size_type cNewItems)
    {
        pointer new_p = allocate_(cNewItems, p);

        // This test is needed, since some allocators may not throw
        // bad_alloc
        if(NULL != new_p)
        {
            block_copy(new_p, p, cItems);

            deallocate_(p, cItems);
        }

        return new_p;
    }
protected:
    static void block_copy(pointer dest, const_pointer src, size_type cItems)
    {
        pod_copy_n(dest, src, cItems);
    }
    static void block_set(pointer dest, size_type cItems, const_reference value)
    {
        pod_fill_n(dest, cItems, value);
    }
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an auto_buffer with the given number of elements
    ///
    /// Constructs an auto_buffer with the given number of elements. If the
    /// allocation fails by throwing an exception, that exception is passed
    /// through to the caller. If allocation fails by returning a null
    /// pointer the auto_buffer instance is correctly constructed, and the
    /// \link #size size() \endlink method returns 0.
    ///
    /// \see \link #size size() \endlink
    /// \param cItems The number of items in the constructed instance
    ss_explicit_k auto_buffer(size_type cItems)
        : m_buffer((space < cItems) ? allocate_(cItems) : const_cast<pointer>(&m_internal[0]))
        , m_cItems((NULL != m_buffer) ? cItems : 0)
        , m_bExternal(space < cItems)
    {
        // Can't create one with an empty buffer. Though such is not legal
        // it is supported by some compilers, so we must ensure it cannot be
        // so
        STLSOFT_STATIC_ASSERT(0 != space);

        // These assertions ensure that the member ordering is not
        // changed, invalidating the initialisation logic of m_buffer and
        // m_cItems. The runtime assert is included for those compilers that
        // do not implement compile-time asserts.
#ifdef STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT
        STLSOFT_STATIC_ASSERT(STLSOFT_RAW_OFFSETOF(class_type, m_buffer) < STLSOFT_RAW_OFFSETOF(class_type, m_cItems));
#endif /* STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT */
        STLSOFT_MESSAGE_ASSERT("m_buffer must be before m_cItems in the auto_buffer definition", stlsoft_reinterpret_cast(ss_byte_t*, &m_buffer) < stlsoft_reinterpret_cast(ss_byte_t*, &m_cItems));

#ifndef _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD
        // Use the must_be_pod constraint to ensure that
        // no type is managed in auto_buffer which would result in
        // dangerous mismanagement of the lifetime of its instances.
        //
        // Preprocessor specification of _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD
        // prevents this, but the natural rules of the language will
        // still prevent non POD types being placed in m_internal[].
        stlsoft_constraint_must_be_pod(value_type);
#endif /* _STLSOFT_AUTO_BUFFER_ALLOW_NON_POD */

        STLSOFT_ASSERT(is_valid());
    }
    /// \brief Releases the allocated element array
    ///
    /// Releases any allocated memory. If the internal memory buffer was
    /// used, then nothing is done, otherwise the allocated memory is
    /// returned to the allocator.
#if defined(STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT)
    ~auto_buffer()
#else /* ? STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT */
    ~auto_buffer() stlsoft_throw_0()
#endif /* STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT */
    {
        STLSOFT_ASSERT(is_valid());

        if(is_in_external_array_())
        {
            STLSOFT_ASSERT(NULL != m_buffer);
            STLSOFT_ASSERT(m_bExternal);
            STLSOFT_ASSERT(&m_internal[0] != m_buffer);

            deallocate_(m_buffer, m_cItems);
        }
    }
/// @}

/// \name Operations
/// @{
private:
    // Policy functions
    ss_bool_t   is_in_external_array_() const
    {
#if defined(STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK)
        // Old implementation always uses internal array if size() <= internal_size()
        STLSOFT_ASSERT((space < m_cItems) == (m_buffer != &m_internal[0]));

        return space < m_cItems;
#else /* ? STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */
        // Old implementation always uses internal array if size() <= internal_size()
//        STLSOFT_ASSERT((m_buffer != &m_internal[0]) || !(space < m_cItems));
        STLSOFT_ASSERT((m_buffer != &m_internal[0]) == m_bExternal);
        STLSOFT_ASSERT(m_bExternal || !(space < m_cItems));

//        return m_buffer != &m_internal[0];
        return m_bExternal;
#endif /* STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */
    }

public:
    /// \brief Expands or contracts the number of items in the buffer
    ///
    /// \param cItems The number of items to change in the buffer. If 0, the
    ///   external array (if allocated) will be deallocated.
    /// \return Returns \c true if successful. Function failure occurs when
    ///   sufficient storage for the requested items cannot be allocated. In
    ///   that case, std::bad_alloc will be throw for allocators that
    ///   support it, otherwise the function will return \c false. In either
    ///   case, the original storage and contents of the buffer will remain
    ///   unchanged.
    ///
    /// \note When reducing the number of elements, the implementation
    ///   favours speed above memory consumption. If the new item size is
    ///   still larger than the internal storage size (\c internal_size())
    ///   then the heap allocated block will not be changed (i.e. it will
    ///   not be exchanged for a smaller block).
    ///
    /// \note As from STLSoft version 1.9, the external array is not
    ///   discarded in favour of the internal array when
    ///   <code>0 < cItems < internal_size()</code>.
    ///   Only <code>resize(0)</code> will deallocate the external array.
    ss_bool_t resize(size_type cItems)
    {
        STLSOFT_ASSERT(is_valid());

        // There are four changes possible:
        //
        // 1. Expansion within the internal buffer
        // 2. Contraction within the internal buffer
        // 3. Expansion from the internal buffer to an allocated buffer
        // 4. Contraction from an allocated buffer to the internal buffer
        //  4.a Where n is 0, or when STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK is defined
        //  4.b Where 0 < n <= internal_size() - this is new behaviour - we do not go to the internal array
        // 5. Expansion from the allocated buffer to another allocated buffer
        // 6. Contraction from the allocated buffer to another allocated buffer

        if(m_cItems < cItems)
        {
            // Expansion; cases 1, 3 & 5

            if(is_in_external_array_())
            {
                // Current buffer is allocated: case 5
                pointer new_buffer  =   reallocate_(m_buffer, m_cItems, cItems);

                // Still test for NULL here, since some allocators will
                // not throw bad_alloc.
                if(NULL == new_buffer)
                {
                    return false;
                }

                // Now repoint to the new buffer
                m_buffer = new_buffer;
            }
            else
            {
                // Expanding from internal buffer; cases 1 & 3

                if(space < cItems)
                {
                    // Expanding to allocated buffer; case 3

                    pointer new_buffer = allocate_(cItems);

                    // Still test for NULL here, since some allocators will
                    // not throw bad_alloc.
                    if(NULL == new_buffer)
                    {
                        return false;
                    }

                    block_copy(new_buffer, m_buffer, m_cItems);

                    m_buffer = new_buffer;

                    m_bExternal = true;
                }
                else
                {
                    // Expanding to internal buffer; case 1

                    // Nothing to do
                    STLSOFT_ASSERT(!(space < cItems));
                }
            }
        }
        else
        {
            // Contraction; cases 2, 4 & 6

            if(is_in_external_array_())
            {
                // Current buffer is allocated: cases 4 & 6

                if(space < cItems)
                {
                    // Contracting within allocated buffer; case 6

                    // Nothing to do
                    STLSOFT_ASSERT(space < cItems);
                }
#if defined(STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK)
                else
#else /* ? STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */
                else if(0 == cItems)
#endif /* STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */
                {
                    // Contracting back to internal; case 4

                    block_copy(const_cast<pointer>(&m_internal[0]), m_buffer, cItems);

                    deallocate_(m_buffer, m_cItems);

                    m_buffer = const_cast<pointer>(&m_internal[0]);

                    m_bExternal = false;
                }
            }
            else
            {
                // Current buffer is internal; case 2

                // Nothing to do
                STLSOFT_ASSERT(!(space < cItems));
            }
        }

        m_cItems = cItems;

        STLSOFT_ASSERT(is_valid());

        return true;
    }

    /// \brief Swaps contents with the given buffer
    ///
    /// \note This method is only constant time when the memory for two buffers
    /// has been acquired via the allocator. Otherwise, it will depend on the
    /// costs of exchanging the memory
    ///
    /// \note Exception-safety: Provides the no-throw guarantee
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        STLSOFT_ASSERT(is_valid());

        // Swap:
        //
        // 1. Allocator
        // 2. Member variables

        // 1. Allocator
#if !defined(STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE)
        std_swap(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(rhs));
#endif /* !STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */

        // 2. Member variables
        if( is_in_external_array_() &&
            rhs.is_in_external_array_())
        {
            // Both are allocated, so just swap them
            std_swap(m_buffer, rhs.m_buffer);
        }
        else if(is_in_external_array_())
        {
            // *this is allocated on the heap, rhs is using m_internal

            // 1. Copy the contents of rhs.m_internal to this->m_internal
            block_copy(&m_internal[0], &rhs.m_internal[0], rhs.m_cItems);

            // 2. Move m_buffer from *this to rhs
            rhs.m_buffer = m_buffer;

            // 3. Tell *this to use its internal buffer
            m_buffer = &m_internal[0];
        }
        else if(rhs.is_in_external_array_())
        {
            // This is a lazy cheat, but eminently effective.
            rhs.swap(*this);

            return;
        }
        else
        {
            // Both are using internal buffers, so we exchange the contents
            value_type  t[space];

            block_copy(&t[0],               &rhs.m_internal[0], rhs.m_cItems);
            block_copy(&rhs.m_internal[0],  &m_internal[0],     m_cItems);
            block_copy(&m_internal[0],      &t[0],              rhs.m_cItems);
        }

        std_swap(m_cItems,      rhs.m_cItems);
        std_swap(m_bExternal,   rhs.m_bExternal);

        STLSOFT_ASSERT(is_valid());
    }
/// @}

/// \name Operators
/// @{
public:
    // Note: The following two const and non-const implicit conversion
    // operators are correctly implemented. However, GCC will pedantically
    // give a verbose warning describing its having selected one over the
    // other, and this is, in current versions of the compiler, not
    // suppressable. The warnings must, alas, simply be ignored.

#ifdef _STLSOFT_AUTO_BUFFER_ALLOW_NON_CONST_CONVERSION_OPERATOR
    /// \brief An implicit conversion to a pointer to the start of the element array
    ///
    /// \deprecate This is deprecated
    operator pointer ()
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }
#else /* ? _STLSOFT_AUTO_BUFFER_ALLOW_NON_CONST_CONVERSION_OPERATOR */
    /// \brief Subscript operator
    reference operator [](size_type index)
    {
        STLSOFT_MESSAGE_ASSERT("Index is out of bounds", index <= m_cItems);

        STLSOFT_ASSERT(is_valid());

        return m_buffer[index];
    }

    /// \brief Subscript operator
    const_reference operator [](size_type index) const
    {
        STLSOFT_MESSAGE_ASSERT("Index is out of bounds", index <= m_cItems);

        STLSOFT_ASSERT(is_valid());

        return m_buffer[index];
    }
#endif /* _STLSOFT_AUTO_BUFFER_ALLOW_NON_CONST_CONVERSION_OPERATOR */

#ifdef _STLSOFT_AUTO_BUFFER_ALLOW_CONST_CONVERSION_OPERATOR
    /// \brief An implicit conversion to a pointer-to-const to the start of the element array
    operator const_pointer () const
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }
#endif /* _STLSOFT_AUTO_BUFFER_ALLOW_CONST_CONVERSION_OPERATOR */
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Returns a pointer to the element array
    pointer data()
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }
    /// \brief Returns a pointer-to-const to the element array
    const_pointer data() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }

    /// \brief Returns a reference to the last element in the buffer
    ///
    /// \pre The buffer instance must not be empty
    reference front()
    {
        STLSOFT_ASSERT(is_valid());

        STLSOFT_MESSAGE_ASSERT("Cannot call front() on an empty buffer!", !empty());

        return m_buffer[0];
    }
    /// \brief Returns a reference to the last element in the buffer
    ///
    /// \pre The buffer instance must not be empty
    reference back()
    {
        STLSOFT_ASSERT(is_valid());

        STLSOFT_MESSAGE_ASSERT("Cannot call back() on an empty buffer!", !empty());

        return m_buffer[size() - 1];
    }
    /// \brief Returns a non-mutating (const) reference to the last element
    ///   in the buffer
    ///
    /// \pre The buffer instance must not be empty
    const_reference front() const
    {
        STLSOFT_ASSERT(is_valid());

        STLSOFT_MESSAGE_ASSERT("Cannot call front() on an empty buffer!", !empty());

        return m_buffer[0];
    }
    /// \brief Returns a non-mutating (const) reference to the last element
    ///   in the buffer
    ///
    /// \pre The buffer instance must not be empty
    const_reference back() const
    {
        STLSOFT_ASSERT(is_valid());

        STLSOFT_MESSAGE_ASSERT("Cannot call back() on an empty buffer!", !empty());

        return m_buffer[size() - 1];
    }
/// @}

/// \name Iteration
/// @{
public:
    /// \brief Returns a non-mutating iterator representing the start of the sequence
    const_iterator begin() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }
    /// \brief Returns a non-mutating iterator representing the end of the sequence
    ///
    /// \note In the case where memory allocation has failed in the context
    /// where exceptions are not thrown for allocation failure, this method will
    /// return the same value as begin(). Hence, operations on the <i>empty</i>
    /// auto_buffer<> instance will be safe if made in respect of the range
    /// defined by [begin(), end()).
    const_iterator end() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer + m_cItems;
    }

    /// \brief Returns a mutable iterator representing the start of the sequence
    iterator begin()
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer;
    }
    /// \brief Returns a mutable iterator representing the end of the sequence
    ///
    /// \note In the case where memory allocation has failed in the context
    /// where exceptions are not thrown for allocation failure, this method will
    /// return the same value as begin(). Hence, operations on the <i>empty</i>
    /// auto_buffer<> instance will be safe if made in respect of the range
    /// defined by [begin(), end()).
    iterator end()
    {
        STLSOFT_ASSERT(is_valid());

        return m_buffer + m_cItems;
    }

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator rbegin() const
    {
        STLSOFT_ASSERT(is_valid());

        return const_reverse_iterator(end());
    }
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator rend() const
    {
        STLSOFT_ASSERT(is_valid());

        return const_reverse_iterator(begin());
    }
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator  rbegin()
    {
        STLSOFT_ASSERT(is_valid());

        return reverse_iterator(end());
    }
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator  rend()
    {
        STLSOFT_ASSERT(is_valid());

        return reverse_iterator(begin());
    }
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

/// @}

/// \name Attributes
/// @{
public:
    /// \brief Returns the number of elements in the auto_buffer
    ///
    /// \note In the case where memory allocation has failed in the context
    /// where exceptions are not thrown for allocation failure in the
    /// constructor, this method will return 0. Hence, operations on the
    /// <i>empty</i> auto_buffer<> instance will be safe if made in respect of
    /// the value returned by this method.
    size_type size() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_cItems;
    }

    /// \brief Returns the number of elements in the auto_buffer's internal buffer
    static size_type internal_size()
    {
        return space;
    }

    /// \brief Indicates whether the buffer has any contents
    ///
    /// \note This will only ever be true when an allocation above the number
    /// of elements in the internal array has been requested, and failed.
    ss_bool_t empty() const
    {
        STLSOFT_ASSERT(is_valid());

        return 0 == m_cItems;
    }

#if defined(STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVEx)
    /// \brief Returns an instance of the allocator used to specialise the
    ///  instance.
    static allocator_type &get_allocator()
    {
# if !defined(STLSOFT_STRICT) && \
     defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER >= 1310
#  pragma warning(push)
#  pragma warning(disable : 4640)   /* "construction of local static object is not thread-safe" - since it is here! (As long as one uses a 'conformant' allocator) - maybe use a spin_mutex in future */
# endif /* compiler */

        static allocator_type   s_allocator;

        return s_allocator;

# if !defined(STLSOFT_STRICT) && \
     defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER >= 1310
#  pragma warning(pop)
# endif /* compiler */
    }
#else /* ? STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
    /// \brief Returns an instance of the allocator used to specialise the
    ///  instance.
    allocator_type get_allocator() const
    {
# if defined(STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE)
        return allocator_type();
# else /* ? STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
        return *this;
# endif /* STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
    }
#endif /* STLSOFT_CF_ALLOCATOR_BASE_EXPENSIVE */
/// @}

/// \name Implementation
/// @{
private:
    ss_bool_t   is_valid() const
    {
        ss_bool_t   bRet    =   true;

#if defined(STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK)
        if( space < m_cItems &&
            !m_bExternal)
        {
# ifdef STLSOFT_UNITTEST
            printf("auto_buffer is in external domain, but think's it isn't\n");
# endif /* STLSOFT_UNITTEST */

            bRet = false;
        }
        if( !(space < m_cItems) &&
            m_bExternal)
        {
# ifdef STLSOFT_UNITTEST
            printf("auto_buffer is in internal domain, but think's it isn't\n");
# endif /* STLSOFT_UNITTEST */

            bRet = false;
        }
#else /* ? STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */

        if( space < m_cItems &&
            !m_bExternal)
        {
# ifdef STLSOFT_UNITTEST
            printf("auto_buffer is in external domain, but think's it isn't\n");
# endif /* STLSOFT_UNITTEST */

            bRet = false;
        }
#endif /* STLSOFT_AUTO_BUFFER_AGGRESSIVE_SHRINK */

        if(m_bExternal)
        {
            if(m_buffer == &m_internal[0])
            {
#ifdef STLSOFT_UNITTEST
                printf("auto_buffer is in external domain, but buffer refers to internal array\n");
#endif /* STLSOFT_UNITTEST */

                bRet = false;
            }
        }
        else
        {
            if(m_buffer != &m_internal[0])
            {
#ifdef STLSOFT_UNITTEST
                printf("auto_buffer is in internal domain, but buffer does not refer to internal array\n");
#endif /* STLSOFT_UNITTEST */

                bRet = false;
            }
        }

        return bRet;
    }
/// @}

/// \name Members
/// @{
private:
    pointer     m_buffer;           // Pointer to used buffer
    size_type   m_cItems;           // Number of items in buffer
    ss_bool_t   m_bExternal;        // This is required, since not allowed to compare m_buffer with &m_internal[0] - can't remember why; // NOTE: Check std
    value_type  m_internal[space];  // Internal storage
/// @}

// Not to be implemented
private:
    auto_buffer(class_type const& rhs);
    auto_buffer const& operator =(class_type const& rhs);
};


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k T
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = ss_typename_type_def_k allocator_selector<T>::allocator_type
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT
#  if !defined(STLSOFT_COMPILER_IS_BORLAND) && \
      !defined(STLSOFT_COMPILER_IS_DMC)
        ,   ss_size_t           SPACE   =   auto_buffer_internal_size_calculator<T>::value
#  else /* ? compiler */
        ,   ss_size_t           SPACE   =   256
#  endif /* compiler */
# else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        ,   ss_size_t           SPACE /* = auto_buffer_internal_size_calculator<T>::value */
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_FUNDAMENTAL_ARGUMENT_SUPPORT */
        >
class auto_buffer_old
# if defined(STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS)
    : public auto_buffer<T, A, SPACE>
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    : public auto_buffer<T, SPACE, A>
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
{
/// \name Member Types
/// @{
private:
# if defined(STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS)
    typedef auto_buffer<T, A, SPACE>                                        parent_class_type;
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    typedef auto_buffer<T, SPACE, A>                                        parent_class_type;
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    typedef auto_buffer_old<T, A, SPACE>                                    class_type;

public:
    typedef ss_typename_type_k parent_class_type::value_type                value_type;
    typedef ss_typename_type_k parent_class_type::allocator_type            allocator_type;
    typedef ss_typename_type_k parent_class_type::reference                 reference;
    typedef ss_typename_type_k parent_class_type::const_reference           const_reference;
    typedef ss_typename_type_k parent_class_type::pointer                   pointer;
    typedef ss_typename_type_k parent_class_type::const_pointer             const_pointer;
    typedef ss_typename_type_k parent_class_type::size_type                 size_type;
    typedef ss_typename_type_k parent_class_type::difference_type           difference_type;
    typedef ss_typename_type_k parent_class_type::iterator                  iterator;
    typedef ss_typename_type_k parent_class_type::const_iterator            const_iterator;
# if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    typedef ss_typename_type_k parent_class_type::reverse_iterator          reverse_iterator;
    typedef ss_typename_type_k parent_class_type::const_reverse_iterator    const_reverse_iterator;
# endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k auto_buffer_old(size_type cItems)
        : parent_class_type(cItems)
    {}
/// @}

// Not to be implemented
private:
    auto_buffer_old(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

#if !defined(STLSOFT_COMPILER_IS_WATCOM)
template<   ss_typename_param_k T
# ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
        ,   ss_size_t           SPACE
        ,   ss_typename_param_k A
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
        >
# ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
inline void swap(auto_buffer<T, A, SPACE>& lhs, auto_buffer<T, A, SPACE>& rhs)
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
inline void swap(auto_buffer<T, SPACE, A>& lhs, auto_buffer<T, SPACE, A>& rhs)
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
{
    lhs.swap(rhs);
}
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED

template<   ss_typename_param_k T
# ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
        ,   ss_typename_param_k A
        ,   ss_size_t           SPACE
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
        ,   ss_size_t           SPACE
        ,   ss_typename_param_k A
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
        >
# ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
inline ss_bool_t is_empty(auto_buffer<T, A, SPACE> const& b)
# else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
inline ss_bool_t is_empty(auto_buffer<T, SPACE, A> const& b)
# endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
{
    return b.empty();
}

#endif /* !STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/auto_buffer_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* In the special case of Intel behaving as VC++ 7.0 or earlier on Win32, we
 * illegally insert into the std namespace.
 */
#if defined(STLSOFT_CF_std_NAMESPACE)
# if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
         defined(_MSC_VER))) && \
     _MSC_VER < 1310
namespace std
{
    template<   ss_typename_param_k         T
#  ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
            ,   ss_typename_param_k         A
            ,   stlsoft_ns_qual(ss_size_t)  SPACE
#  else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
            ,   stlsoft_ns_qual(ss_size_t)  SPACE
            ,   ss_typename_param_k         A
#  endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
            >
#  ifdef STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS
    inline void swap(stlsoft_ns_qual(auto_buffer)<T, A, SPACE>& lhs, stlsoft_ns_qual(auto_buffer)<T, A, SPACE>& rhs)
#  else /* ? STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    inline void swap(stlsoft_ns_qual(auto_buffer)<T, SPACE, A>& lhs, stlsoft_ns_qual(auto_buffer)<T, SPACE, A>& rhs)
#  endif /* STLSOFT_AUTO_BUFFER_USE_PRE_1_9_CHARACTERISTICS */
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */

/* ///////////////////////////// end of file //////////////////////////// */
