/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/memory/task_allocator.hpp
 *
 * Purpose:     task_allocator class.
 *
 * Created:     19th January 2002
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


/** \file comstl/memory/task_allocator.hpp
 *
 * \brief [C++ only] Definition of the comstl::task_allocator class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef COMSTL_INCL_COMSTL_MEMORY_HPP_TASK_ALLOCATOR
#define COMSTL_INCL_COMSTL_MEMORY_HPP_TASK_ALLOCATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_MEMORY_HPP_TASK_ALLOCATOR_MAJOR      4
# define COMSTL_VER_COMSTL_MEMORY_HPP_TASK_ALLOCATOR_MINOR      0
# define COMSTL_VER_COMSTL_MEMORY_HPP_TASK_ALLOCATOR_REVISION   4
# define COMSTL_VER_COMSTL_MEMORY_HPP_TASK_ALLOCATOR_EDIT       87
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

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */

#ifndef STLSOFT_INCL_NEW
# define STLSOFT_INCL_NEW
# include <new>
#endif /* !STLSOFT_INCL_NEW */

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

/** \brief STL Allocator based on the COM task allocator
 *
 * \ingroup group__library__memory
 *
 * \param T The value_type of the allocator
 */
template <ss_typename_param_k T>
class task_allocator
    : public allocator_base<T, task_allocator<T> >
{
private:
    typedef allocator_base<T, task_allocator<T> >                   parent_class_type;
public:
    /// The parameterisation of the class
    typedef task_allocator<T>                                       class_type;
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
        typedef task_allocator<U>                                   other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */

/// \name Construction
/// @{
public:
    /// Default constructor
    task_allocator() stlsoft_throw_0()
    {}
    /// Copy constructor
#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    template <ss_typename_param_k U>
    task_allocator(task_allocator<U> const&)
    {}
#else /* ? STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
    task_allocator(class_type const&)
    {}
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
/// @}

private:
    friend class allocator_base<T, task_allocator<T> >;

    void *do_allocate(size_type n, void const* hint)
    {
        STLSOFT_SUPPRESS_UNUSED(hint);

        return ::CoTaskMemAlloc(n * sizeof(value_type));
    }
    void do_deallocate(void *pv, size_type n)
    {
        STLSOFT_SUPPRESS_UNUSED(n);

        ::CoTaskMemFree(pv);
    }
    void do_deallocate(void *pv)
    {
        ::CoTaskMemFree(pv);
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Specialisation for void
STLSOFT_TEMPLATE_SPECIALISATION
class task_allocator<void>
{
public:
    typedef void                    value_type;
    typedef task_allocator<void>    class_type;
    typedef void*                   pointer;
    typedef void const*             const_pointer;
    typedef cs_ptrdiff_t            difference_type;
    typedef cs_size_t               size_type;

#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    /// The allocator <b><code>rebind</code></b> structure
    template <ss_typename_param_k U>
    struct rebind
    {
        typedef task_allocator<U>   other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline cs_bool_t operator ==(const task_allocator<T> &/* lhs */, const task_allocator<T> &/* rhs */)
{
    return cs_true_v;
}

template <ss_typename_param_k T>
inline cs_bool_t operator !=(const task_allocator<T> &/* lhs */, const task_allocator<T> &/* rhs */)
{
    return cs_false_v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/task_allocator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace comstl
# else
} // namespace comstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_MEMORY_HPP_TASK_ALLOCATOR */

/* ///////////////////////////// end of file //////////////////////////// */
