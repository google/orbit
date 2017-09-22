/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/environment_sequence.hpp
 *
 * Purpose:     basic_environment_sequence class.
 *
 * Created:     31st December 2002
 * Updated:     10th August 2009
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


/** \file winstl/system/environment_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_environment_sequence
 *  class template
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE_MAJOR    4
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE_MINOR    1
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE_REVISION 1
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE_EDIT     82
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS
# include <winstl/system/system_traits.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYSTEM_HPP_SYSTEM_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS
# include <stlsoft/util/std/iterator_generators.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */
#ifndef _WINSTL_WINDOW_FUNCTIONALS_NO_STD
# ifndef STLSOFT_INCL_FUNCTIONAL
#  define STLSOFT_INCL_FUNCTIONAL
#  include <functional>
# endif /* !STLSOFT_INCL_FUNCTIONAL */
#else /* ? _WINSTL_WINDOW_FUNCTIONALS_NO_STD */
# error Now need to write that std_binary_function stuff!!
#endif /* _WINSTL_WINDOW_FUNCTIONALS_NO_STD */

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

/** \brief STL-like sequence for the system environment variables
 *
 * \ingroup group__library__system
 *
 * \param C The character type
 *
 * \note Even though Win32 treats environment variables in a case-insensitive
 * manner, it is possible for the raw environment information (access via the
 * GetEnvironmentStrings() function) to contain multiple entries whose names
 * differ only by case. Thus, later versions of the sequence class support the
 * \c ignoreCase member constant, which is passed by default to the
 * constructor, in order to facilitate "normal" Win32 operation while
 * supporting all possible modes.
 */
template <ss_typename_param_k C>
class basic_environment_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C                                                   char_type;
    /// Structure representing the items
    struct symbol
    {
        /// The symbol name
        char_type const* name;
        /// The symbol value
        char_type const* value;
    };
    /// The value type
    typedef symbol                                              value_type;
    /// The current parameterisation of the type
    typedef basic_environment_sequence<C>                       class_type;
    /// The non-mutable (const) pointer type
    typedef value_type const*                                   const_pointer;
    /// The non-mutable (const) reference type
    typedef value_type const&                                   const_reference;
    /// The size type
    typedef ws_size_t                                           size_type;
    /// The difference type
    typedef ws_ptrdiff_t                                        difference_type;
    /// The non-mutating (const) iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
      ss_typename_type_k
#endif /* compiler */
        stlsoft_ns_qual(pointer_iterator)   <   value_type
                                            ,   const_pointer
                                            ,   const_reference
                                            >::type             const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The non-mutating (const) reverse iterator type
    typedef
#if !defined(STLSOFT_COMPILER_IS_BORLAND)
            ss_typename_type_k
#endif /* compiler */
                               stlsoft_ns_qual(const_reverse_iterator_generator)<   const_iterator
                                                                                ,   value_type
                                                                                ,   const_reference
                                                                                ,   const_pointer
                                                                                ,   difference_type
                                                                                >::type     const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Member Constants
/// @{
public:
    enum
    {
            showHidden  =   0x0001  //!< Include the hidden environment variables (those beginning with '=') in the sequence
        ,   noSort      =   0x0002  //!< Do not explicitly sort the contents
        ,   ignoreCase  =   0x0004  //!< Ignore case in when comparing names / values in find() methods
    };
/// @}

// Construction
public:
    /// Construct a sequence of the current process's environment entries, according
    /// to the given criteria
    ///
    /// \param flags One or more of the member constants
    ss_explicit_k basic_environment_sequence(ws_int_t flags = ignoreCase);
    /// Destructor, which releases any resources acquired
    ~basic_environment_sequence() stlsoft_throw_0();

/// \name Iteration
/// @{
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

    /// Searches for an entry holding the given name
    ///
    /// \param name The name of the entry. Must not be NULL
    const_iterator  find(char_type const* name) const;

    /// Searches for an entry holding the given name and value
    ///
    /// \param name The name of the entry. Must not be NULL
    /// \param value The value of the entry. Must not be NULL
    const_iterator  find(char_type const* name, char_type const* value) const;
/// @}

/** \brief Size
 * @{
 *
 * \ingroup group__library__system
 */
public:
    /// Returns the number of elements in the enviroment sequence
    size_type size() const;
    /// Indicates whether the enviroment sequence is empty
    ws_bool_t empty() const;
/// @}

// Accessors
public:
    /// Returns the name-value item for the given index
    value_type operator [](size_type index) const;

// TODO: Add ARV to this class

// Implementation
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   char_type
                                            ,   processheap_allocator<char_type>
                                            >           environment_buffer_type;
    typedef stlsoft_ns_qual(auto_buffer_old)<   symbol
                                            ,   processheap_allocator<symbol>
                                            >           symbols_buffer_type;

    static ws_size_t    calc_items_(char_type const* p, char_type const** q, ws_int_t flags);
    static void         prepare_items_(symbols_buffer_type &symbols, environment_buffer_type &environment, char_type *p, char_type *q, ws_int_t flags);
private:
    static ws_int_t     compare_strings_(char_type const* s1, char_type const* s2, ws_int_t flags);

