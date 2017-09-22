/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/automation/automation_collections.hpp
 *
 * Purpose:     Adaptor classes for creating COM collection instances.
 *
 * Created:     16th April 1999
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1999-2009, Matthew Wilson and Synesis Software
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


/** \file atlstl/automation/automation_collections.hpp
 * \brief [C++ only; requires ATL library] Definition of the
 *  atlstl::generic_automation_collection class template, with which
 *  COM Collections may be readily defined
 *   (\ref group__library__com_automation "COM Automation" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS
#define ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS_MAJOR     3
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS_MINOR     2
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS_REVISION  2
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS_EDIT      107
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_DMC:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */

#ifndef STLSOFT_INCL_SYS_H_ATLCOM
# define STLSOFT_INCL_SYS_H_ATLCOM
# include <atlcom.h>
#endif /* !STLSOFT_INCL_SYS_H_ATLCOM */

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

// [[synesis:class:collection: atlstl::generic_collection_base<T<D>, T<ThreadModel>, T<I>, int>]]
template<   ss_typename_param_k D
        ,   ss_typename_param_k ThreadModel
        ,   ss_typename_param_k I
        ,   int                 DispidCount
        >
class generic_collection_base
    : public I
    , public CComObjectRootEx<ThreadModel>
{
/// \name Member Types
/// @{
public:
    typedef D                                                       derived_class_type;
//    typedef generic_collection_base<D, pfn, ThreadModel>            class_type;
    typedef generic_collection_base<D, ThreadModel, I, DispidCount> class_type;
/// @}

/// \name Construction
/// @{
protected:
    generic_collection_base()
    {}
/// @}

/// \name Interface map
/// @{
protected:
    BEGIN_COM_MAP(generic_collection_base)
        COM_INTERFACE_ENTRY(IDispatch)  // This is a constraint for I to be IDispatch, or a dual interface
    END_COM_MAP()
/// @}

/// \name IDispatch members
/// @{
protected:
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {
        *pctinfo = 0;

        return S_OK;
    }
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID /* lcid */, ITypeInfo** ppTInfo)
    {
        if(0 != iTInfo)
        {
            return DISP_E_BADINDEX;
        }
        else if(NULL == ppTInfo)
        {
            return E_POINTER;
        }
        else
        {
            *ppTInfo = NULL;

            return S_FALSE;
        }
    }
    STDMETHOD(GetIDsOfNames)(   REFIID      /* riid */
                            ,   LPOLESTR*   rgszNames
                            ,   UINT        cNames
                            ,   LCID        /* lcid */
                            ,   DISPID*     rgDispId)
    {
        if(1 == cNames)
        {
            if(0 == ::wcscmp(rgszNames[0], L"Count"))
            {
                derived_class_type* pThis   =   static_cast<derived_class_type*>(this);
                HRESULT             hr      =   pThis->SupportsCount();

                if(S_OK == hr)
                {
                    rgDispId[0] = DispidCount;

                    return hr;
                }
            }
        }

        return DISP_E_UNKNOWNNAME;
    }
    STDMETHOD(Invoke)(  DISPID      dispidMember
                    ,   REFIID      /* riid */
                    ,   LCID        /* lcid */
                    ,   WORD        /* wFlags */
                    ,   DISPPARAMS* pDispParams
                    ,   VARIANT*    pVarResult
                    ,   EXCEPINFO*  /* pExcepInfo */
                    ,   UINT*       /* puArgErr */)
    {
        if(DISPID_NEWENUM == dispidMember)
        {
            derived_class_type* pThis = static_cast<derived_class_type*>(this);

            if(0 != pDispParams->cArgs)
            {
                return DISP_E_BADPARAMCOUNT;
            }
            else
            {
                LPUNKNOWN   punkEnumerator;
                HRESULT     hr;

                ::VariantInit(pVarResult);

//              hr  =   (pThis->*pfn)(&punkEnumerator);
                hr  =   pThis->get__NewEnum(&punkEnumerator);

                if(SUCCEEDED(hr))
                {
                    pVarResult->vt      =   VT_UNKNOWN;
                    pVarResult->punkVal =   punkEnumerator;
                }

                return hr;
            }
        }
        else if(DispidCount == dispidMember)
        {
            derived_class_type* pThis = static_cast<derived_class_type*>(this);
            HRESULT             hr;

            ::VariantInit(pVarResult);

            hr  =   pThis->get_Count(&pVarResult->lVal);

            if(SUCCEEDED(hr))
            {
                pVarResult->vt = VT_I4;
            }

            return hr;
        }
        else
        {
            return DISP_E_MEMBERNOTFOUND;
        }
    }
