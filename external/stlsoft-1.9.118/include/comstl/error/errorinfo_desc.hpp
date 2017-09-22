/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/error/errorinfo_desc.hpp
 *
 * Purpose:     errorinfo_desc class for accessing description from the COM error.
 *
 * Created:     19th December 2002
 * Updated:     15th February 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
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

/** \file comstl/error/errorinfo_desc.hpp
 *
 * \brief [C++ only] Definition of the comstl::errorinfo_desc class
 *   (\ref group__library__error "Error" Library).
 *
 * \warning The contents of this file are not a final form, and <b>will</b>
 *  change in a future release.
 */

#ifndef COMSTL_INCL_COMSTL_ERROR_HPP_ERRORINFO_DESC
#define COMSTL_INCL_COMSTL_ERROR_HPP_ERRORINFO_DESC

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_ERROR_HPP_ERRORINFO_DESC_MAJOR       0
# define COMSTL_VER_COMSTL_ERROR_HPP_ERRORINFO_DESC_MINOR       5
# define COMSTL_VER_COMSTL_ERROR_HPP_ERRORINFO_DESC_REVISION    8
# define COMSTL_VER_COMSTL_ERROR_HPP_ERRORINFO_DESC_EDIT        32
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */
#ifdef STLSOFT_CF_THROW_BAD_ALLOC
# include <new>
#endif /* STLSOFT_CF_THROW_BAD_ALLOC */

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

/** \brief Provides access to the COM Error Info objecg
 *
 * \ingroup group__library__error
 */
class errorinfo_desc
{
/// \name Types
/// @{
public:
    typedef errorinfo_desc  class_type;
/// @}

/// \name Construction
/// @{
public:
    errorinfo_desc()
        : m_description(NULL)
        , m_description_a(NULL)
        , m_len(0)
    {
        IErrorInfo* pei;

        if(S_OK == ::GetErrorInfo(0, &pei))
        {
            get_description(pei, m_description, m_len);

            pei->Release();
        }
    }
    errorinfo_desc(IErrorInfo* pei)
        : m_description(NULL)
        , m_description_a(NULL)
    {
        get_description(pei, m_description, m_len);
    }
    errorinfo_desc(errorinfo_desc const& rhs)
        : m_description(::SysAllocString(rhs.m_description))
        , m_description_a(NULL)
        , m_len(rhs.m_len)
    {}

    ~errorinfo_desc() stlsoft_throw_0()
    {
        ::SysFreeString(m_description);
        ::CoTaskMemFree(m_description_a);
    }
/// @}

/// \name Accessors
/// @{
public:
    LPCOLESTR c_str_w() const
    {
        return (NULL == m_description) ? L"" : m_description;
    }
    char const* c_str_a() const
    {
        return const_cast<class_type*>(this)->check_description_a_();
    }
#ifdef UNICODE
    LPCOLESTR c_str() const
    {
        return c_str_w();
    }
#else /* ? UNICODE */
    char const* c_str() const
    {
        return c_str_a();
    }
#endif /* UNICODE */
    cs_size_t length() const
    {
        return m_len;
    }
/// @}

/// \name Implementation
/// @{
private:
    char const* check_description_a_()
    {
        if( NULL == m_description_a &&
            NULL != m_description)
        {
            int cch = ::WideCharToMultiByte(0, 0, m_description, -1, NULL, 0, NULL, NULL);

            m_description_a = static_cast<char*>(::CoTaskMemAlloc((1 + cch) * sizeof(char)));

            if(NULL == m_description_a)
            {
#ifdef STLSOFT_CF_THROW_BAD_ALLOC
                STLSOFT_THROW_X(stlsoft_ns_qual_std(bad_alloc)());
#endif /* STLSOFT_CF_THROW_BAD_ALLOC */
            }
            else
            {
                ::WideCharToMultiByte(0, 0, m_description, -1, m_description_a, 1 + cch, NULL, NULL);
            }
        }

        return (NULL == m_description_a) ? "" : m_description_a;
    }

