/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/acyclic_connector.hpp
 *
 * Purpose:     A component for relating two COM objects without cycles.
 *
 * Created:     25th March 2006
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


/** \file comstl/util/acyclic_connector.hpp
 *
 * \brief [C++ only; requires COM] Definition of the
 *   comstl::acyclic_connector class template
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR
#define COMSTL_INCL_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR_MAJOR     1
# define COMSTL_VER_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR_MINOR     2
# define COMSTL_VER_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR_REVISION  7
# define COMSTL_VER_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR_EDIT      21
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_OBJECT_FUNCTIONS
# include <comstl/util/object_functions.h>
#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_OBJECT_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE
# include <stlsoft/synch/lock_scope.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE */

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

/** \brief Interface for acyclic communication.
 */
#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
struct IAcyclicSide
    : public IUnknown
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
DECLARE_INTERFACE_(IAcyclicSide, IUnknown)
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */
{
    /** \brief Causes knowledge of the presence of the peer to be discarded
     */
    STDMETHOD_(void, Clear)() PURE;
    /** \brief Queries for the given interface on the peer.
     *
     * \param riid The interface identifier of the requested interface.
     * \param ppv Address of the interface pointer.
     *
     * \return A standard HRESULT status code indicating success/failure.
     */
    STDMETHOD(QueryPeer)(THIS_ REFIID riid, void **ppv) PURE;

    /** \brief returns the IID for the IAcyclicSide interface. */
    static REFIID iid()
    {
        static IID s_iid = { 0x8D5D0B0A, 0x4429, 0x4be1, { 0x8C, 0x00, 0xDE, 0xE0, 0xA8, 0xFF, 0xD0, 0xAF } };

        return s_iid;
    }
};

/** \brief A component that allows two objects to be connected without
 *   creating cyclic dependencies.
 *
 * Reference counting architectures, such as COM, rely on there being no
 * dependency cycles. A cyclic dependency is the situation where, say,
 * object <b>a</b> holds a reference to object <b>b</b>, and object
 * <b>b</b> holds a reference to object <b>a</b>. Since neither will
 * release the references it holds (held on its efferent dependents)
 * until all references to itself (held by its afferent dependents) the
 * circle will never be broken.
 *
 * COM lore thus proscribes the use of mutual references, or prescribes
 * rigid protocols (e.g. Connection Points) for their management.
 *
 * acyclic_connector is designed to help in the case where two objects
 * need to have knowledge of each other, but not hold references on each
 * other. It works by acting as an intermediary that each of two objects
 * in the relationship keep informed as to their existance.
 *
 * \param MX The type of the mutex that will be used to provide exclusive
 *  access to the instance state during the Clear() and QueryPeer() methods.
 */
template <ss_typename_param_k MX>
class acyclic_connector
{
/// \name Member Types
/// @{
public:
    typedef MX                      mutex_type;
private:
    typedef acyclic_connector<MX>   connector_type;
public:
    typedef acyclic_connector<MX>   class_type;
private:
    class side
        : public IAcyclicSide
    {
    /// \name Member Types
    /// @{
    public:
        typedef side            class_type;
    /// @}

    /// \name Construction
    /// @{
    public:
        ss_explicit_k side(connector_type &connector, LPUNKNOWN peer, IAcyclicSide **side)
            : m_connector(connector)
            , m_peer(peer)
            , m_refCount(1)
        {
            COMSTL_ASSERT(NULL != side);
            *side = this;
        }
    /// @}

    /// \name IUnknown
    /// @{
    public:
        STDMETHOD_(ULONG, AddRef)()
        {
            return ++m_refCount;
        }
        STDMETHOD_(ULONG, Release)()
        {
            class_type  &other  =   (this == &m_connector.m_left) ? m_connector.m_right : m_connector.m_left;

            if(0 == --m_refCount)
            {
                m_peer = NULL;

                if(0 == other.m_refCount)
                {
                    delete &m_connector;

                    return 0;
                }
            }

            return m_refCount;
        }
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv)
        {
            if( IID_IUnknown == riid ||
                IAcyclicSide::iid() == riid)
            {
                *ppv = static_cast<LPUNKNOWN>(this);
                static_cast<LPUNKNOWN>(*ppv)->AddRef();
                return S_OK;
            }

            return E_NOINTERFACE;
        }
    /// @}

    /// \name IAcyclicSide
    /// @{
    public:
        STDMETHOD_(void, Clear)()
        {
            stlsoft::lock_scope<mutex_type> lock(m_connector.m_mx);

            m_peer = NULL;
        }
        STDMETHOD(QueryPeer)(THIS_ REFIID riid, void **ppv)
        {
            COMSTL_ASSERT(NULL != ppv);

            stlsoft::lock_scope<mutex_type> lock(m_connector.m_mx);
            class_type                      &other  =   (this == &m_connector.m_left) ? m_connector.m_right : m_connector.m_left;

            if(NULL == other.m_peer)
            {
                return E_POINTER;
            }
            else
            {
                return other.m_peer->QueryInterface(riid, ppv);
            }
        }
    /// @}

