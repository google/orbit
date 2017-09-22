/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/automation/multiple_dispatch.hpp
 *
 * Purpose:     A class template that makes the methods and properties exhibited
 *              through multiple IDispatch interfaces visible to script clients.
 *
 * Created:     15th May 2006
 * Updated:     18th June 2012
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2007-2012, Matthew Wilson and Synesis Software
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


/** \file atlstl/automation/multiple_dispatch.hpp
 * \brief [C++ only; requires ATL library] Definition of the
 *  atlstl::IDispatchImpl2 and atlstl::IDispatchImpl3
 *  class templates, which make the methods and properties exhibited through
 *  multiple IDispatch interfaces visible to scripting clients
 *   (\ref group__library__com_automation "COM Automation" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH
#define ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH_MAJOR      2
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH_MINOR      1
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH_REVISION   2
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH_EDIT       17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */

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

/** \brief Class template that enables the methods and properties exhibited
 * through two IDispatch interfaces to be visible to scripting clients.
 *
 * \ingroup group__library__com_automation
 *
 * The class template is used in place of ATL's IDispatchImpl class in the
 * parent class list of a class template that support two dispinterfaces.
 * Consider the <a href = "http://openrj.org/">Open-RJ</a> COM mapping's
 * Field class [IDL]:
\code
  // openrj.com.idl
  [ . . . ]
  interface IField
    : IDispatch
  {
    [propget, id(1), . . .]
    HRESULT Name([out, retval] BSTR* pVal);
    [propget, id(2), . . .]
    HRESULT Value([out, retval] BSTR* pVal);
  };

  [ . . . ]
  interface IDocumenter
    : IDispatch
  {
    [propget, id(1), . . .]
    HRESULT DocString([out, retval] BSTR* pVal);
  };

  coclass Field
  {
    [default] interface IField;
    interface IDocumenter;
  };
\endcode
 * As indicated, it supports two dispinterfaces: <b>IField</b> and
 * <b>IDocumenter</b>. Note that each interface has a property with dispid
 * <b>1</b>.
 *
 * By default, the ATL class template IDispatchImpl assumes that just a
 * single "active" dispinterface. We might envisage the following
 * (C++) class definition for <b>Field</b>:
 *
\code
  class ATL_NO_VTABLE Field
    : public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<Field, &CLSID_Field>
    , public atlstl::SupportErrorInfoImpl<&IID_IField>
    , public IDispatchImpl<IField, &IID_IField, &LIBID_OPENRJ_COMLib>
    , public IDispatchImpl<IDocumenter, &IID_IDocumenter, &LIBID_OPENRJ_COMLib>
  {
    . . .
  };
\endcode
 * Unfortunately, scripting clients, which elicit DISPIDs at runtime via an
 * automation server's <code>IDispatch::GetIDsOfNames()</code> method, will
 * see only those methods and properties from <b>IField</b> in such a case.
 * No parts of the <b>IDocumenter</b> interface will be visible or
 * accessible.
 *
 * This is where IDispatchImpl2 comes in. It implements GetIDsOfNames() and
 * Invoke(), operating over both its dispinterfaces to elicit the dispid(s)
 * for requested name(s) by querying each interface in turn. It is used with
 * the Field class as follows:
 *
\code
  class ATL_NO_VTABLE Field
    : public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<Field, &CLSID_Field>
    , public atlstl::SupportErrorInfoImpl<&IID_IField>
    , public atlstl::IDispatchImpl2<IField, &IID_IField, IDocumenter, &IID_IDocumenter, &LIBID_OPENRJ_COMLib>
  {
    . . .
  };
\endcode
 *
 * Now all members of all dispinterfaces are visible to scripting clients.
 * Note that the class also handles the case where the dispinterfaces have
 * members/properties with the same dispids. (See GetIDsOfNames() and
 * Invoke() for details of the mechanism.)
 */
// [[synesis:class: atlstl::IDispatchImpl2<T<I0>, IID const*, T<I1>, IID const*, GUID const*>]]
template<   ss_typename_param_k I0
        ,   IID const*          IID0
        ,   ss_typename_param_k I1
        ,   IID const*          IID1
        ,   GUID const*         LibID
        >
