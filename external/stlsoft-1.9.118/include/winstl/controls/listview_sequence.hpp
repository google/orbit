/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/listview_sequence.hpp
 *
 * Purpose:     Contains the listview_sequence class template.
 *
 * Created:     8th May 2003
 * Updated:     29th March 2010
 *
 * Thanks:      To Pablo Aguilar for making the requisite feature requests.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/controls/listview_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::listview_sequence class
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE
#define WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE_MAJOR     4
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE_MINOR     3
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE_REVISION  2
# define WINSTL_VER_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE_EDIT      76
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
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING
# include <stlsoft/string/shim_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SHIM_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
# include <stlsoft/string/simple_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef WINSTL_INCL_WINSTL_CONTROLS_H_COMMCTRL_FUNCTIONS
# include <winstl/controls/commctrl_functions.h>
#endif /* !WINSTL_INCL_WINSTL_CONTROLS_H_COMMCTRL_FUNCTIONS */
#ifdef WINSTL_LISTVIEW_SEQUENCE_CUSTOM_STRING_TYPE
typedef WINSTL_LISTVIEW_SEQUENCE_CUSTOM_STRING_TYPE lvs_string_t;
#else /* ? WINSTL_LISTVIEW_SEQUENCE_CUSTOM_STRING_TYPE */
# ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
typedef stlsoft_ns_qual(basic_simple_string)<TCHAR> lvs_string_t;
# else
typedef stlsoft_ns_qual(basic_simple_string)<   TCHAR
                                            ,   stlsoft_ns_qual(stlsoft_char_traits)<TCHAR>
                                            ,   winstl_ns_qual(processheap_allocator)<TCHAR>
                                            >       lvs_string_t;
# endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
#endif /* WINSTL_LISTVIEW_SEQUENCE_CUSTOM_STRING_TYPE */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */

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
 * Typedefs
 */

#ifndef _WINSTL_NO_NAMESPACE
using ::lvs_string_t;
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Item class used by the listview_sequence class
 *
 * \ingroup group__library__windows_controls
 */
//template<ss_typename_param_k S = lvs_string_t>
class listview_sequence_item
{
public:
//  typedef S           string_type;

public:
    listview_sequence_item(HWND hwndListView, int iIndex)
        : m_hwndListView(hwndListView)
        , m_index(iIndex)
    {}

public:
    lvs_string_t text(int iSubItem = 0) const
    {
        typedef stlsoft_ns_qual(auto_buffer_old)<   TCHAR
                                                ,   processheap_allocator<TCHAR>
                                                ,   256
                                                >       buffer_t;

        ws_size_t   cb  =   buffer_t::internal_size();
        LV_ITEM     item;

        item.mask       =   LVIF_TEXT;
        item.iItem      =   m_index;
        item.iSubItem   =   iSubItem;

        for(;; cb += buffer_t::internal_size())
        {
            buffer_t    buffer(cb);

            item.cchTextMax =   static_cast<int>(cb);
            item.pszText    =   &buffer[0];

            if(!ListView_GetItem(m_hwndListView, &item))
            {
                item.cchTextMax = 0;
                break;
            }
            else
            {
                ss_size_t len =   static_cast<ss_size_t>(lstrlen(item.pszText));

                if(len + 1 < cb)
                {
                    return lvs_string_t(item.pszText, len);
                }
            }
        }

        return lvs_string_t();
    }

    int             index() const
    {
        return m_index;
    }
    HWND            hwnd() const
    {
        return m_hwndListView;
    }

    int             image() const;
    int             selected_image() const;
    ws_dword_t      data() const
    {
        LV_ITEM item;

        item.mask       =   LVIF_PARAM;
        item.iItem      =   m_index;
        item.iSubItem   =   0;

        return ListView_GetItem(m_hwndListView, &item) ? static_cast<ws_dword_t>(item.lParam) : 0;
    }
    /// The item's state
    UINT            state(UINT mask = 0xffffffff) const
    {
        return ListView_GetItemState(m_hwndListView, m_index, mask);
    }

private:
    HWND    m_hwndListView;
    int     m_index;
};


/** \brief Provides an STL-like sequence over the contents of a Windows List-view (<code>"SysListView32"</code>)
 *
 * \ingroup group__library__windows_controls
 */
class listview_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// The value type
    typedef listview_sequence_item  sequence_value_type;
    typedef sequence_value_type     value_type;
    /// The size type
    typedef ss_size_t               size_type;
    /// The difference type
    typedef ws_ptrdiff_t            difference_type;
    ///
    typedef listview_sequence       sequence_class_type;


public:
    ss_explicit_k listview_sequence(HWND hwndListView)
        : m_hwndListView(hwndListView)
    {}

