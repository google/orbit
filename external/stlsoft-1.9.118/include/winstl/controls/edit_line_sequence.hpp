/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/edit_line_sequence.hpp
 *
 * Purpose:     Contains the edit_line_sequence class.
 *
 * Created:     23rd March 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/controls/edit_line_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::edit_line_sequence class
 *  template
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE
#define WINSTL_INCL_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE_MAJOR    1
# define WINSTL_VER_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE_MINOR    0
# define WINSTL_VER_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE_REVISION 7
# define WINSTL_VER_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE_EDIT     14
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_CONTROLS_H_FUNCTIONS
# include <winstl/controls/functions.h>      // for combobox_addstring(), etc.
#endif /* !WINSTL_INCL_WINSTL_CONTROLS_H_FUNCTIONS */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
# include <stlsoft/string/simple_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

#ifndef STLSOFT_INCL_VECTOR
# define STLSOFT_INCL_VECTOR
# include <vector>
#endif /* !STLSOFT_INCL_VECTOR */

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

/** \brief STL-like sequence for edit control contents
 *
 * \ingroup group__library__windows_controls
 *
 * This class presents an STL-like sequence interface to an edit control
 *
 * \param C The character type
 * \param A The allocator type. Defaults to winstl::processheap_allocator
 * \param S The string type. Defaults to stlsoft::simple_string
 * \param Q The queue container type. Defaults to std::vector
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A = processheap_allocator<C>
        ,   ss_typename_param_k S = stlsoft_ns_qual(basic_simple_string)<C, stlsoft_ns_qual(char_traits)<C>, A>
        ,   ss_typename_param_k Q = stlsoft_ns_qual_std(vector)<S>
        >
class edit_line_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// \brief The character type
    typedef C                                                       char_type;
    /// \brief The allocator type
    typedef A                                                       allocator_type;
    /// \brief The string type
    typedef S                                                       string_type;
    /// \brief The value type
    typedef string_type                                             value_type;
    /// \brief The container type
    typedef Q                                                       queue_type;
    /// \brief The current specialisation of this type
    typedef edit_line_sequence<S>                                   class_type;
    /// \brief The non-mutating (const) iterator type
    typedef ss_typename_type_k queue_type::const_iterator           const_iterator;
    /// \brief The non-mutating (const) reverse iterator type
    typedef ss_typename_type_k queue_type::const_reverse_iterator   const_reverse_iterator;
    /// The non-mutable (const) reference type
    typedef ss_typename_type_k const_iterator::const_reference      const_reference;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs on the given edit control
    ss_explicit_k edit_line_sequence(HWND hwndEdit, unsigned = 0);
/// @}

/// \name Iteration methods
/// @{
public:
    const_iterator          begin() const;
    const_iterator          end() const;
    const_reverse_iterator  rbegin() const;
    const_reverse_iterator  rend() const;
/// @}

/// \name Members
/// @{
private:
    HWND        m_hwndEdit;
    queue_type  m_queue;
/// @}
};


template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ws_bool_t operator ==(ss_typename_type_k edit_line_sequence<C, A, S, Q>::const_iterator const& lhs, ss_typename_type_k edit_line_sequence<C, A, S, Q>::const_iterator const& rhs)
{
    return lhs.equal(rhs);
}

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ws_bool_t operator !=(ss_typename_type_k edit_line_sequence<C, A, S, Q>::const_iterator const& lhs, ss_typename_type_k edit_line_sequence<C, A, S, Q>::const_iterator const& rhs)
{
    return !lhs.equal(rhs);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/edit_line_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline /* ss_explicit_k */ edit_line_sequence<C, A, S, Q>::edit_line_sequence(HWND hwndEdit, unsigned /* = 0 */)
    : m_hwndEdit(hwndEdit)
{
    WINSTL_ASSERT(NULL != hwndEdit);
    WINSTL_ASSERT(::IsWindow(hwndEdit));
    WINSTL_MESSAGE_ASSERT("Edit control must have both ES_MULTILINE and ES_AUTOHSCROLL styles", (ES_MULTILINE | ES_AUTOHSCROLL) == ((ES_MULTILINE | ES_AUTOHSCROLL) & ::GetWindowLong(hwndEdit, GWL_STYLE)));

    queue_type  queue;
    int         numLines    =   edit_getcount(hwndEdit);

    { int   charTotal = 0;
        for(int i = 0; i < numLines; ++i)
    {
        int                                 charIndex   =   charTotal + i;
        int                                 lineLength  =   edit_linelength(hwndEdit, charIndex);
        stlsoft_ns_qual(auto_buffer)<char>  line(1 + lineLength);

        lineLength  =   edit_getline(hwndEdit, i, &line[0], line.size());

        line[lineLength] = '\0';

        queue.push_back(line.data());

        charTotal += lineLength + 2;
    }}

    m_queue.swap(queue);
}

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ss_typename_type_ret_k edit_line_sequence<C, A, S, Q>::const_iterator edit_line_sequence<C, A, S, Q>::begin() const
{
    return m_queue.begin();
}

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ss_typename_type_ret_k edit_line_sequence<C, A, S, Q>::const_iterator edit_line_sequence<C, A, S, Q>::end() const
{
    return m_queue.end();
}

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ss_typename_type_ret_k edit_line_sequence<C, A, S, Q>::const_reverse_iterator edit_line_sequence<C, A, S, Q>::rbegin() const
{
    return const_reverse_iterator(end());
}

template<ss_typename_param_k C, ss_typename_param_k A, ss_typename_param_k S, ss_typename_param_k Q>
inline ss_typename_type_ret_k edit_line_sequence<C, A, S, Q>::const_reverse_iterator edit_line_sequence<C, A, S, Q>::rend() const
{
    return const_reverse_iterator(begin());
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

#endif /* WINSTL_INCL_WINSTL_CONTROLS_HPP_EDIT_LINE_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