    /// \name Members
    /// @{
    private:
        connector_type  &m_connector;
        LPUNKNOWN       m_peer;
        LONG            m_refCount;
    /// @}
    };
    friend class side;
/// @}

/// \name Construction
/// @{
public:
    acyclic_connector(  LPUNKNOWN       leftPeer
                    ,   IAcyclicSide    **leftSide
                    ,   LPUNKNOWN       rightPeer
                    ,   IAcyclicSide    **rightSide);
private:
    ~acyclic_connector() stlsoft_throw_0();
/// @}

/// \name Implementation
/// @{
private:
    class_type& get_this_();
/// @}

/// \name Construction
/// @{
private:
    side        m_left;
    side        m_right;
    mutex_type  m_mx;
/// @}

/// \name Not to be implemented
/// @{
private:
    acyclic_connector(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// acyclic_connector::side

#if 0
template <ss_typename_param_k MX>
inline acyclic_connector<MX>::side::side(connector_type &connector, LPUNKNOWN peer, IAcyclicSide **side)
    : m_connector(connector)
    , m_peer(peer)
    , m_refCount(1)
{
    COMSTL_ASSERT(NULL != side);
    *side = this;
}

template <ss_typename_param_k MX>
inline STDMETHODIMP_(ULONG) acyclic_connector<MX>::side::AddRef()
{
    return ++m_refCount;
}

template <ss_typename_param_k MX>
inline STDMETHODIMP_(ULONG) acyclic_connector<MX>::side::Release()
{
    class_type  &other  =   (this == &m_connector.m_left) ? m_connector.m_right : m_connector.m_left;

    if( 0 == --m_refCount &&
        0 == other.m_refCount)
    {
        delete &m_connector;

        return 0;
    }

    return m_refCount;
}

template <ss_typename_param_k MX>
inline STDMETHODIMP acyclic_connector<MX>::side::QueryInterface(REFIID riid, void **ppv)
{
    if( IID_IUnknown == riid ||
        IAcyclicSide::iid() == riid)
    {
        *ppv = static_cast<LPUNKNOWN>(this);
        static_cast<LPUNKNOWN>(*ppv)->AddRef();
        return S_OK;
    }

    return E_INTERFACE;
}
#endif /* 0 */

#if 0
template <ss_typename_param_k MX>
inline STDMETHODIMP_(void) acyclic_connector<MX>::side::Clear()
{
    stlsoft::lock_scope<mutex_type> lock(m_mx);

    m_peer = NULL;
}

template <ss_typename_param_k MX>
inline STDMETHODIMP acyclic_connector<MX>::side::QueryPeer(THIS_ REFIID riid, void **ppv)
{
    COMSTL_ASSERT(NULL != ppv);

    stlsoft::lock_scope<mutex_type> lock(m_mx);

    if(NULL == m_peer)
    {
        return E_POINTER;
    }
    else
    {
        return m_peer->QueryInterface(riid, ppv);
    }
}
#endif /* 0 */

// acyclic_connector

template <ss_typename_param_k MX>
inline acyclic_connector<MX>::acyclic_connector(LPUNKNOWN       leftPeer
                                            ,   IAcyclicSide    **leftSide
                                            ,   LPUNKNOWN       rightPeer
                                            ,   IAcyclicSide    **rightSide)
    : m_left(get_this_(), leftPeer, leftSide)
    , m_right(get_this_(), rightPeer, rightSide)
    , m_mx()
{
    COMSTL_MESSAGE_ASSERT("Acyclic connector cannot be initialised with null pointers", NULL != leftPeer);
    COMSTL_MESSAGE_ASSERT("Acyclic connector cannot be initialised with null pointers", NULL != rightPeer);
    COMSTL_MESSAGE_ASSERT("Acyclic connector cannot be initialised with null pointers", NULL != leftSide);
    COMSTL_MESSAGE_ASSERT("Acyclic connector cannot be initialised with null pointers", NULL != rightSide);
#if 0

    HRESULT hr;

    hr = get_object_identity(left, &m_left);
    if(FAILED(hr))
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(com_exception("Could not acquire left-side identity", hr));
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    else
    {
        hr = get_object_identity(right, &m_right);

        if(FAILED(hr))
        {
            m_left->Release();

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(com_exception("Could not acquire right-side identity", hr));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            m_left = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
    }
#endif /* 0 */
}

template <ss_typename_param_k MX>
inline acyclic_connector<MX>::~acyclic_connector() stlsoft_throw_0()
{}

template <ss_typename_param_k MX>
inline ss_typename_type_ret_k acyclic_connector<MX>::class_type& acyclic_connector<MX>::get_this_()
{
    return *this;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_ACYCLIC_CONNECTOR */

/* ///////////////////////////// end of file //////////////////////////// */
