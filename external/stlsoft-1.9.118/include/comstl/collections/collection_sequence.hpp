/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/collections/collection_sequence.hpp (originally MOEnSeq.h, ::SynesisCom)
 *
 * Purpose:     STL sequence for COM collection interfaces.
 *
 * Created:     17th September 1998
 * Updated:     3rd February 2012
 *
 * Thanks:      To Eduardo Bezerra and Vivi Orunitia for reporting
 *              incompatibilities with Borland's 5.82 (Turbo C++). The awful
 *              preprocessor hack around retrievalQuanta are the result. ;)
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2012, Matthew Wilson and Synesis Software
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


/** \file comstl/collections/collection_sequence.hpp
 *
 * \brief [C++ only] Definition of the comstl::collection_sequence
 *   collection class template
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef COMSTL_INCL_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE
#define COMSTL_INCL_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE_MAJOR    6
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE_MINOR    1
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE_REVISION 10
# define COMSTL_VER_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE_EDIT     104
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_UTIL_H_REFCOUNT_FUNCTIONS
# include <comstl/util/refcount_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_H_REFCOUNT_FUNCTIONS */
#ifndef COMSTL_INCL_COMSTL_COLLECTIONS_HPP_ENUMERATION_POLICIES
# include <comstl/collections/enumeration_policies.hpp>
#endif /* !COMSTL_INCL_COMSTL_COLLECTIONS_HPP_ENUMERATION_POLICIES */
#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS
# include <comstl/util/interface_traits.hpp>
#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS
#  include <comstl/error/exceptions.hpp>
# endif /* !COMSTL_INCL_COMSTL_ERROR_HPP_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef COMSTL_INCL_COMSTL_COLLECTIONS_HPP_ENUMERATOR_SEQUENCE
# include <comstl/collections/enumerator_sequence.hpp>
#endif /* !COMSTL_INCL_COMSTL_COLLECTIONS_HPP_ENUMERATOR_SEQUENCE */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */

#ifdef STLSOFT_UNITTEST
# include <comstl/util/value_policies.hpp>
# if !defined(STLSOFT_COMPILER_IS_DMC)
#  include "./unittest/_recls_COM_decl_.h"
# endif /* compiler */
# include <stdio.h>
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::comstl */
namespace comstl
{
# else
/* Define stlsoft::comstl_project */

namespace stlsoft
{

namespace comstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief A template for adapting COM collections to STL-compatible sequence iteration.
 *
 * \ingroup group__library__collections
 *
 * \param I Interface
 * \param V Value type
 * \param VP Value policy type
 * \param R Reference type. Defaults to <code>V const&</code>
 * \param CP Cloning policy type. Defaults to comstl::input_cloning_policy
 * \param Q Quanta. Defaults to 8
 * \param EAP Enumerate acquisition policy type. Defaults to comstl::new_enum_property_policy
 *
 * The various parameterising types are used to stipulate the interface and the
 * value type, and how they are to be handled.
 *
 * For example, the following parameterisation defines a sequence operating
 * over a notional <b>IGUIDCollection</b> collection instance.
 *
\code
typedef collection_sequence<IGUIDCollection
                          , IEnumGUID
                          , GUID
                          , GUID_policy
                          , GUID const&
                          , forward_cloning_policy<IEnumGUID>
                          , 5
                          >    collection_sequence_t;
\endcode
 *
 * The value type is <b>GUID</b> and it is returned as a reference, as
 * the <b>GUID const&</b> in fact.
 *
 * The \ref group__project__comstl type <b>GUID_policy</b> controls how the <b>GUID</b>
 * instances are initialised, copied and destroyed.
 *
 * The \ref group__project__comstl type forward_cloning_policy allows the sequence to provide
 * <a href = "http://sgi.com/tech/stl/ForwardIterator.html">Forward Iterator</a>
 * semantics.
 *
 * And the <b>5</b> indicates that the sequence should grab five values at a time,
 * to save round trips to the enumerator.
 *
 * This would be used as follows:
 *
\code
  void dump_GUID(GUID const&);

  IGUIDCollection       *penGUIDs = . . .;      // Create an instance from wherever
  collection_sequence_t guids(penGUIDs, false); // Eat the reference

