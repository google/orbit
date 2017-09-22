/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/collections/clist_adaptors.hpp
 *
 * Purpose:     Contains the definition of the CList_cadaptor and CList_iadaptor
 *              class templates.
 *
 * Created:     1st December 2002
 * Updated:     3rd February 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2012, Matthew Wilson and Synesis Software
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


/** \file mfcstl/collections/clist_adaptors.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::CList_cadaptor and
 *   mfcstl::CList_iadaptor traits class templates
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS
#define MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS_MAJOR     3
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS_MINOR     0
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS_REVISION  6
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS_EDIT      63
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#ifndef MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR
# include <mfcstl/memory/afx_allocator.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR */
#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_SWAP
# include <mfcstl/collections/clist_swap.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_SWAP */
#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS
# include <mfcstl/collections/clist_traits.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_TRAITS */
#ifndef MFCSTL_INCL_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES
# include <mfcstl/util/memory_exception_translation_policies.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_UTIL_HPP_MEMORY_EXCEPTION_TRANSLATION_POLICIES */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS
# include <stlsoft/util/std/iterator_generators.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_GENERATORS */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES
# include <stlsoft/meta/capabilities.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_CAPABILITIES */
#ifdef STLSOFT_META_HAS_IS_SAME_TYPE
# ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE
#  include <stlsoft/meta/is_same_type.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_SAME_TYPE */
#endif /* STLSOFT_META_HAS_IS_SAME_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#if defined(STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT) && \
    !defined(MFCSTL_NO_INCLUDE_AFXTEMPL_BY_DEFAULT)
# include <afxtempl.h>
#endif /* STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT && !MFCSTL_NO_INCLUDE_AFXTEMPL_BY_DEFAULT */

#ifdef STLSOFT_UNITTEST
# include <afxtempl.h>
# include <stlsoft/string/simple_string.hpp>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Pre-processor options
 *
 * Because the CObList, CPtrList, CStringList and CList<,> implementations all
 * internally represent their logical position indicators (of type POSTION) as
 * pointers to the nodes within the lists, it is workable to be able to copy
 * these position variables.
 *
 * However, nothing in the MFC documentation stipulates this to be a reliable
 * and documented part of the classes' interfaces, so this is a potentially
 * unsafe assumption.
 *
 * Therefore, the iterator model for the CList_adaptor_base class is Input Iterator.
 * If you wish to use forward iterators, you may specify the preprocessor
 * symbol _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR, in which case the iterator
 * classes will implement copy semantics, rather than the default move
 * semantics.
 */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
// Forward declarations
template<   ss_typename_param_k L
        ,   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
class CList_adaptor_base;

// CList_cadaptor is not currently defined
template<   ss_typename_param_k L
        ,   ss_typename_param_k T
        >
class CList_cadaptor;

template<   ss_typename_param_k L
        ,   ss_typename_param_k T
        >
class CList_iadaptor;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Adaptor class, providing implementation for CList_cadaptor and
 *    CList_iadaptor classes
 *
 * \ingroup group__library__collections
 *
 * \param L The list class, e.g. CObList, CList<long>, etc.
 * \param I The interface specialisation, e.g. CList_cadaptor<CObList>, CList_iadaptor<CList<long> >, etc.
 * \param T The traits class, e.g. CList_traits<CObList>
 *
 */

template<   ss_typename_param_k L
        ,   ss_typename_param_k I
        ,   ss_typename_param_k T
        >
class CList_adaptor_base
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
public:
    /// The type of the underlying MFC list
    typedef L                                                                   list_type;
private:
    typedef I                                                                   interface_class_type;
    typedef T                                                                   list_traits_type;
#if defined(MFCSTL_CLIST_ADAPTORS_USE_BAD_ALLOC_POLICY)
    typedef bad_alloc_throwing_policy                                           exception_translation_policy_type;
#else /* ? MFCSTL_CLIST_ADAPTORS_USE_BAD_ALLOC_POLICY */
    typedef CMemoryException_throwing_policy                                    exception_translation_policy_type;