class IDispatchImpl2
    : public IDispatchImpl<I0, IID0, LibID>
    , public IDispatchImpl<I1, IID1, LibID>
{
/// \name Member Types
/// @{
public:
    typedef IDispatchImpl<I0, IID0, LibID>  dispatch_parent_0_type; //!< \brief The type of the first dispinterface
    typedef IDispatchImpl<I1, IID1, LibID>  dispatch_parent_1_type; //!< \brief The type of the second dispinterface
/// @}

/// \name IDispatch methods
/// @{
protected:
    /** \brief Provides the required behaviour of
     *  <code>IDispatch::GetIDsOfNames()</code>, by querying the two
     *  dispinterfaces, in order, to match the name(s).
     *
     * This method operates by first determining which, if any, of the
     * two parent dispinterfaces can resolve the names. If successful, the
     * resultant dispatch Ids are then striped with a bit in their
     * most-significant byte(s) to record the index of the dispinterface
     * which has thus undertaken to interpret them. This stripe is then
     * detected
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If a dispid returned from a successful call to one of the
     *  underlying dispinterfaces' <code>GetIDsOfNames()</code> already
     *  uses the striping bit, it is left alone. Such methods will be
     *  successfully called in Invoke(), in its post-stripe processing.
     */
    STDMETHOD(GetIDsOfNames)(   REFIID      riid
                            ,   LPOLESTR*   rgszNames
                            ,   UINT        cNames
                            ,   LCID        lcid
                            ,   DISPID*     rgdispid)
    {
        unsigned    index   =   1;
        HRESULT     hr      =   dispatch_parent_0_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_1_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        // Encode interface info into the dispid
        if(SUCCEEDED(hr))
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= (index - 1);

            for(UINT i = 0; i < cNames; ++i)
            {
                if(rgdispid[i] < 0)
                {
                    // Leave these alone. They'll be processed on a first-come-first-serve
                    // basis, which assumes that the GetIDsOfNames() and Invoke() of I0 and
                    // I1 are faithfully inter-related.
                }
                else
                {
                    ATLSTL_MESSAGE_ASSERT("Dispatch Id is out of range!", 0 == (dispidFlag & rgdispid[i]));

                    rgdispid[i] |= dispidFlag;
                }
            }
        }

        return hr;
    }
    /** \brief Provides the required behaviour of
     *  <code>IDispatch::Invoke()</code>, by invoking this method on the
     *  requisite dispinterface.
     *
     * This method operates by detecting the striping bit on the dispid,
     * from which the appropriate dispiniterface is determined. The
     * stripe is then removed, and the method invoked.
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If no striping is apparent, the invocation is conducted on
     *  each interface in turn.
     */
    STDMETHOD(Invoke)(  DISPID      dispidMember
                    ,   REFIID      riid
                    ,   LCID        lcid
                    ,   WORD        wFlags
                    ,   DISPPARAMS* pdispparams
                    ,   VARIANT*    pvarResult
                    ,   EXCEPINFO*  pexcepinfo
                    ,   UINT*       puArgErr)
    {
        if(dispidMember >= 0)
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= 0;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_0_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_1_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }
        }

        HRESULT hr = dispatch_parent_0_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_1_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        return hr;
    }
/// @}
};

/** \brief Class template that enables the methods and properties exhibited
 * through three IDispatch interfaces to be visible to scripting clients.
 *
 * \ingroup group__library__com_automation
 *
 * IDispatchImpl3 is used (and operates) in exactly the same way as
 * IDispatchImpl2, except that it supports three dispinterfaces, rather than
 * two.
 */
// [[synesis:class: atlstl::IDispatchImpl2<T<I0>, IID const*, T<I1>, IID const*, T<I2>, IID const*, GUID const*>]]
template<   ss_typename_param_k I0
        ,   IID const*          IID0
        ,   ss_typename_param_k I1
        ,   IID const*          IID1
        ,   ss_typename_param_k I2
        ,   IID const*          IID2
        ,   GUID const*         LibID
        >
