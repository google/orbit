/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/shell/memory/functions.h
 *
 * Purpose:     Shell memory functions.
 *
 * Created:     2nd March 1996
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1996-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/shell/memory/functions.h
 *
 * \brief [C, C++] Shell memory functions
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SHELL_MEMORY_H_FUNCTIONS
#define WINSTL_INCL_WINSTL_SHELL_MEMORY_H_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SHELL_MEMORY_H_FUNCTIONS_MAJOR       6
# define WINSTL_VER_WINSTL_SHELL_MEMORY_H_FUNCTIONS_MINOR       0
# define WINSTL_VER_WINSTL_SHELL_MEMORY_H_FUNCTIONS_REVISION    3
# define WINSTL_VER_WINSTL_SHELL_MEMORY_H_FUNCTIONS_EDIT        53
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[DocumentationStatus:Ready]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#ifndef STLSOFT_INCL_H_SHLOBJ
# define STLSOFT_INCL_H_SHLOBJ
# include <shlobj.h>
#endif /* !STLSOFT_INCL_H_SHLOBJ */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(_WINSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
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
 * C functions
 */

/** \brief [C only] Allocates a block of shell memory.
 *
 * \ingroup group__library__memory
 *
 * This function uses the shell allocator to allocate a memory block.
 *
 * \param cb The size, in bytes, of the memory block to be allocated.
 * \return Pointer to the allocated memory block, or NULL if the request
 *   failed.
 *
 * \remarks On failure, the function will set the thread error information,
 *   which may be retrieved by the Win32 API function
 *   <code>GetLastError()</code>
 *
 * \note [C++] This function is wrapped by the winstl::SHMemAlloc()
 *   function.
 */
STLSOFT_INLINE void *winstl__SHMemAlloc(ws_size_t cb)
{
    LPMALLOC    lpmalloc;
    void        *pv;
    HRESULT     hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(&lpmalloc);

    if(SUCCEEDED(hr))
    {
        pv = WINSTL_ITF_CALL(lpmalloc)->Alloc(WINSTL_ITF_THIS(lpmalloc) cb);
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));

        if(NULL == pv)
        {
            STLSOFT_NS_GLOBAL(SetLastError)(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
        pv = NULL;
    }

    return pv;
}

/** \brief [C only] Deallocates a block of shell memory previously allocated
 *    by a call to winstl__SHMemAlloc(), winstl__SHMemRealloc() or through
 *    the <code>IMalloc::Alloc()</code> or <code>IMalloc::Realloc()</code>
 *    methods on the shell allocator (obtained via <code>SHGetMalloc()</code>).
 *
 * \ingroup group__library__memory
 *
 * This function uses the shell allocator to allocate a memory block.
 *
 * \param pv Pointer to the memory block to be deallocated
 *
 * \remarks On failure, the function will set the thread error information,
 *   which may be retrieved by the Win32 API function
 *   <code>GetLastError()</code>
 *
 * \note [C++] This function is wrapped by the winstl::SHMemFree()
 *   function.
 */
STLSOFT_INLINE void winstl__SHMemFree(void *pv)
{
    LPMALLOC    lpmalloc;
    HRESULT     hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(&lpmalloc);

    if(SUCCEEDED(hr))
    {
        WINSTL_ITF_CALL(lpmalloc)->Free(WINSTL_ITF_THIS(lpmalloc) pv);
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
    }
}

#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 4 && \
    __GNUC_MINOR__ < 3

/* GCC pre 3.3 contains an invalid definition of IMalloc, where
 * the Realloc() method is called ReAlloc().
 *
 * We use a bit of casting to get round it here.
 */

# undef INTERFACE
# define INTERFACE IMallocGcc32
DECLARE_INTERFACE_(IMallocGcc32,IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;
    STDMETHOD_(void*,Alloc)(THIS_ ULONG) PURE;
    STDMETHOD_(void*,Realloc)(THIS_ void*,ULONG) PURE;
    STDMETHOD_(void,Free)(THIS_ void*) PURE;
    STDMETHOD_(ULONG,GetSize)(THIS_ void*) PURE;
    STDMETHOD_(int,DidAlloc)(THIS_ void*) PURE;
    STDMETHOD_(void,HeapMinimize)(THIS) PURE;
};