public:
    /// A function class that compares environment symbols for the basic_environment_sequence class
    // [[synesis:class:binary-functor: compare_symbol]]
    struct compare_symbol
        : stlsoft_ns_qual_std(binary_function)<symbol, symbol, ws_bool_t>
//      , stlsoft_ns_qual(base_property)<ws_bool_t, 0>
    {
    public:
        ss_explicit_k compare_symbol(ws_bool_t bIgnoreCase = true)
            : m_bIgnoreCase(bIgnoreCase)
//          , stlsoft_ns_qual(base_property)<ws_bool_t, 0>(b
        {}

    public:
        /// Function call operator, which returns \c true if \c lhs is lexicographically less than \c rhs
        ws_bool_t operator ()(symbol const& lhs, symbol const& rhs)
        {
            return compare_strings_(lhs.name, rhs.name, m_bIgnoreCase ? ignoreCase : 0) < 0;
        }

    private:
        ws_bool_t   m_bIgnoreCase;
    };

    friend struct compare_symbol; // This is needed by Borland, CodeWarrior, DMC++ and Visual C++

private:
    static ws_int_t         validate_flags_(ws_int_t flags);
    static char_type const* get_environment_strings_();
    static void             free_environment_strings_(char_type *);

private:
    const ws_int_t          m_flags;        // The flags as specified to the ctor
    C const                 *m_p;           // Pointer to the start of the raw environment block
    C const                 *m_q;           // Pointer to the (one off the) end of the raw environment block
    symbols_buffer_type     m_symbols;      // Array of symbols representing the parsed environment block
    environment_buffer_type m_environment;  // The editable (and edited) copy of the environment block

// Not to be implemented
private:
    basic_environment_sequence(class_type const&);
    class_type const& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** \brief Specialisation of the basic_environment_sequence template for the ANSI character type \c char
 *
 * \ingroup group__library__system
 */
typedef basic_environment_sequence<ws_char_a_t>     environment_sequence_a;
/** \brief Specialisation of the basic_environment_sequence template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__system
 */
typedef basic_environment_sequence<ws_char_w_t>     environment_sequence_w;
/** \brief Specialisation of the basic_environment_sequence template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__system
 */
typedef basic_environment_sequence<TCHAR>           environment_sequence;

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k C>
inline /* static */ ss_typename_type_ret_k basic_environment_sequence<C>::size_type basic_environment_sequence<C>::calc_items_(ss_typename_type_k basic_environment_sequence<C>::char_type const* p, ss_typename_type_k basic_environment_sequence<C>::char_type const** q, ws_int_t flags)
{
    size_type           c;
    char_type const*    v;

    for(c = 0, v = p;; ++p)
    {
        if(*p == 0) // End of an environment variable?
        {
            if( showHidden == (showHidden & flags) ||
                '=' != v[0])
            {
                ++c;
            }

            v = p + 1;

            if(*(p + 1) == 0) // End of all environment variable
            {
                *q = p + 1;
                break;
            }
        }
    }

    return c;
}


template <ss_typename_param_k C>
inline /* static */ void basic_environment_sequence<C>::prepare_items_(ss_typename_type_k basic_environment_sequence<C>::symbols_buffer_type &symbols, ss_typename_type_k basic_environment_sequence<C>::environment_buffer_type &environment, ss_typename_type_k basic_environment_sequence<C>::char_type *p, ss_typename_type_k basic_environment_sequence<C>::char_type* q, ws_int_t flags)
{
    ss_typename_type_k environment_buffer_type::iterator    env_begin   =   environment.begin();
    ss_typename_type_k symbols_buffer_type::iterator        sym_begin   =   symbols.begin();
    char_type*                                              begin       =   p;
    char_type *const                                        end         =   q;
    char_type*                                              last_src    =   begin;
    char_type*                                              last_dest   =   &*env_begin; // Need this because may be using VC7.0 style pointer iterators

    // This loop does two things in one go (for efficiency).
    //
    // Firstly, it copies the source (which is read-only) to the m_environment
    // buffer.
    //
    // Secondly, it processes the source, and adjusts the m_symbols and
    // m_environment contents accordingly.
    for(; begin != end;)
    {
        *env_begin = *begin;

        if(*begin == 0)
        {
            const ws_bool_t bHidden =   ('=' == last_dest[0]);

            sym_begin->name  =   last_dest;
            for(; last_src != begin; ++last_src, ++last_dest)
            {
                if( *last_src == '=' &&
                    (   !bHidden ||
                        sym_begin->name != last_dest))
                {
                    *last_dest = '\0';
//                      ++last_src;
                    ++last_dest;
                    break;
                }
            }
            sym_begin->value    =   last_dest;
            last_src            =   ++begin;
            last_dest           =   &*++env_begin;

            if( showHidden == (showHidden & flags) ||
                !bHidden)
            {
                ++sym_begin;
            }
        }
        else
        {
            ++begin;
            ++env_begin;
        }
    }

    if(0 == (noSort & flags))
    {
        winstl_ns_qual_std(sort)(symbols.begin(), symbols.end(), compare_symbol());
    }
}

