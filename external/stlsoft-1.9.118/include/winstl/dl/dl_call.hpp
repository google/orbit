/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/dl/dl_call.hpp
 *
 * Purpose:     Invocation of functions in dynamic libraries.
 *
 * Created:     sometime in 1998
 * Updated:     24th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/dl/dl_call.hpp
 *
 * \brief [C++ only] Definition of the winstl::dl_call() function suite
 *   (\ref group__library__dl "DL" Library).
 */

#ifndef WINSTL_INCL_WINSTL_DL_HPP_DL_CALL
#define WINSTL_INCL_WINSTL_DL_HPP_DL_CALL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_DL_HPP_DL_CALL_MAJOR     2
# define WINSTL_VER_WINSTL_DL_HPP_DL_CALL_MINOR     7
# define WINSTL_VER_WINSTL_DL_HPP_DL_CALL_REVISION  3
# define WINSTL_VER_WINSTL_DL_HPP_DL_CALL_EDIT      47
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
// Alas, Como experiences an ICE when compiling dl_call
[Incompatibilies-start]
STLSOFT_COMPILER_IS_COMO:
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#if defined(STLSOFT_COMPILER_IS_COMO)
# error dl_call is not compatible with Como, which experienced an ICE
#endif /* compiler */
#if defined(STLSOFT_COMPILER_IS_WATCOM)
# error dl_call is not compatible with Watcom, which does not have sufficient template support
#endif /* compiler */


