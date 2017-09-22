/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/memory/heapwalk_sequence.hpp (originally MWHeapWk.h; ::SynesisWin)
 *
 * Purpose:     Contains the heapwalk_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Notes:       The original implementation of the class had the const_iterator
 *              and value_type as nested classes. Unfortunately, Visual C++ 5 &
 *              6 both had either compilation or linking problems so these are
 *              regretably now implemented as independent classes.
 *
 * Created:     15th January 2002
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


/** \file winstl/memory/heapwalk_sequence.hpp
 *
 * \brief [C++ only] Definition of the winstl::heapwalk_sequence class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE
#define WINSTL_INCL_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE_MAJOR       4
# define WINSTL_VER_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE_MINOR       0
# define WINSTL_VER_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE_REVISION    3
# define WINSTL_VER_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE_EDIT        62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifdef STLSOFT_UNITTEST
# include <algorithm>
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
 * Forward declarations
 */

class heapwalk_sequence_const_iterator;

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

// class heapwalk_sequence
/** \brief An STL-like sequence for walking heap allocated blocks
 *
 * \ingroup group__library__memory
 *
 * The following code illustrates the use of the class to enumerate all
 * currently allocated blocks in the given heap:
\code
HANDLE                      hHeap = . . . // Handle to an existing heap
winstl::heapwalk_sequence   blocks(hHeap);

winstl::heapwalk_sequence::const_iterator   b = blocks.begin();
winstl::heapwalk_sequence::const_iterator   e = blocks.end();

for(; b != e; ++b)
{
  printf("  Block of size %lu at %p\n", (*b).cbData, (*b).lpData);
}
\endcode
 *
 * If you do not specify a heap handle, the sequence uses the
 * process heap, as shown in the following code:
\code
winstl::heapwalk_sequence   blocks; // Uses process heap

winstl::heapwalk_sequence::const_iterator   b = blocks.begin();
winstl::heapwalk_sequence::const_iterator   e = blocks.end();

for(; b != e; ++b)
{
  printf("  Block of size %lu at %p\n", (*b).cbData, (*b).lpData);
}
\endcode

 */
class heapwalk_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef PROCESS_HEAP_ENTRY                  entry_type;
public:
    /// \brief The class
    typedef heapwalk_sequence                   class_type;
    /// \brief The value type
    typedef entry_type const&                   value_type;
    /// \brief The non-mutating (const) iterator type
    typedef heapwalk_sequence_const_iterator    const_iterator;
    /// \brief The non-mutating (const) reference type
    typedef entry_type const&                   const_reference;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Creates an instance that walks the default process heap
    heapwalk_sequence();
    /// \brief Creates an instance that walks the given heap
    ///
    /// \note The heapwalk_sequence does <i>not</i> take ownership of the given heap handle
    ss_explicit_k heapwalk_sequence(HANDLE hHeap);
    /// Destructor
    ~heapwalk_sequence() stlsoft_throw_0();
/// @}

/// \name Iteration
/// @{
public:
    /// \brief Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator  begin() const;
    /// \brief Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const;
/// @}

/// \name Implementation
/// @{
private:
    friend class heapwalk_sequence_const_iterator;
/// @}

/// \name Members
/// @{
private:
    HANDLE  m_hHeap;
/// @}

/// \name Not to be implemented
/// @{
private:
    heapwalk_sequence(class_type const&);
    heapwalk_sequence const& operator =(class_type const&);
/// @}
};

// class heapwalk_sequence_const_iterator
/** \brief Iterator for the heapwalk_sequence class
 *
 * \ingroup group__library__memory
 *
 */
