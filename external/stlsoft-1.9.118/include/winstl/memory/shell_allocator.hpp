/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/memory/shell_allocator.hpp
 *
 * Purpose:     shell_allocator class.
 *
 * Created:     2nd March 2002
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


/** \file winstl/memory/shell_allocator.hpp
 *
 * \brief [C++ only] Definition of the winstl::shell_allocator class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR
#define WINSTL_INCL_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR_MAJOR     4
# define WINSTL_VER_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR_MINOR     1
# define WINSTL_VER_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR_REVISION  5
# define WINSTL_VER_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR_EDIT      83
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
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE
# include <stlsoft/memory/allocator_base.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_BASE */

#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::runtime_error
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifndef STLSOFT_INCL_H_SHLOBJ
# define STLSOFT_INCL_H_SHLOBJ
# include <shlobj.h>                     // for SHGetMalloc()
#endif /* !STLSOFT_INCL_H_SHLOBJ */

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

/** \brief STL Allocator based on the Win32 shell allocator
 *
 * \ingroup group__library__memory
 *
 * \param T The value_type of the allocator
 */
template <ss_typename_param_k T>
class shell_allocator
    : public allocator_base<T, shell_allocator<T> >
{
private:
    typedef allocator_base<T, shell_allocator<T> >                  parent_class_type;
public:
    /// The parameterisation of the class
    typedef shell_allocator<T>                                      class_type;
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
        typedef shell_allocator<U>                                  other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */

/// \name Construction
/// @{
public:
    /// Default constructor
    shell_allocator() stlsoft_throw_1(stlsoft_ns_qual_std(runtime_error) )
        : m_malloc(get_malloc_())
    {}
    /// Copy constructor
#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    template <ss_typename_param_k U>
    shell_allocator(shell_allocator<U> const& rhs) stlsoft_throw_0()
        : m_malloc(get_malloc_())
    {}
#else /* ? STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
    shell_allocator(class_type const& rhs) stlsoft_throw_0()
        : m_malloc(addref_malloc_(rhs.m_malloc))
    {}
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
    /// Destructor
    ~shell_allocator() stlsoft_throw_0()
    {
        if(m_malloc != NULL)
        {
            m_malloc->Release();
        }
    }

    /// Copy assignment operator
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    class_type const& operator =(class_type const& rhs)
    {
        return *this;
    }
/// @}

private:
    friend class allocator_base<T, shell_allocator<T> >;

    void *do_allocate(size_type n, void const* hint)
    {
        STLSOFT_SUPPRESS_UNUSED(hint);

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        if(NULL != m_malloc)
        {
            return NULL;
        }
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

        return static_cast<void*>(m_malloc->Alloc(n * sizeof(value_type)));
    }
    void do_deallocate(void *pv, size_type n)
    {
        STLSOFT_SUPPRESS_UNUSED(n);

        m_malloc->Free(pv);
    }
    void do_deallocate(void *pv)
    {
        m_malloc->Free(pv);
    }

// Implementation
private:
    static LPMALLOC get_malloc_() stlsoft_throw_1(stlsoft_ns_qual_std(runtime_error) )
    {
        LPMALLOC    lpMalloc;
        HRESULT     hr  =   ::SHGetMalloc(&lpMalloc);

        if(FAILED(hr))
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(stlsoft_ns_qual_std(runtime_error)("failed to retrieve the shell allocator"));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            lpMalloc = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }

        return lpMalloc;
    }
    static LPMALLOC addref_malloc_(LPMALLOC lpMalloc)
    {
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        if(NULL != lpMalloc)
        {
            return NULL;
        }
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

        lpMalloc->AddRef();

        return lpMalloc;
    }

// Members
private:
    LPMALLOC    m_malloc;
};

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// Specialisation for void
STLSOFT_TEMPLATE_SPECIALISATION
class shell_allocator<void>
{
public:
    typedef void                    value_type;
    typedef shell_allocator<void>   class_type;
    typedef void*                   pointer;
    typedef void const*             const_pointer;
    typedef ws_ptrdiff_t            difference_type;
    typedef ws_size_t               size_type;

#ifdef STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT
    /// The allocator <b><code>rebind</code></b> structure
    template <ss_typename_param_k U>
    struct rebind
    {
        typedef shell_allocator<U>  other;
    };
#endif /* STLSOFT_CF_ALLOCATOR_REBIND_SUPPORT */
};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k T>
inline ws_bool_t operator ==(const shell_allocator<T> &/* lhs */, const shell_allocator<T> &/* rhs */)
{
    return ws_true_v;
}

template <ss_typename_param_k T>
inline ws_bool_t operator !=(const shell_allocator<T> &/* lhs */, const shell_allocator<T> &/* rhs */)
{
    return ws_false_v;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/shell_allocator_unittest_.h"
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

#endif /* WINSTL_INCL_WINSTL_MEMORY_HPP_SHELL_ALLOCATOR */

/* ///////////////////////////// end of file //////////////////////////// */
