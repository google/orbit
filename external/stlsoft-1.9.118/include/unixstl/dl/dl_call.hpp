/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/dl/dl_call.hpp
 *
 * Purpose:     Invocation of functions in dynamic libraries.
 *
 * Created:     sometime in 1998
 * Updated:     10th August 2009
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


/** \file unixstl/dl/dl_call.hpp
 *
 * \brief [C++ only] Definition of the unixstl::dl_call() function suite
 *   (\ref group__library__dl "DL" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_DL_HPP_DL_CALL
#define UNIXSTL_INCL_UNIXSTL_DL_HPP_DL_CALL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_DL_HPP_DL_CALL_MAJOR       2
# define UNIXSTL_VER_UNIXSTL_DL_HPP_DL_CALL_MINOR       3
# define UNIXSTL_VER_UNIXSTL_DL_HPP_DL_CALL_REVISION    6
# define UNIXSTL_VER_UNIXSTL_DL_HPP_DL_CALL_EDIT        41
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_HPP_ERROR_UNIX_EXCEPTIONS
# include <unixstl/error/exceptions.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_UNIX_EXCEPTIONS */
#ifndef UNIXSTL_INCL_UNIXSTL_DL_HPP_MODULE
# include <unixstl/dl/module.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_D:_HPP_MODULE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE
# include <stlsoft/meta/is_function_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNCTION_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE
# include <stlsoft/meta/is_fundamental_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_FUNDAMENTAL_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE
# include <stlsoft/meta/is_pointer_type.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_IS_POINTER_TYPE */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::unixstl */
namespace unixstl
{
# else
/* Define stlsoft::unixstl_project */

namespace stlsoft
{

namespace unixstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Indicates an entry point cannot be located in a dynamic library.
 *
 * \ingroup group__library__dl__error
 */
class missing_entry_point_exception
    : public unix_exception
{
/// \name Types
/// @{
public:
    typedef unix_exception                      parent_class_type;
    typedef missing_entry_point_exception       class_type;
private:
    typedef parent_class_type::string_type      string_type;
public:
    typedef parent_class_type::error_code_type  error_code_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance of the exception based on the given missing
    /// function name, and Windows error code.
    missing_entry_point_exception(char const* functionName, error_code_type err)
        : parent_class_type(class_type::create_reason_(functionName), err)
    {}
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    virtual ~missing_entry_point_exception() stlsoft_throw_0()
    {}
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/// @}

// Implementation
private:
    static string_type create_reason_(char const* functionName)
    {
        string_type reason("Failed to find procedure \"");

        reason += functionName;
        reason += '"';

        return reason;
    }

// Not to be implemented
private:
    class_type& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * Traits
 */

/** \brief Traits class that provides a mechanism for declaring specific
 *   (e.g. aggregate and user-defined) types to be compatible with
 *   \link unixstl::dl_call dl_call()\endlink.
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

namespace unixstl
{
  template <>
  struct is_valid_dl_call_arg<MyNamespace::MyType>
  {
    enum { value = 1 };
  };
} // namespace unixstl
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
    typedef void            (*entry_point_type)(void);
    typedef void*           library_handle_type;
    typedef unixstl::module module_wrapper_type;
/// @}

/// \name Dynamic Library Functions
/// @{
public:
    static entry_point_type get_symbol(library_handle_type hLib, char const* functionName)
    {
#if 0

        return (dl_call_traits::entry_point_type)::dlsym(hLib, functionName);

#else /* ? 0 */

        // If this static assert fires, this component cannot be used. (It'd also be
        // something amazing, unfathomable operating system architecture, but it's
        // possible.)
        STLSOFT_STATIC_ASSERT(sizeof(dl_call_traits::entry_point_type) == sizeof(::dlsym(hLib, functionName)));

        union
        {
            dl_call_traits::entry_point_type    pfn;
            void*                               pv;

        } u;

        u.pv = ::dlsym(hLib, functionName);

        return u.pfn;

#endif /* 0 */
    }
/// @}

/// \name Function Descriptor Discrimination
/// @{
public:
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

    if(dl_call_traits::entry_point_type() == fp)
    {
        STLSOFT_THROW_X(missing_entry_point_exception(functionName, errno));
    }

    return fp;
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

template< ss_typename_param_k R
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp)
{
  R (STLSOFT_CDECL* pfn)();

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn();
}

// 1 param

template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0)
{
  R (STLSOFT_CDECL* pfn)(A0 a0);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0);
}

// 2 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1);
}

// 3 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2);
}

// 4 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3);
}

// 5 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4);
}

// 6 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5);
}

// 7 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6);
}

// 8 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7);
}

// 9 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

// 10 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