template <ss_typename_param_k C>
inline /* static */ ws_int_t basic_environment_sequence<C>::validate_flags_(ws_int_t flags)
{
    const ws_int_t  validFlags  =   0
                                |   showHidden
                                |   noSort
                                |   ignoreCase
                                |   0;

    WINSTL_MESSAGE_ASSERT("Specification of unrecognised/unsupported flags", flags == (flags & validFlags));
    STLSOFT_SUPPRESS_UNUSED(validFlags);

    return flags;
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ basic_environment_sequence<ws_char_a_t>::char_type const* basic_environment_sequence<ws_char_a_t>::get_environment_strings_()
{
    return static_cast<ws_char_a_t const*>(::GetEnvironmentStringsA());
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ basic_environment_sequence<ws_char_w_t>::char_type const* basic_environment_sequence<ws_char_w_t>::get_environment_strings_()
{
    return static_cast<ws_char_w_t const*>(::GetEnvironmentStringsW());
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ void basic_environment_sequence<ws_char_a_t>::free_environment_strings_(basic_environment_sequence<ws_char_a_t>::char_type *s)
{
    ::FreeEnvironmentStringsA(s);
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ void basic_environment_sequence<ws_char_w_t>::free_environment_strings_(basic_environment_sequence<ws_char_w_t>::char_type *s)
{
    ::FreeEnvironmentStringsW(s);
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ ws_int_t basic_environment_sequence<ws_char_a_t>::compare_strings_(ws_char_a_t const* s1, ws_char_a_t const* s2, ws_int_t flags)
{
    typedef system_traits<ws_char_a_t>  traits_t;

    return (ignoreCase & flags) ? traits_t::str_compare_no_case(s1, s2) : traits_t::str_compare(s1, s2);
}

STLSOFT_TEMPLATE_SPECIALISATION
inline /* static */ ws_int_t basic_environment_sequence<ws_char_w_t>::compare_strings_(ws_char_w_t const* s1, ws_char_w_t const* s2, ws_int_t flags)
{
    typedef system_traits<ws_char_w_t>  traits_t;

    return (ignoreCase & flags) ? traits_t::str_compare_no_case(s1, s2) : traits_t::str_compare(s1, s2);
}

template <ss_typename_param_k C>
inline /* ss_explicit_k */ basic_environment_sequence<C>::basic_environment_sequence(ws_int_t flags)
    : m_flags(validate_flags_(flags))
    , m_p(get_environment_strings_())
    , m_symbols(calc_items_(m_p, &m_q, m_flags))
    , m_environment(static_cast<ss_size_t>(m_q - m_p))
{
    prepare_items_(m_symbols, m_environment, const_cast<char_type*>(m_p), const_cast<char_type*>(m_q), flags);
}

template <ss_typename_param_k C>
inline basic_environment_sequence<C>::~basic_environment_sequence() stlsoft_throw_0()
{
    // The documentation for FreeEnvironmentStrings does not explicitly state
    // that it is legal to free a null string, so we must do the test.
    if(0 != m_p)
    {
        free_environment_strings_(const_cast<char_type*>(m_p));
    }
}

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_iterator basic_environment_sequence<C>::begin() const
{
    return &*m_symbols.begin();
}

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_iterator basic_environment_sequence<C>::end() const
{
    return &*m_symbols.end();
}

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_reverse_iterator basic_environment_sequence<C>::rbegin() const
{
    return const_reverse_iterator(end());
}

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_reverse_iterator basic_environment_sequence<C>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_iterator basic_environment_sequence<C>::find(ss_typename_type_k basic_environment_sequence<C>::char_type const* name) const
{
    const_iterator  b   =   this->begin();
    const_iterator  e   =   this->end();

    for(; b != e; ++b)
    {
        if(0 == compare_strings_(name, (*b).name, m_flags))
        {
            break;
        }
    }

    return b;
}

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::const_iterator basic_environment_sequence<C>::find(ss_typename_type_k basic_environment_sequence<C>::char_type const* name, ss_typename_type_k basic_environment_sequence<C>::char_type const* value) const
{
    const_iterator  b   =   this->begin();
    const_iterator  e   =   this->end();

    for(; b != e; ++b)
    {
        if( 0 == compare_strings_(name, (*b).name, m_flags) &&
            (   NULL == value ||
                0 == compare_strings_(value, (*b).value, m_flags)))
        {
            break;
        }
    }

    return b;
}


template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::size_type basic_environment_sequence<C>::size() const
{
    return m_symbols.size();
}

template <ss_typename_param_k C>
inline ws_bool_t basic_environment_sequence<C>::empty() const
{
    return size() == 0;
}

template <ss_typename_param_k C>
inline ss_typename_type_ret_k basic_environment_sequence<C>::value_type basic_environment_sequence<C>::operator [](ss_typename_type_k basic_environment_sequence<C>::size_type index) const
{
    WINSTL_MESSAGE_ASSERT("index access out of range in basic_environment_sequence", index < size() + 1);   // Has to be +1, since legitimate to take address of one-past-the-end

    return m_symbols.data()[index];
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/environment_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
