/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/smartptr/shared_ptr.hpp (originally MLShrPtr.h, ::SynesisStd)
 *
 * Purpose:     Contains the shared_ptr template class.
 *
 * Created:     17th June 2002
 * Updated:     3rd March 2011
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2011, Matthew Wilson and Synesis Software
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


/** \file stlsoft/smartptr/shared_ptr.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::shared_ptr smart
 *   pointer class template
 *   (\ref group__library__smart_pointers "Smart Pointers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR
#define STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SHARED_PTR_MAJOR       3
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SHARED_PTR_MINOR       3
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SHARED_PTR_REVISION    1
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SHARED_PTR_EDIT        38
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

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

/** \brief This class enables sharing of arbitrary types.
 *
 * \ingroup group__library__smart_pointers
 *
 * \remarks The functionality is based in concept on the Boost shared_ptr,
 *   which is set for inclusion in the next instalment of the C++ standard.
 *   The implementation is entirely original.
 *
 * \param T The value type
 */
template <ss_typename_param_k T>
class shared_ptr
{
/// \name Types
/// @{
public:
    typedef T                   value_type;
    typedef value_type*         pointer;
    typedef value_type const*   const_pointer;
    typedef value_type&         reference;
    typedef value_type const&   const_reference;
    typedef shared_ptr<T>       class_type;

    typedef pointer             resource_type;
    typedef const_pointer       const_resource_type;
/// @}

/// \name Construction
/// @{
public:
    shared_ptr()
        : m_p(NULL)
        , m_pc(NULL)
    {
        STLSOFT_ASSERT(is_valid());
    }
    /// 
    ///
    /// \note If exception handling is not enabled and memory cannot be
    ///   acquired to hold the sharing resource the object represented
    ///   by \c p will be deleted, and get() will return \c NULL
    ///
    /// \exception std::bad_alloc If exception support is enabled,
    ///   an instance of <code>std::bad_alloc</code> will be thrown if
    ///   memory cannot be acquired to hold the sharing resource. In this
    ///   case, the object represented by \c p will be deleted
    explicit shared_ptr(T* p)
        : m_p(p)
        , m_pc(NULL)
    {
        // This code prevents leaks in the following case:
        //
        //   shared_ptr<Class>  p(new Class());
        //
        // if the count cannot be allocated

        if(NULL != p)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            try
            {
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

                m_pc = new long(1);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            }
            catch(std::bad_alloc&)
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            if(NULL == m_pc)
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            {
                delete m_p;

                m_p = NULL; // benign for X; necessary for NoX

                throw;
            }
        }

        STLSOFT_ASSERT(is_valid());
    }

    shared_ptr(class_type const& rhs)
        : m_p(rhs.m_p)
        , m_pc(rhs.m_pc)
    {
        STLSOFT_ASSERT(rhs.is_valid());

        if(NULL != m_pc)
        {
            ++*m_pc;
        }

        STLSOFT_ASSERT(is_valid());
    }

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER > 1200)
    template <ss_typename_param_k T2>
    shared_ptr(shared_ptr<T2> const& rhs)
        : m_p(rhs.m_p)
        , m_pc(rhs.m_pc)
    {
        STLSOFT_ASSERT(rhs.is_valid());

        STLSOFT_ASSERT((NULL == m_p) == (NULL == m_pc));

        if(NULL != m_pc)
        {
            ++*m_pc;
        }

        STLSOFT_ASSERT(is_valid());
    }
#endif /* member template support? */

    ~shared_ptr() stlsoft_throw_0()
    {
        STLSOFT_ASSERT(is_valid());

        STLSOFT_ASSERT((NULL == m_p) == (NULL == m_pc));
        STLSOFT_ASSERT((NULL == m_pc) || (0 < *m_pc));

        if( NULL != m_pc &&
            0 == --*m_pc)
        {
            delete m_p;
            delete m_pc;
        }
    }

    class_type& operator =(class_type const& rhs)
    {
        STLSOFT_ASSERT(is_valid());

        class_type  this_(rhs);

        this_.swap(*this);

        STLSOFT_ASSERT(is_valid());

        return *this;
    }

#if defined(STLSOFT_CF_MEMBER_TEMPLATE_FUNCTION_SUPPORT) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER > 1200)
    template <ss_typename_param_k T2>
    class_type& operator =(shared_ptr<T2> const& rhs)
    {
        STLSOFT_ASSERT(rhs.is_valid());

        STLSOFT_ASSERT(is_valid());

        class_type  this_(rhs);

        this_.swap(*this);

        STLSOFT_ASSERT(is_valid());

        return *this;
    }
