/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/smartptr/scoped_handle_borland_.hpp (evolved from MLResPtr.h, ::SynesisStd)
 *
 * Purpose:     scoped_handle - parameterisable RAII class for arbitrary
 *              resource types; special implementation for Borland.
 *
 * Created:     1st November 1994
 * Updated:     10th August 2009
 *
 * Thanks to:   Maciej Kaniewski, for requesting Borland compatibility (in
 *              order to use FastFormat and Pantheios)
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1994-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/smartptr/scoped_handle_borland_.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::scoped_handle smart
 *   pointer class template
 *   (\ref group__library__smart_pointers "Smart Pointers" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND_
#define STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND_

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND__MAJOR      6
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND__MINOR      1
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND__REVISION   2
# define STLSOFT_VER_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND__EDIT       670
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

//#if !defined(STLSOFT_COMPILER_IS_MSVC)
#if !defined(STLSOFT_COMPILER_IS_BORLAND)

# error This file is only defined for Borland C/C++ compiler versions
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Helper classes
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define USE_INTERNAL_MEMORY

struct scoped_handle_borland_impl_
{
    template <ss_typename_param_k H>
    struct function_type
    {
    public:
        typedef H       resource_type;

    public:
        virtual void destroy(resource_type h) = 0;
        virtual void release() = 0;
    };

    struct void_function_type
    {
    public:
        virtual void destroy() = 0;
        virtual void release() = 0;
    };


    /* Template for function of the form 
     *
     *  R (cdecl*)(H)
     */
    template<   ss_typename_param_k H
            ,   ss_typename_param_k R
            >
    struct cdecl_function_type
        : function_type<H>
    {
    public:
        typedef H                           resource_type;
        typedef cdecl_function_type<H, R>   class_type;
        typedef R (STLSOFT_CDECL*           function_type)(H);

    private:
        cdecl_function_type(function_type fn)
            : m_fn(fn)
        {}
        cdecl_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type   m_fn;
    };

    /* Template for function of the form 
     *
     *  R (fastcall*)(H)
     */
    template<   ss_typename_param_k H
            ,   ss_typename_param_k R
            >
    struct fastcall_function_type
        : function_type<H>
    {
    public:
        typedef H                               resource_type;
        typedef fastcall_function_type<H, R>    class_type;
        typedef R (STLSOFT_FASTCALL*            function_type)(H);

    private:
        fastcall_function_type(function_type fn)
            : m_fn(fn)
        {}
        fastcall_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type   m_fn;
    };

    /* Template for function of the form 
     *
     *  R (stdcall*)(H)
     */
    template<   ss_typename_param_k H
            ,   ss_typename_param_k R
            >
    struct stdcall_function_type
        : function_type<H>
    {
    public:
        typedef H                           resource_type;
        typedef stdcall_function_type<H, R> class_type;
        typedef R (STLSOFT_STDCALL*         function_type)(H);

    private:
        stdcall_function_type(function_type fn)
            : m_fn(fn)
        {}
        stdcall_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type   m_fn;
    };

    /* Template for function of the form 
     *
     *  void (cdecl*)(H)
     */
    template <ss_typename_param_k H>
    struct cdecl_function_type_v
        : function_type<H>
    {
    public:
        typedef H                           resource_type;
        typedef cdecl_function_type_v<H>    class_type;
        typedef void (STLSOFT_CDECL*        function_type)(H);

    public:
        cdecl_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        cdecl_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  void (fastcall*)(H)
     */
    template <ss_typename_param_k H>
    struct fastcall_function_type_v
        : function_type<H>
    {
    public:
        typedef H                           resource_type;
        typedef fastcall_function_type_v<H> class_type;
        typedef void (STLSOFT_FASTCALL*     function_type)(H);

    public:
        fastcall_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        fastcall_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  void (stdcall*)(H)
     */
    template <ss_typename_param_k H>
    struct stdcall_function_type_v
        : function_type<H>
    {
    public:
        typedef H                           resource_type;
        typedef stdcall_function_type_v<H>  class_type;
        typedef void (STLSOFT_STDCALL*      function_type)(H);

    public:
        stdcall_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        stdcall_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy(resource_type h)
        {
            m_fn(h);
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };


    /* Template for function of the form 
     *
     *  R (cdecl*)(void)
     */
    template <ss_typename_param_k R>
    struct cdecl_void_function_type
        : void_function_type
    {
    public:
        typedef cdecl_void_function_type<R> class_type;
        typedef R (STLSOFT_CDECL*           function_type)();

    private:
        cdecl_void_function_type(function_type fn)
            : m_fn(fn)
        {}
        cdecl_void_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  R (fastcall*)(void)
     */
    template <ss_typename_param_k R>
    struct fastcall_void_function_type
        : void_function_type
    {
    public:
        typedef fastcall_void_function_type<R>  class_type;
        typedef R (STLSOFT_FASTCALL*            function_type)();

    private:
        fastcall_void_function_type(function_type fn)
            : m_fn(fn)
        {}
        fastcall_void_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  R (stdcall*)(void)
     */
    template <ss_typename_param_k R>
    struct stdcall_void_function_type
        : void_function_type
    {
    public:
        typedef stdcall_void_function_type<R>   class_type;
        typedef R (STLSOFT_STDCALL*             function_type)();

    private:
        stdcall_void_function_type(function_type fn)
            : m_fn(fn)
        {}
        stdcall_void_function_type(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }


    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  void (cdecl*)(void)
     */
    struct cdecl_void_function_type_v
        : void_function_type
    {
    public:
        typedef cdecl_void_function_type_v  class_type;
        typedef void (STLSOFT_CDECL*        function_type)();

    public:
        cdecl_void_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        cdecl_void_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  void (fastcall*)(void)
     */
    struct fastcall_void_function_type_v
        : void_function_type
    {
    public:
        typedef fastcall_void_function_type_v   class_type;
        typedef void (STLSOFT_FASTCALL*         function_type)();

    public:
        fastcall_void_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        fastcall_void_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };

    /* Template for function of the form 
     *
     *  void (stdcall*)(void)
     */
    struct stdcall_void_function_type_v
        : void_function_type
    {
    public:
        typedef stdcall_void_function_type_v    class_type;
        typedef void (STLSOFT_STDCALL*          function_type)();

    public:
        stdcall_void_function_type_v(function_type fn)
            : m_fn(fn)
        {}
        stdcall_void_function_type_v(class_type const&);
        class_type& operator =(class_type const&);
    private:
        void* operator new(size_t cb, void* p, size_t n)
        {
            STLSOFT_ASSERT(cb <= n);

#ifdef USE_INTERNAL_MEMORY
            return p;
#else /* ? USE_INTERNAL_MEMORY */
            return ::operator new(cb);
#endif /* USE_INTERNAL_MEMORY */
        }
        void operator delete(void* pv)
        {
#ifndef USE_INTERNAL_MEMORY
            ::operator delete(pv);
#else /* ? USE_INTERNAL_MEMORY */
            STLSOFT_SUPPRESS_UNUSED(pv);
#endif /* USE_INTERNAL_MEMORY */
        }
#ifndef __BORLANDC__
        void operator delete(void* pv, void* p, size_t n)
        {
            operator delete(pv);
        }
#endif /* compiler */

    public:
        static class_type* create(void* p, size_t n, function_type fn)
        {
            return new(p, n) class_type(fn);
        }

    public:
        virtual void destroy()
        {
            m_fn();
        }
        virtual void release()
        {
            delete this;
        }

    private:
        function_type m_fn;
    };

};

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

template <ss_typename_param_k H>
class scoped_handle
{
public:
    typedef H                   resource_type;
    typedef H                   handle_type;
    typedef scoped_handle<H>    class_type;
private:
    typedef scoped_handle_borland_impl_::function_type<H>           function_type;

public: // NOTE: These constants have to be public for the Borland compiler
    enum
    {
            cdecl_function_type_v_size  =   sizeof(scoped_handle_borland_impl_::cdecl_function_type_v<H>)
        ,   cdecl_function_type_size    =   sizeof(scoped_handle_borland_impl_::cdecl_function_type<H, int>)
    };

public:
#ifdef STLSOFT_CF_CDECL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(
        resource_type   h
    ,   void            (STLSOFT_CDECL *fn)(resource_type)
    ,   resource_type   hNull = 0
    )
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::cdecl_function_type_v<H>::create(&m_bytes, sizeof(m_bytes), fn))
    {}

    scoped_handle(  resource_type*  ph
                ,   void            (STLSOFT_CDECL *fn)(resource_type*)
                ,   resource_type   hNull = 0);
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(
        resource_type   h
    ,   R               (STLSOFT_CDECL *fn)(resource_type)
    ,   resource_type   hNull = 0
    )
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::cdecl_function_type<H, R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}
    template <ss_typename_param_k R>
    scoped_handle(  resource_type*  ph
                ,   R               (STLSOFT_CDECL *fn)(resource_type*)
                ,   resource_type   hNull = 0);
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_CDECL_SUPPORTED */

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(  resource_type   h
                ,   void            (STLSOFT_FASTCALL *fn)(resource_type)
                ,   resource_type   hNull = 0)
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::fastcall_function_type_v<H>::create(&m_bytes, sizeof(m_bytes), fn))
    {}

    scoped_handle(  resource_type   h
                ,   void            (STLSOFT_FASTCALL *fn)(resource_type *)
                ,   resource_type   hNull = 0);
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(  resource_type   h
                ,   R               (STLSOFT_FASTCALL *fn)(resource_type)
                ,   resource_type   hNull = 0)
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::fastcall_function_type<H, R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}
    template <ss_typename_param_k R>
    scoped_handle(  resource_type*  ph
                ,   R               (STLSOFT_FASTCALL *fn)(resource_type*)
                ,   resource_type   hNull = 0);
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */


#ifdef STLSOFT_CF_STDCALL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(  resource_type   h
                ,   void            (STLSOFT_STDCALL *fn)(resource_type)
                ,   resource_type   hNull = 0)
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::stdcall_function_type_v<H>::create(&m_bytes, sizeof(m_bytes), fn))
    {}

    scoped_handle(  resource_type   *ph
                ,   void            (STLSOFT_STDCALL *fn)(resource_type*)
                ,   resource_type   hNull = 0);
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(  resource_type   h
                ,   R               (STLSOFT_STDCALL *fn)(resource_type)
                ,   resource_type   hNull = 0)
        : m_handle(h)
        , m_hNull(hNull)
        , m_fn(scoped_handle_borland_impl_::stdcall_function_type<H, R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}

    template <ss_typename_param_k R>
    scoped_handle(  resource_type*  ph
                ,   R               (STLSOFT_STDCALL *fn)(resource_type*)
                ,   resource_type   hNull = 0);
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */

    ~scoped_handle()
    {
        STLSOFT_MESSAGE_ASSERT("Invariant violation: function pointer must not be NULL", NULL != m_fn);

        if(!empty())
        {
            m_fn->destroy(m_handle);
        }

        m_fn->release();
    }

public:
    bool empty() const
    {
        STLSOFT_MESSAGE_ASSERT("Invariant violation: function pointer must not be NULL", NULL != m_fn);

        return get_null_value_() == m_handle;
    }

public:
    void close()
    {
        STLSOFT_MESSAGE_ASSERT("Invariant violation: function pointer must not be NULL", NULL != m_fn);

        if(!empty())
        {
            m_fn->destroy(m_handle);

            m_handle = get_null_value_();
        }
    }

    resource_type detach()
    {
        STLSOFT_MESSAGE_ASSERT("Invariant violation: function pointer must not be NULL", NULL != m_fn);

        resource_type h = m_handle;

        m_handle = get_null_value_();

        return h;
    }

public:
    resource_type handle() const
    {
        return m_handle;
    }
    resource_type get() const
    {
        return m_handle;
    }

private:
    resource_type get_null_value_() const
    {
        return m_hNull;
    }

private:
    handle_type             m_handle;
    const resource_type     m_hNull;
    function_type* const    m_fn;
    union
    {
        long double ld;
#if !defined(STLSOFT_COMPILER_IS_BORLAND) || \
    __BORLANDC__ > 0x0560
        ss_byte_t   cdecl_R[cdecl_function_type_size];
#endif /* compiler */

    }                       m_bytes;

private:
    scoped_handle(class_type const&);
    class_type& operator =(class_type const&);
};

STLSOFT_TEMPLATE_SPECIALISATION
class scoped_handle<void>
{
public:
    typedef void                resource_type;
    typedef void                handle_type;
    typedef scoped_handle<void> class_type;
private:
    typedef scoped_handle_borland_impl_::void_function_type function_type;
public:
    enum
    {
            cdecl_function_type_size    =   sizeof(function_type)
    };

public:
#ifdef STLSOFT_CF_CDECL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(  void            (STLSOFT_CDECL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::cdecl_void_function_type_v::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(  R               (STLSOFT_CDECL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::cdecl_void_function_type<R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_CDECL_SUPPORTED */

#ifdef STLSOFT_CF_FASTCALL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(  void            (STLSOFT_FASTCALL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::fastcall_void_function_type_v::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(  R               (STLSOFT_FASTCALL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::fastcall_void_function_type<R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */


#ifdef STLSOFT_CF_STDCALL_SUPPORTED
# if !defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT) || \
     defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED)
    scoped_handle(  void            (STLSOFT_STDCALL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::stdcall_void_function_type_v::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* !STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT || STLSOFT_CF_MEMBER_TEMPLATE_CTOR_OVERLOAD_DISCRIMINATED */

# if defined(STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT)
    template <ss_typename_param_k R>
    scoped_handle(  R               (STLSOFT_STDCALL *fn)())
        : m_bInvoked(false)
        , m_fn(scoped_handle_borland_impl_::stdcall_void_function_type<R>::create(&m_bytes, sizeof(m_bytes), fn))
    {}
# endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */

    ~scoped_handle()
    {
        close();
    }

public:
    bool empty() const
    {
        return m_bInvoked;
    }

public:
    void close()
    {
        if(!empty())
        {
            m_fn->destroy();
            m_bInvoked = true;
        }
    }

    resource_type detach()
    {
        m_bInvoked = true;
    }

public:
    resource_type handle() const
    {
    }
    resource_type get() const
    {
    }

private:
    ss_bool_t               m_bInvoked;
    function_type* const    m_fn;
    union
    {
        long double ld;
        ss_byte_t   cdecl_R[cdecl_function_type_size];

    }                       m_bytes;

private:
    scoped_handle(class_type const&);
    class_type& operator =(class_type const&);
};

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k H
        >
inline void swap(scoped_handle<H>& lhs, scoped_handle<H>& rhs)
{
    lhs.swap(rhs);
}

////////////////////////////////////////////////////////////////////////////
// Shims

template<ss_typename_param_k H>
#if defined(STLSOFT_COMPILER_IS_WATCOM)
inline H get_handle(scoped_handle<H> const& h)
#else /* ? compiler */
inline ss_typename_type_ret_k scoped_handle<H>::handle_type get_handle(scoped_handle<H> const& h)
#endif /* compiler */
{
    return h.get();
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/scoped_handle_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler warnings
 */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE_BORLAND_ */

/* ///////////////////////////// end of file //////////////////////////// */