#endif /* MFCSTL_CLIST_ADAPTORS_USE_BAD_ALLOC_POLICY */
public:
    /// The value type
    ///
    /// \note If the compiler report "use of undefined type" when you're using the adaptor class(es)
    /// with CList<>, ensure that you've included <b>afxtempl</b> <i>before</i> you include this file.
    typedef ss_typename_type_k list_traits_type::value_type                     value_type;
    /// The allocator type
    typedef afx_allocator<value_type>                                           allocator_type;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k allocator_type::reference                        reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k allocator_type::const_reference                  const_reference;
    /// The mutating (non-const) pointer type
    typedef ss_typename_type_k allocator_type::pointer                          pointer;
    /// The non-mutating (const) pointer type
    typedef ss_typename_type_k allocator_type::const_pointer                    const_pointer;
    /// The size type
    typedef ms_size_t                                                           size_type;
    /// The difference type
    typedef ms_ptrdiff_t                                                        difference_type;
    /// The instantiation of the current type
    typedef CList_adaptor_base<L, I, T>                                         class_type;
public:
    /// Non-mutating (const) iterator for the CList_adaptor_base class
    ///
    /// \note This currently supports the Input Iterator concept only
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<mfcstl_ns_qual_std(input_iterator_tag)
                                            ,   value_type
                                            ,   ms_ptrdiff_t
                                            ,   void        // By-Value Temporary reference
                                            ,   value_type  // By-Value Temporary reference
                                            >
    {
        friend class CList_adaptor_base<L, I, T>;

        typedef const_iterator                                              class_type;
        // NOTE: If you get a compiler error on the next line, referring to
        // undefined 'value_type' then you need to provide a traits type
        // with the member type 'value_type' defined.
# ifdef STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT
        typedef ss_typename_type_k CList_adaptor_base<L, I, T>::value_type  value_type;
# endif /* !STLSOFT_CF_TEMPLATE_PARTIAL_SPECIALISATION_SUPPORT */
# ifndef _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR
        typedef stlsoft_define_move_rhs_type(class_type)                    rhs_type;
# endif /* !_MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */

    // Construction
    private:
        /// Constructor
        const_iterator(list_type const* list, POSITION pos)
            : m_list(list)
            , m_pos(pos)
            , m_value()
        {
            operator ++();
        }
    public:
        /// Default constructor
        const_iterator()
            : m_list(NULL)
            , m_pos(NULL)
            , m_value()
        {}
# ifdef _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR
        // The copy constructor and copy assignment operator are not defined,
        // which allows the class to support copy-semantics. See the
        // description of _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR given for
        // a discussion of the ramifications of this choice.

# else /* ? _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */
        /// <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">Move constructor</a>
        const_iterator(rhs_type rhs)
            : m_list(rhs.m_list)
            , m_pos(rhs.m_pos)
            , m_value(rhs.m_value)
        {
            move_lhs_from_rhs(rhs).m_pos = NULL;
        }

        /// <a href = "http://synesis.com.au/resources/articles/cpp/movectors.pdf">Move assignment</a> operator
        const_iterator const& operator =(rhs_type rhs)
        {
            m_list  =   rhs.m_list;
            m_pos   =   rhs.m_pos;
            m_value =   rhs.m_value;

            move_lhs_from_rhs(rhs).m_pos = NULL;

            return *this;
        }
# endif /* _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */

    // Operators
    public:
        /// Dereference operator
        value_type operator *() const
        {
            MFCSTL_MESSAGE_ASSERT("", NULL != m_list);

            return m_value;
        }

        /// \brief Pre-increment operator
        const_iterator& operator ++()
        {
            if(m_pos == NULL)
            {
                MFCSTL_MESSAGE_ASSERT("operator ++() called on invalid iterator", NULL != m_list);

                m_list = NULL;
            }
            else
            {
                m_value = m_list->GetNext(m_pos);
            }

            return *this;
        }

        /// \brief Post-increment operator
# ifdef _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR
        const_iterator operator ++(int)
# else /* ? _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */
        void operator ++(int)
# endif /* _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */
        {
# ifdef _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR
            class_type  ret(*this);
# endif /* _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */

            operator ++();

# ifdef _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR
            return ret;
# endif /* _MFCSTL_LIST_ADAPTOR_ENABLE_FWD_ITERATOR */
        }

        /// Evaluates whether \c this is equivalent to \c rhs
        ///
        /// \param rhs The instance from which to copy construct
        /// \retval true The two iterators refer to the same position in the same container
        /// \retval false The two iterators do not refer to the same position in the same container
        ms_bool_t operator ==(const_iterator const& rhs) const
        {
            // Because the C<Type><Container> containers, e.g. CStringList
            // work on the basis of get-and-advance, m_pos alone cannot be
            // the sentinel for an ended sequence. Hence, combining the
            // implementation of op++ to set m_list to NULL when m_pos is NULL, we
            // can test both members, which results in the after-the-fact
            // equality evaluating correctly.

            MFCSTL_MESSAGE_ASSERT("invalid comparison between iterators from different ranges", (NULL == m_list || NULL == rhs.m_list || m_list == rhs.m_list));

            return m_pos == rhs.m_pos && m_list == rhs.m_list;
        }
        /// Evaluates whether \c this is not equivalent to \c rhs
        ///
        /// \param rhs The instance from which to copy construct
        /// \retval true The two iterators do not refer to the same position in the same container
        /// \retval false The two iterators refer to the same position in the same container
        ms_bool_t operator !=(const_iterator const& rhs) const
        {
            return !operator ==(rhs);
        }

    // Members
    private:
        list_type const*    m_list;
        POSITION            m_pos;
        value_type          m_value;
    };