class IDispatchImpl3
    : public IDispatchImpl<I0, IID0, LibID>
    , public IDispatchImpl<I1, IID1, LibID>
    , public IDispatchImpl<I2, IID2, LibID>
{
/// \name Member Types
/// @{
public:
    typedef IDispatchImpl<I0, IID0, LibID>  dispatch_parent_0_type; //!< \brief The type of the first dispinterface
    typedef IDispatchImpl<I1, IID1, LibID>  dispatch_parent_1_type; //!< \brief The type of the second dispinterface
    typedef IDispatchImpl<I2, IID2, LibID>  dispatch_parent_2_type; //!< \brief The type of the third dispinterface
/// @}

/// \name IDispatch methods
/// @{
protected:
    /** \brief Provides the required behaviour of
     *   <code>IDispatch::GetIDsOfNames()</code>, by querying the three
     *   dispinterfaces, in order, to match the name(s).
     *
     * This method operates by first determining which, if any, of the
     * three parent dispinterfaces can resolve the names. If successful, the
     * resultant dispatch Ids are then striped with a bit in their
     * most-significant byte(s) to record the index of the dispinterface
     * which has thus undertaken to interpret them. This stripe is then
     * detected
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If a dispid returned from a successful call to one of the
     *  underlying dispinterfaces' <code>GetIDsOfNames()</code> already
     *  uses the striping bit, it is left alone. Such methods will be
     *  successfully called in Invoke(), in its post-stripe processing.
     */
    STDMETHOD(GetIDsOfNames)(   REFIID      riid
                            ,   LPOLESTR*   rgszNames
                            ,   UINT        cNames
                            ,   LCID        lcid
                            ,   DISPID*     rgdispid)
    {
        unsigned    index   =   1;
        HRESULT     hr      =   dispatch_parent_0_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_1_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_2_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        // Encode interface info into the dispid
        if(SUCCEEDED(hr))
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= (index - 1);

            for(UINT i = 0; i < cNames; ++i)
            {
                if(rgdispid[i] < 0)
                {
                    // Leave these alone. They'll be processed on a first-come-first-serve
                    // basis, which assumes that the GetIDsOfNames() and Invoke() of I0 and
                    // I1 are faithfully inter-related.
                }
                else
                {
                    ATLSTL_MESSAGE_ASSERT("Dispatch Id is out of range!", 0 == (dispidFlag & rgdispid[i]));

                    rgdispid[i] |= dispidFlag;
                }
            }
        }

        return hr;
    }
    /** \brief Provides the required behaviour of
     *  <code>IDispatch::Invoke()</code>, by invoking this method on the
     *  requisite dispinterface.
     *
     * This method operates by detecting the striping bit on the dispid,
     * from which the appropriate dispiniterface is determined. The
     * stripe is then removed, and the method invoked.
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If no striping is apparent, the invocation is conducted on
     *  each interface in turn.
     */
    STDMETHOD(Invoke)(  DISPID      dispidMember
                    ,   REFIID      riid
                    ,   LCID        lcid
                    ,   WORD        wFlags
                    ,   DISPPARAMS* pdispparams
                    ,   VARIANT*    pvarResult
                    ,   EXCEPINFO*  pexcepinfo
                    ,   UINT*       puArgErr)
    {
        if(dispidMember >= 0)
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= 0;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_0_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_1_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_2_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }
        }

        HRESULT hr = dispatch_parent_0_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_1_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_2_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        return hr;
    }
/// @}
};

/** \brief Class template that enables the methods and properties exhibited
 *    through four IDispatch interfaces to be visible to scripting clients.
 *
 * \ingroup group__library__com_automation
 *
 * IDispatchImpl4 is used (and operates) in exactly the same way as
 * IDispatchImpl3, except that it supports four dispinterfaces, rather than
 *  three.
 */
// [[synesis:class: atlstl::IDispatchImpl2<T<I0>, IID const*, T<I1>, IID const*, T<I2>, IID const*, T<I3>, IID const*, GUID const*>]]
template<   ss_typename_param_k I0
        ,   IID const*          IID0
        ,   ss_typename_param_k I1
        ,   IID const*          IID1
        ,   ss_typename_param_k I2
        ,   IID const*          IID2
        ,   ss_typename_param_k I3
        ,   IID const*          IID3
        ,   GUID const*         LibID
        >
