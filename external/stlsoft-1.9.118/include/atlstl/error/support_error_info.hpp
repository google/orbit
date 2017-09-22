/* /////////////////////////////////////////////////////////////////////////
 * File:        atlstl/error/support_error_info.hpp (originally MAErInfo.h, ::SynesisAtl)
 *
 * Purpose:     SupportErrorInfoImpl class.
 *
 * Created:     17th April 1999
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


/** \file atlstl/error/support_error_info.hpp
 *
 * \brief [C++ only] Definition of the atlstl::SupportErrorInfoImpl,
 *   atlstl::SupportErrorInfoImpl2, atlstl::SupportErrorInfoImpl3
 *   atlstl::SupportErrorInfoImpl4 and atlstl::SupportErrorInfoImpl5
 *   class templates
 *   (\ref group__library__error "Error" Library).
 */

#ifndef ATLSTL_INCL_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO
#define ATLSTL_INCL_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define ATLSTL_VER_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO_MAJOR     5
# define ATLSTL_VER_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO_MINOR     0
# define ATLSTL_VER_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO_REVISION  1
# define ATLSTL_VER_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO_EDIT      67
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

/** \brief Provides implementation of ISupportErrorInfo for support for
 *   errors on one interface
 *
 * \ingroup group__library__error
 */
// [[synesis:class: atlstl::SupportErrorInfoImpl<IID const*>]]
template <IID const* piid>
class ATL_NO_VTABLE SupportErrorInfoImpl
    : public ISupportErrorInfo
{
public:
    typedef SupportErrorInfoImpl<piid>  class_type;

// ISupportErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (InlineIsEqualGUID(riid, *piid))
                    ? S_OK
                    : S_FALSE;
    }
};


/** \brief Provides implementation of ISupportErrorInfo for support for
 *   errors on two interfaces
 *
 * \ingroup group__library__error
 */
// [[synesis:class: atlstl::SupportErrorInfoImpl<IID const*, IID const*>]]
template <IID const* piid1, IID const* piid2>
class ATL_NO_VTABLE SupportErrorInfoImpl2
    : public ISupportErrorInfo
{
public:
    typedef SupportErrorInfoImpl2<piid1, piid2> class_type;

// ISupportErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (InlineIsEqualGUID(riid, *piid1) ||
                InlineIsEqualGUID(riid, *piid2))
                    ? S_OK
                    : S_FALSE;
    }
};


/** \brief Provides implementation of ISupportErrorInfo for support for
 *   errors on three interfaces
 *
 * \ingroup group__library__error
 */
// [[synesis:class: atlstl::SupportErrorInfoImpl<IID const*, IID const*, IID const*>]]
template <IID const* piid1, IID const* piid2, IID const* piid3>
class ATL_NO_VTABLE SupportErrorInfoImpl3
    : public ISupportErrorInfo
{
public:
    typedef SupportErrorInfoImpl3<piid1, piid2, piid3>  class_type;

// ISupportErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (InlineIsEqualGUID(riid, *piid1) ||
                InlineIsEqualGUID(riid, *piid2) ||
                InlineIsEqualGUID(riid, *piid3))
                    ? S_OK
                    : S_FALSE;
    }
};


/** \brief Provides implementation of ISupportErrorInfo for support for
 *   errors on four interfaces
 *
 * \ingroup group__library__error
 */
// [[synesis:class: atlstl::SupportErrorInfoImpl<IID const*, IID const*, IID const*, IID const*>]]
template <IID const* piid1, IID const* piid2, IID const* piid3, IID const* piid4>
class ATL_NO_VTABLE SupportErrorInfoImpl4
    : public ISupportErrorInfo
{
public:
    typedef SupportErrorInfoImpl4<piid1, piid2, piid3, piid4>   class_type;

// ISupportErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (InlineIsEqualGUID(riid, *piid1) ||
                InlineIsEqualGUID(riid, *piid2) ||
                InlineIsEqualGUID(riid, *piid3) ||
                InlineIsEqualGUID(riid, *piid4))
                    ? S_OK
                    : S_FALSE;
    }
};


/** \brief Provides implementation of ISupportErrorInfo for support for
 *   errors on five interfaces
 *
 * \ingroup group__library__error
 */
// [[synesis:class: atlstl::SupportErrorInfoImpl<IID const*, IID const*, IID const*, IID const*, IID const*>]]
template <IID const* piid1, IID const* piid2, IID const* piid3, IID const* piid4, IID const* piid5>
class ATL_NO_VTABLE SupportErrorInfoImpl5
    : public ISupportErrorInfo
{
public:
    typedef SupportErrorInfoImpl5<piid1, piid2, piid3, piid4, piid5>   class_type;

// ISupportErrorInfo
public:
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)
    {
        return (InlineIsEqualGUID(riid, *piid1) ||
                InlineIsEqualGUID(riid, *piid2) ||
                InlineIsEqualGUID(riid, *piid3) ||
                InlineIsEqualGUID(riid, *piid4) ||
                InlineIsEqualGUID(riid, *piid5))
                    ? S_OK
                    : S_FALSE;
    }
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

#endif /* !ATLSTL_INCL_ATLSTL_ERROR_HPP_SUPPORT_ERROR_INFO */

/* ///////////////////////////// end of file //////////////////////////// */