class heapwalk_sequence_const_iterator
    : public stlsoft_ns_qual(iterator_base) <   winstl_ns_qual_std(forward_iterator_tag)
                                            ,   PROCESS_HEAP_ENTRY const
                                            ,   ws_ptrdiff_t
                                            ,   PROCESS_HEAP_ENTRY const*
                                            ,   PROCESS_HEAP_ENTRY const&>
{
/// \name Member Types
/// @{
private:
    typedef PROCESS_HEAP_ENTRY                  entry_type;
public:
    /// \brief The value type
    typedef entry_type const                    value_type;
    /// \brief The class
    typedef heapwalk_sequence_const_iterator    class_type;
/// @}

/// \name Construction
/// @{
private:
    heapwalk_sequence_const_iterator(heapwalk_sequence const& l, entry_type const& value)
        : m_list(&l)
        , m_value(value)
    {}
    heapwalk_sequence_const_iterator(heapwalk_sequence const& l);
public:
    /// \brief Default constructor
    heapwalk_sequence_const_iterator();
    /// \brief Copy constructor
    heapwalk_sequence_const_iterator(class_type const& rhs);
    /// \brief Destructor
    ~heapwalk_sequence_const_iterator() stlsoft_throw_0();

    /// \brief Copy asignment operator
    heapwalk_sequence_const_iterator& operator =(class_type const& rhs);
/// @}

/// \name Forward Iterator methods
/// @{
public:
    /// \brief Pre-increment operator
    class_type& operator ++();
    /// \brief Post-increment operator
    class_type const operator ++(int);
    /// \brief Dereferences and returns a reference to the \c PROCESS_HEAP_ENTRY structure for the current position
    value_type& operator *() const;
    /// \brief Evaluates whether \c this and \c rhs are equivalent
    ws_bool_t operator ==(class_type const& rhs) const;
    /// \brief Evaluates whether \c this and \c rhs are not equivalent
    ws_bool_t operator !=(class_type const& rhs) const;
/// @}

/// \name Implementation
/// @{
private:
    friend class heapwalk_sequence;
/// @}

/// \name Members
/// @{
private:
    heapwalk_sequence const* const  m_list;
    entry_type                      m_value;
/// @}
};

////////////////////////////////////////////////////////////////////////////

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// heapwalk_sequence

inline heapwalk_sequence::heapwalk_sequence()
    : m_hHeap(::GetProcessHeap())
{}

inline heapwalk_sequence::heapwalk_sequence(HANDLE hHeap)
    : m_hHeap(hHeap)
{}

inline heapwalk_sequence::~heapwalk_sequence() stlsoft_throw_0()
{}

inline heapwalk_sequence::const_iterator heapwalk_sequence::begin() const
{
    entry_type  value;

    value.lpData    =   NULL;   // Want start of iteration

    if(::HeapWalk(m_hHeap, &value))
    {
        return const_iterator(*this, value);
    }
    else
    {
        return const_iterator(*this);
    }
}

inline heapwalk_sequence::const_iterator heapwalk_sequence::end() const
{
    return const_iterator(*this);
}

// heapwalk_sequence_const_iterator

inline heapwalk_sequence_const_iterator::heapwalk_sequence_const_iterator()
    : m_list(NULL)
{
    m_value.lpData  =   reinterpret_cast<LPVOID>(-1);
}

inline heapwalk_sequence_const_iterator::heapwalk_sequence_const_iterator(heapwalk_sequence const& l)
    : m_list(&l)
{
    m_value.lpData  =   reinterpret_cast<LPVOID>(-1);
}

inline heapwalk_sequence_const_iterator::heapwalk_sequence_const_iterator(class_type const& rhs)
    : m_list(rhs.m_list)
    , m_value(rhs.m_value)
{}

inline heapwalk_sequence_const_iterator::class_type& heapwalk_sequence_const_iterator::operator =(const heapwalk_sequence_const_iterator::class_type& rhs)
{
    WINSTL_ASSERT(m_list == rhs.m_list);

    m_value =   rhs.m_value;

    return *this;
}

inline heapwalk_sequence_const_iterator::~heapwalk_sequence_const_iterator() stlsoft_throw_0()
{}

inline heapwalk_sequence_const_iterator::class_type& heapwalk_sequence_const_iterator::operator ++()
{
    if( NULL != m_list &&
        !::HeapWalk(m_list->m_hHeap, &m_value))
    {
        m_value.lpData  =   reinterpret_cast<LPVOID>(-1);
    }

    return *this;
}

inline heapwalk_sequence_const_iterator::class_type const heapwalk_sequence_const_iterator::operator ++(int)
{
    class_type  it(*this);

    operator ++();

    return it;
}

inline heapwalk_sequence_const_iterator::value_type &heapwalk_sequence_const_iterator::operator *() const
{
    return m_value;
}

inline ws_bool_t heapwalk_sequence_const_iterator::operator ==(class_type const& rhs) const
{
    ws_bool_t    eq;

    WINSTL_ASSERT(m_list == rhs.m_list);    // Should only be comparing iterators from same container

    // Not equal if one but not both handles are not equal.
    if(m_value.lpData != rhs.m_value.lpData)
    {
        eq = ws_false_v;
    }
    else
    {
        eq = ws_true_v;
    }

    return eq;
}

inline ws_bool_t heapwalk_sequence_const_iterator::operator !=(class_type const& rhs) const
{
    return ! operator ==(rhs);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/heapwalk_sequence_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_MEMORY_HPP_HEAPWALK_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
