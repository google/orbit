/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/combobox_sequence.hpp
 *
 * Purpose:     Contains the combobox_sequence class.
 *
 * Created:     13th November 2002
 * Updated:     10th August 2009
 *
 * Thanks:      To Pablo Aguilar for some patches.
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


/** \file winstl/controls/combobox_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::combobox_sequence class
 *  template
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE
#define WINSTL_INCL_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE_MAJOR      3
# define WINSTL_VER_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE_MINOR      4
# define WINSTL_VER_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE_REVISION   2
# define WINSTL_VER_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE_EDIT       60
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTBOX_CONST_ITERATOR
# include <winstl/controls/listbox_const_iterator.hpp>
#endif /* !WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTBOX_CONST_ITERATOR */
#ifndef WINSTL_INCL_WINSTL_CONTROLS_H_FUNCTIONS
# include <winstl/controls/functions.h>     // for combobox_addstring(), etc.
#endif /* !WINSTL_INCL_WINSTL_CONTROLS_H_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct combobox_operation_traits
{
public:
    static ws_int_t err_constant()
    {
        return CB_ERR;
    }
    static ws_int_t get_count(HWND hwnd)
    {
        return combobox_getcount(hwnd);
    }
    static ws_int_t get_text_len(HWND hwnd, ws_int_t index)
    {
        return combobox_gettextlen(hwnd, index);
    }
    static ws_int_t get_text(HWND hwnd, ws_int_t index, ws_char_a_t *s)
    {
        return combobox_gettext_a(hwnd, index, s);
    }
    static ws_int_t get_text(HWND hwnd, ws_int_t index, ws_char_w_t *s)
    {
        return combobox_gettext_w(hwnd, index, s);
    }
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief STL-like sequence for combo-box contents
 *
 * \ingroup group__library__windows_controls
 *
 * This class presents an STL-like sequence interface to a combo-box
 *
 * \param S The string type
 */
template <ss_typename_param_k S>
class combobox_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// The current parameterisation of the type
    typedef combobox_sequence<S>                                            class_type;
    /// The non-mutating (const) iterator type
    typedef listbox_const_iterator<S, combobox_operation_traits>            const_iterator;
    /// The character type
    typedef ss_typename_type_k const_iterator::char_type                    char_type;
    /// The value type
    typedef ss_typename_type_k const_iterator::value_type                   value_type;
    /// The non-mutable (const) reference type
    typedef ss_typename_type_k const_iterator::const_reference              const_reference;
    /// The size type
    typedef ss_typename_type_k const_iterator::size_type                    size_type;
    /// The difference type
    typedef ss_typename_type_k const_iterator::difference_type              difference_type;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The non-mutating (const) reverse iterator type
    typedef ss_typename_type_k const_iterator::const_reverse_iterator_type  const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The allocator type
    typedef ss_typename_type_k const_iterator::allocator_type               allocator_type;
private:
    typedef ss_typename_type_k const_iterator::buffer_type                  buffer_type;
    typedef combobox_operation_traits                                       control_traits_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs from the given combo-box
    ss_explicit_k combobox_sequence(HWND hwndComboBox)
        : m_hwnd(hwndComboBox)
    {}
/// @}

/// \name State
/// @{
public:
    /// Returns the number of elements in the combo-box
    size_type size() const
    {
        return static_cast<size_type>(control_traits_type::get_count(m_hwnd));
    }
    /// Indicates whether the combo-box is empty
    ws_bool_t empty() const
    {
        return size() == 0;
    }
    /// Returns the maximum number of items that the combo-box can contain
    static size_type max_size()
    {
        return static_cast<size_type>(-1) / sizeof(LPCTSTR);
    }
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator  begin() const
    {
        return const_iterator(m_hwnd, 0);
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const
    {
        return const_iterator(m_hwnd, static_cast<int>(size()));
    }

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// Begins the reverse iteration
    ///
    /// \return An iterator representing the start of the reverse sequence
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    /// Ends the reverse iteration
    ///
    /// \return An iterator representing the end of the reverse sequence
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Element Access
/// @{
public:
    /// Returns the item at the given index
    value_type operator [](difference_type index) const
    {
        return const_iterator::get_value_at_(m_hwnd, index);
    }
/// @}

/// \name Members
/// @{
private:
    HWND    m_hwnd;
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/combobox_sequence_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_CONTROLS_HPP_COMBOBOX_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
