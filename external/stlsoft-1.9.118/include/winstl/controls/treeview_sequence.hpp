/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/controls/treeview_sequence.hpp
 *
 * Purpose:     Contains the treeview sequence classes.
 *
 * Created:     1st December 2002
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


/** \file winstl/controls/treeview_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::treeview_child_sequence,
 *  winstl::treeview_peer_sequence and winstl::treeview_visible_sequence
 *  classes, and their supporting types
 *   (\ref group__library__windows_controls "Windows Controls" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE
#define WINSTL_INCL_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE_MAJOR     4
# define WINSTL_VER_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE_MINOR     0
# define WINSTL_VER_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE_REVISION  6
# define WINSTL_VER_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE_EDIT      71
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
#ifndef WINSTL_INCL_WINSTL_CONTROLS_H_COMMCTRL_FUNCTIONS
# include <winstl/controls/commctrl_functions.h>    // for treeview_getnext(), etc.
#endif /* !WINSTL_INCL_WINSTL_CONTROLS_H_COMMCTRL_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

/* ////////////////////////////////////////////////////////////////////// */

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

/** \brief Iterator for the treeview_child_sequence, treeview_peer_sequence and treeview_visible_sequence classes
 *
 * \ingroup group__library__windows_controls
 *
 * \param N The windows message that is used to access the next element in the iteration sequence
 * \param P The windows message that is used to access the previous element in the iteration sequence
 */
template <UINT N, UINT P>
class treeview_sequence_const_iterator
    : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(forward_iterator_tag)
                                        ,   HTREEITEM
                                        ,   ws_ptrdiff_t
                                        ,   void        // By-Value Temporary reference category
                                        ,   HTREEITEM   // By-Value Temporary reference category
                                        >
{
/// \name Member Types
/// @{
public:
    /// The current parameterisation of the type
    typedef treeview_sequence_const_iterator<N, P>  class_type;
    /// The value type
    typedef HTREEITEM                               value_type;
    /// The difference type
    typedef ws_ptrdiff_t                            difference_type;
    /// The effective reference type
    typedef value_type                              effective_reference;
/// @}

/// \name Construction
/// @{
private:
#if 0
//    friend class treeview_sequence_base<N, P>;
    friend struct treeview_sequence_const_iterator_friend;
    friend class treeview_visible_sequence;
#else /* ? 0 */
public:
#endif /* 0 */
    treeview_sequence_const_iterator(HWND hwndTree, HTREEITEM hitem);
public:
    /// Default constructor
    treeview_sequence_const_iterator();
    /// Copy constructor
    treeview_sequence_const_iterator(class_type const& rhs);

    /// Copy assignment operator
    treeview_sequence_const_iterator& operator =(class_type const& rhs);
/// @}

/// \name Forward Iterator methods
/// @{
public:
    /// Derefences and returns the current item
    effective_reference operator *() const;
    /// Pre-increment
    class_type& operator ++();
    /// Post-increment
    class_type operator ++(int);

    /// Evaluates whether \c this and \c rhs are equivalent
    ws_bool_t operator ==(class_type const& rhs) const;
    /// Evaluates whether \c this and \c rhs are not equivalent
    ws_bool_t operator !=(class_type const& rhs) const;
/// @}

/// \name Members
/// @{
private:
    HWND        m_hwnd;
    HTREEITEM   m_hitem;
/// @}
};


/** \brief Base class for the treeview_child_sequence,
 *  treeview_peer_sequence and treeview_visible_sequence classes.
 *
 * \ingroup group__library__windows_controls
 */
template <UINT N, UINT P>
class treeview_sequence_base
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
#if defined(STLSOFT_COMPILER_IS_DMC) &&  \
    __DMC__ < 0x0840
