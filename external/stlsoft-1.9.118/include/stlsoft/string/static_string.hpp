/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/string/static_string.hpp (originally MStkStr.h, ::SynesisDev)
 *
 * Purpose:     basic_static_string class template.
 *
 * Created:     11th June 1994
 * Updated:     10th August 2009
 *
 * Thanks:      To Cl�udio Albuquerque for supplying the pop_back() member.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/string/static_string.hpp
 *
 * [C++ only] Definition of the stlsoft::basic_static_string class
 *  template
 *   (\ref group__library__string "String" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STATIC_STRING
#define STLSOFT_INCL_STLSOFT_STRING_HPP_STATIC_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_STRING_HPP_STATIC_STRING_MAJOR    4
# define STLSOFT_VER_STLSOFT_STRING_HPP_STATIC_STRING_MINOR    3
# define STLSOFT_VER_STLSOFT_STRING_HPP_STATIC_STRING_REVISION 2
# define STLSOFT_VER_STLSOFT_STRING_HPP_STATIC_STRING_EDIT     198
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_DMC:  __DMC__<0x0839
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_DMC) && \
    __DMC__ < 0x0839
# error stlsoft/string/static_string.hpp is not compatible with Digital Mars C/C++ 3.38 or earlier
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/string/static_string.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
# include <stlsoft/memory/allocator_selector.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS
# include <stlsoft/util/std/iterator_generators.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# include <stdexcept>                   // for std::out_of_range
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

#ifdef STLSOFT_UNITTEST
# include <string>
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