/// @}

/// \name Underlying Container Access
/// @{
public:
    /// \brief Returns a mutating (non-const) reference to the underlying list
    list_type&          get_CList()
    {
        return static_cast<interface_class_type*>(this)->get_actual_list();
    }
    /// \brief Returns a non-mutating (const) reference to the underlying list
    list_type const&    get_CList() const
    {
        return static_cast<interface_class_type const*>(this)->get_actual_list();
    }
/// @}

/// \name Construction
/// @{
protected:
    /// \brief Default constructor.
    ///
    /// This is protected, because CList_adaptor_base serves as an abstract base
    /// for CList_cadaptor and CList_iadaptor
    CList_adaptor_base()
    {}
    /// \brief Destructor
    ~CList_adaptor_base() stlsoft_throw_0()
    {}
public:
    /// Returns a copy of the allocator used by the container
    allocator_type get_allocator() const
    {
        return allocator_type();
    }
/// @}

/// \name Size and Capacity
/// @{
public:
    /// Returns the number of elements in the sequence
    size_type size() const
    {
        return static_cast<size_type>(get_CList().GetSize());
    }
    /// \brief The maximum number of items that can be stored in the list
    size_type max_size() const
    {
        return get_allocator().max_size();
    }
    /// Indicates whether the search sequence is empty
    ms_bool_t empty() const
    {
        return 0 == size();
    }
/// @}

/// \name Element access
/// @{
public:
/// @}

/// \name Modifiers
/// @{
public:
    void push_back(value_type const& val)
    {
        get_CList().AddTail(val);
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
        return const_iterator(&get_CList(), get_CList().GetHeadPosition());
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator  end() const
    {
        return const_iterator();
    }
/// @}
};

/** \brief Adaptor class, representing an Instance Adaptor over the CList
 *    family of MFC containers
 *
 * \ingroup group__library__collections
 *
 * It is used as follows:
 *
\code
  CStringList                          li;
  mfcstl::CList_iadaptor<CStringList>  lip(li);

  // As an MFC CStringList:
  li.AddHead("String 1");

  // As an STL container
  lip.push_back("String 2");
  std::list<CString>  l;
  l.push_back("String 3");
  l.push_back("String 4");
  lip.insert(lip.begin() + 2, l.begin(), l.end());
  std::sort(lip.begin(), lip.end());
\endcode
 *
 * \param A The list class, e.g. CObList, CList<long>, etc.
 *
 * \note The elements in an adapted list are moved, during insertion / erasure, rather than copied. This
 *   means that if the elements in the container maintain pointers to their elements, or their peers, then
 *   they are not suitable for use.
 */
template<   ss_typename_param_k A
        ,   ss_typename_param_k T = CList_traits<A>
        >