// 11 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

// 12 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

// 13 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

// 14 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

// 15 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

// 16 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

// 17 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}

// 18 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}

// 19 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}

// 20 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}

// 21 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}

// 22 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}

// 23 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}

// 24 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}

// 25 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}

// 26 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}

// 27 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}

// 28 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}

// 29 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}

// 30 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}

// 31 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}

// 32 params

template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_invoke_cdecl(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  R (STLSOFT_CDECL* pfn)(A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31);

  reinterpret_cast<dl_call_traits::entry_point_type&>(pfn) = fp;

  return pfn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

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
inline R dl_call_dispatch_0(dl_call_traits::entry_point_type fp)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp);
}


// 1 param
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_dispatch_1(dl_call_traits::entry_point_type fp, A0 a0)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0);
}


// 2 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_dispatch_2(dl_call_traits::entry_point_type fp, A0 a0, A1 a1)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1);
}


// 3 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_dispatch_3(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2);
}


// 4 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_dispatch_4(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3);
}


// 5 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_dispatch_5(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4);
}


// 6 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_dispatch_6(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5);
}


// 7 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_dispatch_7(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_dispatch_8(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_dispatch_9(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_dispatch_10(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_dispatch_11(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_dispatch_12(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_dispatch_13(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_dispatch_14(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_dispatch_15(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_dispatch_16(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_dispatch_17(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_dispatch_18(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_dispatch_19(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_dispatch_20(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_dispatch_21(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_dispatch_22(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_dispatch_23(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_dispatch_24(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_dispatch_25(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_dispatch_26(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_dispatch_27(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_dispatch_28(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_dispatch_29(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_dispatch_30(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_dispatch_31(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_dispatch_32(dl_call_traits::entry_point_type fp, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_invoke_cdecl<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
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
inline R dl_call_lookup_0( void*       hinst
                        ,   char const* functionName)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_0<R>(fp);
}


// 1 param
template< ss_typename_param_k R
        , ss_typename_param_k A0
        >
inline R dl_call_lookup_1( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_1<R>(fp, a0);
}


// 2 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_lookup_2( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_2<R>(fp, a0, a1);
}


// 3 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_lookup_3( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_3<R>(fp, a0, a1, a2);
}


// 4 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_lookup_4( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_4<R>(fp, a0, a1, a2, a3);
}


// 5 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_lookup_5( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_5<R>(fp, a0, a1, a2, a3, a4);
}


// 6 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_lookup_6( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_6<R>(fp, a0, a1, a2, a3, a4, a5);
}


// 7 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_lookup_7( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_7<R>(fp, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_lookup_8( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_8<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_lookup_9( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_9<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_lookup_10( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_10<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_lookup_11( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_11<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_lookup_12( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_12<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_lookup_13( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_13<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_lookup_14( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_14<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_lookup_15( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_15<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_lookup_16( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_16<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_lookup_17( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_17<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_lookup_18( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_18<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_lookup_19( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_19<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_lookup_20( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_20<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_lookup_21( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_21<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_lookup_22( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_22<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_lookup_23( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_23<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_lookup_24( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_24<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_lookup_25( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_25<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_lookup_26( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_26<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_lookup_27( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_27<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_lookup_28( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_28<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_lookup_29( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_29<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_lookup_30( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_30<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_lookup_31( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_31<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params
template< ss_typename_param_k R
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_lookup_32( void*       hinst
                        ,   char const* functionName
                        ,   A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  dl_call_traits::entry_point_type fp = lookup_symbol_(hinst, functionName);

  UNIXSTL_ASSERT(NULL != fp);

  return dl_call_dispatch_32<R>(fp, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
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
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd)
{
  return dl_call_lookup_0<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          );
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd);
}


// 1 param

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0)
{
  return dl_call_lookup_1<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0);
}


// 2 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1)
{
  return dl_call_lookup_2<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1);
}


// 3 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2)
{
  return dl_call_lookup_3<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2);
}


// 4 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3)
{
  return dl_call_lookup_4<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3);
}


// 5 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  return dl_call_lookup_5<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4);
}


// 6 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  return dl_call_lookup_6<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5);
}


// 7 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  return dl_call_lookup_7<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6);
}


// 8 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  return dl_call_lookup_8<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7);
}


// 9 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  return dl_call_lookup_9<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8);
}


// 10 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  return dl_call_lookup_10<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}


// 11 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  return dl_call_lookup_11<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}


// 12 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  return dl_call_lookup_12<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


// 13 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  return dl_call_lookup_13<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}


// 14 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  return dl_call_lookup_14<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}


// 15 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  return dl_call_lookup_15<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}


// 16 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  return dl_call_lookup_16<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}


