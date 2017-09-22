/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/functional/function_pointer_adaptors.hpp (originally ::SynesisStd)
 *
 * Purpose:     Contains the stlsoft::ptr_fun calling convention-aware function adaptors.
 *
 * Created:     13th June 1999
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


/** \file stlsoft/functional/function_pointer_adaptors.hpp
 *
 * \brief [C++ only] Function classes that adapt non-member functions (and
 *   handle different calling conventions)
 *   (\ref group__library__functional "Functional" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS
#define STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS_MAJOR    4
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS_MINOR    2
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS_REVISION 2
# define STLSOFT_VER_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS_EDIT     62
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_INCL_FUNCTIONAL
# define STLSOFT_INCL_FUNCTIONAL
# include <functional>
#endif /* !STLSOFT_INCL_FUNCTIONAL */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

# ifdef STLSOFT_CF_CDECL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with CDecl (__cdecl) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_cdecl_function_pointer<T<R>, T<A>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
struct unary_cdecl_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, R>
{
public:
    typedef R                           return_type;
    typedef A                           argument_type;
    typedef return_type (STLSOFT_CDECL  *function_type)(argument_type);
public:
    ss_explicit_k unary_cdecl_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(argument_type arg) const
    {
        return (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with CDecl (__cdecl) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_cdecl_function_pointer<T<R>, T<A0>, T<A1>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
struct binary_cdecl_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, R>
{
public:
    typedef R                           return_type;
    typedef A0                          first_argument_type;
    typedef A1                          second_argument_type;
    typedef return_type (STLSOFT_CDECL  *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_cdecl_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(first_argument_type a0, second_argument_type a1) const
    {
        return (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

# ifdef STLSOFT_CF_FASTCALL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with FastCall (__fastcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_fastcall_function_pointer<T<R>, T<A>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
struct unary_fastcall_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, R>
{
public:
    typedef R                               return_type;
    typedef A                               argument_type;
    typedef return_type (STLSOFT_FASTCALL   *function_type)(argument_type);
public:
    ss_explicit_k unary_fastcall_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(argument_type arg) const
    {
        return (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with FastCall (__fastcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_fastcall_function_pointer<T<R>, T<A0>, T<A1>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
struct binary_fastcall_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, R>
{
public:
    typedef R                               return_type;
    typedef A0                              first_argument_type;
    typedef A1                              second_argument_type;
    typedef return_type (STLSOFT_FASTCALL   *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_fastcall_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(first_argument_type a0, second_argument_type a1) const
    {
        return (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

# ifdef STLSOFT_CF_STDCALL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with StdCall (__stdcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_stdcall_function_pointer<T<R>, T<A>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
struct unary_stdcall_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, R>
{
public:
    typedef R                               return_type;
    typedef A                               argument_type;
    typedef return_type (STLSOFT_STDCALL    *function_type)(argument_type);
public:
    ss_explicit_k unary_stdcall_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(argument_type arg) const
    {
        return (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with StdCall (__stdcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_stdcall_function_pointer<T<R>, T<A0>, T<A1>>]]
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
struct binary_stdcall_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, R>
{
public:
    typedef R                               return_type;
    typedef A0                              first_argument_type;
    typedef A1                              second_argument_type;
    typedef return_type (STLSOFT_STDCALL    *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_stdcall_function_pointer(function_type func)
        : m_func(func)
    {}
    return_type operator ()(first_argument_type a0, second_argument_type a1) const
    {
        return (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */


#if !defined(STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID)
# ifdef STLSOFT_CF_CDECL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_cdecl_void_function_pointer<T<R>, T<A>>]]
template <ss_typename_param_k A>
struct unary_cdecl_void_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, void>
{
public:
    typedef void                        return_type;
    typedef A                           argument_type;
    typedef return_type (STLSOFT_CDECL  *function_type)(argument_type);
public:
    ss_explicit_k unary_cdecl_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(argument_type arg) const
    {
        (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_cdecl_void_function_pointer<T<R>, T<A0>, T<A1>>]]
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
struct binary_cdecl_void_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, void>
{
public:
    typedef void                        return_type;
    typedef A0                          first_argument_type;
    typedef A1                          second_argument_type;
    typedef return_type (STLSOFT_CDECL  *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_cdecl_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(first_argument_type a0, second_argument_type a1) const
    {
        (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

# ifdef STLSOFT_CF_FASTCALL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_fastcall_void_function_pointer<T<R>, T<A>>]]
template <ss_typename_param_k A>
struct unary_fastcall_void_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, void>
{
public:
    typedef void                            return_type;
    typedef A                               argument_type;
    typedef return_type (STLSOFT_FASTCALL   *function_type)(argument_type);
public:
    ss_explicit_k unary_fastcall_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(argument_type arg) const
    {
        (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_fastcall_void_function_pointer<T<R>, T<A0>, T<A1>>]]
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
struct binary_fastcall_void_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, void>
{
public:
    typedef void                            return_type;
    typedef A0                              first_argument_type;
    typedef A1                              second_argument_type;
    typedef return_type (STLSOFT_FASTCALL   *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_fastcall_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(first_argument_type a0, second_argument_type a1) const
    {
        (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

# ifdef STLSOFT_CF_STDCALL_SUPPORTED
/** \brief A unary function adaptor for pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:unary-function: unary_stdcall_void_function_pointer<T<R>, T<A>>]]
template <ss_typename_param_k A>
struct unary_stdcall_void_function_pointer
    : public stlsoft_ns_qual_std(unary_function)<A, void>
{
public:
    typedef void                            return_type;
    typedef A                               argument_type;
    typedef return_type (STLSOFT_STDCALL    *function_type)(argument_type);
public:
    ss_explicit_k unary_stdcall_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(argument_type arg) const
    {
        (*m_func)(arg);
    }
private:
    function_type m_func;
};

/** \brief A binary function adaptor for pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
// [[synesis:class:function-class:binary-function: binary_stdcall_void_function_pointer<T<R>, T<A0>, T<A1>>]]
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
struct binary_stdcall_void_function_pointer
    : public stlsoft_ns_qual_std(binary_function)<A0, A1, void>
{
public:
    typedef void                            return_type;
    typedef A0                              first_argument_type;
    typedef A1                              second_argument_type;
    typedef return_type (STLSOFT_STDCALL    *function_type)(first_argument_type, second_argument_type);
public:
    ss_explicit_k binary_stdcall_void_function_pointer(function_type func)
        : m_func(func)
    {}
    void operator ()(first_argument_type a0, second_argument_type a1) const
    {
        (*m_func)(a0, a1);
    }
private:
    function_type m_func;
};
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */
#endif /* !STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID */

/* /////////////////////////////////////////////////////////////////////////
 * Creator functions
 */

#ifdef STLSOFT_CF_CDECL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
unary_cdecl_function_pointer<R, A> ptr_fun(R (STLSOFT_CDECL *func)(A))
{
    return unary_cdecl_function_pointer<R, A>(func);
}

/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
binary_cdecl_function_pointer<R, A0, A1> ptr_fun(R (STLSOFT_CDECL *func)(A0, A1))
{
    return binary_cdecl_function_pointer<R, A0, A1>(func);
}
#endif /* STLSOFT_CF_CDECL_SUPPORTED */

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
unary_fastcall_function_pointer<R, A> ptr_fun(R (STLSOFT_FASTCALL *func)(A))
{
    return unary_fastcall_function_pointer<R, A>(func);
}

/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
binary_fastcall_function_pointer<R, A0, A1> ptr_fun(R (STLSOFT_FASTCALL *func)(A0, A1))
{
    return binary_fastcall_function_pointer<R, A0, A1>(func);
}
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

#ifdef STLSOFT_CF_STDCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A
        >
unary_stdcall_function_pointer<R, A> ptr_fun(R (STLSOFT_STDCALL *func)(A))
{
    return unary_stdcall_function_pointer<R, A>(func);
}

/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention
 *
 * \ingroup group__library__functional
 *
 */
template< ss_typename_param_k R
        , ss_typename_param_k A0
        , ss_typename_param_k A1
        >
binary_stdcall_function_pointer<R, A0, A1> ptr_fun(R (STLSOFT_STDCALL *func)(A0, A1))
{
    return binary_stdcall_function_pointer<R, A0, A1>(func);
}
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */


#if defined(STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID)

# ifdef STLSOFT_CF_CDECL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_cdecl_function_pointer<void, A> ptr_fun_void(void (STLSOFT_CDECL *func)(A))
{
    return unary_cdecl_function_pointer<void, A>(func);
}

/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_cdecl_function_pointer<void, A0, A1> ptr_fun_void(void (STLSOFT_CDECL *func)(A0, A1))
{
    return binary_cdecl_function_pointer<void, A0, A1>(func);
}
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

# ifdef STLSOFT_CF_FASTCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_fastcall_function_pointer<void, A> ptr_fun_void(void (STLSOFT_FASTCALL *func)(A))
{
    return unary_fastcall_function_pointer<void, A>(func);
}

/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_fastcall_function_pointer<void, A0, A1> ptr_fun_void(void (STLSOFT_FASTCALL *func)(A0, A1))
{
    return binary_fastcall_function_pointer<void, A0, A1>(func);
}
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

# ifdef STLSOFT_CF_STDCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_stdcall_function_pointer<void, A> ptr_fun_void(void (STLSOFT_STDCALL *func)(A))
{
    return unary_stdcall_function_pointer<void, A>(func);
}

/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_stdcall_function_pointer<void, A0, A1> ptr_fun_void(void (STLSOFT_STDCALL *func)(A0, A1))
{
    return binary_stdcall_function_pointer<void, A0, A1>(func);
}
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */


#else /* ? STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID */

# ifdef STLSOFT_CF_CDECL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_cdecl_void_function_pointer<A> ptr_fun_void(void (STLSOFT_CDECL *func)(A))
{
    return unary_cdecl_void_function_pointer<A>(func);
}
/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_cdecl_void_function_pointer<A> ptr_fun(void (STLSOFT_CDECL *func)(A))
{
    return ptr_fun_void(func);
}

/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_cdecl_void_function_pointer<A0, A1> ptr_fun_void(void (STLSOFT_CDECL *func)(A0, A1))
{
    return binary_cdecl_void_function_pointer<A0, A1>(func);
}
/** \brief Creator function to adapt pointers to functions with CDecl (__cdecl) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_cdecl_void_function_pointer<A0, A1> ptr_fun(void (STLSOFT_CDECL *func)(A0, A1))
{
    return ptr_fun_void(func);
}
# endif /* STLSOFT_CF_CDECL_SUPPORTED */

# ifdef STLSOFT_CF_FASTCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_fastcall_void_function_pointer<A> ptr_fun_void(void (STLSOFT_FASTCALL *func)(A))
{
    return unary_fastcall_void_function_pointer<A>(func);
}
/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_fastcall_void_function_pointer<A> ptr_fun(void (STLSOFT_FASTCALL *func)(A))
{
    return ptr_fun_void(func);
}

/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_fastcall_void_function_pointer<A0, A1> ptr_fun_void(void (STLSOFT_FASTCALL *func)(A0, A1))
{
    return binary_fastcall_void_function_pointer<A0, A1>(func);
}
/** \brief Creator function to adapt pointers to functions with FastCall (__fastcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_fastcall_void_function_pointer<A0, A1> ptr_fun(void (STLSOFT_FASTCALL *func)(A0, A1))
{
    return ptr_fun_void(func);
}
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

# ifdef STLSOFT_CF_STDCALL_SUPPORTED
/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_stdcall_void_function_pointer<A> ptr_fun_void(void (STLSOFT_STDCALL *func)(A))
{
    return unary_stdcall_void_function_pointer<A>(func);
}
/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template <ss_typename_param_k A>
unary_stdcall_void_function_pointer<A> ptr_fun(void (STLSOFT_STDCALL *func)(A))
{
    return ptr_fun_void(func);
}

/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_stdcall_void_function_pointer<A0, A1> ptr_fun_void(void (STLSOFT_STDCALL *func)(A0, A1))
{
    return binary_stdcall_void_function_pointer<A0, A1>(func);
}
/** \brief Creator function to adapt pointers to functions with StdCall (__stdcall) calling convention that
 * have a void return type
 *
 * \ingroup group__library__functional
 *
 */
template<   ss_typename_param_k A0
        ,   ss_typename_param_k A1
        >
binary_stdcall_void_function_pointer<A0, A1> ptr_fun(void (STLSOFT_STDCALL *func)(A0, A1))
{
    return ptr_fun_void(func);
}
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */

#endif /* STLSOFT_CF_COMPILER_SUPPORTS_RETURN_VOID */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_FUNCTIONAL_HPP_FUNCTION_POINTER_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