public:
#else /* ? __DMC__ < 0x0840 */
protected:
#endif /* __DMC__ < 0x0840 */
    /// This class
    typedef treeview_sequence_base<N, P>                                class_type;
    /// The non-mutating (const) iterator type
    typedef treeview_sequence_const_iterator<N, P>                      const_iterator;
    /// The value type
    typedef ss_typename_type_k const_iterator::value_type               value_type;
    /// The difference type
    typedef ss_typename_type_k const_iterator::difference_type          difference_type;
/// @}

/// \name Construction
/// @{
protected:
    /// Constructs from the given tree and item
    treeview_sequence_base(HWND hwndTree, HTREEITEM hitem);
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator begin() const;
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator end() const;
/// @}

/// \name Members
/// @{
private:
    HWND        m_hwnd;
    HTREEITEM   m_hitem;
/// @}
};

// class treeview_child_sequence
/** \brief Presents an STL-like sequence interface to the children of a given node in a tree-view
 *
 * \ingroup group__library__windows_controls
 */
class treeview_child_sequence
    : public treeview_sequence_base<TVGN_NEXT, TVGN_PREVIOUS>
{
/// \name Member Types
/// @{
private:
    typedef treeview_sequence_base<TVGN_NEXT, TVGN_PREVIOUS>            base_class_type;
public:
    /// This class
    typedef treeview_child_sequence                                     class_type;
    /// The non-mutating (const) iterator type
    typedef base_class_type::const_iterator                             const_iterator;
    /// The value type
    typedef base_class_type::value_type                                 value_type;
    /// The difference type
    typedef base_class_type::difference_type                            difference_type;
/// @}

/// \name Construction
/// @{
public:
    /// Create sequence of the children of \c hitem in the given tree
    treeview_child_sequence(HWND hwndTree, HTREEITEM hitem);
    /// Create sequence of the children of the root in the given tree
    ss_explicit_k treeview_child_sequence(HWND hwndTree);
/// @}
};

// class treeview_peer_sequence
/** \brief brief Presents an STL-like sequence interface to the peers of a given node in a tree-view
 *
 * \ingroup group__library__windows_controls
 */
class treeview_peer_sequence
    : public treeview_sequence_base<TVGN_NEXT, TVGN_PREVIOUS>
{
/// \name Member Types
/// @{
private:
    typedef treeview_sequence_base<TVGN_NEXT, TVGN_PREVIOUS>            base_class_type;
public:
    /// This class
    typedef treeview_child_sequence                                     class_type;
    /// The non-mutating (const) iterator type
    typedef base_class_type::const_iterator                             const_iterator;
    /// The value type
    typedef base_class_type::value_type                                 value_type;
    /// The difference type
    typedef base_class_type::difference_type                            difference_type;
/// @}

/// \name Construction
/// @{
public:
    /// Create sequence of the peers of \c hitem in the given tree
    treeview_peer_sequence(HWND hwndTree, HTREEITEM hitem);
/// @}
};

// class treeview_visible_sequence
/** \brief Presents an STL-like sequence interface to the visible items in a tree-view
 *
 * \ingroup group__library__windows_controls
 */
