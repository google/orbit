/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/automation/property_method_helpers.hpp
 *
 * Purpose:     Contains functions for assisting in the implementation of
 *              property methods of ATL COM server classes.
 *
 * Created:     25th June 2002
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


/** \file atlstl/automation/property_method_helpers.hpp
 * \brief [C++ only; requires ATL library] Definition of the
 *  atlstl::get_MemberValue(), atlstl::put_MemberValue() and
 *  atlstl::get_ConstantValue() function suites, which simplify the
 *  definition of property methods in ATL COM servers
 *   (\ref group__library__com_automation "COM Automation" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS
#define ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS_MAJOR    4
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS_MINOR    0
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS_REVISION 3
# define ATLSTL_VER_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS_EDIT     69
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:
STLSOFT_COMPILER_IS_DMC:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef ATLSTL_INCL_ATLSTL_HPP_ATLSTL
# include <atlstl/atlstl.hpp>
#endif /* !ATLSTL_INCL_ATLSTL_HPP_ATLSTL */

#if defined(STLSOFT_COMPILER_IS_BORLAND)
# error No recognised Borland compiler generates correct code when used with these functions
#endif /* compiler */

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
 * get_MemberValue
 *
 * This can be used to get a member variable value. The function has the
 * general usage:
 *
 *   HRESULT Class::get_Member(MemberType *ret)
 *   {
 *       return get_MemberValue(this, ret, &Class::m_memberVariable);
 *   }
 */

/** \brief Inline retrieval of member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a pointer to a return value
 * and a pointer to member of the given class, and retrieves the value
 * of the member into the return value.
 *
 * \note This generic version should only be used for built-in types, or those which have value semantics.
 *
 * \param cls Pointer to the class instance
 * \param ret Pointer to the return value
 * \param mem Pointer to the member variable
 * \return An HRESULT code indicating whether the access succeeded or failed
 * \retval E_POINTER ret was a null pointer
 * \retval S_OK The value was retrieved successfully
 */
template <class C, ss_typename_param_k T>
inline HRESULT get_MemberValue(C *const cls, T *ret, T C::*mem)
{
    return (ret == 0) ? E_POINTER : (*ret = cls->*mem, S_OK);
}

/** \brief Ghost overload to prevent use of get_MemberValue with pointer types
 *
 * \ingroup group__library__com_automation
 *
 * This version is overloaded to deal with pointer types, and is not
 * implemented so as to prevent the generic version being used with such types.
 *
 * Although this is inconvenient, there is no other way to prevent the use of
 * free functions. In such circumstances, the shorthand of get_memberValue()
 * must be eschewed for full and congnisant implementation.
 *
 * \note This is deemed worth the inconvenience since using the generic version would like lead to code that violated COM's memory rules
 *
 */
template <class C, ss_typename_param_k T>
inline HRESULT get_MemberValue(C *const cls, T **ret, T *C::*mem);

/** \brief Inline retrieval of a CComBSTR member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a pointer to a return value
 * and a pointer to a CComBSTR member of the given class, and retrieves the
 * value of the member into the BSTR return value.
 *
 * \param cls Pointer to the class instance
 * \param ret Pointer to the BSTR return value
 * \param mem Pointer to the CComBSTR member variable
 * \return An HRESULT code indicating whether the access succeeded or failed
 * \retval E_POINTER ret was a null pointer
 * \retval E_OUTOFMEMORY Not enough memory to create a copy for the returned value
 * \retval S_OK The value was retrieved successfully
 */
template <class C>
inline HRESULT get_MemberValue(C *const cls, BSTR *ret, CComBSTR C::*mem)
{
    return (ret == 0) ? E_POINTER : (*ret = (cls->*mem).Copy(), (*ret != 0 ? S_OK :  E_OUTOFMEMORY));
}

/** \brief Inline retrieval of a VARIANT member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a pointer to a return value
 * and a pointer to a VARIANT member of the given class, and retrieves the
 * value of the member into the VARIANT return value.
 *
 * \param cls Pointer to the class instance
 * \param ret Pointer to the VARIANT return value
 * \param mem Pointer to the VARIANT member variable
 * \return An HRESULT code indicating whether the access succeeded or failed
 * \retval E_POINTER ret was a null pointer
 * \retval DISP_E_ARRAYISLOCKED The variant contains an array that is locked.
 * \retval DISP_E_BADVARTYPE The source and destination have an invalid variant type (usually uninitialized).
 * \retval E_OUTOFMEMORY Memory could not be allocated for the copy.
 * \retval E_INVALIDARG One of the arguments is invalid.
 * \retval S_OK The value was retrieved successfully
 */
