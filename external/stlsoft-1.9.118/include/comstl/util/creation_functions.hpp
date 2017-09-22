/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/util/creation_functions.hpp
 *
 * Purpose:     COM instance creation helper functions
 *
 * Created:     21st September 2005
 * Updated:     6th June 2010
 *
 * Thanks:      To Adi Shavit for demanding more usability in these
 *              functions, which led to the adoption of stlsoft::ref_ptr<X>
 *              as the unit of currency throughout COMSTL.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2010, Matthew Wilson and Synesis Software
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


/** \file comstl/util/creation_functions.hpp
 *
 * \brief [C++ only; requires COM] COM instance creation helper functions
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_CREATION_FUNCTIONS
#define COMSTL_INCL_COMSTL_UTIL_HPP_CREATION_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_UTIL_HPP_CREATION_FUNCTIONS_MAJOR    2
# define COMSTL_VER_COMSTL_UTIL_HPP_CREATION_FUNCTIONS_MINOR    3
# define COMSTL_VER_COMSTL_UTIL_HPP_CREATION_FUNCTIONS_REVISION 2
# define COMSTL_VER_COMSTL_UTIL_HPP_CREATION_FUNCTIONS_EDIT     22
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS
# include <comstl/util/interface_traits.hpp>
#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR
# include <stlsoft/smartptr/ref_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR */

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
 * Functions
 */

/** \brief Type-safe creation of a COM object, acquiring the requisite
 *  interface pointer.
 *
 * \ingroup group__library__utility__com
 *
 * \param clsid The CLSID of the component to be created.
 * \param ppi Pointer to the interface pointer.
 * \param dwClsContext Class creation context
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying its
 * CLSID, and requesting the <code>IDispatch</code> interface:
\code
const CLSID CLSID_pantheios_COM_LoggerManager  = { 0x4E7D5C47, 0x8F96, 0x45DE, { 0x90, 0x5D, 0xAA, 0x3E, 0x9E, 0x59, 0x2D, 0xE3 } };

IDispatch* logmgr;
if(SUCCEEDED(comstl::co_create_instance(CLSID_pantheios_COM_LoggerManager, &logmgr)))
{
  logmgr->Release();
}
\endcode
 */
template <ss_typename_param_k I>
inline HRESULT co_create_instance(
    REFCLSID    clsid
,   I**         ppi
,   DWORD       dwClsContext = CLSCTX_ALL
)
{
    return ::CoCreateInstance(clsid, NULL, dwClsContext, IID_traits<I>::iid(), reinterpret_cast<void**>(ppi));
}

/** \brief Type-safe creation of a COM object from a Programmatic Id,
 *  acquiring the requisite interface pointer.
 *
 * \ingroup group__library__utility__com
 *
 * \param id Can be the Programatic Identifier (ProgId) - e.g.
 *    pantheios.com.LoggerManager - or the string form of the
 *    class id - e.g. {4E7D5C47-8F96-45DE-905D-AA3E9E592DE3}
 * \param ppi Pointer to the interface pointer.
 * \param dwClsContext Class creation context
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying the
 * string form of its CLSID, and requesting the <code>IDispatch</code>
 * interface:
\code
IDispatch* logmgr;
if(SUCCEEDED(comstl::co_create_instance(L"{4E7D5C47-8F96-45DE-905D-AA3E9E592DE3}", &logmgr)))
{
  logmgr->Release();
}
\endcode
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying the
 * Programmatic Identifier (ProgID), and requesting the <code>IDispatch</code>
 * interface:
\code
IDispatch* logmgr;
if(SUCCEEDED(comstl::co_create_instance(L"pantheios.COM.LoggerManager", &logmgr)))
{
  logmgr->Release();
}
\endcode
 */
