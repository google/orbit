/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/automation/enumerators.hpp (originally MAEnum.h)
 *
 * Purpose:     Enumerator classes.
 *
 * Created:     11th November 1998
 * Updated:     26th December 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file atlstl/automation/enumerators.hpp
 * \brief [C++ only; requires ATL library] Definition of the
 *  atlstl::copy_enumerator class template (and its supporting components),
 *  which provides a copying alternative to the stock <code>CComEnum</code>
 *  ATL component that may be initialised from any range and whose contents
 *  may be modified subsequent to initialisation
 *   (\ref group__library__com_automation "COM Automation" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_ENUMERATORS
#define ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_ENUMERATORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_ENUMERATORS_MAJOR    4
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_ENUMERATORS_MINOR    0
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_ENUMERATORS_REVISION 5
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_ENUMERATORS_EDIT     67
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

////////////////////////////////////////////////////////////////////////////
// Includes

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR
# include <stlsoft/smartptr/ref_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR */
#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_INDIRECT_FUNCTION_POINTER_ADAPTORS
# include <stlsoft/functional/indirect_function_pointer_adaptors.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_INDIRECT_FUNCTION_POINTER_ADAPTORS */

#ifndef STLSOFT_INCL_SYS_H_ATLCOM
# define STLSOFT_INCL_SYS_H_ATLCOM
# include <atlcom.h>
#endif /* !STLSOFT_INCL_SYS_H_ATLCOM */
#ifndef STLSOFT_INCL_ALGORITHM
# define STLSOFT_INCL_ALGORITHM
# include <algorithm>
#endif /* !STLSOFT_INCL_ALGORITHM */
#ifndef STLSOFT_INCL_LIST
# define STLSOFT_INCL_LIST
# include <list>
#endif /* !STLSOFT_INCL_LIST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::atlstl */
namespace atlstl
{
# else
/* Define stlsoft::atlstl_project */

namespace stlsoft
{

namespace atlstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** A policy that indicates that values should be copied between internal
 * and external representations in the enumerator
 */
template<   ss_typename_param_k XV  // Enumerator interface value type (External value type)
        ,   ss_typename_param_k IV  // Internal value type
        >
struct copy_policy
{
public:
    typedef XV  external_value_type;
    typedef IV  internal_value_type;

public:
    static void init(external_value_type *xv);
#if 0
    {
        *xv = external_value_type();
    }
#endif /* 0 */
    static HRESULT copy(external_value_type *xv, internal_value_type const& iv);
#if 0
    {
        *xv = iv;

        return S_OK;
    }
#endif /* 0 */
    /// This is only called when copy() has failed, and a number of copies must be undone
    static void clear(external_value_type *xv);
#if 0
    {
        STLSOFT_SUPPRESS_UNUSED(xv);
    }
#endif /* 0 */
};

////////////////////////////////////////////////////////////////////////////
// Classes


/** \brief Modifiable, copying enumerator class template
 *
 * \ingroup group__library__com_automation
 *
 * copy_enumerator_impl is the analogue to CComEnumImpl, but the advantages
 * over the ATL class is that it can be added to/removed from after
 * initialisation, and it can be initialised/appended from a sequence (as
 * defined by the start and end iterators), rather than assuming an array as
 * source.
 *
 * copy_enumerator is the analogue to CComEnum, but uses the
 * atlstl::copy_enumerator_impl as its 'implementation' class template.
 *
 * \param I The interface, e.g. IEnumString
 * \param piid The address of the interface, e.g. &IID_IEnumString
 * \param V The value type of the enumeration, e.g. LPOLESTR
 * \param It The iterator class, e.g. LPOLESTR*
 * \param Copy A class that implements copy behaviour (see CComEnumImpl)
 * \param ThreadModel The threading model of the instances (see CComObjectRoot)
 *
 * \note The copy_enumerator_impl class only supports copy semantics on the
 * Init and Add methods, i.e. it always creates its enumeration contents by
 * taking a copy of the contents of its source.
 */
template<   ss_typename_param_k I                                   //!< Enumerator interface
        ,   IID const*          piid                                //!< Enumerator interface Id
        ,   ss_typename_param_k V                                   //!< Enumerator interface value type
        ,   ss_typename_param_k IV      =   V                       //!< Internal type. Must have value semantics
        ,   ss_typename_param_k I2ETx   =   copy_policy<IV, V>      //!< Internal to external transformation policy
        ,   ss_typename_param_k TM      =   CComObjectThreadModel   //!< Thread model
        >
class ATL_NO_VTABLE copy_enumerator
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
;

// class copy_enumerator_impl
// [[synesis:class: atlstl::copy_enumerator_impl<T<I>, IID const*, T<V>, T<IV>, T<I2ETx>>]]
template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
class ATL_NO_VTABLE copy_enumerator_impl
    : public I
{
/// \name Member Types
/// @{
public:
    typedef I                                               interface_type;
    typedef V                                               value_type;
    typedef IV                                              internal_value_type;
    typedef I2ETx                                           internal_to_external_transformer_type;
//    typedef V const*                                          const_pointer;
    typedef copy_enumerator_impl<I, piid, V, IV, I2ETx>     class_type;
    typedef ss_size_t                                       size_type;
    typedef ss_ptrdiff_t                                    difference_type;
private:
    typedef stlsoft_ns_qual_std(list)<internal_value_type>  values_type;
public:
    typedef ss_typename_type_k values_type::iterator        iterator;
    typedef ss_typename_type_k values_type::const_iterator  const_iterator;
/// @}

/// \name Construction
/// @{
public:
    copy_enumerator_impl();
    ~copy_enumerator_impl() stlsoft_throw_0();
/// @}

/// \name Enumeration
/// @{
public:
    STDMETHOD(Next)(ULONG celt, V *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(I **ppEnum);
/// @}

/// \name Operations
/// @{
public:
    template<ss_typename_param_k I, ss_typename_param_k F>
    HRESULT Init(I begin, I end, F fn)
    {
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        try
        {
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            m_values.clear();

            for(; begin != end; ++begin)
            {
                m_values.push_back(fn(*begin));
            }

            m_current = m_values.begin();

            return S_OK;
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        }
        catch(std::bad_alloc &)
        {
            return E_OUTOFMEMORY;
        }
        catch(std::exception &)
        {
            return E_UNEXPECTED;
        }
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }
    template<ss_typename_param_k I>
    HRESULT Init(I begin, I end)
    {
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        try
        {
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
            m_values.clear();

            for(; begin != end; ++begin)
            {
                m_values.push_back(*begin);
            }

            m_current = m_values.begin();

            return S_OK;
# ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        }
        catch(std::bad_alloc &)
        {
            return E_OUTOFMEMORY;
        }
        catch(std::exception &)
        {
            return E_UNEXPECTED;
        }
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    size_type       size() const;

    const_iterator  begin() const;
    iterator        begin();

    const_iterator  end() const;
    iterator        end();
/// @}

/// \name Implementation
/// @{
private:
    virtual class_type  *CreateEmptyClone() const = 0;

    template <typename T>
    static difference_type count_all(T const& b, T const& e)
    {
        difference_type d       =   0;
        T               begin   =   b;
        T               end     =   e;

        for(; begin != end; ++begin)
        {
            ++d;
        }

        return d;
    }

    template <typename T>
    static T increment_by(T it, difference_type by)
    {
        for(; by-- > 0; )
        {
            ++it;
        }

        return it;
    }
/// @}

/// \name Members
/// @{
private:
    values_type     m_values;
    const_iterator  m_current;
/// @}

/// \name Not to be implemented
/// @{
private:
    copy_enumerator_impl(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

# if defined(STLSOFT_COMPILER_IS_INTEL) || \
     defined(STLSOFT_COMPILER_IS_MSVC)
#  if _MSC_VER >= 1200
#   pragma warning(push)
#  endif /* compiler */
#  pragma warning(disable : 4348)
# endif /* compiler */

// class copy_enumerator
// [[synesis:class: atlstl::copy_enumerator<T<I>, IID const*, T<V>, T<IV>, T<I2ETx>, T<TM>>]]
template<   ss_typename_param_k I                                   // Enumerator interface
        ,   IID const*          piid                                // Enumerator interface Id
        ,   ss_typename_param_k V                                   // Enumerator interface value type
        ,   ss_typename_param_k IV      =   V                       // Internal type
        ,   ss_typename_param_k I2ETx   =   copy_policy<IV, V>      // Internal to external transformation policy
        ,   ss_typename_param_k TM      =   CComObjectThreadModel   // Thread model
        >
class ATL_NO_VTABLE copy_enumerator
    : public copy_enumerator_impl<I, piid, V, IV, I2ETx>
    , public CComObjectRootEx<TM>
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
{
public:
    typedef copy_enumerator<I, piid, V, IV, I2ETx, TM>      class_type;
private:
    typedef copy_enumerator_impl<I, piid, V, IV, I2ETx>     impl_type_;

public:
    BEGIN_COM_MAP(class_type)
        COM_INTERFACE_ENTRY_IID(*piid, impl_type_)
    END_COM_MAP()

protected:
    virtual impl_type_ *CreateEmptyClone() const
    {
        return new CComObject<class_type>;
    }
};

#if defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MSVC)
# if _MSC_VER >= 1200
#  pragma warning(pop)
# else /* ? compiler */
#  pragma warning(default: 4348)
# endif /* _MSC_VER */
#endif /* compiler */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline copy_enumerator_impl<I, piid, V, IV, I2ETx>::copy_enumerator_impl()
    : m_values()
    , m_current(m_values.begin())
{}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline copy_enumerator_impl<I, piid, V, IV, I2ETx>::~copy_enumerator_impl() stlsoft_throw_0()
{}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline STDMETHODIMP copy_enumerator_impl<I, piid, V, IV, I2ETx>::Next(ULONG celt, V *rgelt, ULONG *pceltFetched)
{
    HRESULT hr;

    if( rgelt == NULL ||
        (   celt != 1 &&
            pceltFetched == NULL))
    {
        hr = E_POINTER;
    }
    else
    {
        ULONG   celtFetched_;

        if(NULL == pceltFetched)
        {
            pceltFetched = &celtFetched_;
        }

        { for(*pceltFetched = 0, hr = S_OK; /* SUCCEEDED(hr) && */ celt > 0 && m_current != m_values.end(); --celt, ++m_current, ++rgelt, ++*pceltFetched)
        {
            internal_to_external_transformer_type::init(rgelt);
            hr = internal_to_external_transformer_type::copy(rgelt, *m_current);

            if(FAILED(hr))
            {
                break;
            }
        }}

        if(FAILED(hr))
        {
            for(; 0 != *pceltFetched; --*pceltFetched)
            {
                internal_to_external_transformer_type::clear(--rgelt);
            }
        }

        if(SUCCEEDED(hr))
        {
            hr = (0 == celt) ? S_OK : S_FALSE;
        }
    }

    return hr;
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline STDMETHODIMP copy_enumerator_impl<I, piid, V, IV, I2ETx>::Skip(ULONG celt)
{
    { for(; celt > 0 && m_current == m_values.end(); --celt, ++m_current)
    {
    }}

    return (0 == celt) ? S_OK : S_FALSE;
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline STDMETHODIMP copy_enumerator_impl<I, piid, V, IV, I2ETx>::Reset(void)
{
    m_current = m_values.begin();

    return S_OK;
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline STDMETHODIMP copy_enumerator_impl<I, piid, V, IV, I2ETx>::Clone(I** ppEnum)
{
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    try
    {
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        if(NULL == ppEnum)
        {
            return E_POINTER;
        }
        else
        {
            *ppEnum = NULL;

            HRESULT     hr;
            class_type  *pThis  =   this;
            class_type  *p      =   pThis->CreateEmptyClone();

            if(p == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                stlsoft::ref_ptr<class_type, I>     en(p, true);    // Sink the instance, with a bumped (to 1) ref-count

                hr = p->Init(this->begin(), this->end());

                if(SUCCEEDED(hr))
                {
                    const_iterator  begin   =   this->m_values.begin();
                    const_iterator  current =   this->m_current;

                    std::advance(p->m_current, std::distance(begin, current));

                    hr = en->QueryInterface(*piid, reinterpret_cast<void**>(ppEnum));
                }
            }

            return hr;
        }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
    }
    catch(std::bad_alloc &)
    {
        return E_OUTOFMEMORY;
    }
    catch(std::exception &)
    {
        return E_UNEXPECTED;
    }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline ss_size_t copy_enumerator_impl<I, piid, V, IV, I2ETx>::size() const
{
    return m_values.size();
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline ss_typename_type_ret_k copy_enumerator_impl<I, piid, V, IV, I2ETx>::const_iterator copy_enumerator_impl<I, piid, V, IV, I2ETx>::begin() const
{
    return m_values.begin();
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline ss_typename_type_ret_k copy_enumerator_impl<I, piid, V, IV, I2ETx>::iterator copy_enumerator_impl<I, piid, V, IV, I2ETx>::begin()
{
    return m_values.begin();
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline ss_typename_type_ret_k copy_enumerator_impl<I, piid, V, IV, I2ETx>::const_iterator copy_enumerator_impl<I, piid, V, IV, I2ETx>::end() const
{
    return m_values.end();
}

template<   ss_typename_param_k I
        ,   IID const*          piid
        ,   ss_typename_param_k V
        ,   ss_typename_param_k IV
        ,   ss_typename_param_k I2ETx
        >
inline ss_typename_type_ret_k copy_enumerator_impl<I, piid, V, IV, I2ETx>::iterator copy_enumerator_impl<I, piid, V, IV, I2ETx>::end()
{
    return m_values.end();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _ATLSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace atlstl
# else
} // namespace atlstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_ATLSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_ENUMERATORS */

/* ///////////////////////////// end of file //////////////////////////// */