// 17 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  return dl_call_lookup_17<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
}


// 18 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  return dl_call_lookup_18<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17);
}


// 19 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  return dl_call_lookup_19<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18);
}


// 20 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  return dl_call_lookup_20<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19);
}


// 21 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  return dl_call_lookup_21<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
}


// 22 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  return dl_call_lookup_22<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21);
}


// 23 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  return dl_call_lookup_23<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22);
}


// 24 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  return dl_call_lookup_24<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23);
}


// 25 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  return dl_call_lookup_25<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24);
}


// 26 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  return dl_call_lookup_26<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25);
}


// 27 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  return dl_call_lookup_27<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26);
}


// 28 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  return dl_call_lookup_28<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27);
}


// 29 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  return dl_call_lookup_29<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28);
}


// 30 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  return dl_call_lookup_30<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29);
}


// 31 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  return dl_call_lookup_31<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
}


// 32 params

template< ss_typename_param_k R
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_MOD(dl_call_traits::library_is_handle, void* hinst, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  return dl_call_lookup_32<R>( hinst
                          ,   stlsoft::c_str_ptr(fd)
                          , a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
}

template< ss_typename_param_k R
        , ss_typename_param_k S
        , ss_typename_param_k FD
        , ss_typename_param_k A0, ss_typename_param_k A1, ss_typename_param_k A2, ss_typename_param_k A3, ss_typename_param_k A4, ss_typename_param_k A5, ss_typename_param_k A6, ss_typename_param_k A7, ss_typename_param_k A8, ss_typename_param_k A9, ss_typename_param_k A10, ss_typename_param_k A11, ss_typename_param_k A12, ss_typename_param_k A13, ss_typename_param_k A14, ss_typename_param_k A15, ss_typename_param_k A16, ss_typename_param_k A17, ss_typename_param_k A18, ss_typename_param_k A19, ss_typename_param_k A20, ss_typename_param_k A21, ss_typename_param_k A22, ss_typename_param_k A23, ss_typename_param_k A24, ss_typename_param_k A25, ss_typename_param_k A26, ss_typename_param_k A27, ss_typename_param_k A28, ss_typename_param_k A29, ss_typename_param_k A30, ss_typename_param_k A31
        >
inline R dl_call_MOD(dl_call_traits::library_is_not_handle, S const& library, FD const& fd, A0 a0, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19, A20 a20, A21 a21, A22 a22, A23 a23, A24 a24, A25 a25, A26 a26, A27 a27, A28 a28, A29 a29, A30 a30, A31 a31)
{
  return dl_call_MOD<R>(dl_call_traits::library_is_handle(), unixstl::module(stlsoft::c_str_ptr(library)).get_module_handle(), fd, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31);
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
 *
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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || unixstl::is_valid_dl_call_arg<A27>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || unixstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || unixstl::is_valid_dl_call_arg<A28>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || unixstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || unixstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || unixstl::is_valid_dl_call_arg<A29>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || unixstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || unixstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || unixstl::is_valid_dl_call_arg<A29>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A30>::value || stlsoft::is_pointer_type<A30>::value || stlsoft::is_function_pointer_type<A30>::value || unixstl::is_valid_dl_call_arg<A30>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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
#ifndef UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A0>::value || stlsoft::is_pointer_type<A0>::value || stlsoft::is_function_pointer_type<A0>::value || unixstl::is_valid_dl_call_arg<A0>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A1>::value || stlsoft::is_pointer_type<A1>::value || stlsoft::is_function_pointer_type<A1>::value || unixstl::is_valid_dl_call_arg<A1>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A2>::value || stlsoft::is_pointer_type<A2>::value || stlsoft::is_function_pointer_type<A2>::value || unixstl::is_valid_dl_call_arg<A2>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A3>::value || stlsoft::is_pointer_type<A3>::value || stlsoft::is_function_pointer_type<A3>::value || unixstl::is_valid_dl_call_arg<A3>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A4>::value || stlsoft::is_pointer_type<A4>::value || stlsoft::is_function_pointer_type<A4>::value || unixstl::is_valid_dl_call_arg<A4>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A5>::value || stlsoft::is_pointer_type<A5>::value || stlsoft::is_function_pointer_type<A5>::value || unixstl::is_valid_dl_call_arg<A5>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A6>::value || stlsoft::is_pointer_type<A6>::value || stlsoft::is_function_pointer_type<A6>::value || unixstl::is_valid_dl_call_arg<A6>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A7>::value || stlsoft::is_pointer_type<A7>::value || stlsoft::is_function_pointer_type<A7>::value || unixstl::is_valid_dl_call_arg<A7>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A8>::value || stlsoft::is_pointer_type<A8>::value || stlsoft::is_function_pointer_type<A8>::value || unixstl::is_valid_dl_call_arg<A8>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A9>::value || stlsoft::is_pointer_type<A9>::value || stlsoft::is_function_pointer_type<A9>::value || unixstl::is_valid_dl_call_arg<A9>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A10>::value || stlsoft::is_pointer_type<A10>::value || stlsoft::is_function_pointer_type<A10>::value || unixstl::is_valid_dl_call_arg<A10>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A11>::value || stlsoft::is_pointer_type<A11>::value || stlsoft::is_function_pointer_type<A11>::value || unixstl::is_valid_dl_call_arg<A11>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A12>::value || stlsoft::is_pointer_type<A12>::value || stlsoft::is_function_pointer_type<A12>::value || unixstl::is_valid_dl_call_arg<A12>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A13>::value || stlsoft::is_pointer_type<A13>::value || stlsoft::is_function_pointer_type<A13>::value || unixstl::is_valid_dl_call_arg<A13>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A14>::value || stlsoft::is_pointer_type<A14>::value || stlsoft::is_function_pointer_type<A14>::value || unixstl::is_valid_dl_call_arg<A14>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A15>::value || stlsoft::is_pointer_type<A15>::value || stlsoft::is_function_pointer_type<A15>::value || unixstl::is_valid_dl_call_arg<A15>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A16>::value || stlsoft::is_pointer_type<A16>::value || stlsoft::is_function_pointer_type<A16>::value || unixstl::is_valid_dl_call_arg<A16>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A17>::value || stlsoft::is_pointer_type<A17>::value || stlsoft::is_function_pointer_type<A17>::value || unixstl::is_valid_dl_call_arg<A17>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A18>::value || stlsoft::is_pointer_type<A18>::value || stlsoft::is_function_pointer_type<A18>::value || unixstl::is_valid_dl_call_arg<A18>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A19>::value || stlsoft::is_pointer_type<A19>::value || stlsoft::is_function_pointer_type<A19>::value || unixstl::is_valid_dl_call_arg<A19>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A20>::value || stlsoft::is_pointer_type<A20>::value || stlsoft::is_function_pointer_type<A20>::value || unixstl::is_valid_dl_call_arg<A20>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A21>::value || stlsoft::is_pointer_type<A21>::value || stlsoft::is_function_pointer_type<A21>::value || unixstl::is_valid_dl_call_arg<A21>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A22>::value || stlsoft::is_pointer_type<A22>::value || stlsoft::is_function_pointer_type<A22>::value || unixstl::is_valid_dl_call_arg<A22>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A23>::value || stlsoft::is_pointer_type<A23>::value || stlsoft::is_function_pointer_type<A23>::value || unixstl::is_valid_dl_call_arg<A23>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A24>::value || stlsoft::is_pointer_type<A24>::value || stlsoft::is_function_pointer_type<A24>::value || unixstl::is_valid_dl_call_arg<A24>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A25>::value || stlsoft::is_pointer_type<A25>::value || stlsoft::is_function_pointer_type<A25>::value || unixstl::is_valid_dl_call_arg<A25>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A26>::value || stlsoft::is_pointer_type<A26>::value || stlsoft::is_function_pointer_type<A26>::value || unixstl::is_valid_dl_call_arg<A26>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A27>::value || stlsoft::is_pointer_type<A27>::value || stlsoft::is_function_pointer_type<A27>::value || unixstl::is_valid_dl_call_arg<A27>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A28>::value || stlsoft::is_pointer_type<A28>::value || stlsoft::is_function_pointer_type<A28>::value || unixstl::is_valid_dl_call_arg<A28>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A29>::value || stlsoft::is_pointer_type<A29>::value || stlsoft::is_function_pointer_type<A29>::value || unixstl::is_valid_dl_call_arg<A29>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A30>::value || stlsoft::is_pointer_type<A30>::value || stlsoft::is_function_pointer_type<A30>::value || unixstl::is_valid_dl_call_arg<A30>::value);
  STLSOFT_STATIC_ASSERT(stlsoft::is_fundamental_type<A31>::value || stlsoft::is_pointer_type<A31>::value || stlsoft::is_function_pointer_type<A31>::value || unixstl::is_valid_dl_call_arg<A31>::value);
#endif /* !UNIXSTL_DL_CALL_NO_ARG_TYPE_CHECK */

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

#ifndef _UNIXSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace unixstl
# else
} // namespace unixstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_UNIXSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* UNIXSTL_INCL_UNIXSTL_DL_HPP_DL_CALL */

/* ///////////////////////////// end of file //////////////////////////// */