#endif /* compiler */





/** \brief [C only] Rellocates a block of shell memory.
 *
 * \ingroup group__library__memory
 *
 * This function uses the shell allocator to allocate a memory block.
 *
 * \param pv Pointer to the memory block to be reallocated. Can be NULL, in
 *   which case the function acts like SHMemAlloc()
 * \param cb The size, in bytes, of the memory block to be reallocated. Can
 *   be 0, in which case the function acts like SHMemFree() (if pv is not
 *   NULL), or like SHMemAlloc() (if pv is NULL).
 * \return Pointer to the allocated memory block, or NULL if the request
 *   failed or cb is 0 and pv is not NULL.
 *
 * \remarks On failure, the function will set the thread error information,
 *   which may be retrieved by the Win32 API function
 *   <code>GetLastError()</code>
 *
 * \note [C++] This function is wrapped by the winstl::SHMemRealloc()
 *   function.
 */

STLSOFT_INLINE void *winstl__SHMemRealloc(void *pv, ws_size_t cb)
{
#if defined(STLSOFT_COMPILER_IS_GCC) && \
    __GNUC__ < 4 && \
    __GNUC_MINOR__ < 3

    IMallocGcc32    *lpmalloc;
#else /* ? compiler */
    LPMALLOC        lpmalloc;
#endif /* compiler */
    void            *pvNew;
    HRESULT         hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(stlsoft_reinterpret_cast(LPMALLOC*, &lpmalloc));

    if(SUCCEEDED(hr))
    {
        pvNew = WINSTL_ITF_CALL(lpmalloc)->Realloc(WINSTL_ITF_THIS(lpmalloc) pv, cb);
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));

        if( NULL == pvNew &&
            0 != cb &&
            NULL != pv)
        {
            STLSOFT_NS_GLOBAL(SetLastError)(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
        pvNew = NULL;
    }

    return pvNew;
}

/** \brief [C only] Gives the size of a memory block
 *
 * \ingroup group__library__memory
 *
 * This function returns the size of a memory block relative to the COM task
 * alloctor, as per <code>IMalloc::GetSize()</code>
 *
 * \param pv Pointer to the memory block
 * \return The size of the memory block (in bytes)
 *
 * \note [C++] This function is wrapped by the winstl::SHMemGetSize()
 *   function.
 */
STLSOFT_INLINE ws_size_t winstl__SHMemGetSize(void *pv)
{
    LPMALLOC    lpmalloc;
    ws_size_t   ulRet;
    HRESULT     hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(&lpmalloc);

    if(SUCCEEDED(hr))
    {
        ulRet = WINSTL_ITF_CALL(lpmalloc)->GetSize(WINSTL_ITF_THIS(lpmalloc) pv);
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
        ulRet = 0;
    }

    return ulRet;
}

/** \brief [C only] Determines allocation ownership of a memory block
 *
 * \ingroup group__library__memory
 *
 * This function returns a value indicating whether a memory block was allocated
 * by the COM task allocator, as per <code>IMalloc::DidAlloc()</code>
 *
 * \param pv Pointer to the memory block
 * \return Result indicating ownership
 * \retval 1 The memory block was allocated by the task allocator
 * \retval 0 The memory block was <i>not</i> allocated by the task allocator
 * \retval -1 SHMemDidAlloc() cannot determine whether the memory block was allocated by the task allocator
 *
 * \note [C++] This function is wrapped by the winstl::SHMemDidAlloc()
 *   function.
 */
STLSOFT_INLINE ws_sint_t winstl__SHMemDidAlloc(void *pv)
{
    LPMALLOC    lpmalloc;
    ws_sint_t   iRet;
    HRESULT     hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(&lpmalloc);

    if(SUCCEEDED(hr))
    {
        iRet = WINSTL_ITF_CALL(lpmalloc)->DidAlloc(WINSTL_ITF_THIS(lpmalloc) pv);
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
        iRet = -1;
    }

    return iRet;
}

