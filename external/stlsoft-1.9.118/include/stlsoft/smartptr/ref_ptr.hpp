/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/smartptr/ref_ptr.hpp (originally MLRelItf.h, ::SynesisStd)
 *
 * Purpose:     Contains the ref_ptr template class.
 *
 * Created:     2nd November 1994
 * Updated:     14th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/smartptr/ref_ptr.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::ref_ptr smart
 *   pointer class template
 *   (\ref group__library__smart_pointers "Smart Pointers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR
#define STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_REF_PTR_MAJOR      5
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_REF_PTR_MINOR      3
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_REF_PTR_REVISION   2
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_REF_PTR_EDIT       489
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper shims
 */

/** \brief Control shim for adding a reference on a reference-counted
 *    interface (RCI)
 *
 * \ingroup group__library__smart_pointers
 *
 * \note The generic shim expects the RCI to have a method named AddRef(), which
 * has either no parameters, or has all default parameters
 *
 * \note The behaviour of the ref_ptr is undefined if this method throws an
 * exception
 */
template<ss_typename_param_k I>
inline void add_reference(I* pi)
{
    STLSOFT_ASSERT(NULL != pi);

    pi->AddRef();
}

/** \brief Control shim for releasing a reference on a reference-counted
 *    interface (RCI)
 *
 * \ingroup group__library__smart_pointers
 *
 * \note The generic shim expects the RCI to have a method named Release(), which
 * has either no parameters, or has all default parameters
 *
 * \note The behaviour of the ref_ptr is undefined if this method throws an
 * exception
 */
template<ss_typename_param_k I>
inline void release_reference(I* pi)
{
    STLSOFT_ASSERT(NULL != pi);

    pi->Release();
}

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief This class provides RAII-safe handling of reference-counted
 *    interfaces (RCIs). Its notable feature is that it supports forward
 *    declaration of the leaf interface so long as the base counting
 *    interface is visible in the scope of the template parameterisation.
 *
 * \ingroup group__library__smart_pointers
 *
 * \param T The counted type (i.e. a concrete class)
 * \param I The interface type
 * \param U The upcast intermediate type
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k I = T
        ,   ss_typename_param_k U = I
        >
class ref_ptr
{
/// \name Types
/// @{
public:
    /// \brief The Boolean type
    typedef bool_t              bool_type;
    /// \brief The interface type: the type of the RCI (Reference-Counted Interface)
    typedef I                   interface_type;
    /// \brief The counted type: the concrete type of the objects whose instances will be managed
    typedef T                   counted_type;
    /// \brief The up-cast type: the type used to disambiguate upcasts between T and I
    typedef U                   upcast_type;
    /// \brief The current instantiation of the type
    typedef ref_ptr<T, I, U>    class_type;

    /// \brief This to be member-type-compatible with std::auto_ptr
    typedef I                   element_type;
    /// \brief This is to be compatible with the get_invoker component
    typedef counted_type*       resource_type;
    typedef counted_type const* const_resource_type;
/// @}

/// \name Implementation
/// @{
private:
    /// \brief Helper function to effect downcast from interface type to counted type
    static counted_type* c_from_i(interface_type* i)
    {
        return static_cast<counted_type*>(static_cast<upcast_type*>(i));
    }
    /// \brief Helper function to effect downcast from interface type to counted type
    static counted_type const* c_from_i(interface_type const* i)
    {
        return static_cast<counted_type const*>(static_cast<upcast_type const*>(i));
    }
    /// \brief Helper function to effect upcast from counted type to interface type
    static interface_type* i_from_c(counted_type* c)
    {
        return static_cast<upcast_type*>(c);
    }

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1300
    /// \brief Helper function to effect upcast from const counted type to interface type
    static interface_type* i_from_const_c(counted_type const* cc)
    {
        counted_type* c = const_cast<counted_type*>(cc);

        return i_from_c(c);
    }
#endif /* compiler */
/// @}

/// \name Construction
/// @{
public:
    /// \brief Default constructor
    ///
    /// Constructs and empty instance
    ref_ptr()
        : m_pi(NULL)
    {}
    /// \brief Construct from a raw pointer to the counted type, and a boolean that
    /// indicates whether a reference should be taken on the instance.
    ///
    /// \param c Pointer to a counted_type. May be NULL
    /// \param bAddRef parameter that determines whether reference will be
    ///   <i>consumed</i> (<code>false</code>) or <i>borrowed</i>
    ///   (<code>true</code>).
    ///
    /// \note It is usual that ref_ptr is used to "sink" an instance, i.e. to take
    /// ownership of it. In such a case, \c false should be specified as the second
    /// parameter. If, however, a reference is being "borrowed", then \c true should
    /// be specified.
    ref_ptr(counted_type* c, bool_type bAddRef)
        : m_pi(i_from_c(c))
    {
        if( bAddRef &&
            NULL != m_pi)
        {
            add_reference(m_pi);
        }
    }

    /// \brief Creates a copy of the given ref_ptr instance, and increments the
    /// reference count on its referent object, if any
    ///
    /// \param rhs The instance to copy
    ref_ptr(class_type const& rhs)
        : m_pi(rhs.m_pi)
    {
        if(NULL != m_pi)
        {
            add_reference(m_pi);
        }
    }

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER > 1100
    /// \brief Copy constructs from an instance with different interface and/or
    ///   counted type
    ///
    /// \note The interface types of the copying and copied instance must be
    ///   compatible
    template<   ss_typename_param_k T2
            ,   ss_typename_param_k I2
            ,   ss_typename_param_k U2
            >
# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER == 1300
    ref_ptr(ref_ptr<T2, I2, U2> const& rhs)
#  if 0
        // We cannot use this form, as it would lead to instances with different
        // counted_type being cross cast invisibly. This would be a *very bad thing*
        : m_pi(rhs.m_pi)
#  else /* ? 0 */
        : m_pi(i_from_const_c(rhs.get()))
#  endif /* 0 */
    {
        if(NULL != m_pi)
        {
            add_reference(m_pi);
        }
    }
# else /* ? compiler */
    ref_ptr(ref_ptr<T2, I2, U2>& rhs)
#  if 0
        // We cannot use this form, as it would lead to instances with different
        // counted_type being cross cast invisibly. This would be a *very bad thing*
        : m_pi(rhs.m_pi)
#  else /* ? 0 */
        : m_pi(i_from_c(rhs.get()))
#  endif /* 0 */
    {
        if(NULL != m_pi)
        {
            add_reference(m_pi);
        }
    }
# endif /* compiler */
#endif /* compiler */

#if !defined(STLSOFT_COMPILER_IS_INTEL) && \
    !defined(STLSOFT_COMPILER_IS_MWERKS) && \
    0
    template<   ss_typename_param_k I2
            ,   ss_typename_param_k U2
            >
    explicit ref_ptr(ref_ptr<T, I2, U2>& rhs)
        : m_pi(rhs.m_pi)
    {
        if(NULL != m_pi)
        {
            add_reference(m_pi);
        }
    }
#endif /* compiler */

    /// \brief Destructor
    ///
    /// If the ref_ptr instance is still holding a pointer to a managed instance,
    /// it will be released.
    ~ref_ptr() stlsoft_throw_0()
    {
        if(NULL != m_pi)
        {
            release_reference(m_pi);
        }
    }

    /// \brief Copy assignment from a ref_ptr instance of the same type
    ///
    /// \note It is strongly exception-safe, as long as the implementations of the
    /// add-ref and release functions - as utilised in the \c add_reference() and
    /// \c release_reference() control shims - do not throw (which they must not).
    class_type& operator =(class_type const& rhs)
    {
        class_type  t(rhs);

        t.swap(*this);

        return *this;
    }

#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    (   _MSC_VER > 1100 && \
        _MSC_VER != 1300)
    /// \brief Copy assignment from an instance of ref_ptr with a different counted_type (but
    /// the same interface type).
    ///
    /// \note This function template uses the copy constructor template, and has the same
    /// instantiation restrictions
    ///
    /// \note It is strongly exception-safe, as long as the implementations of the
    /// add-ref and release functions - as utilised in the \c add_reference() and
    /// \c release_reference() control shims - do not throw (which they must not).
    template<   ss_typename_param_k T2
            ,   ss_typename_param_k U2
            >
    class_type& operator =(ref_ptr<T2, I, U2>& rhs)
    {
        class_type  t(rhs);

        t.swap(*this);

        return *this;
    }
#endif /* compiler */

#if !defined(STLSOFT_COMPILER_IS_INTEL) && \
    !defined(STLSOFT_COMPILER_IS_MWERKS) && \
    0
    template<   ss_typename_param_k I2
            ,   ss_typename_param_k U2
            >
    class_type& operator =(ref_ptr<T, I2, U2>& rhs)
    {
        class_type  t(rhs);

        t.swap(*this);

        return *this;
    }
#endif /* compiler */
/// @}

/// \name Operations
/// @{
public:
    /// \brief Swaps the managed instance of \c this with \c rhs
    ///
    /// \note It provides the no-throw guarantee
    void swap(class_type& rhs)
    {
        interface_type* t           =   rhs.m_pi;
                        rhs.m_pi    =   m_pi;
                        m_pi        =   t;
    }

    /// \brief Assigns a reference-counted type to the smart pointer.
    ///
    /// \param c Pointer to a counted_type. May be NULL
    /// \param bAddRef parameter that determines whether reference will be
    ///   <i>consumed</i> (<code>false</code>) or <i>borrowed</i>
    ///   (<code>true</code>).
    void set(counted_type* c, bool_type bAddRef)
    {
        class_type  t(c, bAddRef);

        t.swap(*this);
    }

    /// Closes the instance, releasing the managed pointer.
    ///
    /// \note Calling this method more than once has no effect.
    void close()
    {
        if(NULL != m_pi)
        {
            release_reference(m_pi);
            m_pi = NULL;
        }
    }

    /// \brief Detaches the managed instance, and returns it to the caller, which
    /// takes responsibility for ensuring that the resource is not leaked
    counted_type* detach()
    {
        counted_type* r = class_type::c_from_i(m_pi);

        m_pi = NULL;

        return r;
    }
/// @}

/// \name Equality Comparison
/// @{
public:
    /// \brief Evaluates whether two instances are equal
    bool_type equal(class_type const& rhs) const
    {
        return m_pi == rhs.m_pi;
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Determines whether the instance is empty
    bool_type empty() const
    {
        return NULL == m_pi;
    }

    /// \brief Determines whether the instance is empty
    bool_type operator !() const
    {
        return empty();
    }

    /// \brief Provides raw-pointer access to the instance
    counted_type* get() const
    {
        return class_type::c_from_i(m_pi);
    }

    /// \brief Returns the interface pointer
    ///
    /// \pre The instance must not be empty; otherwise behaviour is
    ///   undefined
    counted_type* operator ->()
    {
        STLSOFT_MESSAGE_ASSERT("Dereferencing a NULL pointer!", NULL != m_pi);

        return class_type::c_from_i(m_pi);
    }

    /// \brief Returns the interface pointer
    ///
    /// \pre The instance must not be empty; otherwise behaviour is
    ///   undefined
    counted_type const* operator ->() const
    {
        STLSOFT_MESSAGE_ASSERT("Dereferencing a NULL pointer!", NULL != m_pi);

        return class_type::c_from_i(m_pi);
    }

    /// \brief Returns a reference to the managed instance
    ///
    /// \pre The instance must not be empty; otherwise behaviour is
    ///   undefined
    counted_type& operator *()
    {
        STLSOFT_MESSAGE_ASSERT("Dereferencing a NULL pointer!", NULL != m_pi);

        return *class_type::c_from_i(m_pi);
    }

    /// \brief Returns a reference to the managed instance
    ///
    /// \pre The instance must not be empty; otherwise behaviour is
    ///   undefined
    counted_type const& operator *() const
    {
        STLSOFT_MESSAGE_ASSERT("Dereferencing a NULL pointer!", NULL != m_pi);

        return *class_type::c_from_i(m_pi);
    }
/// @}

/// \name Members
/// @{
private:
    interface_type* m_pi;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k I
        ,   ss_typename_param_k U
        >
inline ss_bool_t operator ==(ref_ptr<T, I, U> const& lhs, ref_ptr<T, I, U> const& rhs)
{
    return lhs.equal(rhs);
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k I
        ,   ss_typename_param_k U
        >
inline ss_bool_t operator !=(ref_ptr<T, I, U> const& lhs, ref_ptr<T, I, U> const& rhs)
{
    return !lhs.equal(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k T
        ,   ss_typename_param_k I
        ,   ss_typename_param_k U
        >
inline void swap(ref_ptr<T, I, U>& lhs, ref_ptr<T, I, U>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

/** \brief is_empty shim
 *
 * \ingroup group__library__smart_pointers
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k I /* = T */
        ,   ss_typename_param_k U /* = I */
        >
inline ss_bool_t is_empty(ref_ptr<T, I, U> const& p)
{
    return NULL == p.get();
}

/** \brief get_ptr shim
 *
 * \ingroup group__library__smart_pointers
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k I /* = T */
        ,   ss_typename_param_k U /* = I */
        >
inline T* get_ptr(ref_ptr<T, I, U> const& p)
{
    return p.get();
}

/** \brief Insertion operator shim
 *
 * \ingroup group__library__smart_pointers
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        ,   ss_typename_param_k I /* = T */
        ,   ss_typename_param_k U /* = I */
        >
inline S& operator <<(S& s, ref_ptr<T, I, U> const& p)
{
    return s << *p;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/ref_ptr_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* In the special case of Intel behaving as VC++ 7.0 or earlier on Win32, we
 * illegally insert into the std namespace.
 */
#if defined(STLSOFT_CF_std_NAMESPACE)
# if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
         defined(_MSC_VER))) && \
     _MSC_VER < 1310
namespace std
{
    template<   ss_typename_param_k T
            ,   ss_typename_param_k I
            ,   ss_typename_param_k U
            >
    inline void swap(stlsoft_ns_qual(ref_ptr)<T, I, U>& lhs, stlsoft_ns_qual(ref_ptr)<T, I, U>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR */

/* ///////////////////////////// end of file //////////////////////////// */