    static void get_description(IErrorInfo* pei, BSTR& description, cs_size_t& len)
    {
        BSTR    bstr;

        if( 0 != pei &&
            SUCCEEDED(pei->GetDescription(&bstr)))
        {
            // Now trim backwards to get rid of any trailing
            // whitespace
            LPOLESTR    begin   =   bstr;
            LPOLESTR    end     =   begin + ::SysStringLen(bstr);
            LPOLESTR    last    =   end;

            for(; begin != last; --last)
            {
                OLECHAR ch = *(last - 1);

                // This is not terribly internationalised, but
                // will suffice for these simple purposes.
                if( ch == ' ' ||
                    ch == '\t' ||
                    ch == '\r' ||
                    ch == '\n')
                {
                }
                else
                {
                    break;
                }
            }

            description = ::SysAllocStringLen(bstr, static_cast<UINT>(last - begin));

            if(NULL == description)
            {
                // Failed, so grab what there is, which is better than nothing
                description = bstr;
            }
            else
            {
                ::SysFreeString(bstr);
            }

            len = static_cast<cs_size_t>(last - begin);
        }
    }
/// @}

/// \name Members
/// @{
private:
    BSTR        m_description;
    char*       m_description_a;
    cs_size_t   m_len;
/// @}

// Not to be implemented
private:
    class_type const& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline cs_char_a_t const* c_str_data_a(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_a();
}

inline cs_char_w_t const* c_str_data_w(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_w();
}

inline cs_char_o_t const* c_str_data_o(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_w();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for errorinfo_desc
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCTSTR c_str_data(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str();
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline cs_size_t c_str_len_a(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.length();
}

inline cs_size_t c_str_len_w(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.length();
}

inline cs_size_t c_str_len_o(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.length();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for errorinfo_desc
 *
 * \ingroup group__concept__shim__string_access
 */
inline cs_size_t c_str_len(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.length();
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline cs_char_a_t const* c_str_ptr_a(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_a();
}

inline cs_char_w_t const* c_str_ptr_w(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_w();
}

inline cs_char_o_t const* c_str_ptr_o(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str_w();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for errorinfo_desc
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCTSTR c_str_ptr(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return eid.c_str();
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline cs_char_a_t const* c_str_ptr_null_a(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return (0 != eid.length()) ? eid.c_str_a() : NULL;
}

inline cs_char_w_t const* c_str_ptr_null_w(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return (0 != eid.length()) ? eid.c_str_w() : NULL;
}

inline cs_char_o_t const* c_str_ptr_null_o(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return (0 != eid.length()) ? eid.c_str_w() : NULL;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for errorinfo_desc
 *
 * \ingroup group__concept__shim__string_access
 */
inline LPCTSTR c_str_ptr_null(comstl_ns_qual(errorinfo_desc) const& eid)
{
    return (0 != eid.length()) ? eid.c_str() : NULL;
}


/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for comstl::errorinfo_desc
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        >
inline S& operator <<(S& s, comstl_ns_qual(errorinfo_desc) const& eid)
{
    s << eid.c_str();

    return s;
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

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _COMSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::comstl::c_str_data;
using ::comstl::c_str_data_a;
using ::comstl::c_str_data_w;
using ::comstl::c_str_data_o;

using ::comstl::c_str_len;
using ::comstl::c_str_len_a;
using ::comstl::c_str_len_w;
using ::comstl::c_str_len_o;

using ::comstl::c_str_ptr;
using ::comstl::c_str_ptr_a;
using ::comstl::c_str_ptr_w;
using ::comstl::c_str_ptr_o;

using ::comstl::c_str_ptr_null;
using ::comstl::c_str_ptr_null_a;
using ::comstl::c_str_ptr_null_w;
using ::comstl::c_str_ptr_null_o;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !COMSTL_INCL_COMSTL_ERROR_HPP_ERRORINFO_DESC */

/* ///////////////////////////// end of file //////////////////////////// */