  std::for_each(guids.begin(), guids.end(), dump_GUID);
\endcode
 *
 * \note The iterator instances returned by begin() and end() are valid outside
 *   the lifetime of the collection instance from which they are obtained
 *
 * \remarks A detailed examination of the design and implementation of this
 *   class template is described in Chapters 28 and 29 of
 *   <a href="http://www.extendedstl.com/"><b>Extended STL, volume 1</b></a>
 *   (published by Addison-Wesley, June 2007).
 *
 * \sa comstl::enumerator_sequence
 */
// [[synesis:class:collection: comstl::collection_sequence<T<CI>, T<EI>, T<V>, T<VP>, T<R>, T<CP>, size_t, T<EAP>>]]
template<   ss_typename_param_k CI                                      /* Collection interface */
        ,   ss_typename_param_k EI                                      /* Enumerator interface */
        ,   ss_typename_param_k V                                       /* Value type */
        ,   ss_typename_param_k VP                                      /* Value policy type */
        ,   ss_typename_param_k R   =   V const&                        /* Reference type */
        ,   ss_typename_param_k CP  =   input_cloning_policy<EI>        /* Cloning policy type */
        ,   cs_size_t           Q   =   8                               /* Quanta */
        ,   ss_typename_param_k EAP =   new_enum_property_policy<CI>    /* Policy for acquiring the enumerator from the collection */
        >
class collection_sequence
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef enumerator_sequence<EI, V, VP, R, CP, Q>                            enumerator_sequence_type;
public:
    /// \brief Collection interface type
    typedef CI                                                                  collection_interface_type;
    /// \brief Enumerator interface type
    typedef ss_typename_type_k enumerator_sequence_type::interface_type         enumerator_interface_type;
    /// \brief Value type
    typedef ss_typename_type_k enumerator_sequence_type::value_type             value_type;
    /// \brief Value policy type
    typedef ss_typename_type_k enumerator_sequence_type::value_policy_type      value_policy_type;
    /// \brief Reference type
    typedef ss_typename_type_k enumerator_sequence_type::reference              reference;
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    typedef ss_typename_type_k enumerator_sequence_type::reference_type         reference_type; // For backwards compatiblity
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
    /// \brief The mutating (non-const) pointer type
    typedef ss_typename_type_k enumerator_sequence_type::pointer                pointer;
    /// \brief The non-mutating (const) pointer type
    typedef ss_typename_type_k enumerator_sequence_type::const_pointer          const_pointer;
    /// \brief The mutating (non-const) iterator type
    typedef ss_typename_type_k enumerator_sequence_type::iterator               iterator;
    /// \brief The non-mutating (const) iterator type
    typedef ss_typename_type_k enumerator_sequence_type::const_iterator         const_iterator;
    /// \brief Cloning policy type
    typedef ss_typename_type_k enumerator_sequence_type::cloning_policy_type    cloning_policy_type;
    /// \brief Iterator tag type
    typedef ss_typename_type_k enumerator_sequence_type::iterator_tag_type      iterator_tag_type;
#ifdef STLSOFT_COMPILER_IS_BORLAND
# define retrievalQuanta                                                        Q
#else /* ? compiler */
    /// \brief Retrieval quanta
    enum                                                                      { retrievalQuanta = enumerator_sequence_type::retrievalQuanta };
#endif /* compiler */
    /// \brief The policy for acquiring the enumerator from the collection
    typedef EAP                                                                 enumerator_acquisition_policy_type;
    /// \brief Type of the current parameterisation
    typedef collection_sequence<CI, EI, V, VP, R, CP, Q, EAP>                   class_type;
    /// \brief The size type
    typedef ss_typename_type_k enumerator_sequence_type::size_type              size_type;
    /// \brief The difference type
    typedef ss_typename_type_k enumerator_sequence_type::difference_type        difference_type;
/// @}

public:
    /// \brief Conversion constructor
    ///
    /// \param i The enumeration interface pointer to adapt
    /// \param bAddRef Causes a reference to be added if \c true, otherwise the sequence is deemed to <i>sink</i>, or consume, the interface pointer
    /// \param quanta The actual quanta required for this instance. Must be <= Q
    ///
    /// \note This does not throw an exception, so it is safe to be used to "eat" the
    /// reference. The only possible exception to this is if COMSTL_ASSERT(), which is
    /// used to validate that the given quanta size is within the limit specified in
    /// the specialisation, has been redefined to throw an exception. But since
    /// precondition violations are no more recoverable than any others (see the article
    /// "The Nuclear Reactor and the Deep Space Probe"), this does not represent
    /// a concerning contradiction to the no-throw status of the constructor.
    collection_sequence(collection_interface_type *i, cs_bool_t bAddRef, size_type quanta = 0)
        : m_i(i)
        , m_quanta(validate_quanta_(quanta))
    {
        COMSTL_ASSERT(NULL != i);
        COMSTL_MESSAGE_ASSERT("Cannot set a quantum that exceeds the value specified in the template specialisation", quanta <= retrievalQuanta); // Could have named these things better!

        if(bAddRef)
        {
            m_i->AddRef();
        }

        COMSTL_ASSERT(is_valid());
    }
    /// \brief Releases the adapted interface pointer
    ~collection_sequence() stlsoft_throw_0()
    {
        COMSTL_ASSERT(is_valid());

        m_i->Release();
    }

/// \name Iteration
/// @{
public:
    /// \brief Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    iterator begin() const
    {
        COMSTL_ASSERT(is_valid());

        LPUNKNOWN   punkEnum;
        HRESULT     hr  =   enumerator_acquisition_policy_type::acquire(m_i, &punkEnum);

        if(SUCCEEDED(hr))
        {
            enumerator_interface_type   *ei;

            hr = punkEnum->QueryInterface(IID_traits<enumerator_interface_type>::iid(), reinterpret_cast<void**>(&ei));

            punkEnum->Release();

            if(SUCCEEDED(hr))
            {
                COMSTL_ASSERT(is_valid());

                return enumerator_sequence_type(ei, false, m_quanta).begin();
            }
            else
            {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
                COMSTL_ASSERT(is_valid());

                STLSOFT_THROW_X(com_exception("the enumerator does not provide the requested interface", hr));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            }
        }
        else
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            COMSTL_ASSERT(is_valid());

            STLSOFT_THROW_X(com_exception("enumerator could not be elicited from the collection", hr));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        COMSTL_ASSERT(is_valid());

        return iterator();
    }
    /// \brief Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    iterator end() const
    {
        COMSTL_ASSERT(is_valid());

        return iterator();
    }
/// @}

/// \name Capacity
/// @{
public:
    /** \brief Returns the number of items in the collection
     *
    \code
    IGUIDCollection       *penGUIDs = . . .;        // Create an instance from wherever
    collection_sequence_t guids(penGUIDs, false);   // Eat the reference
    size_t                numItems = guids.size();  // Evaluate the number of elements in the collection
    \endcode
     *
     *
     * \note This method will not compile for collection interfaces
     * that do not contain the get_Count method
     */
    size_type size() const
    {
        COMSTL_ASSERT(is_valid());

        ULONG   count;
        HRESULT hr  =   m_i->get_Count(&count);

        COMSTL_ASSERT(is_valid());

        return SUCCEEDED(hr) ? count : 0;
    }
/// @}

/// \name Invariant
/// @{
private:
    cs_bool_t is_valid() const
    {
        if(NULL == m_i)
        {
            return false;
        }

        return true;
    }
/// @}

// Implementation
private:
    static size_type validate_quanta_(size_type quanta)
    {
        COMSTL_MESSAGE_ASSERT("Cannot set a quantum that exceeds the value specified in the template specialisation", quanta <= retrievalQuanta); // Could have named these things better!

        if( 0 == quanta ||
            quanta > retrievalQuanta)
        {
            quanta = retrievalQuanta;
        }

        return quanta;
    }

// Members
private:
    collection_interface_type   *m_i;
    size_type const             m_quanta;

// Not to be implemented
private:
    collection_sequence(class_type const&);
    class_type const& operator =(class_type const&);
};

////////////////////////////////////////////////////////////////////////////
// Compiler compatibility

#ifdef STLSOFT_COMPILER_IS_BORLAND
# undef retrievalQuanta
#endif /* compiler */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/collection_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace stlsoft::comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_COLLECTIONS_HPP_COLLECTION_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