class treeview_visible_sequence
    : public treeview_sequence_base<TVGN_NEXTVISIBLE, TVGN_PREVIOUSVISIBLE>
{
/// \name Member Types
/// @{
private:
    typedef treeview_sequence_base<TVGN_NEXTVISIBLE, TVGN_PREVIOUSVISIBLE>  base_class_type;
public:
    /// This class
    typedef treeview_child_sequence                                         class_type;
    /// The non-mutating (const) iterator type
    typedef base_class_type::const_iterator                                 const_iterator;
    /// The value type
    typedef base_class_type::value_type                                     value_type;
    /// The difference type
    typedef base_class_type::difference_type                                difference_type;
/// @}

/// \name Construction
/// @{
public:
    /// Create sequence of the visible items in the given tree
    ss_explicit_k treeview_visible_sequence(HWND hwndTree);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/treeview_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// treeview_sequence_const_iterator

template <UINT N, UINT P>
inline treeview_sequence_const_iterator<N, P>::treeview_sequence_const_iterator(HWND hwndTree, HTREEITEM hitem)
    : m_hwnd(hwndTree)
    , m_hitem(hitem)
{}

template <UINT N, UINT P>
inline treeview_sequence_const_iterator<N, P>::treeview_sequence_const_iterator()
    : m_hwnd(NULL)
    , m_hitem(NULL)
{}

template <UINT N, UINT P>
inline treeview_sequence_const_iterator<N, P>::treeview_sequence_const_iterator(treeview_sequence_const_iterator<N, P> const& rhs)
    : m_hwnd(rhs.m_hwnd)
    , m_hitem(rhs.m_hitem)
{}

template <UINT N, UINT P>
inline treeview_sequence_const_iterator<N, P> &treeview_sequence_const_iterator<N, P>::operator =(treeview_sequence_const_iterator<N, P> const& rhs)
{
    m_hwnd = rhs.m_hwnd;
    m_hitem = rhs.m_hitem;

    return *this;
}

template <UINT N, UINT P>
inline HTREEITEM treeview_sequence_const_iterator<N, P>::operator *() const
{
    return m_hitem;
}

template <UINT N, UINT P>
inline ss_typename_type_ret_k treeview_sequence_const_iterator<N, P>::class_type& treeview_sequence_const_iterator<N, P>::operator ++()
{
    if(m_hitem != NULL)
    {
        m_hitem = treeview_getnextitem(m_hwnd, m_hitem, N);
    }

    return *this;
}

template <UINT N, UINT P>
inline ss_typename_type_ret_k treeview_sequence_const_iterator<N, P>::class_type treeview_sequence_const_iterator<N, P>::operator ++(int)
{
    class_type  ret(*this);

    operator ++();

    return ret;
}

template <UINT N, UINT P>
inline ws_bool_t treeview_sequence_const_iterator<N, P>::operator ==(treeview_sequence_const_iterator<N, P> const& rhs) const
{
    return m_hitem == rhs.m_hitem;
}

template <UINT N, UINT P>
inline ws_bool_t treeview_sequence_const_iterator<N, P>::operator !=(treeview_sequence_const_iterator<N, P> const& rhs) const
{
    return !operator ==(rhs);
}

// treeview_sequence_base

template <UINT N, UINT P>
inline treeview_sequence_base<N, P>::treeview_sequence_base(HWND hwndTree, HTREEITEM hitem)
    : m_hwnd(hwndTree)
    , m_hitem(hitem)
{}

template <UINT N, UINT P>
inline ss_typename_type_ret_k treeview_sequence_base<N, P>::const_iterator treeview_sequence_base<N, P>::begin() const
{
    return const_iterator(m_hwnd, m_hitem);
}

template <UINT N, UINT P>
inline ss_typename_type_ret_k treeview_sequence_base<N, P>::const_iterator treeview_sequence_base<N, P>::end() const
{
    return const_iterator();
}

// treeview_child_sequence

inline treeview_child_sequence::treeview_child_sequence(HWND hwndTree, HTREEITEM hitem)
    : base_class_type(hwndTree, treeview_getchilditem(hwndTree, hitem))
{}

inline treeview_child_sequence::treeview_child_sequence(HWND hwndTree)
    : base_class_type(hwndTree, treeview_getchilditem(hwndTree, treeview_getrootitem(hwndTree)))
{}

// treeview_peer_sequence

inline treeview_peer_sequence::treeview_peer_sequence(HWND hwndTree, HTREEITEM hitem)
    : base_class_type(hwndTree, hitem)
{}

// treeview_visible_sequence

inline treeview_visible_sequence::treeview_visible_sequence(HWND hwndTree)
    : base_class_type(hwndTree, treeview_getnextitem(hwndTree, NULL, TVGN_FIRSTVISIBLE))
{}

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

#endif /* WINSTL_INCL_WINSTL_CONTROLS_HPP_TREEVIEW_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