template <class C>
inline HRESULT get_MemberValue(C *const cls, VARIANT *ret, CComVariant C::*mem)
{
    return (ret == 0) ? E_POINTER : ::VariantCopy(ret, &(cls->*mem));
}



/** \brief Inline retrieval of method value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a pointer to a return value
 * and a pointer to method of the given class, and retrieves the return value
 * of the method into the return value.
 *
 * \note This generic version should only be used for built-in types, or those which have value semantics.
 *
 * \param cls Pointer to the class instance
 * \param ret Pointer to the return value
 * \param pfn Pointer to the method
 * \return An HRESULT code indicating whether the access succeeded or failed
 * \retval E_POINTER ret was a null pointer
 * \retval S_OK The value was retrieved successfully
 */
template<class C, ss_typename_param_k T, ss_typename_param_k T2>
inline HRESULT get_MemberValue(C *const cls, T *ret, T2 (C::*pfn)() const)
{
    return (ret == 0) ? E_POINTER : (*ret = (cls->*pfn)(), S_OK);
}

/* /////////////////////////////////////////////////////////////////////////
 * put_MemberValue
 *
 * This can be used to put a member variable value. The function has the
 * general usage:
 *
 *   HRESULT Class::put_Member(MemberType newValue)
 *   {
 *       return put_MemberValue(this, newValue, &Class::m_memberVariable);
 *   }
 */

/** \brief Inline assignment of a member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a new value and a pointer to
 * member of the given class, and assigns the new value to the member.
 *
 * \note This generic version should only be used for built-in types, or those which have value semantics.
 *
 * \param cls Pointer to the class instance
 * \param newVal The new value
 * \param mem Pointer to the member variable
 * \retval S_OK The value was assigned successfully
 */
template <ss_typename_param_k C, ss_typename_param_k T>
inline HRESULT put_MemberValue(C *const cls, T const& newVal, T C::*mem)
{
    return (cls->*mem = newVal, S_OK);
}

/** \brief Inline assignment of a CComBSTR member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a new BSTR value and a pointer to
 * CComBSTR  member of the given class, and assigns the new value to the
 * member.
 *
 * \param cls Pointer to the class instance
 * \param newVal The new BSTR value
 * \param mem Pointer to the CComBSTR member variable
 * \retval S_OK The value was assigned successfully
 */
template <ss_typename_param_k C>
inline HRESULT put_MemberValue(C *const cls, BSTR newVal, CComBSTR C::*mem)
{
    return (cls->*mem = newVal, S_OK);
}

/** \brief Inline assignment of a CComBSTR member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a new CComBSTR value and a pointer
 * to CComBSTR  member of the given class, and assigns the new value to the
 * member.
 *
 * \param cls Pointer to the class instance
 * \param newVal The new CComBSTR  value
 * \param mem Pointer to the CComBSTR member variable
 * \retval S_OK The value was assigned successfully
 */
template <ss_typename_param_k C>
inline HRESULT put_MemberValue(C *const cls, CComBSTR const& newVal, CComBSTR C::*mem)
{
    return put_MemberValue(cls, (BSTR)newVal, mem);
}

/** \brief Inline assignment of a CComVariant member variable value
 *
 * \ingroup group__library__com_automation
 *
 * This function takes a pointer to a class, a new CComVariant value and a
 * pointer to CComVariant member of the given class, and assigns the new value
 * to the member.
 *
 * \param cls Pointer to the class instance
 * \param newVal The new CComVariant value
 * \param mem Pointer to the CComVariant member variable
 * \retval S_OK The value was assigned successfully
 */
template <ss_typename_param_k C>
inline HRESULT put_MemberValue(C *const cls, CComVariant const& newVal, CComVariant C::*mem)
{
    return (cls->*mem = newVal, (VT_ERROR == (cls->*mem).vt) ? (cls->*mem).scode : S_OK);
}







/** \brief Shorthand for implementing methods that return a constant value
 *
 * \ingroup group__library__com_automation
 *
 * This method is used to implement methods that return a value of a
 * known constant, e.g.
 *
\code

STDMETHODIMP DatabaseFlags::get_OrderFields(BOOL *pVal)
{
    return atlstl::get_ConstantValue(pVal, openrj::ORJ_FLAG_ORDERFIELDS);
}

\endcode
 *
 * \param ret Pointer to the return value
 *
 */

template <ss_typename_type_k T1, ss_typename_type_k T2>
inline HRESULT get_ConstantValue(T1 *ret, T2 const& value)
{
    return (NULL == ret) ? E_POINTER : (*ret = value, S_OK);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/property_method_helpers_unittest_.h"
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

#endif /* !ATLSTL_INCL_ATLSTL_AUTOMATION_HPP_PROPERTY_METHOD_HELPERS */

/* ///////////////////////////// end of file //////////////////////////// */