class CList_iadaptor
        : public CList_adaptor_base<A, CList_iadaptor<A, T>, T>
{
/// \name Member Types
/// @{
private:
    typedef CList_adaptor_base<A, CList_iadaptor<A, T>, T>        parent_class_type;
public:
    /// The type of the underlying MFC list
    typedef ss_typename_type_k parent_class_type::list_type        list_type;
    /// The value type
    typedef ss_typename_type_k parent_class_type::value_type        value_type;
    /// The allocator type
    typedef ss_typename_type_k parent_class_type::allocator_type    allocator_type;
    /// The mutating (non-const) reference type
    typedef ss_typename_type_k parent_class_type::reference         reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k parent_class_type::const_reference   const_reference;
    /// The mutating (non-const) pointer type
    typedef ss_typename_type_k parent_class_type::pointer           pointer;
    /// The non-mutating (const) pointer type
    typedef ss_typename_type_k parent_class_type::const_pointer     const_pointer;
#if 0
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k parent_class_type::const_iterator    const_iterator;
#endif /* 0 */
    /// The size type
    typedef ss_typename_type_k parent_class_type::size_type         size_type;
    /// The difference type
    typedef ss_typename_type_k parent_class_type::difference_type   difference_type;
    /// The instantiation of the current type
    typedef CList_iadaptor<A, T>                                   class_type;
/// @}

/// \name Identity
/// @{
private:
    friend class CList_adaptor_base<A, CList_iadaptor<A, T>, T>;

    list_type          &get_actual_list()
    {
        MFCSTL_ASSERT(NULL != m_pList);
        return *m_pList;
    }
    list_type const    &get_actual_list() const
    {
        MFCSTL_ASSERT(NULL != m_pList);
        return *m_pList;
    }
/// @}

/// \name Construction
/// @{
public:
    template <ss_typename_param_k A2>
    CList_iadaptor(A2 &list)
         : m_pList(&list)
    {
        STLSOFT_STATIC_ASSERT(sizeof(list_type) == sizeof(A2));
#ifdef STLSOFT_META_HAS_IS_SAME_TYPE
        STLSOFT_STATIC_ASSERT((stlsoft::is_same_type<list_type, A2>::value));
#else /* ? STLSOFT_META_HAS_IS_SAME_TYPE */
        ASSERT(0 == ::lstrcmpA(list.GetRuntimeClass()->m_lpszClassName, list_type().GetRuntimeClass()->m_lpszClassName));
# ifdef _CPPRTTI
        ASSERT(0 == ::lstrcmpA(typeid(A2).name(), typeid(list_type).name()));
# endif /* _CPPRTTI */
#endif /* STLSOFT_META_HAS_IS_SAME_TYPE */
    }
    template <ss_typename_param_k A2>
    CList_iadaptor(A2 *pList)
         : m_pList(pList)
    {
        MFCSTL_MESSAGE_ASSERT("Cannot initialise a CList_iadaptor with a NULL pointer", NULL != pList);

        STLSOFT_STATIC_ASSERT(sizeof(list_type) == sizeof(A2));
#ifdef STLSOFT_META_HAS_IS_SAME_TYPE
        STLSOFT_STATIC_ASSERT((stlsoft::is_same_type<list_type, A2>::value));
#else /* ? STLSOFT_META_HAS_IS_SAME_TYPE */
        ASSERT(0 == ::lstrcmpA(pList->GetRuntimeClass()->m_lpszClassName, list_type().GetRuntimeClass()->m_lpszClassName));
# ifdef _CPPRTTI
        ASSERT(0 == ::lstrcmpA(typeid(A2).name(), typeid(list_type).name()));
# endif /* _CPPRTTI */
#endif /* STLSOFT_META_HAS_IS_SAME_TYPE */
    }
/// @}

/// \name Members
/// @{
private:
    list_type  *m_pList;
/// @}

/// \name Not to be implemented
/// @{
private:
    CList_iadaptor(class_type const& rhs);            // Only possible semantics for copy-ctor are share underlying list
    class_type& operator =(class_type const& rhs);  // Could either repoint, or could do deep copy.
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/clist_adaptors_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace mfcstl */
# else
} /* namespace mfcstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CLIST_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