#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
# include <winstl/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#ifndef WINSTL_INCL_WINSTL_DL_HPP_MODULE
# include <winstl/dl/module.hpp>
#endif /* !WINSTL_INCL_WINSTL_DL_HPP_MODULE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE
# include <stlsoft/meta/is_function_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE
# include <stlsoft/meta/is_fundamental_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SPLIT_FUNCTIONS
# include <stlsoft/string/split_functions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SPLIT_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW
# include <stlsoft/string/string_view.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_VIEW */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::winstl */
namespace winstl
{
# else
/* Define stlsoft::winstl_project */

namespace stlsoft
{

namespace winstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# define WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

/** \def WINSTL_DL_CALL_WINx_STDCALL_LITERAL(name)
 *
 * String pastes the given name with the requisite Windows API standard call
 * calling convention ("stdcall" on Win32, "cdecl" on Win64)
 *
 * \param name A string literal specifying the dynamically invoked function
 */

#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define WINSTL_DL_CALL_WINx_STDCALL_LITERAL(name)          <appropriate prefix> name
#elif defined(WINSTL_OS_IS_WIN64)
# define WINSTL_DL_CALL_WINx_STDCALL_LITERAL(name)          "cdecl:" name
#elif defined(WINSTL_OS_IS_WIN32)
# define WINSTL_DL_CALL_WINx_STDCALL_LITERAL(name)          "stdcall:" name
#else /* ? WIN?? */
# error Windows operating system not recognised
#endif /* WIN?? */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** Indicates an entry point cannot be located in a dynamic library.
 *
 * \ingroup group__library__dl__error
 */
class missing_entry_point_exception
    : public windows_exception
{
/// \name Types
/// @{
public:
    typedef windows_exception                   parent_class_type;
    typedef missing_entry_point_exception       class_type;
private:
    typedef parent_class_type::string_type      string_type;
public:
    typedef parent_class_type::error_code_type  error_code_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs an instance of the exception based on the given missing
    /// function name, and Windows error code.
    missing_entry_point_exception(char const* functionName, error_code_type err)
        : parent_class_type(class_type::create_reason_(functionName), err)
    {}
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    virtual ~missing_entry_point_exception() stlsoft_throw_0()
    {}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Implementation
/// @{
private:
    static string_type create_reason_(char const* functionName)
    {
        string_type reason("Failed to find procedure \"");

        return reason + functionName + '"';
    }
/// @}

/// \name Not to be implemented
/// @{
private:
    class_type& operator =(class_type const&);
/// @}
};

/** Indicates an invalid calling convention specifier.
 *
 * \ingroup group__library__dl__error
 */
class invalid_calling_convention_exception
    : public windows_exception
{
/// \name Types
/// @{
public:
    typedef windows_exception                       parent_class_type;
    typedef invalid_calling_convention_exception    class_type;
private:
    typedef parent_class_type::string_type          string_type;
public:
    typedef parent_class_type::error_code_type      error_code_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs an instance of the exception based on the given
    /// function name, and Windows error code.
    invalid_calling_convention_exception(char const* callingConventionSpecifier)
        : parent_class_type(class_type::create_reason_(callingConventionSpecifier), ERROR_INVALID_FUNCTION)
        , m_ccs(callingConventionSpecifier)
    {}
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    virtual ~invalid_calling_convention_exception() stlsoft_throw_0()
    {}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

/// \name Accessors
/// @{
public:
    string_type const& get_specifier() const
    {
        return m_ccs;
    }
/// @}

/// \name Implementation
/// @{
private:
    static string_type create_reason_(char const* callingConventionSpecifier)
    {
        return "Unrecognised or unsupported calling convention \"" + string_type(callingConventionSpecifier) + '"';
    }
/// @}

/// \name Members
/// @{
private:
    const string_type   m_ccs;
/// @}

// Not to be implemented
/// @}
private:
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Enumerations
 */

namespace calling_convention
{
    /** Calling conventions supported by winstl::dl_call()
     */
    enum calling_convention
    {
            unknownCallConv     =   -1
#ifdef STLSOFT_CF_CDECL_SUPPORTED
        ,   cdeclCallConv       =   STLSOFT_CDECL_VALUE
#endif /* STLSOFT_CF_CDECL_SUPPORTED */
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
        ,   fastcallCallConv    =   STLSOFT_FASTCALL_VALUE
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
        ,   stdcallCallConv     =   STLSOFT_STDCALL_VALUE
#endif // STLSOFT_CF_STDCALL_SUPPORTED
    };

    inline calling_convention from_int(int i)
    {
        switch(i)
        {
            default:
                STLSOFT_MESSAGE_ASSERT("Invalid/unrecognised calling convention specifier. cdecl will be assumed", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
            case    cdeclCallConv:
                return cdeclCallConv;
#endif /* STLSOFT_CF_CDECL_SUPPORTED */
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
            case    fastcallCallConv:
                return fastcallCallConv;
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
            case    stdcallCallConv:
                return stdcallCallConv;
#endif // STLSOFT_CF_STDCALL_SUPPORTED
        }
    }

} // namespace calling_convention

/* ////////////////////////////////////////////////////////////////////// */

/** Anchors the function descriptor specialisations with a common base to
 * facilitate function descriptor meta-programming selection
 */
struct function_descriptor_base
{
    operator function_descriptor_base const* () const
    {
        return this;
    }
};

/** Specifies a compile-time function descriptor
 *
 * \param CC The calling convention, one of the \link calling_convention::calling_convention calling_convention\endlink enumerators
 * \param S The string type
 */
template<   int                 CC
        ,   ss_typename_param_k S
        >
struct function_descriptor
    : public function_descriptor_base
{
    enum { value = CC };

    ss_explicit_k function_descriptor(S const& functionName)
        : FunctionName(functionName)
        , CallingConvention(CC)
    {
#if defined(STLSOFT_CF_FASTCALL_SUPPORTED)
# if defined(STLSOFT_CF_STDCALL_SUPPORTED)
        STLSOFT_STATIC_ASSERT(  CC == calling_convention::cdeclCallConv ||  CC == calling_convention::fastcallCallConv ||  CC == calling_convention::stdcallCallConv);
# else /* ? STLSOFT_CF_STDCALL_SUPPORTED */
        STLSOFT_STATIC_ASSERT(  CC == calling_convention::cdeclCallConv ||  CC == calling_convention::fastcallCallConv);
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */
#else /* ? STLSOFT_CF_FASTCALL_SUPPORTED */
# if defined(STLSOFT_CF_STDCALL_SUPPORTED)
        STLSOFT_STATIC_ASSERT(  CC == calling_convention::cdeclCallConv ||  CC == calling_convention::stdcallCallConv);
# else /* ? STLSOFT_CF_STDCALL_SUPPORTED */
        STLSOFT_STATIC_ASSERT(  CC == calling_convention::cdeclCallConv);
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */
    }

    ss_explicit_k function_descriptor(S const& functionName, int cc)
        : FunctionName(functionName)
        , CallingConvention(cc)
    {}

    S const&    FunctionName;
    const int   CallingConvention;

private:
    function_descriptor& operator =(function_descriptor const&);
};

template<   int                 cc
        ,   ss_typename_param_k S
        >
inline function_descriptor<cc, S> fn_desc(S const& functionName)
{
    return function_descriptor<cc, S>(functionName);
}

template<   ss_typename_param_k    S
        >
inline function_descriptor<0, S> fn_desc(int cc, S const& functionName)
{
    return function_descriptor<0, S>(functionName, cc);
}

/* /////////////////////////////////////////////////////////////////////////
 * Traits
 */

/** Traits class that provides a mechanism for declaring specific
 *   (e.g. aggregate and user-defined) types to be compatible with
 *   \link winstl::dl_call dl_call()\endlink.
 *
 * \ingroup group__library__dl
 *
 * To specify your type being dl_call()-compatible, simply specialise the
 * traits template as follows (for the notional type <code>MyType</code>):
 *
\code
namespace MyNamespace
{
  class MyType
  {};

} // MyNamespace

namespace winstl
{
  template <>
  struct is_valid_dl_call_arg<MyNamespace::MyType>
  {
    enum { value = 1 };
  };
} // namespace winstl
\endcode
 */
template<ss_typename_param_k T>
struct is_valid_dl_call_arg
{
    enum { value = 0 };
};

/** Internal traits class used by the DL Library.
 *
 * \note This is a struct, rather than a namespace, because namespaces are
 *        open, and we want this to be closed.
 */
struct dl_call_traits
{
/// \name Member Types
/// @{
public:
    typedef FARPROC         entry_point_type;
    typedef HINSTANCE       library_handle_type;
    typedef winstl::module  module_wrapper_type;
/// @}

/// \name Dynamic Library Functions
/// @{
public:
    static entry_point_type get_symbol(library_handle_type hLib, char const* functionName)
    {
        return ::GetProcAddress(hLib, functionName);
    }
/// @}

/// \name Function Descriptor Discrimination
/// @{
public:
    /// Tag that denotes that the function is identified by name
    struct is_not_fd
    {};

    /// Tag that denotes that the function is identified by a function descriptor
    struct is_fd
    {};
/// @}

/// \name Module Discrimination
/// @{
public:
    /// Tag that denotes that the library argument is a handle
    struct library_is_handle
    {};

    /// Tag that denotes that the library argument is not a handle
    struct library_is_not_handle
    {};
/// @}

};


// These structures used for selecting lock_name_() function templates
template<   ss_typename_param_k T
        >
inline T const& lock_name_(T const& t, dl_call_traits::is_not_fd)
{
    return t;
}

template<   int                 cc
        ,   ss_typename_param_k S
        >
inline S const& lock_name_(function_descriptor<cc, S> const& fd, dl_call_traits::is_fd)
{
    return fd.FunctionName;
}

inline dl_call_traits::is_fd test_fd_(function_descriptor_base const*)
{
    return dl_call_traits::is_fd();
}

inline dl_call_traits::is_not_fd test_fd_(...)
{
    return dl_call_traits::is_not_fd();
}

#if defined(STLSOFT_COMPILER_IS_MSVC) || \
    defined(STLSOFT_COMPILER_IS_GCCx)
inline dl_call_traits::library_is_handle test_library_(dl_call_traits::library_handle_type )
{
    return dl_call_traits::library_is_handle();
}
#else /* ? compiler */
inline dl_call_traits::library_is_handle test_library_(dl_call_traits::library_handle_type const&)
{
    return dl_call_traits::library_is_handle();
}
#endif /* compiler */

template <ss_typename_param_k T>
inline dl_call_traits::library_is_not_handle test_library_(T const&)
{
    return dl_call_traits::library_is_not_handle();
}

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

inline dl_call_traits::entry_point_type lookup_symbol_(dl_call_traits::library_handle_type hinst, char const* functionName)
{
    dl_call_traits::entry_point_type    fp  =   dl_call_traits::get_symbol(hinst, functionName);

    if(NULL == fp)
    {
        STLSOFT_THROW_X(missing_entry_point_exception(functionName, ::GetLastError()));
    }

    return fp;
}

template <ss_typename_param_k C>
inline calling_convention::calling_convention determine_calling_convention_(C const*& functionName)
{
    typedef stlsoft::basic_string_view<C>   string_t;

    calling_convention::calling_convention  cc = calling_convention::cdeclCallConv;
    string_t                                s0;
    string_t                                s1;

    if(stlsoft::split(functionName, ':', s0, s1))
    {
#ifdef STLSOFT_CF_CDECL_SUPPORTED
        if( s0 == "C" ||
            s0 == "cdecl")
        {
                cc = calling_convention::cdeclCallConv;
        } else
#endif /* STLSOFT_CF_CDECL_SUPPORTED */
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
        if( s0 == "F" ||
            s0 == "fastcall")
        {
                cc = calling_convention::fastcallCallConv;
        } else
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
        if( s0 == "S" ||
            s0 == "stdcall")
        {
                cc = calling_convention::stdcallCallConv;
        } else
#endif // STLSOFT_CF_STDCALL_SUPPORTED
        {
            STLSOFT_THROW_X(invalid_calling_convention_exception(s0.c_str()));
        }

        functionName = s1.base();
    }

    return cc;
}

template <ss_typename_param_k S>
char const* detect_cc_( dl_call_traits::is_not_fd
                    ,   char const*                             functionName
                    ,   S const&
                    ,   calling_convention::calling_convention& cc)
{
    cc = determine_calling_convention_(functionName);

    return functionName;
}

template<   int                 CC
        ,   ss_typename_param_k C
        >
char const* detect_cc_( dl_call_traits::is_fd
                    ,   char const*                             functionName
                    ,   function_descriptor<CC, C> const&       fd
                    ,   calling_convention::calling_convention& cc)
{
    cc = calling_convention::from_int(fd.CallingConvention);

    return functionName;
}

/* ////////////////////////////////////////////////////////////////////// */

/** \name Invocators
 *
 * \ingroup group__library__dl
 *
 * These calling convention-specific functions perform the invocation of the
 * given function pointer with the requisite arguments.
 *
 * @{
 */

//[<[STLSOFT-AUTO:DL_CALL-INVOCATORS:BEGIN]>]

// 0 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp)
{
  R (STLSOFT_CDECL* pfn)();

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn();
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp)
{
  R (STLSOFT_FASTCALL* pfn)();

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn();
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp)
{
  R (STLSOFT_STDCALL* pfn)();

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn();
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 1 param

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0)
{
  R (STLSOFT_CDECL* pfn)(A0 a0);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 2 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 3 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 4 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 5 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 6 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 7 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 8 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 9 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 10 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 11 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 12 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 13 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 14 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 15 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 16 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 17 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 18 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 19 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 20 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 21 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 22 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 23 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 24 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 25 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 26 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 27 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 28 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 29 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 30 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 31 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


// 32 params

#ifdef STLSOFT_CF_CDECL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}
#endif // STLSOFT_CF_CDECL_SUPPORTED

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_invoke_fastcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  R (STLSOFT_FASTCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}
#endif // STLSOFT_CF_FASTCALL_SUPPORTED

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_invoke_stdcall(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  R (STLSOFT_STDCALL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}
#endif // STLSOFT_CF_STDCALL_SUPPORTED


//[<[STLSOFT-AUTO:DL_CALL-INVOCATORS:END]>]

/// @}

/* ////////////////////////////////////////////////////////////////////// */

/** \name Dispatchers
 *
 * \ingroup group__library__dl
 *
 * These calling convention-agnostic functions dispatch the function to the
 * appropriate invocator, with the requisite arguments.
 *
 * @{
 */

//[<[STLSOFT-AUTO:DL_CALL-DISPATCHERS:BEGIN]>]


// 0 params
template< ss_typename_param_k R
        >
inline R dl_call_dispatch_0(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 1 param
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_dispatch_1(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 2 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_dispatch_2(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 3 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_dispatch_3(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 4 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_dispatch_4(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 5 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_dispatch_5(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 6 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_dispatch_6(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 7 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_dispatch_7(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 8 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_dispatch_8(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 9 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_dispatch_9(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 10 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_dispatch_10(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 11 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_dispatch_11(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 12 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_dispatch_12(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 13 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_dispatch_13(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 14 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_dispatch_14(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 15 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_dispatch_15(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 16 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_dispatch_16(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 17 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_dispatch_17(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 18 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_dispatch_18(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 19 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_dispatch_19(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 20 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_dispatch_20(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 21 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_dispatch_21(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 22 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_dispatch_22(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 23 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_dispatch_23(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 24 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_dispatch_24(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 25 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_dispatch_25(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 26 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_dispatch_26(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 27 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_dispatch_27(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 28 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_dispatch_28(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 29 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_dispatch_29(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 30 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_dispatch_30(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 31 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_dispatch_31(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}


// 32 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_dispatch_32(dl_call_traits::entry_point_type fp, calling_convention::calling_convention cc, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  WINSTL_ASSERT(NULL != fp);

  switch(cc)
  {
      default:
          STLSOFT_MESSAGE_ASSERT("Invalid calling convention", 0);
#ifdef STLSOFT_CF_CDECL_SUPPORTED
      case    calling_convention::cdeclCallConv:
          return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
#endif // STLSOFT_CF_CDECL_SUPPORTED
#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
      case    calling_convention::fastcallCallConv:
          return dl_call_invoke_fastcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
#endif // STLSOFT_CF_FASTCALL_SUPPORTED
#ifdef STLSOFT_CF_STDCALL_SUPPORTED
      case    calling_convention::stdcallCallConv:
          return dl_call_invoke_stdcall<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
#endif // STLSOFT_CF_STDCALL_SUPPORTED
  }
}

//[<[STLSOFT-AUTO:DL_CALL-DISPATCHERS:END]>]

/// @}

/* ////////////////////////////////////////////////////////////////////// */

/** \name Lookup-ers
 *
 * \ingroup group__library__dl
 *
 * These calling convention-agnostic functions look up the symbol from the
 * library handle, and then call the dispatcher.
 *
 * @{
 */

//[<[STLSOFT-AUTO:DL_CALL-LOOKUPS:BEGIN]>]


// 0 params
template< ss_typename_param_k R
        >
inline R dl_call_lookup_0(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_0<R>(fp, cc);
}


// 1 param
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_lookup_1(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_1<R>(fp, cc, a0);
}


// 2 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_lookup_2(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_2<R>(fp, cc, a0, a1);
}


// 3 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_lookup_3(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_3<R>(fp, cc, a0, a1, a2);
}


// 4 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_lookup_4(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_4<R>(fp, cc, a0, a1, a2, a3);
}


// 5 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_lookup_5(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_5<R>(fp, cc, a0, a1, a2, a3, a4);
}


// 6 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_lookup_6(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_6<R>(fp, cc, a0, a1, a2, a3, a4, a5);
}


// 7 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_lookup_7(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_7<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_lookup_8(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_8<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_lookup_9(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_9<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_lookup_10(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_10<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_lookup_11(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_11<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_lookup_12(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_12<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_lookup_13(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_13<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_lookup_14(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_14<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_lookup_15(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_15<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_lookup_16(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_16<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_lookup_17(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_17<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_lookup_18(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_18<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_lookup_19(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_19<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_lookup_20(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_20<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_lookup_21(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_21<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_lookup_22(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_22<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_lookup_23(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_23<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_lookup_24(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_24<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_lookup_25(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_25<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_lookup_26(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_26<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_lookup_27(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_27<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_lookup_28(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_28<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_lookup_29(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_29<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_lookup_30(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_30<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_lookup_31(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_31<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_lookup_32(  dl_call_traits::library_handle_type             hinst
                        ,   char const*                                     functionName
                        ,   calling_convention::calling_convention const&   cc
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  dl_call_traits::entry_point_type fp  =   lookup_symbol_(hinst, functionName);

  WINSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_32<R>(fp, cc, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

//[<[STLSOFT-AUTO:DL_CALL-LOOKUPS:END]>]

/// @}

/* ////////////////////////////////////////////////////////////////////// */

/** \name Module-ers
 *
 * \ingroup group__library__dl
 *
 * These calling convention-agnostic functions acquire the instance handle for
 * the library, and then call the lookup-ers.
 *
 * @{
 */

//[<[STLSOFT-AUTO:DL_CALL-MODULES:BEGIN]>]


// 0 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  // lock_name_() is used in order to select the appropriate characteristics from
  //  the file descriptor without allowing it to leave the current statement sequence
  //
  // test_fd_() is used to determine the whether the function descriptor is a
  // function_descriptor, or something else entirely (i.e. a string)
  //
  // detect_cc_() is used in order to take a function descriptor, its string access
  //  shim converted (char const*) form, and a calling convention variable to instantiate

  return dl_call_lookup_0<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)   // This first one is needed for some, but not all, compilers
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd);
}


// 1 param

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_1<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0);
}


// 2 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_2<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1);
}


// 3 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_3<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2);
}


// 4 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_4<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3);
}


// 5 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_5<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4);
}


// 6 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_6<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5);
}


// 7 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_7<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_8<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_9<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_10<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_11<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_12<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_13<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_14<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_15<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_16<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_17<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_18<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_19<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_20<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_21<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_22<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_23<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_24<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_25<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_26<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_27<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_28<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_29<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_30<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_31<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, dl_call_traits::library_handle_type hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
    calling_convention::calling_convention  cc  =   calling_convention::unknownCallConv;

  return dl_call_lookup_32<R>( hinst
                          ,   detect_cc_( test_fd_(&fd)
                                      ,   stlsoft::c_str_ptr(lock_name_(  fd
                                                                      ,   test_fd_(&fd)))
                                      ,   fd
                                      ,   cc)
                          ,   cc
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), dl_call_traits::module_wrapper_type(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

//[<[STLSOFT-AUTO:DL_CALL-MODULES:END]>]

/// @}

/* ////////////////////////////////////////////////////////////////////// */

/** \name API functions
 *
 * \ingroup group__library__dl
 *
 * Their action is to determine (at compile-time) the type of the library
 * argument, and then invoke the appropriate dl_call_MOD() overload
 * @{
 */

//[<[STLSOFT-AUTO:DL_CALL-FUNCTIONS:BEGIN]>]


// 0 params

/** Invoke a dynamic function with 0 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        >
inline R dl_call(L const& library, FD const& fd)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd);
}


// 1 param

/** Invoke a dynamic function with 1 parameter
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0
        >
inline R dl_call(L const& library, FD const& fd, A0 a0)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0);
}


// 2 params

/** Invoke a dynamic function with 2 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1);
}


// 3 params

/** Invoke a dynamic function with 3 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2);
}


// 4 params

/** Invoke a dynamic function with 4 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3);
}


// 5 params

/** Invoke a dynamic function with 5 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4);
}


// 6 params

/** Invoke a dynamic function with 6 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5);
}


// 7 params

/** Invoke a dynamic function with 7 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params

/** Invoke a dynamic function with 8 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params

/** Invoke a dynamic function with 9 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params

/** Invoke a dynamic function with 10 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params

/** Invoke a dynamic function with 11 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params

/** Invoke a dynamic function with 12 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params

/** Invoke a dynamic function with 13 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params

/** Invoke a dynamic function with 14 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params

/** Invoke a dynamic function with 15 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params

/** Invoke a dynamic function with 16 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params

/** Invoke a dynamic function with 17 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params

/** Invoke a dynamic function with 18 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params

/** Invoke a dynamic function with 19 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params

/** Invoke a dynamic function with 20 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params

/** Invoke a dynamic function with 21 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params

/** Invoke a dynamic function with 22 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params

/** Invoke a dynamic function with 23 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params

/** Invoke a dynamic function with 24 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params

/** Invoke a dynamic function with 25 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params

/** Invoke a dynamic function with 26 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params

/** Invoke a dynamic function with 27 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params

/** Invoke a dynamic function with 28 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || winstl::is_valid_dl_call_arg<A27>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params

/** Invoke a dynamic function with 29 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || winstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || winstl::is_valid_dl_call_arg<A28>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params

/** Invoke a dynamic function with 30 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || winstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || winstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || winstl::is_valid_dl_call_arg<A29>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params

/** Invoke a dynamic function with 31 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || winstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || winstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || winstl::is_valid_dl_call_arg<A29>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A30>::value || stlsoft::is_pointer_type<A30>::value || stlsoft::is_function_pointer_type<A30>::value || winstl::is_valid_dl_call_arg<A30>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params

/** Invoke a dynamic function with 32 parameters
 * \ingroup group__library__dl
 */
template< ss_typename_param_k R
        , ss_typename_param_k L
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call(L const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
#ifndef WINSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || winstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || winstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || winstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || winstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || winstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || winstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || winstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || winstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || winstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || winstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || winstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || winstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || winstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || winstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || winstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || winstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || winstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || winstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || winstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || winstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || winstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || winstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || winstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || winstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || winstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || winstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || winstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || winstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || winstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || winstl::is_valid_dl_call_arg<A29>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A30>::value || stlsoft::is_pointer_type<A30>::value || stlsoft::is_function_pointer_type<A30>::value || winstl::is_valid_dl_call_arg<A30>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A31>::value || stlsoft::is_pointer_type<A31>::value || stlsoft::is_function_pointer_type<A31>::value || winstl::is_valid_dl_call_arg<A31>::value);
#endif /* !WINSTL_DL_CALL_NO_ARG_TYPE_CHECK */

  return dl_call_MOD<R>(test_library_(library), library, fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

//[<[STLSOFT-AUTO:DL_CALL-FUNCTIONS:END]>]

/// @}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/dl_call_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_DL_HPP_DL_CALL */

/* ///////////////////////////// end of file //////////////////////////// */