public:
    /// const_iterator for the listview_sequence
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(random_access_iterator_tag)
                                            ,   sequence_value_type
                                            ,   ws_ptrdiff_t
                                            ,   void                // By-Value Temporary reference
                                            ,   sequence_value_type // By-Value Temporary reference
                                            >
    {
        typedef const_iterator                  class_type;
    public:
        typedef sequence_value_type             value_type;

    public:
        const_iterator()
            : m_hwndListView(NULL)
            , m_index(-1)
        {}
        const_iterator(HWND hwndListView, int iIndex)
            : m_hwndListView(hwndListView)
            , m_index(iIndex)
        {}
        const_iterator(class_type const& rhs)
            : m_hwndListView(rhs.m_hwndListView)
            , m_index(rhs.m_index)
        {}

        class_type& operator =(class_type const& rhs)
        {
            m_hwndListView  =   rhs.m_hwndListView;
            m_index         =   rhs.m_index;

            return *this;
        }

    public:
        /// Dereference operator
        value_type operator *() const
        {
            return value_type(m_hwndListView, m_index);
        }

        bool operator ==(class_type const& rhs) const
        {
            WINSTL_MESSAGE_ASSERT("Comparing iterators from different listview_sequence instances!", m_hwndListView == rhs.m_hwndListView);

            return m_index == rhs.m_index;
        }

        bool operator !=(class_type const& rhs) const
        {
            WINSTL_MESSAGE_ASSERT("Comparing iterators from different listview_sequence instances!", m_hwndListView == rhs.m_hwndListView);

            return m_index != rhs.m_index;
        }

        /// Pre-increment operator
        class_type& operator ++()
        {
            WINSTL_MESSAGE_ASSERT("Attempting to increment an off-the-end iterator", m_index < ListView_GetItemCount(m_hwndListView));

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

        /// Pre-decrement operator
        class_type& operator --()
        {
            WINSTL_MESSAGE_ASSERT("Attempting to decrement an iterator at the start of the sequence", 0 < m_index);

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

        // Random access operations

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
            return value_type(m_hwndListView, m_index + index);
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

    // Members
    private:
        HWND    m_hwndListView;
        int     m_index;
    };

#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The non-mutating (const) reverse iterator type
    typedef stlsoft_ns_qual(const_reverse_iterator_base)<   const_iterator
                                                        ,   value_type
                                                        ,   value_type  // By-Value Temporary reference category
                                                        ,   void        // By-Value Temporary reference category
                                                        ,   difference_type
                                                        >   const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */

// State
public:
    /// Returns the number of elements in the list-box
    size_type size() const
    {
        return static_cast<size_type>(ListView_GetItemCount(m_hwndListView));
    }
    /// Indicates whether the list-box is empty
    ws_bool_t empty() const
    {
        return 0 == size();
    }
    /// Returns the maximum number of items that the list-box can contain
    static size_type max_size()
    {
        return static_cast<size_type>(-1) / sizeof(LPCTSTR);
    }

// Iteration
public:
    const_iterator  begin() const
    {
        return const_iterator(m_hwndListView, 0);
    }
    const_iterator  end() const
    {
        return const_iterator(m_hwndListView, static_cast<int>(size()));
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

// Accessors
public:
    value_type operator [](size_type index) const
    {
        return value_type(m_hwndListView, static_cast<int>(index));
    }

private:
    HWND    m_hwndListView;
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_ptr_null(listview_sequence_item const& lvi)
{
    return stlsoft_ns_qual(c_str_ptr_null)(lvi.text());
}
#ifdef UNICODE
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_ptr_null_w(listview_sequence_item const& lvi)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_ptr_null_a(listview_sequence_item const& lvi)
#endif /* UNICODE */
{
    return stlsoft_ns_qual(c_str_ptr_null)(lvi.text());
}

inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, false, processheap_allocator<TCHAR> > c_str_ptr(listview_sequence_item const& lvi)
{
    return stlsoft_ns_qual(c_str_ptr)(lvi.text());
}
#ifdef UNICODE
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_ptr_w(listview_sequence_item const& lvi)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_ptr_a(listview_sequence_item const& lvi)
#endif /* UNICODE */
{
    return stlsoft_ns_qual(c_str_ptr)(lvi.text());
}

inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, false, processheap_allocator<TCHAR> > c_str_data(listview_sequence_item const& lvi)
{
    return stlsoft_ns_qual(c_str_data)(lvi.text());
}
#ifdef UNICODE
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_data_w(listview_sequence_item const& lvi)
#else /* ? UNICODE */
inline stlsoft_ns_qual(basic_shim_string)<TCHAR, 64, true, processheap_allocator<TCHAR> > c_str_data_a(listview_sequence_item const& lvi)
#endif /* UNICODE */
{
    return stlsoft_ns_qual(c_str_data)(lvi.text());
}

inline ws_size_t c_str_len(listview_sequence_item const& lvi)
{
    return stlsoft_ns_qual(c_str_len)(lvi.text());
}



template<   ss_typename_param_k S
        >
inline S& operator <<(S& s, listview_sequence_item const& lvi)
{
    s << lvi.text();

    return s;
}

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

using ::winstl::c_str_ptr_null;
using ::winstl::c_str_ptr_null_a;
using ::winstl::c_str_ptr_null_w;

using ::winstl::c_str_ptr;
using ::winstl::c_str_ptr_a;
using ::winstl::c_str_ptr_w;

using ::winstl::c_str_data;
using ::winstl::c_str_data_a;
using ::winstl::c_str_data_w;

using ::winstl::c_str_len;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_CONTROLS_HPP_LISTVIEW_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
