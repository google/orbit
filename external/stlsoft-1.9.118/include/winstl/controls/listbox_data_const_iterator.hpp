/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/listbox_data_const_iterator.hpp
 *
 * Purpose:     Contains the listbox_data_const_iterator class.
 *
 * Created:     16th March 2008
 * Updated:     29th March 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2008-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/controls/listbox_data_const_iterator.hpp
 *
 * \brief [C++ only] Definition of the winstl::listbox_data_const_iterator class
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR
#define WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR_MAJOR       1
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR_MINOR       1
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR_REVISION    1
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR_EDIT        5
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_ERROR_HPP_EXTERNAL_ITERATOR_INVALIDATION
# include <stlsoft/error/external_iterator_invalidation.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_ERROR_HPP_EXTERNAL_ITERATOR_INVALIDATION */
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

/** \brief Iterator for listbox_sequence class
 *
 * \ingroup group__library__windows_controls
 *
 * This class acts as the iterator for enumerating the data elements of a
 * listbox_sequence class, and implements the Random Access Iterator concept
 *
 * \param CT The control traits type
 */
template <ss_typename_param_k CT>
class listbox_data_const_iterator
    : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(random_access_iterator_tag)
                                        ,   DWORD
                                        ,   ws_ptrdiff_t
                                        ,   void
                                        ,   DWORD   // BVT
                                        >
{
/// \name Member Types
/// @{
public:
    /// The value type
    typedef DWORD                                                           value_type;
    /// The current parameterisation of the type
    typedef listbox_data_const_iterator<CT>                                 class_type;
    /// The size type
    typedef ws_size_t                                                       size_type;
    /// The difference type
    typedef ws_ptrdiff_t                                                    difference_type;
    /// The non-mutating (const) reference type
    typedef value_type const                                                const_reference;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// \brief The non-mutating (const) reverse iterator type
    ///
    /// This typedef is provided as a convenience for the sequence.
    ///
    /// \note Even though the iterator supports Transient semantics, the
    /// reverse iterator <b>must</b> be <i>By-Value Temporary</i>, since
    /// there is no underlying range of values of which the offset-by-one
    /// mechanism of the std::reverse_iterator adaptor can access and
    /// return a valid reference.
    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   class_type
                                                        ,   value_type
                                                        ,   value_type  // By-Value Temporary element references
                                                        ,   void        // By-Value Temporary element references
                                                        ,   difference_type
                                                        >                   const_reverse_iterator_type;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The control traits type
    typedef CT                                                              control_traits_type;
/// @}

/// \name Construction
/// @{
public:
    /// Construct an instance from the list-box \c hwndListBox at the given \c index
    listbox_data_const_iterator(HWND hwndListBox, int index)
        : m_hwnd(hwndListBox)
        , m_index(index)
    {}
/// @}

/// \name Forward Iterator Methods
/// @{
public:
    /// Dereferences the iterator and returns a reference to the current value
    const_reference operator *() const
    {
        return get_value_at_(m_hwnd, m_index);
    }

    /// Pre-increment operator
    class_type& operator ++()
    {
        ++m_index;

        return *this;
    }

    /// Post-increment operator
    class_type operator ++(int)
    {
        class_type  ret(*this);

        operator ++();

        return ret;
    }

    /// Compares \c this and \c rhs for equivalence
    difference_type compare(class_type const& rhs) const
    {
        WINSTL_MESSAGE_ASSERT("invalid comparison between iterators from different ranges", (m_hwnd == rhs.m_hwnd || NULL == m_hwnd || NULL == rhs.m_hwnd));

        return m_index - rhs.m_index;
    }

    /// Indicates whether \c this and \c rhs are equivalent
    bool operator == (class_type const& rhs) const
    {
        return 0 == compare(rhs);
    }

    /// Indicates whether \c this and \c rhs are not equivalent
    bool operator != (class_type const& rhs) const
    {
        return 0 != compare(rhs);
    }
/// @}

/// \name Bidirectional Iterator methods
/// @{
public:
    /// Pre-decrement operator
    class_type& operator --()
    {
        --m_index;

        return *this;
    }

    /// Post-decrement operator
    class_type operator --(int)
    {
        class_type  ret(*this);

        operator --();

        return ret;
    }
/// @}

/// \name Random Access Iterator methods
/// @{
public:
    /// Offset
    class_type& operator +=(difference_type index)
    {
        m_index += index;

        return *this;
    }

    /// Offset
    class_type& operator -=(difference_type index)
    {
        m_index -= index;

        return *this;
    }

    /// Subscript operator
    value_type operator [](difference_type index) const
    {
        // PA: Emulate pointer-like operation where it[0] returns the current item's value
        return get_value_at_(m_hwnd, m_index + index);
    }

    /// Calculate the distance between \c this and \c rhs
    difference_type distance(class_type const& rhs) const
    {
        return m_index - rhs.m_index;
    }

    /// Pointer subtraction
    class_type operator -(difference_type n) const
    {
        return class_type(*this) -= n;
    }

    /// Pointer addition
    class_type operator +(difference_type n) const
    {
        return class_type(*this) += n;
    }

    /// Pointer difference
    difference_type operator -(class_type const& rhs) const
    {
        return distance(rhs);
    }

    /// Indicates whether \c this is less than \c rhs
    ws_bool_t operator <(class_type const& rhs) const
    {
        return compare(rhs) < 0;
    }

    /// Indicates whether \c this is greater than \c rhs
    ws_bool_t operator >(class_type const& rhs) const
    {
        return compare(rhs) > 0;
    }

    /// Indicates whether \c this is less than or equal \c rhs
    ws_bool_t operator <=(class_type const& rhs) const
    {
        return compare(rhs) <= 0;
    }

    /// Indicates whether \c this is greater than or equal \c rhs
    ws_bool_t operator >=(class_type const& rhs) const
    {
        return compare(rhs) >= 0;
    }
/// @}

/// \name Implementation
/// @{
public:
    static value_type get_value_at_(HWND hwnd, difference_type index)
    {
        WINSTL_MESSAGE_ASSERT("Invalid index", index >= 0);

        return control_traits_type::get_data(hwnd, index);
    }
/// @}

/// \name Members
/// @{
private:
    HWND    m_hwnd;
    int     m_index;
/// @}
};

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

#endif /* WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTBOX_DATA_CONST_ITERATOR */

/* ///////////////////////////// end of file //////////////////////////// */