/** Simple string class using fixed-size static-based storage
 *
 * \param C The character type
 * \param CCH The number of characters in the fixed-side buffer, not including the null-terminator
 * \param T The traits type. On translators that support default template arguments this is defaulted to char_traits<C>
 *
 * \ingroup group__library__string
 */
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = char_traits<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T = /* char_traits<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
// class static_string
class basic_static_string
    : public stl_collection_tag
{
private:
    enum { internalSize = CCH };

/// \name Types
/// @{
public:
    /// The value type
    typedef C                                                           value_type;
    /// The traits type
    typedef T                                                           traits_type;
    /// The allocator type
    typedef ss_typename_type_k allocator_selector<C>::allocator_type    allocator_type;
    /// The current parameterisation of the type
    typedef basic_static_string<C, CCH, T>                              class_type;
    /// The character type
    typedef value_type                                                  char_type;
    /// The pointer type
    typedef value_type*                                                 pointer;
    /// The non-mutable (const) pointer type
    typedef value_type const*                                           const_pointer;
    /// The reference type
    typedef value_type&                                                 reference;
    /// The non-mutable (const) reference type
    typedef value_type const&                                           const_reference;
    /// The size type
    typedef ss_size_t                                                   size_type;
    /// The difference type
    typedef ss_ptrdiff_t                                                difference_type;

    /// The iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type                         iterator;
    /// The non-mutating (const) iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
#endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type                         const_iterator;

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The mutating (non-const) reverse iterator type
    typedef ss_typename_type_k reverse_iterator_generator   <   iterator
                                                            ,   value_type
                                                            ,   reference
                                                            ,   pointer
                                                            ,   difference_type
                                                            >::type     reverse_iterator;

    /// The non-mutating (const) reverse iterator type
    typedef ss_typename_type_k const_reverse_iterator_generator <   const_iterator
                                                            ,   value_type
                                                            ,   const_reference
                                                            ,   const_pointer
                                                            ,   difference_type
                                                            >::type     const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

private:
    typedef auto_buffer_old<char_type
                        ,   allocator_type
                        >                                               buffer_type_;
/// @}

/// \name Construction
/// @{
public:
    /// Default constructor
    basic_static_string();
    /// Copy constructor
    basic_static_string(class_type const& rhs);
    /// Construct from the given string at the specified position
    basic_static_string(class_type const& s, size_type pos);
    /// Construct with \c n characters from the given string at the specified position
    basic_static_string(class_type const& s, size_type pos, size_type n);
    /// Construct from the given character string
    basic_static_string(char_type const* s);
    /// Construct with \c n characters from the given character string
    basic_static_string(char_type const* s, size_type n);
    /// Construct with \c n characters each set to \c ch
    basic_static_string(size_type n, char_type ch);
    /// Construct from the range [first:last)
#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
    basic_static_string(char_type const* f, char_type const* t);
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */
    template <ss_typename_param_k II>
    basic_static_string(II first, II last)
        : m_length(static_cast<ss_size_t>(stlsoft_ns_qual_std(distance)(first, last)))
    {
        STLSOFT_MESSAGE_ASSERT("incident string too large for static_string construction", !(max_size() < m_length));

        buffer_type_    buffer(m_length);

        stlsoft_ns_qual_std(copy)(first, last, buffer.begin());

        traits_type::copy(&m_buffer[0], buffer.data(), buffer.size());

        m_buffer[m_length]      =   '\0';
        m_buffer[max_size()]    =   '\0';
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */
    /// Destructor
    ~basic_static_string() stlsoft_throw_0();
/// @}

/// \name Assignment
/// @{
public:
    /// Assigns from the given character string
    class_type& assign(char_type const* s);
    /// Assigns with \c n characters from the given character string
    class_type& assign(char_type const* s, size_type n);
    /// Assigns with \c n characters from the given character string at the specified position
    class_type& assign(class_type const& str, size_type pos, size_type n);
    /// Assigns from the given string
    class_type& assign(class_type const& str);
    /// Assigns \c n characters with the value \c ch
    class_type& assign(size_type n, char_type c);
    /// Assigns from the range [first:last)
#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
    class_type& assign(const_iterator first, const_iterator last);
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */
    template <ss_typename_param_k II>
    class_type& assign(II first, II last)
    {
# if defined(STLSOFT_COMPILER_IS_GCC) && \
     __GNUC__ < 3
        typedef ss_typename_type_k std::iterator_traits<II> traits_t;

        return assign_(first, last, traits_t::iterator_category());
# elif defined(STLSOFT_COMPILER_IS_MWERKS) || \
       defined(STLSOFT_COMPILER_IS_DMC)
        return assign_(first, last, stlsoft_iterator_query_category_ptr(II, first));
# else /* ? compiler */
        return assign_(first, last, stlsoft_iterator_query_category(II, first));
# endif /* compiler */
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

    /// Copy assignment operator
    class_type const& operator =(class_type const& rhs);
    /// Assignment operator
    class_type const& operator =(char_type const* s);
    /// Assignment operator
    class_type const& operator =(char_type ch);
/// @}

/// \name Appending
/// @{
public:
    /// Appends the given character string
    class_type& append(char_type const* s);
    /// Appends \c cch characters from the given character string
    class_type& append(char_type const* s, size_type cch);
    /// Assigns \c cch characters from the given character string at the specified position
    class_type& append(class_type const& str, size_type pos, size_type cch);
    /// Appends the given string
    class_type& append(class_type const& str);
    /// Appends \c cch characters with the value \c ch
    class_type& append(size_type cch, char_type ch);
    /// Appends the range [first:last)
#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
    class_type& append(const_iterator first, const_iterator last);
#else /* ? STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */
    template <ss_typename_param_k II>
    class_type& append(II first, II last)
    {
# if defined(STLSOFT_COMPILER_IS_GCC) && \
     __GNUC__ < 3
        typedef ss_typename_type_k std::iterator_traits<II> traits_t;

        return append_(first, last, traits_t::iterator_category());
# elif defined(STLSOFT_COMPILER_IS_MWERKS) || \
       defined(STLSOFT_COMPILER_IS_DMC)
        return append_(first, last, stlsoft_iterator_query_category_ptr(II, first));
# else /* ? compiler */
        return append_(first, last, stlsoft_iterator_query_category(II, first));
# endif /* compiler */
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

    /// Concatenation operator
    class_type& operator +=(char_type ch);
    /// Concatenation operator
    class_type& operator +=(char_type const* s);
    /// Concatenation operator
    class_type& operator +=(class_type const& rhs);

    /// Appends a single character
    void push_back(char_type ch);
    /// Removes the last character
    void pop_back();
/// @}

/// \name Operations
/// @{
public:
    /// Reserves at least n characters
    void reserve(size_type n);
    /// Swaps the contents between \c this and \c other
    void swap(class_type& other);

    /// Resizes the string
    ///
    /// \param n The new size of the string
    /// \param ch The value with which to initialise additional items if the string is expanded
    void resize(size_type n, value_type ch = value_type());

    /// Empties the string
    void clear();
/// @}

/// \name Attributes
/// @{
public:
    /// The number of elements in the string
    size_type size() const;
    /// The maximum number of elements that can be stored in the string
    static size_type max_size();
    /// The number of elements in the string
    size_type length() const;
    /// The storage currently allocated by the string
    size_type capacity() const;
    /// Indicates whether the string is empty
    ss_bool_t empty() const;
/// @}

/// \name Comparison
/// @{
public:
    /// Compares \c this with the given string
    ss_sint_t compare(size_type pos, size_type n, value_type const* s, size_type cchRhs) const;
    /// Compares \c this with the given string
    ss_sint_t compare(size_type pos, size_type n, value_type const* s) const;
    /// Compares \c this with the given string
    ss_sint_t compare(value_type const* s) const;
    /// Compares \c this with the given string
    ss_sint_t compare(size_type pos, size_type n, class_type const& rhs, size_type posRhs, size_type cchRhs) const;
    /// Compares \c this with the given string
    ss_sint_t compare(size_type pos, size_type n, class_type const& rhs) const;
    /// Compares \c this with the given string
    ss_sint_t compare(class_type const& rhs) const;
/// @}

/// \name Accessors
/// @{
public:
    /// Returns mutable reference at the given index
    reference       operator [](size_type index);
    /// Returns non-mutable (const) reference at the given index
    const_reference operator [](size_type index) const;

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    /// Returns mutable (non-const) reference at the given index
    ///
    /// \note Throws std::out_of_range if index >= size()
    reference               at(size_type index);
    /// Returns non-mutable (const) reference at the given index
    ///
    /// \note Throws std::out_of_range if index >= size()
    const_reference         at(size_type index) const;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

    /// Returns null-terminated non-mutable (const) pointer to string data
    value_type const    *c_str() const;
    /// Returns non-mutable (const) pointer to string data
    value_type const    *data() const;

    /// Copies elements into the given destination
    size_type copy(value_type *dest, size_type cch_, size_type pos = 0) const;
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return A non-mutable (const) iterator representing the start of the sequence
    const_iterator          begin() const;
    /// Ends the iteration
    ///
    /// \return A non-mutable (const) iterator representing the end of the sequence
    const_iterator          end() const;
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator                begin();
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator                end();

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// Ends the reverse iteration
    ///
    /// \return A non-mutable (const) iterator representing the end of the reverse sequence
    const_reverse_iterator  rend() const;
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    reverse_iterator        rbegin();
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    reverse_iterator        rend();
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Implementation
/// @{
private:
    // Length
    size_type length_() const;

    // Invariance
    ss_bool_t is_valid() const;

    // Comparison
    static ss_sint_t compare_(char_type const* lhs, size_type lhs_len, char_type const* rhs, size_type rhs_len);

    // Assignment
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
    template <ss_typename_param_k II>
# if defined(STLSOFT_COMPILER_IS_MWERKS) || \
     defined(STLSOFT_COMPILER_IS_DMC)
    // There seems to be a bug in CodeWarrior that makes it have a cow with iterator tags by value, so we just use a ptr
    class_type& assign_(II first, II last, stlsoft_ns_qual_std(input_iterator_tag) const*)
# else /* ? compiler */
    class_type& assign_(II first, II last, stlsoft_ns_qual_std(input_iterator_tag))
# endif /* compiler */
    {
        stlsoft_ns_qual_std(copy)(first, last, stlsoft_ns_qual_std(back_inserter)(*this));

        return *this;
    }
    template <ss_typename_param_k II>
# if defined(STLSOFT_COMPILER_IS_MWERKS) || \
     defined(STLSOFT_COMPILER_IS_DMC)
    // There seems to be a bug in CodeWarrior that makes it have a cow with iterator tags by value, so we just use a ptr
    class_type& assign_(II first, II last, stlsoft_ns_qual_std(forward_iterator_tag) const*)
# else /* ? compiler */
    class_type& assign_(II first, II last, stlsoft_ns_qual_std(forward_iterator_tag))
# endif /* compiler */
    {
        buffer_type_    buffer(static_cast<ss_size_t>(stlsoft_ns_qual_std(distance)(first, last)));

        stlsoft_ns_qual_std(copy)(first, last, buffer.begin());
        assign(buffer.data(), buffer.size());

        return *this;
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

    // Appending
#if defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
    template <ss_typename_param_k II>
# if defined(STLSOFT_COMPILER_IS_MWERKS) || \
     defined(STLSOFT_COMPILER_IS_DMC)
    class_type& append_(II first, II last, stlsoft_ns_qual_std(input_iterator_tag) const*)
# else /* ? compiler */
    class_type& append_(II first, II last, stlsoft_ns_qual_std(input_iterator_tag))
# endif /* compiler */
    {
        stlsoft_ns_qual_std(copy)(first, last, stlsoft_ns_qual_std(back_inserter)(*this));

        STLSOFT_ASSERT(is_valid());
        return *this;
    }
    template <ss_typename_param_k II>
# if defined(STLSOFT_COMPILER_IS_MWERKS) || \
     defined(STLSOFT_COMPILER_IS_DMC)
    class_type& append_(II first, II last, stlsoft_ns_qual_std(forward_iterator_tag) const*)
# else /* ? compiler */
    class_type& append_(II first, II last, stlsoft_ns_qual_std(forward_iterator_tag))
# endif /* compiler */
    {
        buffer_type_    buffer(static_cast<ss_size_t>(stlsoft_ns_qual_std(distance)(first, last)));

        stlsoft_ns_qual_std(copy)(first, last, &buffer[0]);
        append(buffer.data(), buffer.size());

        STLSOFT_ASSERT(is_valid());
        return *this;
    }
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */
/// @}

/// \name Members
/// @{
private:
    value_type  m_buffer[internalSize + 1];
    ss_size_t   m_length;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// operator ==

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator ==(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
#ifdef _DEBUG
//  printf("lhs(%s); rhs(%s); lhs.compare(rhs)=%d\n", lhs.c_str(), lhs.c_str(), lhs.compare(rhs));
#endif /* _DEBUG */

    return lhs.compare(rhs) == 0;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator ==(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator ==(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) == 0;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator ==(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator ==(C *lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) == 0;
}

// operator !=

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator !=(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
    return lhs.compare(rhs) != 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator !=(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator !=(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) != 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator !=(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator !=(C const* lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) != 0;
}

// operator <

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator <(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
    return lhs.compare(rhs) < 0;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator <(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator <(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) < 0;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator <(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator <(C const* lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) > 0;
}

// operator <=

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator <=(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
    return lhs.compare(rhs) <= 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator <=(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator <=(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) <= 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator <=(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator <=(C const* lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) >= 0;
}

// operator >

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator >(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
    return lhs.compare(rhs) > 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator >(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator >(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) > 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator >(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator >(C const* lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) < 0;
}

// operator >=

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t operator >=(basic_static_string<C, CCH, T> const& lhs, basic_static_string<C, CCH, T> const& rhs)
{
    return lhs.compare(rhs) >= 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator >=(basic_static_string<C, CCH, T> const& lhs, ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator >=(basic_static_string<C, CCH, T> const& lhs, C const* rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return lhs.compare(rhs) >= 0;
}
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
#ifdef STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT
inline ss_bool_t operator >=(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* lhs, basic_static_string<C, CCH, T> const& rhs)
#else /* ? STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
inline ss_bool_t operator >=(C const* lhs, basic_static_string<C, CCH, T> const& rhs)
#endif /* STLSOFT_CF_TEMPLATE_OUTOFCLASSFN_QUALIFIED_TYPE_SUPPORT */
{
    return rhs.compare(lhs) <= 0;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void swap(basic_static_string<C, CCH, T>& lhs, basic_static_string<C, CCH, T>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED

// c_str_ptr_null

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_a_t const* c_str_ptr_null_a(stlsoft_ns_qual(basic_static_string)<ss_char_a_t, CCH, T> const& s)
{
    return (0 != s.length()) ? s.c_str() : NULL;
}
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_w_t const* c_str_ptr_null_w(stlsoft_ns_qual(basic_static_string)<ss_char_w_t, CCH, T> const& s)
{
    return (0 != s.length()) ? s.c_str() : NULL;
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_ptr_null for stlsoft::basic_static_string
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline C const* c_str_ptr_null(stlsoft_ns_qual(basic_static_string)<C, CCH, T> const& s)
{
    return (0 != s.length()) ? s.c_str() : NULL;
}

// c_str_ptr

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_a_t const* c_str_ptr_a(stlsoft_ns_qual(basic_static_string)<ss_char_a_t, CCH, T> const& s)
{
    return s.c_str();
}
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_w_t const* c_str_ptr_w(stlsoft_ns_qual(basic_static_string)<ss_char_w_t, CCH, T> const& s)
{
    return s.c_str();
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_ptr for stlsoft::basic_static_string
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline C const* c_str_ptr(stlsoft_ns_qual(basic_static_string)<C, CCH, T> const& s)
{
    return s.c_str();
}

// c_str_data

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_a_t const* c_str_data_a(stlsoft_ns_qual(basic_static_string)<ss_char_a_t, CCH, T> const& s)
{
    return s.data();
}
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_char_w_t const* c_str_data_w(stlsoft_ns_qual(basic_static_string)<ss_char_w_t, CCH, T> const& s)
{
    return s.data();
}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_data for stlsoft::basic_static_string
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline C const* c_str_data(stlsoft_ns_qual(basic_static_string)<C, CCH, T> const& s)
{
    return s.data();
}

// c_str_len

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_size_t CCH, ss_typename_param_k T >
inline ss_size_t c_str_len_a(stlsoft_ns_qual(basic_static_string)<ss_char_a_t, CCH, T> const& s)
{
    return s.length();
}
template <ss_size_t CCH, ss_typename_param_k T >
inline ss_size_t c_str_len_w(stlsoft_ns_qual(basic_static_string)<ss_char_w_t, CCH, T> const& s)
{
    return s.length();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \ref group__concept__shim__string_access__c_str_len for stlsoft::basic_static_string
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_size_t c_str_len(stlsoft_ns_qual(basic_static_string)<C, CCH, T> const& s)
{
    return s.length();
}

#endif /* !STLSOFT_CF_TEMPLATE_SHIMS_NOT_SUPPORTED */



/** \ref group__concept__shim__stream_insertion "stream insertion shim" for stlsoft::basic_static_string
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k     S
        ,   ss_typename_param_k     C
        ,   ss_size_t               CCH
        ,   ss_typename_param_k     T
        >
inline S& operator <<(S& s, stlsoft_ns_qual(basic_static_string)<C, CCH, T> const& str)
{
    s << str.c_str();

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/static_string_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::length_() const
{
    if(m_length == static_cast<ss_size_t>(-1))
    {
        ss_size_t   &_m_length  =   const_cast<class_type *>(this)->m_length;

        _m_length = traits_type::length(m_buffer);
    }

    return m_length;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t basic_static_string<C, CCH, T>::is_valid() const
{
    char_type const*        b  =   &m_buffer[0];
    char_type const* const  e  =   &m_buffer[STLSOFT_NUM_ELEMENTS(m_buffer) - 1];

    for(; b != e && '\0' != *b; ++b)
    {}

    if(b == e)
    {
#ifdef STLSOFT_UNITTEST
        printf("%08x: string contents have been overwritten! Contents=[%*s]\n", static_cast<unsigned>(reinterpret_cast<ss_size_t>(this)), static_cast<int>(max_size()), &m_buffer[0]);
#endif /* STLSOFT_UNITTEST */

        return false;
    }
    else if(m_length != static_cast<ss_size_t>(-1) &&
            m_length > max_size())
    {
#ifdef STLSOFT_UNITTEST
        printf("%08x: length (%ld) greater than maximum allowed (%ld)\n", static_cast<unsigned>(reinterpret_cast<ss_size_t>(this)), static_cast<long>(m_length), static_cast<long>(max_size()));
#endif /* STLSOFT_UNITTEST */

        return false;
    }
    else if(m_length != static_cast<ss_size_t>(-1) &&
            '\0' != m_buffer[m_length])
    {
#ifdef STLSOFT_UNITTEST
        printf("%08x: string not nul-terminated at index (%ld); maximum allowed (%ld)\n", static_cast<unsigned>(reinterpret_cast<ss_size_t>(this)), static_cast<long>(m_length), static_cast<long>(max_size()));
#endif /* STLSOFT_UNITTEST */

        return false;
    }

    return true;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string()
    : m_length(static_cast<ss_size_t>(-1))
{
    m_buffer[0]             =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(class_type const& rhs)
    : m_length(rhs.length())
{
    traits_type::copy(m_buffer, rhs.m_buffer, rhs.length());

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());

#ifdef _DEBUG
//printf("*this(%s, %d)\n", &m_buffer[0], m_length);
#endif /* _DEBUG */
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(class_type const& rhs, size_type pos)
    : m_length(rhs.length() - pos)
{
    traits_type::copy(m_buffer, rhs.m_buffer + pos, m_length);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(class_type const& rhs, size_type pos, size_type n)
    : m_length(n)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string construction", !(max_size() < n));

//printf("ctor(%s, %d, %d = %.*s)\n", rhs.c_str(), pos, n, n, rhs.c_str() + pos);
    traits_type::copy(m_buffer, rhs.m_buffer + pos, n);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(char_type const* s)
    : m_length((NULL != s) ? traits_type::length(s) : 0)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string construction", !(max_size() < m_length));

//printf("Length: %u\n", m_length);

    traits_type::copy(m_buffer, s, m_length);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(char_type const* s, size_type n)
    : m_length(n)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string construction", !(max_size() < n));

    traits_type::copy(m_buffer, s, n);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(size_type n, char_type c)
    : m_length(n)
{
    traits_type::assign(m_buffer, n, c);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::basic_static_string(char_type const* f, char_type const* t)
    : m_length(t - f)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string construction", !(max_size() < m_length));

    traits_type::copy(m_buffer, f, m_length);

    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';

    STLSOFT_ASSERT(is_valid());
}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline basic_static_string<C, CCH, T>::~basic_static_string() stlsoft_throw_0()
{
    STLSOFT_ASSERT(is_valid());
}


// Assignment

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(const ss_typename_type_k basic_static_string<C, CCH, T>::char_type *s)
{
    STLSOFT_ASSERT(is_valid());

#if 0
    m_length = static_cast<ss_size_t>(-1);
    traits_type::copy(m_buffer, s, CCH);
    m_buffer[max_size()]    =   '\0';
#else /* ? 0 */
    class_type  t(s);
    t.swap(*this);
#endif /* 0 */

    STLSOFT_ASSERT(is_valid());

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(const ss_typename_type_k basic_static_string<C, CCH, T>::char_type *s, ss_typename_type_k basic_static_string<C, CCH, T>::size_type n)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string assignment", !(max_size() < n));

    STLSOFT_ASSERT(is_valid());

#if 0
    traits_type::copy(m_buffer, s, n);

    m_length = n;
    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';
#else /* ? 0 */
    class_type  t(s, n);
    t.swap(*this);
#endif /* 0 */

    STLSOFT_ASSERT(is_valid());

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(   ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        pos
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        n)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string assignment", !(max_size() < n));

    STLSOFT_ASSERT(is_valid());

#if 0
    traits_type::copy(m_buffer, rhs.m_buffer + pos, n);

    m_length = n;
    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';
#else /* ? 0 */
    class_type  t(rhs, pos, n);
    t.swap(*this);
#endif /* 0 */

    STLSOFT_ASSERT(is_valid());

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs)
{
    STLSOFT_ASSERT(is_valid());

//printf("1\n");

#ifdef _DEBUG
    const class_type    result(rhs);
#endif /* _DEBUG */

//printf("2\n");

//printf("assign(%s, %d)\n", rhs.c_str(), rhs.length());

    class_type  t(rhs);

//printf("3\n");

    t.swap(*this);

//printf("4\n");


//printf("5\n");

#ifdef _DEBUG
//printf("result(%s, %d); rhs(%s, %d); t(%s, %d), *this(%s, %d)\n", result.c_str(), result.length(), rhs.c_str(), rhs.length(), t.c_str(), t.length(), c_str(), length());
    STLSOFT_ASSERT(result == *this);
#endif /* _DEBUG */

//printf("6\n");

    STLSOFT_ASSERT(is_valid());

//printf("7\n");

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(   ss_typename_type_k basic_static_string<C, CCH, T>::size_type n
                                                                                                                ,   ss_typename_type_k basic_static_string<C, CCH, T>::char_type c)
{
    STLSOFT_MESSAGE_ASSERT("incident string too large for static_string assignment", !(max_size() < n));

    STLSOFT_ASSERT(is_valid());

#if 0
    traits_type::assign(m_buffer, n, c);

    m_length = n;
    m_buffer[m_length]      =   '\0';
    m_buffer[max_size()]    =   '\0';
#else /* ? 0 */
    class_type  t(n, c);
    t.swap(*this);
#endif /* 0 */

    STLSOFT_ASSERT(is_valid());

    return *this;
}

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::assign(ss_typename_type_k basic_static_string<C, CCH, T>::const_iterator first, ss_typename_type_k basic_static_string<C, CCH, T>::const_iterator last)
{
    STLSOFT_ASSERT(is_valid());

    // We have to use this strange appearing this, because of Visual C++ .NET's
    // disgusting STL swill. Sigh!
    return assign(&(*first), last - first);
}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type const& basic_static_string<C, CCH, T>::operator =(ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs)
{
    STLSOFT_ASSERT(is_valid());

    return assign(rhs);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type const& basic_static_string<C, CCH, T>::operator =(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* s)
{
    STLSOFT_ASSERT(is_valid());

    return assign(s);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type const& basic_static_string<C, CCH, T>::operator =(ss_typename_type_k basic_static_string<C, CCH, T>::char_type c)
{
    STLSOFT_ASSERT(is_valid());

    m_length = 1;
    m_buffer[0] = c;
    m_buffer[1] = c;

    STLSOFT_ASSERT(is_valid());

    return *this;
}


template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::size() const
{
    return class_type::length();
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline /* static */ ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::max_size()
{
    return internalSize;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::length() const
{
    STLSOFT_ASSERT(is_valid());

    return length_();
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::capacity() const
{
    STLSOFT_ASSERT(is_valid());

    return class_type::max_size();
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_bool_t basic_static_string<C, CCH, T>::empty() const
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer[0] == 0;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline /* static */ ss_sint_t basic_static_string<C, CCH, T>::compare_( ss_typename_type_k basic_static_string<C, CCH, T>::value_type const* lhs
                                                                    ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        lhs_len
                                                                    ,   ss_typename_type_k basic_static_string<C, CCH, T>::value_type const* rhs
                                                                    ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        rhs_len)
{
    size_type   cmp_len =   (lhs_len < rhs_len) ? lhs_len : rhs_len;
    ss_int_t    result  =   traits_type::compare(lhs, rhs, cmp_len);

    if(result == 0)
    {
        result = static_cast<ss_int_t>(lhs_len) - static_cast<ss_int_t>(rhs_len);
    }

#ifdef _DEBUG
//printf("compare_(\"%*s\",%d\n        ,\"%*s\",%d) == %d\n", lhs_len, lhs, lhs_len, rhs_len, rhs, rhs_len, result);
#endif /* _DEBUG */

    return result;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::size_type pos, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cch, ss_typename_type_k basic_static_string<C, CCH, T>::value_type const* s, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cchRhs) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();

    if(!(pos < lhs_len))
    {
        pos = lhs_len;
    }
    else
    {
        lhs_len -= pos;
    }

    if(cch < lhs_len)
    {
        lhs_len = cch;
    }

    size_type   rhs_len =   (s == 0) ? 0 : traits_type::length(s);

    if(cchRhs < rhs_len)
    {
        rhs_len = cchRhs;
    }

    STLSOFT_ASSERT(is_valid());

    return compare_(m_buffer + pos, lhs_len, s, rhs_len);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::size_type pos, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cch, ss_typename_type_k basic_static_string<C, CCH, T>::value_type const* s) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();

    if(!(pos < lhs_len))
    {
        pos = lhs_len;
    }
    else
    {
        lhs_len -= pos;
    }

    if(cch < lhs_len)
    {
        lhs_len = cch;
    }

    size_type   rhs_len =   (s == 0) ? 0 : traits_type::length(s);

    STLSOFT_ASSERT(is_valid());

    return compare_(m_buffer + pos, lhs_len, s, rhs_len);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::value_type const* s) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();
    size_type   rhs_len =   (s == 0) ? 0 : traits_type::length(s);

    STLSOFT_ASSERT(is_valid());

    return compare_(m_buffer, lhs_len, s, rhs_len);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::size_type pos, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cch, ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs, ss_typename_type_k basic_static_string<C, CCH, T>::size_type posRhs, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cchRhs) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();

    if(!(pos < lhs_len))
    {
        pos = lhs_len;
    }
    else
    {
        lhs_len -= pos;
    }

    if(cch < lhs_len)
    {
        lhs_len = cch;
    }

    size_type   rhs_len =   rhs.length();

    if(!(posRhs < rhs_len))
    {
        posRhs = rhs_len;
    }
    else
    {
        rhs_len -= posRhs;
    }

    if(cchRhs < rhs_len)
    {
        rhs_len = cchRhs;
    }

    STLSOFT_ASSERT(is_valid());

    return compare_(m_buffer + pos, lhs_len, rhs.m_buffer + posRhs, rhs_len);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::size_type pos, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cch, ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();

    if(!(pos < lhs_len))
    {
        pos = lhs_len;
    }
    else
    {
        lhs_len -= pos;
    }

    if(cch < lhs_len)
    {
        lhs_len = cch;
    }

    size_type   rhs_len =   rhs.length();

    STLSOFT_ASSERT(is_valid());

    return compare_(m_buffer + pos, lhs_len, rhs.m_buffer, rhs_len);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_sint_t basic_static_string<C, CCH, T>::compare(ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   lhs_len =   length();
    size_type   rhs_len =   rhs.length();

    STLSOFT_ASSERT(is_valid());

#ifdef _DEBUG
//printf("Comparing (\"%*s\",%d; \"%*s\",%d)\n", lhs_len, &m_buffer[0], lhs_len, rhs_len, &rhs.m_buffer[0], rhs_len);
#endif /* _DEBUG */

    return compare_(&m_buffer[0], lhs_len, &rhs.m_buffer[0], rhs_len);
}


template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::resize( ss_typename_type_k basic_static_string<C, CCH, T>::size_type    n
                                                ,   ss_typename_type_k basic_static_string<C, CCH, T>::value_type   ch /* = value_type() */)
{
    STLSOFT_MESSAGE_ASSERT("resize request too large for static_string", !(max_size() < n));

    STLSOFT_ASSERT(is_valid());

    size_type const len =   length();

    if(len != n)
    {
        if(len < n)
        {
            traits_type::assign(m_buffer + len, n - len, ch);
        }

        m_length = n;
        m_buffer[n] = 0;
    }

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::clear()
{
    STLSOFT_ASSERT(is_valid());

    m_length = static_cast<ss_size_t>(-1);
    m_buffer[0] = 0;

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::reference basic_static_string<C, CCH, T>::operator [](ss_typename_type_k basic_static_string<C, CCH, T>::size_type index)
{
    STLSOFT_MESSAGE_ASSERT("index access out of range in static_string", index < size());

    STLSOFT_ASSERT(is_valid());

    return m_buffer[index];
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_reference basic_static_string<C, CCH, T>::operator [](ss_typename_type_k basic_static_string<C, CCH, T>::size_type index) const
{
    STLSOFT_MESSAGE_ASSERT("index access out of range in static_string", index < size() + 1); // Valid to return (const) reference to nul-terminator

    STLSOFT_ASSERT(is_valid());

    return m_buffer[index];
}


#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::reference basic_static_string<C, CCH, T>::at(ss_typename_type_k basic_static_string<C, CCH, T>::size_type index)
{
    STLSOFT_ASSERT(is_valid());

    if(index >= size())
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("index out of range"));
    }

    STLSOFT_ASSERT(is_valid());

    return m_buffer[index];
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_reference basic_static_string<C, CCH, T>::at(ss_typename_type_k basic_static_string<C, CCH, T>::size_type index) const
{
    STLSOFT_ASSERT(is_valid());

    if(index > size())
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("index out of range"));
    }

    STLSOFT_ASSERT(is_valid());

    return m_buffer[index];
}
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */


template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::value_type const* basic_static_string<C, CCH, T>::c_str() const
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer;
}

// Appending

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(   ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* s
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type n)
{
    STLSOFT_MESSAGE_ASSERT("resize request too large for static_string", !(max_size() < n + length()));

    STLSOFT_ASSERT(is_valid());

    if(0 != n)
    {
        traits_type::copy(m_buffer + m_length, s, n);

        m_length += n;
        m_buffer[m_length] = 0;
    }

    STLSOFT_ASSERT(is_valid());

    return *this;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(ss_typename_type_k basic_static_string<C, CCH, T>::char_type const* s)
{
    STLSOFT_ASSERT(is_valid());

    return append(s, (NULL == s) ? 0 : traits_type::length(s));
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(   ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& rhs
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        pos
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::size_type        cch)
{
    STLSOFT_ASSERT(is_valid());

    char_type const* s  =   rhs.m_buffer;
    size_type       len =   rhs.length();

    if(len < pos)
    {
        pos = len;
    }

    if(len - pos < cch)
    {
        cch = len - pos;
    }

    if(NULL != s)
    {
        s += pos;
    }
    else
    {
        cch = 0;
    }

    STLSOFT_ASSERT(is_valid());

    return append(s, cch);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(ss_typename_type_k basic_static_string<C, CCH, T>::class_type const& s)
{
    STLSOFT_ASSERT(is_valid());

    return append(s.m_buffer, s.length());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(   ss_typename_type_k basic_static_string<C, CCH, T>::size_type    n
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::char_type    ch)
{
    STLSOFT_MESSAGE_ASSERT("resize request too large for static_string", !(max_size() < n + length()));

//printf("append(pre): *this(%*s)\n", m_length, &m_buffer[0]);

    STLSOFT_ASSERT(is_valid());

    if(0 != n)
    {
        traits_type::assign(m_buffer + m_length, n, ch);

        m_length += n;
        m_buffer[m_length] = 0;
    }

//printf("append(post): *this(%*s)\n", m_length, &m_buffer[0]);

    STLSOFT_ASSERT(is_valid());

    return *this;
}

#if !defined(STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT)
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::append(   ss_typename_type_k basic_static_string<C, CCH, T>::const_iterator first
                                                                                                            ,   ss_typename_type_k basic_static_string<C, CCH, T>::const_iterator last)
{
    STLSOFT_ASSERT(is_valid());

    // We have to use this strange appearing form, because of Visual C++ .NET's
    // disgusting STL swill. Sigh!
    return append(&(*first), last - first);
}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_RANGE_METHOD_SUPPORT */

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::operator +=(ss_typename_type_k basic_static_string<C, CCH, T>::char_type ch)
{
    STLSOFT_ASSERT(is_valid());

    return append(1, ch);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::operator +=(const ss_typename_type_k basic_static_string<C, CCH, T>::char_type *s)
{
    STLSOFT_ASSERT(is_valid());

    return append(s);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::class_type& basic_static_string<C, CCH, T>::operator +=(const ss_typename_type_k basic_static_string<C, CCH, T>::class_type& rhs)
{
    STLSOFT_ASSERT(is_valid());

    return append(rhs);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::push_back(ss_typename_type_k basic_static_string<C, CCH, T>::char_type ch)
{
    STLSOFT_ASSERT(is_valid());

    append(1, ch);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::pop_back()
{
    STLSOFT_ASSERT(is_valid());

    if (m_length != 0) {
        m_buffer[--m_length] = 0;
    }

}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::reserve(ss_typename_type_k basic_static_string<C, CCH, T>::size_type n)
{
    STLSOFT_ASSERT(is_valid());

    STLSOFT_MESSAGE_ASSERT("reserve request out of range in static_string", !(max_size() < n));

    STLSOFT_SUPPRESS_UNUSED(n);
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline void basic_static_string<C, CCH, T>::swap(ss_typename_type_k basic_static_string<C, CCH, T>::class_type& other)
{
//printf("swap-1\n");

    STLSOFT_ASSERT(is_valid());

//printf("swap-2\n");

    value_type  buffer[internalSize];

//printf("swap-3\n");

    traits_type::copy(        buffer, other.m_buffer, 1 + other.m_length);
    traits_type::copy(other.m_buffer,       m_buffer, 1 +       m_length);
    traits_type::copy(      m_buffer,         buffer, 1 + other.m_length);

//printf("swap-4\n");

    std_swap(m_length, other.m_length);

//printf("swap-5\n");

    STLSOFT_ASSERT(is_valid());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::value_type const* basic_static_string<C, CCH, T>::data() const
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::size_type basic_static_string<C, CCH, T>::copy(ss_typename_type_k basic_static_string<C, CCH, T>::value_type *dest, ss_typename_type_k basic_static_string<C, CCH, T>::size_type cch_, ss_typename_type_k basic_static_string<C, CCH, T>::size_type pos /* = 0 */) const
{
    STLSOFT_ASSERT(is_valid());

    size_type   len =   length();

    if(pos < len)
    {
        if(len < pos + cch_)
        {
            cch_ = len - pos;
        }

        traits_type::copy(dest, data() + pos, cch_);
    }
    else
    {
        cch_ = 0;
    }

    STLSOFT_ASSERT(is_valid());

    return cch_;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_iterator basic_static_string<C, CCH, T>::begin() const
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_iterator basic_static_string<C, CCH, T>::end() const
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer + length_();
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::iterator basic_static_string<C, CCH, T>::begin()
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer;
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::iterator basic_static_string<C, CCH, T>::end()
{
    STLSOFT_ASSERT(is_valid());

    return m_buffer + length_();
}


#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_reverse_iterator basic_static_string<C, CCH, T>::rbegin() const
{
    STLSOFT_ASSERT(is_valid());

    return const_reverse_iterator(end());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::const_reverse_iterator basic_static_string<C, CCH, T>::rend() const
{
    STLSOFT_ASSERT(is_valid());

    return const_reverse_iterator(begin());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::reverse_iterator basic_static_string<C, CCH, T>::rbegin()
{
    STLSOFT_ASSERT(is_valid());

    return reverse_iterator(end());
}

template<   ss_typename_param_k C
        ,   ss_size_t           CCH
        ,   ss_typename_param_k T
        >
inline ss_typename_type_ret_k basic_static_string<C, CCH, T>::reverse_iterator basic_static_string<C, CCH, T>::rend()
{
    STLSOFT_ASSERT(is_valid());

    return reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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
    template<   ss_typename_param_k         C
            ,   stlsoft_ns_qual(ss_size_t)  CCH
            ,   ss_typename_param_k         T
            >
    inline void swap(stlsoft_ns_qual(basic_static_string)<C, CCH, T>& lhs, stlsoft_ns_qual(basic_static_string)<C, CCH, T>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STATIC_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