#endif /* member template support? */
/// @}

/// \name Operations
/// @{
public:
    void close()
    {
        STLSOFT_ASSERT((NULL == m_p) == (NULL == m_pc));
        STLSOFT_ASSERT((NULL == m_pc) || (0 < *m_pc));

        STLSOFT_ASSERT(is_valid());

        if(NULL != m_pc)
        {
            pointer p   =   m_p;
            long    *pc =   m_pc;

            // Set the members to NULL prior to possibly
            // deleting, in case close() is called on a
            // shared_ptr member which is holding a
            // reference to the enclosing instance.

            m_p     =   NULL;
            m_pc    =   NULL;

            if(0 == --*pc)
            {
                delete p;
                delete pc;
            }
        }

        STLSOFT_ASSERT(is_valid());
    }

#if 1 && !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
    void reset()
    {
        close();
    }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

    pointer detach()
    {
        STLSOFT_ASSERT((NULL == m_p) == (NULL == m_pc));
        STLSOFT_ASSERT((NULL == m_pc) || (0 < *m_pc));

        STLSOFT_ASSERT(is_valid());

        pointer p   =   NULL;

        if(NULL != m_pc)
        {
            if(0 == --*m_pc)
            {
                delete m_pc;

                m_pc = NULL;
            }

            std_swap(p, m_p);
        }

        STLSOFT_ASSERT(is_valid());

        return p;
    }

    void swap(class_type& rhs)
    {
        STLSOFT_ASSERT(rhs.is_valid());

        STLSOFT_ASSERT(is_valid());

        std_swap(m_p, rhs.m_p);
        std_swap(m_pc, rhs.m_pc);

        STLSOFT_ASSERT(is_valid());
    }
/// @}

/// \name Accessors
/// @{
public:
    const_pointer operator ->() const
    {
        STLSOFT_ASSERT(NULL != m_p);
        STLSOFT_ASSERT(is_valid());

        return m_p;
    }
    pointer operator ->()
    {
        STLSOFT_ASSERT(NULL != m_p);
        STLSOFT_ASSERT(is_valid());

        return m_p;
    }

    pointer get() const
    {
        STLSOFT_ASSERT(is_valid());

        return m_p;
    }

    const_reference operator *() const
    {
        STLSOFT_ASSERT(NULL != m_p);
        STLSOFT_ASSERT(is_valid());

        return *m_p;
    }
    reference operator *()
    {
        STLSOFT_ASSERT(NULL != m_p);
        STLSOFT_ASSERT(is_valid());

        return *m_p;
    }
/// @}

/// \name Attributes
/// @{
public:
    long    count() const
    {
        STLSOFT_ASSERT(is_valid());

        return (NULL == m_pc) ? 0 : *m_pc;
    }
    /// \brief Returns count()
    long    use_count() const
    {
        STLSOFT_ASSERT(is_valid());

        return this->count();
    }
/// @}

/// \name Implementation
/// @{
private:
    ss_bool_t is_valid() const
    {
        if((NULL == m_p) != (NULL == m_pc))
        {
#ifdef STLSOFT_UNITTEST
            fprintf(err, "Managed object's pointer and shared count pointer must both be NULL, or both non-NULL!\n");
#endif /* STLSOFT_UNITTEST */

            return false;
        }

        if( NULL != m_pc &&
            *m_pc < 1)
        {
#ifdef STLSOFT_UNITTEST
            fprintf(err, "Shared count cannot be less than 1!\n");
#endif /* STLSOFT_UNITTEST */

            return false;
        }

        return true;
    }
/// @}

/// \name Members
/// @{
private:
    pointer m_p;
    long    *m_pc;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template <ss_typename_param_k T>
void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#if !defined(STLSOFT_COMPILER_IS_WATCOM)

/** \brief get_ptr shim
 *
 * \ingroup group__library__smart_pointers
 */
template<ss_typename_param_k T>
inline T* get_ptr(shared_ptr<T> const& p)
{
    return p.get();
}

/** \brief Insertion operator shim
 *
 * \ingroup group__library__smart_pointers
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k T
        >
inline S& operator <<(S& s, shared_ptr<T> const& p)
{
    return s << *p;
}

#endif /* compiler */

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
            >
    inline void swap(stlsoft_ns_qual(shared_ptr)<T>& lhs, stlsoft_ns_qual(shared_ptr)<T>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR */

/* ///////////////////////////// end of file //////////////////////////// */