template <ss_typename_param_k I>
inline HRESULT co_create_instance(
    LPCOLESTR   id
,   I**         ppi
,   DWORD       dwClsContext = CLSCTX_ALL
)
{
    CLSID   clsid;
    HRESULT hr = ::CLSIDFromProgID(id, &clsid);

    if(FAILED(hr))
    {
        hr = ::CLSIDFromString(const_cast<LPOLESTR>(id), &clsid);
    }

    if(SUCCEEDED(hr))
    {
        hr = co_create_instance(clsid, ppi, dwClsContext);
    }

    return hr;
}


/** \brief Type-safe creation of a COM object, acquiring the requisite
 *  interface pointer into an interface wrapper instance.
 *
 * \ingroup group__library__utility__com
 *
 * \param clsid The CLSID of the component to be created.
 * \param wi A mutable (non-const) reference to an interface wrapper
 *   instance. The wrapper's <code>interface_type</code> determines the
 *   interface queried by the COM runtime's creation facilities on the
 *   created instance.
 * \param dwClsContext Class creation context
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying its
 * CLSID, and requesting the <code>IDispatch</code> interface:
\code
const CLSID CLSID_pantheios_COM_LoggerManager  = { 0x4E7D5C47, 0x8F96, 0x45DE, { 0x90, 0x5D, 0xAA, 0x3E, 0x9E, 0x59, 0x2D, 0xE3 } };

stlsoft::ref_ptr<IDispatch> logmgr;
if(SUCCEEDED(comstl::co_create_instance(CLSID_pantheios_COM_LoggerManager, logmgr)))
{
  . . .
} // Release() automatically invoked here
\endcode
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k I
        ,   ss_typename_param_k U
        >
inline HRESULT co_create_instance(
    REFCLSID                            clsid
,   stlsoft_ns_qual(ref_ptr)<T, I, U>&  wi
,   DWORD                               dwClsContext = CLSCTX_ALL
)
{
    I*      pi;
    HRESULT hr = co_create_instance(clsid, &pi, dwClsContext);

    if(FAILED(hr))
    {
        pi = NULL;
    }

    wi.set(pi, false); // Eat the reference

    return hr;
}

/** \brief Type-safe creation of a COM object from a Programmatic Id,
 *  acquiring the requisite interface wrapper instance.
 *
 * \ingroup group__library__utility__com
 *
 * \param id Can be the Programatic Identifier (ProgId) - e.g.
 *    pantheios.com.LoggerManager - or the string form of the
 *    class id - e.g. {4E7D5C47-8F96-45DE-905D-AA3E9E592DE3}
 * \param wi A mutable (non-const) reference to an interface wrapper
 *   instance. The wrapper's <code>interface_type</code> determines the
 *   interface queried by the COM runtime's creation facilities on the
 *   created instance.
 * \param dwClsContext Class creation context
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying the
 * string form of its CLSID, and requesting the <code>IDispatch</code>
 * interface:
\code
stlsoft::ref_ptr<IDispatch> logmgr;
if(SUCCEEDED(comstl::co_create_instance(L"{4E7D5C47-8F96-45DE-905D-AA3E9E592DE3}", logmgr)))
{
  . . .
} // Release() automatically invoked here
\endcode
 *
 * The following example demonstrates how to use the function to create an
 * instance of the Pantheios.COM Logger Manager component by specifying the
 * Programmatic Identifier (ProgID), and requesting the <code>IDispatch</code>
 * interface:
\code
stlsoft::ref_ptr<IDispatch> logmgr;
if(SUCCEEDED(comstl::co_create_instance(L"pantheios.COM.LoggerManager", logmgr)))
{
  . . .
} // Release() automatically invoked here
\endcode
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k I
        ,   ss_typename_param_k U
        >
inline HRESULT co_create_instance(
    LPCOLESTR                           id
,   stlsoft_ns_qual(ref_ptr)<T, I, U>&  wi
,   DWORD                               dwClsContext = CLSCTX_ALL
)
{
    I*      pi;
    HRESULT hr = co_create_instance(id, &pi, dwClsContext);

    if(FAILED(hr))
    {
        pi = NULL;
    }

    wi = stlsoft_ns_qual(ref_ptr)<T, I, U>(pi, false); // Eat the reference

    return hr;
}

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

#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_CREATION_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
