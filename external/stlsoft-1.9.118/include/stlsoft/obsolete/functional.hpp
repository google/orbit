/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/functional.hpp
 *
 * Purpose:     Mappings to stdlib string functions
 *
 * Created:     2nd December 2004
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/obsolete/functional.hpp
///
/// Mappings to stdlib string functions
 */

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FUNCTIONAL
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FUNCTIONAL

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FUNCTIONAL_MAJOR      2
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FUNCTIONAL_MINOR      0
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FUNCTIONAL_REVISION   2
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_FUNCTIONAL_EDIT       17
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_HPP_GET_PTR
# include <stlsoft/shims/attribute/get_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_HPP_GET_PTR */

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
 * Functors
 */

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
class mem_fun_t
    : public stlsoft_ns_qual_std(unary_function)<C *, R>
{
public:
    ss_explicit_k mem_fun_t(R (C::*PFn)())
        : m_pfn(PFn)
    {}

public:
    R operator()(C *c) const
    {
        return invoke_(c);
    }
    template <ss_typename_param_k T1>
    R operator()(T1 &t1) const
    {
        return invoke_(::stlsoft::get_ptr(t1));
    }

/// \name Implementation
/// @{
private:
    R invoke_(C *c) const
    {
        return (c->*m_pfn)();
    }

// Members
private:
    R   (C::*m_pfn)();
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
class mem_fun_const_t
    : public stlsoft_ns_qual_std(unary_function)<C *, R>
{
public:
    ss_explicit_k mem_fun_const_t(R (C::*PFn)() const)
        : m_pfn(PFn)
    {}

public:
    R operator()(C *c) const
    {
        return invoke_(c);
    }
    template <ss_typename_param_k T1>
    R operator()(T1 &t1) const
    {
        return invoke_(::stlsoft::get_ptr(t1));
    }

/// \name Implementation
/// @{
private:
    R invoke_(C *c) const
    {
        return (c->*m_pfn)();
    }

// Members
private:
    R   (C::*m_pfn)() const;
};

template<   ss_typename_param_k C
        >
class mem_fun_void_t
    : public stlsoft_ns_qual_std(unary_function)<C *, void>
{
public:
    ss_explicit_k mem_fun_void_t(void (C::*PFn)())
        : m_pfn(PFn)
    {}

public:
    void operator()(C *c) const
    {
        invoke_(c);
    }
    template <ss_typename_param_k T1>
    void operator()(T1 &t1) const
    {
        invoke_(::stlsoft::get_ptr(t1));
    }

/// \name Implementation
/// @{
private:
    void invoke_(C *c) const
    {
        (c->*m_pfn)();
    }

// Members
private:
    void    (C::*m_pfn)();
};

template<   ss_typename_param_k C
        >
class mem_fun_void_const_t
    : public stlsoft_ns_qual_std(unary_function)<C const*, void>
{
public:
    ss_explicit_k mem_fun_void_const_t(void (C::*PFn)() const)
        : m_pfn(PFn)
    {}

public:
    void operator()(C *c) const
    {
        invoke_(c);
    }
    template <ss_typename_param_k T1>
    void operator()(T1 &t1) const
    {
        invoke_(::stlsoft::get_ptr(t1));
    }

/// \name Implementation
/// @{
private:
    void invoke_(C *c) const
    {
        (c->*m_pfn)();
    }

// Members
private:
    void    (C::*m_pfn)() const;
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_t<R, C> mem_fun(R (C::*PFn)())
{
    return (mem_fun_t<R, C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_void_t<C> mem_fun(void (C::*PFn)())
{
    return (mem_fun_void_t<C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_void_t<C> mem_fun_void(void (C::*PFn)())
{
    return (mem_fun_void_t<C>(PFn));
}

//#if defined(STLSOFT_COMPILER_IS_MSVC) && _MSC_VER < 1310
template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_const_t<R, C> mem_fun(R (C::*PFn)() const)
{
    return (mem_fun_const_t<R, C>(PFn));
}

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_const_t<R, C> mem_fun_const(R (C::*PFn)() const)
{
    return (mem_fun_const_t<R, C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_void_const_t<C> mem_fun(void (C::*PFn)() const)
{
    return (mem_fun_void_const_t<C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_void_const_t<C> mem_fun_void_const(void (C::*PFn)() const)
{
    return (mem_fun_void_const_t<C>(PFn));
}
//#endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1310 */


template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
class mem_fun1_t
    : public stlsoft_ns_qual_std(binary_function)<C *, A, R>
{
public:
    ss_explicit_k mem_fun1_t(R (C::*PFn)(A))
        : m_pfn(PFn)
    {}

public:
    R operator()(C *c, A a0) const
    {
        return (c->*m_pfn)(a0);
    }
private:
    R (C::*m_pfn)(A);
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline mem_fun1_t<R, C, A> mem_fun1(R (C::*PFn)(A))
{
    return (mem_fun1_t<R, C, A>(PFn));
}




template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
class mem_fun_ref_t
    : public stlsoft_ns_qual_std(unary_function)<C, R>
{
public:
    ss_explicit_k mem_fun_ref_t(R (C::*PFn)())
        : m_pfn(PFn)
    {}

public:
    R operator()(C &c) const
    {
        return (c.*m_pfn)();
    }
private:
    R (C::*m_pfn)();
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
class mem_fun_ref_const_t
    : public stlsoft_ns_qual_std(unary_function)<C, R>
{
public:
    ss_explicit_k mem_fun_ref_const_t(R (C::*PFn)() const)
        : m_pfn(PFn)
    {}

public:
    R operator()(C &c) const
    {
        return (c.*m_pfn)();
    }
private:
    R (C::*m_pfn)() const;
};


template<   ss_typename_param_k C
        >
class mem_fun_ref_void_t
    : public stlsoft_ns_qual_std(unary_function)<C, void>
{
public:
    ss_explicit_k mem_fun_ref_void_t(void (C::*PFn)())
        : m_pfn(PFn)
    {}

public:
    void operator()(C &c) const
    {
        (c.*m_pfn)();
    }
private:
    void (C::*m_pfn)();
};

template<   ss_typename_param_k C
        >
class mem_fun_ref_void_const_t
    : public stlsoft_ns_qual_std(unary_function)<C, void>
{
public:
    ss_explicit_k mem_fun_ref_void_const_t(void (C::*PFn)() const)
        : m_pfn(PFn)
    {}

public:
    void operator()(C &c) const
    {
        return (c.*m_pfn)();
    }
private:
    void (C::*m_pfn)() const;
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_ref_t<R, C> mem_fun_ref(R (C::*PFn)())
{
    return (mem_fun_ref_t<R, C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_ref_void_t<C> mem_fun_ref(void (C::*PFn)())
{
    return (mem_fun_ref_void_t<C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_ref_void_t<C> mem_fun_ref_void(void (C::*PFn)())
{
    return (mem_fun_ref_void_t<C>(PFn));
}

//#if defined(STLSOFT_COMPILER_IS_MSVC) && _MSC_VER < 1310
template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_ref_const_t<R, C> mem_fun_ref(R (C::*PFn)() const)
{
    return (mem_fun_ref_const_t<R, C>(PFn));
}

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        >
inline mem_fun_ref_const_t<R, C> mem_fun_ref_const(R (C::*PFn)() const)
{
    return (mem_fun_ref_const_t<R, C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_ref_void_const_t<C> mem_fun_ref(void (C::*PFn)() const)
{
    return (mem_fun_ref_void_const_t<C>(PFn));
}

template<   ss_typename_param_k C
        >
inline mem_fun_ref_void_const_t<C> mem_fun_ref_void_const(void (C::*PFn)() const)
{
    return (mem_fun_ref_void_const_t<C>(PFn));
}
//#endif /* STLSOFT_COMPILER_IS_MSVC && _MSC_VER < 1310 */



template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
class mem_fun1_ref_t
    : public stlsoft_ns_qual_std(binary_function)<C *, A, R>
{
public:
    ss_explicit_k mem_fun1_ref_t(R (C::*PFn)(A))
    : m_pfn(PFn)
    {}

public:
    R operator()(C &c, A a0) const
    {
        return (c.*m_pfn)(a0);
    }

private:
    R (C::*m_pfn)(A);
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline mem_fun1_ref_t<R, C, A> mem_fun1_ref(R (C::*PFn)(A))
{
    return (mem_fun1_ref_t<R, C, A>(PFn));
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \brief
 * \alpha
 *
 * \ingroup group__library__functional
 *
 * \note This is an alpha form, and *will* be changed in a future release
 */
template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k V
        >
// [[synesis:class:function-class:unary-function: mem_fun_ref_1_t<T<R>, T<C>, T<V>>]]
struct mem_fun_ref_1_t
{
public:
    mem_fun_ref_1_t(R (C::*pfn)(V), V value)
        : m_pfn(pfn)
        , m_value(value)
    {}

    void operator ()(C &c)
    {
        (c.*m_pfn)(m_value);
    }

private:
    R (C::*m_pfn)(V);
    V       m_value;
};

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k V
        >
inline mem_fun_ref_1_t<R, C, V> mem_fun_ref_1(R (C::*pfn)(V), V value)
{
    return mem_fun_ref_1_t<R, C, V>(pfn, value);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_FUNCTIONAL */

/* ///////////////////////////// end of file //////////////////////////// */
