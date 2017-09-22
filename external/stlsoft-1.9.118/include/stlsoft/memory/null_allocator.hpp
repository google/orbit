/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/memory/null_allocator.hpp
 *
 * Purpose:     null_allocator class - doesn't allocate.
 *
 * Created:     2nd January 2001
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2001-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/memory/null_allocator.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::null_allocator class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR
#define STLSOFT_INCL_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR_MAJOR    4
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR_MINOR    0
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR_REVISION 3
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR_EDIT     79
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

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# include <new>
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

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

/** \brief STL Allocator that fails all allocation requests
 *
 * \ingroup group__library__memory
 *
 * \param T The value_type of the allocator
 *
 * \note When exception support is enabled, instances of this type will
 *   throw <code>std::bad_alloc</code> for any allocation request (even if
 *   the requested size is 0). When exception support is not enabled,
 *   requesting memory from this class is a contract violation, and an
 *   enforcement will fire (if contract programming enforcement is enabled).
 */
template <ss_typename_param_k T>
class null_allocator
    : public allocator_base<T, null_allocator<T> >
{
private:
    typedef allocator_base<T, null_allocator<T> >                   parent_class_type;
public:
    /// The parameterisation of the class
    typedef null_allocator<T>                                       class_type;
    /// The value type
    typedef ss_typename_type_k parent_class_type::value_type        value_type;
    /// The pointer type
    typedef ss_typename_type_k parent_class_type::pointer           pointer;
    /// The non-mutating (const) pointer type
    typedef ss_typename_type_k parent_class_type::const_pointer     const_pointer;
    /// The reference type
    typedef ss_typename_type_k parent_class_type::reference         reference;
    /// The non-mutating (const) reference type
    typedef ss_typename_type_k parent_class_type::const_reference   const_reference;
    /// The difference type
    typedef ss_typename_type_k parent_class_type::difference_type   difference_type;
    /// The size type
    typedef ss_typename_type_k parent_class_type::size_type         size_type;

public:
#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    /// The allocator's <b><code>rebind</code></b> structure
    template <ss_typename_param_k U>
    struct rebind
    {
        typedef null_allocator<U>                                   other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */

/// \name Construction
/// @{
public:
    /// Default constructor
    null_allocator() stlsoft_throw_0()
    {}
    /// Copy constructor
#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    template <ss_typename_param_k U>
    null_allocator(null_allocator<U> const&)
    {}
#else /* ? STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
    null_allocator(class_type const&)
    {}
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
/// @}

private:
    friend class allocator_base<T, null_allocator<T> >;

    void *do_allocate(size_type n, void const* hint)
    {
        STLSOFT_SUPPRESS_UNUSED(n);
        STLSOFT_SUPPRESS_UNUSED(hint);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        // TODO: change this to 

        STLSOFT_MESSAGE_ASSERT("may not be called when exception support is not enabled", 0);
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return NULL;
    }
    void do_deallocate(void *pv, size_type n)
    {
        STLSOFT_SUPPRESS_UNUSED(n);
        STLSOFT_SUPPRESS_UNUSED(pv);
    }
    void do_deallocate(void *pv)
    {
        STLSOFT_SUPPRESS_UNUSED(pv);
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Specialisation for void
STLSOFT_TEMPLATE_SPECIALISATION
class null_allocator<void>
{
public:
    typedef void                        value_type;
    typedef null_allocator<void>        class_type;
    typedef void*                       pointer;
    typedef void const*                 const_pointer;
    typedef ss_ptrdiff_t                difference_type;
    typedef ss_size_t                   size_type;

#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    /// The allocator <b><code>rebind</code></b> structure
    template <ss_typename_param_k U>
    struct rebind
    {
        typedef null_allocator<U>       other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ss_bool_t operator ==(const null_allocator<T> &/* lhs */, const null_allocator<T> &/* rhs */)
{
    return ss_true_v;
}

template <ss_typename_param_k T>
inline ss_bool_t operator !=(const null_allocator<T> &/* lhs */, const null_allocator<T> &/* rhs */)
{
    return ss_false_v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/null_allocator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_NULL_ALLOCATOR */

/* ///////////////////////////// end of file //////////////////////////// */