class IDispatchImpl4
    : public IDispatchImpl<I0, IID0, LibID>
    , public IDispatchImpl<I1, IID1, LibID>
    , public IDispatchImpl<I2, IID2, LibID>
    , public IDispatchImpl<I3, IID3, LibID>
{
/// \name Member Types
/// @{
public:
    typedef IDispatchImpl<I0, IID0, LibID>  dispatch_parent_0_type; //!< \brief The type of the first dispinterface
    typedef IDispatchImpl<I1, IID1, LibID>  dispatch_parent_1_type; //!< \brief The type of the second dispinterface
    typedef IDispatchImpl<I2, IID2, LibID>  dispatch_parent_2_type; //!< \brief The type of the third dispinterface
    typedef IDispatchImpl<I3, IID3, LibID>  dispatch_parent_3_type; //!< \brief The type of the fourth dispinterface
/// @}

/// \name IDispatch methods
/// @{
protected:
    /** \brief Provides the required behaviour of
     *   <code>IDispatch::GetIDsOfNames()</code>, by querying the four
     *   dispinterfaces, in order, to match the name(s).
     *
     * This method operates by first determining which, if any, of the
     * four parent dispinterfaces can resolve the names. If successful, the
     * resultant dispatch Ids are then striped with a bit in their
     * most-significant byte(s) to record the index of the dispinterface
     * which has thus undertaken to interpret them. This stripe is then
     * detected
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If a dispid returned from a successful call to one of the
     *  underlying dispinterfaces' <code>GetIDsOfNames()</code> already
     *  uses the striping bit, it is left alone. Such methods will be
     *  successfully called in Invoke(), in its post-stripe processing.
     */
    STDMETHOD(GetIDsOfNames)(   REFIID      riid
                            ,   LPOLESTR*   rgszNames
                            ,   UINT        cNames
                            ,   LCID        lcid
                            ,   DISPID*     rgdispid)
    {
        unsigned    index   =   1;
        HRESULT     hr      =   dispatch_parent_0_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_1_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_2_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        if( FAILED(hr) &&
            DISP_E_UNKNOWNNAME == hr)
        {
            ++index;

            hr = dispatch_parent_3_type::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
        }

        // Encode interface info into the dispid
        if(SUCCEEDED(hr))
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= (index - 1);

            for(UINT i = 0; i < cNames; ++i)
            {
                if(rgdispid[i] < 0)
                {
                    // Leave these alone. They'll be processed on a first-come-first-serve
                    // basis, which assumes that the GetIDsOfNames() and Invoke() of I0 and
                    // I1 are faithfully inter-related.
                }
                else
                {
                    ATLSTL_MESSAGE_ASSERT("Dispatch Id is out of range!", 0 == (dispidFlag & rgdispid[i]));

                    rgdispid[i] |= dispidFlag;
                }
            }
        }

        return hr;
    }
    /** \brief Provides the required behaviour of
     *  <code>IDispatch::Invoke()</code>, by invoking this method on the
     *  requisite dispinterface.
     *
     * This method operates by detecting the striping bit on the dispid,
     * from which the appropriate dispiniterface is determined. The
     * stripe is then removed, and the method invoked.
     *
     * \remarks Names are matched en bloc: they are either all matched by one
     *  interface, or all by the other. It is <b>never</b> the case that
     *  some part are matched by one and the remainder by the other.
     *
     * \note If no striping is apparent, the invocation is conducted on
     *  each interface in turn.
     */
    STDMETHOD(Invoke)(  DISPID      dispidMember
                    ,   REFIID      riid
                    ,   LCID        lcid
                    ,   WORD        wFlags
                    ,   DISPPARAMS* pdispparams
                    ,   VARIANT*    pvarResult
                    ,   EXCEPINFO*  pexcepinfo
                    ,   UINT*       puArgErr)
    {
        if(dispidMember >= 0)
        {
            DISPID  dispidFlag  =   DISPID(0x1) << (8 * sizeof(DISPID) - 2);

            dispidFlag >>= 0;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_0_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_1_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_2_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }

            dispidFlag >>= 1;
            if(dispidMember & dispidFlag)
            {
                return dispatch_parent_3_type::Invoke(dispidMember & ~dispidFlag, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
            }
        }

        HRESULT hr = dispatch_parent_0_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_1_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_2_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        if( FAILED(hr) &&
            DISP_E_MEMBERNOTFOUND == hr)
        {
            hr = dispatch_parent_3_type::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

        return hr;
    }
/// @}
};

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

#endif /* !ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_MULTIPLE_DISPATCH */

/* ///////////////////////////// end of file //////////////////////////// */
