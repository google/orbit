/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/memory/afx_allocator.hpp
 *
 * Purpose:     afx_allocator class.
 *
 * Created:     5th August 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file mfcstl/memory/afx_allocator.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::afx_allocator class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR
#define MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR_MAJOR       2
# define MFCSTL_VER_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR_MINOR       0
# define MFCSTL_VER_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR_REVISION    4
# define MFCSTL_VER_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR_EDIT        21
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

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::runtime_error
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifndef STLSOFT_INCL_H_AFX
# define STLSOFT_INCL_H_AFX
# include <afx.h>
#endif /* !STLSOFT_INCL_H_AFX */

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
 * Classes
 */

/** \brief STL Allocator based on the MFC memory framework
 *
 * \ingroup group__library__memory
 *
 * \param T The value_type of the allocator
 */
template <ss_typename_param_k T>
class afx_allocator
    : public allocator_base<T, afx_allocator<T> >
{
private:
    typedef allocator_base<T, afx_allocator<T> >                    parent_class_type;
public:
    /// The parameterisation of the class
    typedef afx_allocator<T>                                        class_type;
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
        typedef afx_allocator<U>                                    other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */

// Construction
public:
    /// Default constructor
    afx_allocator() stlsoft_throw_0()
    {}
    /// Copy constructor
    afx_allocator(class_type const& rhs) stlsoft_throw_0()
    {}
#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    template <ss_typename_param_k U>
    afx_allocator(afx_allocator<U> const& rhs) stlsoft_throw_0()
    {}
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
    /// Destructor
    ~afx_allocator() stlsoft_throw_0()
    {}

    class_type const& operator =(class_type const& rhs)
    {
        return *this;
    }

private:
    friend class allocator_base<T, afx_allocator<T> >;

    void *do_allocate(size_type n, void const* hint)
    {
        STLSOFT_SUPPRESS_UNUSED(hint);

        return new BYTE[n * sizeof(value_type)];
    }
    void do_deallocate(void *pv, size_type n)
    {
        STLSOFT_SUPPRESS_UNUSED(n);

        delete [] static_cast<BYTE*>(pv);
    }
    void do_deallocate(void *pv)
    {
        delete [] static_cast<BYTE*>(pv);
    }
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Specialisation for void
STLSOFT_TEMPLATE_SPECIALISATION
class afx_allocator<void>
{
public:
    typedef void                    value_type;
    typedef afx_allocator<void>     class_type;
    typedef void*                   pointer;
    typedef void const*             const_pointer;
    typedef ms_ptrdiff_t            difference_type;
    typedef ms_size_t               size_type;

#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    /// The allocator <b><code>rebind</code></b> structure
    template <ss_typename_param_k U>
    struct rebind
    {
        typedef afx_allocator<U>  other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ms_bool_t operator ==(const afx_allocator<T> &/* lhs */, const afx_allocator<T> &/* rhs */)
{
    return ms_true_v;
}

template <ss_typename_param_k T>
inline ms_bool_t operator !=(const afx_allocator<T> &/* lhs */, const afx_allocator<T> &/* rhs */)
{
    return ms_false_v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/afx_allocator_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR */

/* ///////////////////////////// end of file //////////////////////////// */