/// @}
};


template<   ss_typename_param_k I1
        ,   ss_typename_param_k I2
        ,   ss_typename_param_k I3
        >
inline LPUNKNOWN get_clone(I1* instance, HRESULT (STDAPICALLTYPE I3::*pfn)(I2**), HRESULT &hr)
{
    I2* clone;

    hr = (instance->*pfn)(&clone);

    if(FAILED(hr))
    {
        clone = NULL;
    }

    return clone;
}

/** Class for defining COM Automation collections
 */
// [[synesis:class:collection: atlstl::generic_automation_collection<T<E>, T<ThreadModel>, T<I>, int>]]
template<   ss_typename_param_k E
        ,   ss_typename_param_k ThreadModel =   CComObjectThreadModel
        ,   ss_typename_param_k I           =   IDispatch
        ,   int                 DispidCount =   20001
        >    // Enumerator type
class generic_automation_collection
    : public generic_collection_base<generic_automation_collection<E, ThreadModel, I, DispidCount>, ThreadModel, I, DispidCount>
{
public:
    typedef E                                                               enumerator_type;
    typedef generic_automation_collection<E, ThreadModel, I, DispidCount>   class_type;

public:
    generic_automation_collection()
        : m_enumerator(NULL)
        , m_count(static_cast<as_size_t>(~0))
    {}
    void SetEnumerator(enumerator_type* enumerator, as_bool_t bAddRef)
    {
        ATLSTL_ASSERT(NULL != enumerator);
        ATLSTL_ASSERT(NULL == m_enumerator);

        m_enumerator = enumerator;
        if(bAddRef)
        {
            m_enumerator->AddRef();
        }
    }
    void SetCount(as_size_t count)
    {
        m_count = count;
    }
    ~generic_automation_collection()
    {
        m_enumerator->Release();
    }

    HRESULT SupportsCount() const
    {
        return (static_cast<as_size_t>(~0) == m_count) ? S_FALSE : S_OK;
    }

public:
    template<   ss_typename_param_k ITER
            ,   ss_typename_param_k ITF
            ,   ss_typename_param_k N
            >
    HRESULT Init(ITER begin, ITER end, ITF* owner, N flags)
    {
        ATLSTL_ASSERT(NULL != m_enumerator);

        return m_enumerator->Init(begin, end, owner, flags);
    }
    template<   ss_typename_param_k ITER
            ,   ss_typename_param_k ITF
            >
    HRESULT Init(ITER begin, ITER end, ITF* owner)
    {
        ATLSTL_ASSERT(NULL != m_enumerator);

        return m_enumerator->Init(begin, end, owner);
    }

public:
    HRESULT get__NewEnum(LPUNKNOWN* punk)
    {
        ATLSTL_ASSERT(NULL != m_enumerator);

        HRESULT hr;

        *punk = get_clone(m_enumerator, &enumerator_type::Clone, hr);

#if 0
        typedef HRESULT (STDAPICALLTYPE enumerator_type::_CComEnumBase::*pfn_t)(LPUNKNOWN*);
        typedef HRESULT (STDAPICALLTYPE enumerator_type::_CComEnumBase::*pfnv_t)(void*);

        union
        {
            pfn_t   pfn;
            pfnv_t  pfnv;
        } u;

        u.pfnv  =   (pfnv_t)(&enumerator_type::Clone);

        return (m_enumerator->*u.pfn)(punk);
#else /* ? 0 */
        return hr;
#endif /* 0 */
    }
    HRESULT get_Count(long* pVal)
    {
        ATLSTL_ASSERT(NULL != pVal);

        if(static_cast<as_size_t>(~0) == m_count)
        {
            return E_UNEXPECTED;
        }
        else
        {
            *pVal = static_cast<long>(m_count);

            return S_OK;
        }
    }

/// \name Member Variables
/// @{
private:
    enumerator_type*    m_enumerator;
    as_size_t           m_count;
/// @}

private:
    generic_automation_collection(class_type const&);
    class_type& operator =(class_type const&);
};

#if 0
template<   ss_typename_param_k C   //!< Collection interface
        ,   ss_typename_param_k E   //!< Enumerator interface
        ,   ss_typename_param_k T   //!< Element type
        ,   ss_typename_param_k XXXXXXXXX
        >
class simple_automation_collection
{
};
#endif /* 0 */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/automation_collections_unittest_.h"
#endif /* STLSOFT_UNITTEST */

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

#endif /* !ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_AUTOMATION_COLLECTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