/** \brief [C only] Minimises the heap
 *
 * \ingroup group__library__memory
 *
 * This function minimises the heap as much as possible by releasing unused
 * memory to the operating system, coalescing adjacent free blocks and
 * committing free pages, as as per <code>IMalloc::HeapMinimize()</code>.
 *
 * \note [C++] This function is wrapped by the winstl::SHMemHeapMinimise()
 *   function.
 */
STLSOFT_INLINE void winstl__SHMemHeapMinimise(void)
{
    LPMALLOC    lpmalloc;
    HRESULT     hr  =   STLSOFT_NS_GLOBAL(SHGetMalloc)(&lpmalloc);

    if(SUCCEEDED(hr))
    {
        WINSTL_ITF_CALL(lpmalloc)->HeapMinimize(WINSTL_ITF_THIS0(lpmalloc));
        WINSTL_ITF_CALL(lpmalloc)->Release(WINSTL_ITF_THIS0(lpmalloc));
    }
    else
    {
        STLSOFT_NS_GLOBAL(SetLastError)(stlsoft_static_cast(DWORD, hr));
    }
}

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
namespace winstl
{
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * C++ functions
 */

#ifdef __cplusplus

/** \brief Allocates a block of shell memory.
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemAlloc().
 *
 * \param cb The size, in bytes, of the memory block to be allocated.
 * \return Pointer to the allocated memory block, or NULL if the request
 *   failed.
 */
inline void *SHMemAlloc(ws_size_t cb)
{
    return winstl__SHMemAlloc(cb);
}

/** \brief Deallocates a block of shell memory.
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemFree().
 *
 * \param pv Pointer to the memory block to be deallocated
 */
inline void SHMemFree(void *pv)
{
    winstl__SHMemFree(pv);
}

/** \brief Changes the size of a previously allocated block of shell memory.
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemRelloc().
 *
 * \param pv Pointer to the memory block to be reallocated. Can be NULL, in
 *   which case the function acts like SHMemAlloc()
 * \param cb The size, in bytes, of the memory block to be reallocated. Can
 *   be 0, in which case the function acts like SHMemFree() (if pv is not
 *   NULL), or like SHMemAlloc() (if pv is NULL).
 * \return Pointer to the allocated memory block, or NULL if the request
 *   failed or cb is 0 and pv is not NULL.
 */
inline void *SHMemRealloc(void *pv, ws_size_t cb)
{
    return winstl__SHMemRealloc(pv, cb);
}

/** \brief Gives the size of a memory block
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemGetSize().
 *
 * \param pv Pointer to the memory block
 * \return The size of the memory block (in bytes)
 */
inline ws_size_t SHMemGetSize(void *pv)
{
    return winstl__SHMemGetSize(pv);
}

/** \brief Determines allocation ownership of a memory block
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemDidAlloc().
 *
 * \param pv Pointer to the memory block
 * \return Result indicating ownership
 * \retval 1 The memory block was allocated by the task allocator
 * \retval 0 The memory block was <i>not</i> allocated by the task allocator
 * \retval -1 SHMemDidAlloc() cannot determine whether the memory block was allocated by the task allocator
 */
inline ws_sint_t SHMemDidAlloc(void *pv)
{
    return winstl__SHMemDidAlloc(pv);
}

/** \brief Minimises the heap
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemHeapMinimise().
 */
inline void SHMemHeapMinimise()
{
    winstl__SHMemHeapMinimise();
}

/** \brief Minimises the heap
 *
 * \ingroup group__library__memory
 *
 * This function is a wrapper for winstl__SHMemHeapMinimise().
 */
inline void SHMemHeapMinimize()
{
    winstl__SHMemHeapMinimise();
}

#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace stlsoft::winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SHELL_MEMORY_H_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
