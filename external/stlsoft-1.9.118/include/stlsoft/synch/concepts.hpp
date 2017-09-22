/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/synch/concepts.hpp
 *
 * Purpose:     Synchronisation concept tags.
 *
 * Created:     16th January 2006
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


/** \file stlsoft/synch/concepts.hpp
 *
 * \brief [C++ only] Synchronisation concept tags
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS
#define STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_HPP_SYNCH_HPP_CONCEPTS_FWD_MAJOR       1
# define STLSOFT_VER_STLSOFT_HPP_SYNCH_HPP_CONCEPTS_FWD_MINOR       0
# define STLSOFT_VER_STLSOFT_HPP_SYNCH_HPP_CONCEPTS_FWD_REVISION    3
# define STLSOFT_VER_STLSOFT_HPP_SYNCH_HPP_CONCEPTS_FWD_EDIT        11
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_YESNO
# include <stlsoft/meta/yesno.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_YESNO */

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

/** \brief Denotes that a deriving class is a synchronisation type
 *
 * \ingroup group__library__synch
 */
#if 0
struct synchronisation_type_tag
{};
#endif /* 0 */

/** \brief Denotes that a deriving class is a wrapper for a native
 *   synchronisation object, and that the underlying object is available via a
 *   <code>get()</code> method.
 *
 * \ingroup group__library__synch
 */
struct synchronisable_object_tag
//    : public synchronisation_type_tag
{};

/** \brief Denotes that a deriving class can be used as a critical section,
 *   i.e. it has methods lock() and unlock() for entering and exiting the
 *   critical sections.
 *
 * \ingroup group__library__synch
 */
struct critical_section_tag
//    : public synchronisation_type_tag
{};

#if 0
/** \brief Denotes that a deriving class can be used as a critical section
 * (see critical_section_tag), and that it can be recursively entered without
 * deadlock, i.e. the following call sequence is well defined:
 *
 * \ingroup group__library__synch
 *
 * obj.lock();
 * obj.lock();
 * obj.unlock();
 * obj.unlock();
 */
struct recursive_critical_section_tag
//    : public critical_section_tag
{};

/** \brief Denotes that a deriving class can be used as a critical section
 * (see critical_section_tag), and that it has a try_lock() method, which returns
 * an integral result, where non-0 indicates that the lock() can be acquired <b>and</b>
 * has been acquired.
 *
 * \ingroup group__library__synch
 */
struct tryable_critical_section_tag
//    : public critical_section_tag
{};
#endif /* 0 */

/** \brief Concept tag class that denotes that an object may be utilised to
 *   serialise access to a critical section of code.
 *
 * \ingroup group__library__synch
 *
 * \param R Integral value indicating whether the object is recursive
 * \param T Integral value indicating whether the object supports the
 *  <code>try_lock()</code> operation.
 */
template<   int R
        ,   int T
        >
struct critical_section
    : public critical_section_tag
{
    enum {  is_recursive    =   R       };
    enum {  is_tryable      =   T       };

    /// \brief Type the indicates whether the deriving type
    ///
    ///
    typedef ss_typename_type_k value_to_yesno_type<is_recursive>::type  is_recursive_type;
    typedef ss_typename_type_k value_to_yesno_type<is_tryable>::type    is_tryable_type;
};


#define STLSOFT_CRITICAL_SECTION_IS_RECURSIVE       (1)
#define STLSOFT_CRITICAL_SECTION_ISNOT_RECURSIVE    (0)

#define STLSOFT_CRITICAL_SECTION_IS_TRYABLE         (1)
#define STLSOFT_CRITICAL_SECTION_ISNOT_TRYABLE      (0)


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
//typedef synchronisation_type_tag        synchronization_type_tag;
typedef synchronisable_object_tag       synchronizable_object_tag;
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Concept checks
 */

namespace concept_check
{
    template <ss_typename_param_k S>
    void synch_conformance_synch_obj(S &s, synchronisable_object_tag const*)
    {
        s.handle();

        s.is_signalled();

        static_cast<int>(s.is_signalled()); // Checks that return type evaluatable as truth
    }
    template <ss_typename_param_k S>
    void synch_conformance_synch_obj(S &s, ...)
    {}


    template <ss_typename_param_k S>
    void synch_conformance_try_lock(S &s, yes_type)
    {
        if(s.try_lock())
        {
            s.unlock();
        }
    }
    template <ss_typename_param_k S>
    void synch_conformance_try_lock(S &s, no_type)
    {}


    template <ss_typename_param_k S>
    void synch_conformance_recursive_lock(S &s, yes_type)
    {
        s.lock();
        s.lock();
        s.unlock();
        s.unlock();
    }
    template <ss_typename_param_k S>
    void synch_conformance_recursive_lock(S &s, no_type)
    {}

    template <ss_typename_param_k S>
    void synch_conformance_lock(S &s, critical_section_tag const*)
    {
        s.lock();
        s.unlock();

        typedef ss_typename_type_k value_to_yesno_type<S::is_tryable>::type    is_tryable_type;

        synch_conformance_try_lock(s, is_tryable_type());

        typedef ss_typename_type_k value_to_yesno_type<S::is_recursive>::type  is_recursive_type;

        synch_conformance_recursive_lock(s, is_recursive_type());
    }
    template <ss_typename_param_k S>
    void synch_conformance_lock(S &s, ...)
    {}



    template <ss_typename_param_k S>
    void synch_conformance(S &s)
    {
        synch_conformance_synch_obj(s,&s);
        synch_conformance_lock(s, &s);
    }

} // namespace concept_check

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_CONCEPTS */

/* ///////////////////////////// end of file //////////////////////////// */
