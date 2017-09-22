/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/synch/atomic_functions.h (originally MLAtomic.cpp, ::SynesisStd)
 *
 * Purpose:     WinSTL atomic functions.
 *
 * Created:     23rd October 1997
 * Updated:     29th April 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1997-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/synch/atomic_functions.h
 *
 * \brief [C++ only] Definition of the atomic functions
 *   (\ref group__library__synch "Synchronisation" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS
#define WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS_MAJOR     4
# define WINSTL_VER_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS_MINOR     4
# define WINSTL_VER_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS_REVISION  1
# define WINSTL_VER_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS_EDIT      203
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MWERKS: __MWERKS__<0x3000
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_TYPES
# include <winstl/synch/atomic_types.h>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_TYPES */
#ifdef __cplusplus
# ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX
#  include <winstl/synch/spin_mutex.hpp>
# endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX */
#endif /* __cplusplus */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

#if !defined(WINSTL_ARCH_IS_X86) && \
    !defined(WINSTL_ARCH_IS_IA64) && \
    !defined(WINSTL_ARCH_IS_X64)
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */

#ifdef STLSOFT_ATOMIC_CALLCONV
# undef STLSOFT_ATOMIC_CALLCONV
#endif /* STLSOFT_ATOMIC_CALLCONV */
#ifdef WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL
# undef WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL
#endif /* WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL */
#ifdef WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL
# undef WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL
#endif /* WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL */

#ifndef STLSOFT_NO_FASTCALL
# if defined(STLSOFT_COMPILER_IS_BORLAND) || \
     defined(STLSOFT_COMPILER_IS_DMC) || \
     defined(STLSOFT_COMPILER_IS_WATCOM)
#  define STLSOFT_NO_FASTCALL
# endif /* compiler */
#endif /* STLSOFT_NO_FASTCALL */

#if defined(WINSTL_ARCH_IS_X86)

# if defined(STLSOFT_CF_FASTCALL_SUPPORTED) && \
     !defined(STLSOFT_NO_FASTCALL)
#  define WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL
#  define WINSTL_ATOMIC_FNS_CALLCONV        __fastcall
# elif defined(STLSOFT_CF_STDCALL_SUPPORTED)
#  define WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL
#  define WINSTL_ATOMIC_FNS_CALLCONV        __stdcall
# else
#  error Need to define calling convention
# endif /* call-conv */

#elif defined(WINSTL_ARCH_IS_IA64) || \
      defined(WINSTL_ARCH_IS_X64)

#  define WINSTL_ATOMIC_FNS_CALLCONV_IS_CDECL
#  define WINSTL_ATOMIC_FNS_CALLCONV        __cdecl

#else /* ? arch */
# error Only defined for the Intel x86 and IA64 architectures
#endif /* arch */

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
 * Implementation options
 *
 * Because some compilers can make the code actually faster when it the naked
 * functions are not inline, we provide for that here. If you want to out-of-line
 * the functions, then you just need to define WINSTL_ATOMIC_FNS_DECLARATION_ONLY
 * in the code that uses it, and define WINSTL_ATOMIC_FNS_DEFINITION in one
 * implementation file.
 */

#ifdef WINSTL_ATOMIC_FNS_DECL_
# undef WINSTL_ATOMIC_FNS_DECL_
#endif /* WINSTL_ATOMIC_FNS_DECL_ */

#ifdef WINSTL_ATOMIC_FNS_IMPL_
# undef WINSTL_ATOMIC_FNS_IMPL_
#endif /* WINSTL_ATOMIC_FNS_IMPL_ */

#if defined(WINSTL_ATOMIC_FNS_DECLARATION_ONLY)
/* Only the function declarations are included */
# define WINSTL_ATOMIC_FNS_DECL_(type)              type WINSTL_ATOMIC_FNS_CALLCONV
#elif defined(WINSTL_ATOMIC_FNS_DEFINITION)
/* Only the function definitions are included */
# ifdef STSLSOFT_INLINE_ASM_SUPPORTED
#  define WINSTL_ATOMIC_FNS_IMPL_(type)             __declspec(naked) type WINSTL_ATOMIC_FNS_CALLCONV
# else /* ? STSLSOFT_INLINE_ASM_SUPPORTED */
#  define WINSTL_ATOMIC_FNS_IMPL_(type)             type WINSTL_ATOMIC_FNS_CALLCONV
# endif /* STSLSOFT_INLINE_ASM_SUPPORTED */
#else /* ? declaration / definition */
# if defined(STLSOFT_COMPILER_IS_MWERKS) && \
     (__MWERKS__ & 0xFF00) < 0x3000
#  error CodeWarrior 7 and earlier does not generate correct code when inline naked functions are used
# endif /* compiler */

#if !defined(__cplusplus) && \
    defined(STSLSOFT_INLINE_ASM_SUPPORTED)
 /* Not currently supporting inline assembler for C compilation. It's perfectly possible, but need more work to sort out. */
# undef STSLSOFT_INLINE_ASM_SUPPORTED
#endif /* !__cplusplus && STSLSOFT_INLINE_ASM_SUPPORTED */

# ifdef STSLSOFT_INLINE_ASM_SUPPORTED
  /* The default is to define them inline */
#  ifdef STSLSOFT_ASM_IN_INLINE_SUPPORTED
#   define WINSTL_ATOMIC_FNS_DECL_(type)             inline type WINSTL_ATOMIC_FNS_CALLCONV
#   define WINSTL_ATOMIC_FNS_IMPL_(type)             inline __declspec(naked) type WINSTL_ATOMIC_FNS_CALLCONV
#  else /* ? STSLSOFT_ASM_IN_INLINE_SUPPORTED */
#   define WINSTL_ATOMIC_FNS_DECL_(type)             type WINSTL_ATOMIC_FNS_CALLCONV
#   define WINSTL_ATOMIC_FNS_IMPL_(type)             static __declspec(naked) type WINSTL_ATOMIC_FNS_CALLCONV
#  endif /* STSLSOFT_ASM_IN_INLINE_SUPPORTED */
# else /* ? STSLSOFT_INLINE_ASM_SUPPORTED */
  /* ASM not supported, so we're using the Win32 functions */
#  if defined(__cplusplus)
#   define WINSTL_ATOMIC_FNS_DECL_(type)             inline type WINSTL_ATOMIC_FNS_CALLCONV
#   define WINSTL_ATOMIC_FNS_IMPL_(type)             inline type WINSTL_ATOMIC_FNS_CALLCONV
#  else /* ? __cplusplus */
#   define WINSTL_ATOMIC_FNS_DECL_(type)             STLSOFT_INLINE type WINSTL_ATOMIC_FNS_CALLCONV
#   define WINSTL_ATOMIC_FNS_IMPL_(type)             STLSOFT_INLINE type WINSTL_ATOMIC_FNS_CALLCONV
#  endif /* __cplusplus */
# endif /* STSLSOFT_INLINE_ASM_SUPPORTED */
#endif /* declaration / definition */

/* /////////////////////////////////////////////////////////////////////////
 * Atomic function declarations
 */

#ifndef WINSTL_ATOMIC_FNS_DEFINITION

/* Uni-processor variants */
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_preincrement_up(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_predecrement_up(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postincrement_up(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postdecrement_up(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(void) atomic_increment_up(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(void) atomic_decrement_up(atomic_int_t volatile* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_write_up(atomic_int_t volatile* pl, atomic_int_t n);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_read_up(atomic_int_t volatile const* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postadd_up(atomic_int_t volatile* pl, atomic_int_t n);
STLSOFT_INLINE atomic_int_t atomic_preadd_up(atomic_int_t volatile* pl, atomic_int_t n);



/* SMP variants */
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_preincrement_smp(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_predecrement_smp(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postincrement_smp(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postdecrement_smp(atomic_int_t volatile* pl);
STLSOFT_INLINE void atomic_increment_smp(atomic_int_t volatile* pl);
STLSOFT_INLINE void atomic_decrement_smp(atomic_int_t volatile* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_write_smp(atomic_int_t volatile* pl, atomic_int_t n);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_read_smp(atomic_int_t volatile const* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postadd_smp(atomic_int_t volatile* pl, atomic_int_t n);
STLSOFT_INLINE atomic_int_t atomic_preadd_smp(atomic_int_t volatile* pl, atomic_int_t n);



/* Multi-processor detection variants */
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_preincrement(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_predecrement(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postincrement(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postdecrement(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(void) atomic_increment(atomic_int_t volatile* pl);
WINSTL_ATOMIC_FNS_DECL_(void) atomic_decrement(atomic_int_t volatile* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_write(atomic_int_t volatile* pl, atomic_int_t n);
WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_read(atomic_int_t volatile const* pl);

WINSTL_ATOMIC_FNS_DECL_(atomic_int_t) atomic_postadd(atomic_int_t volatile* pl, atomic_int_t n);
STLSOFT_INLINE atomic_int_t atomic_preadd(atomic_int_t volatile* pl, atomic_int_t n);


#endif /* !WINSTL_ATOMIC_FNS_DEFINITION */

/* /////////////////////////////////////////////////////////////////////////
 * Atomic function definitions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# if !defined(WINSTL_ATOMIC_FNS_DECLARATION_ONLY)

#  ifdef STSLSOFT_INLINE_ASM_SUPPORTED
/* Inline assembler versions */

#ifdef STLSOFT_COMPILER_IS_BORLAND
# pragma warn -8002     /* Suppresses: "Restarting compile using assembly" */
# pragma warn -8070     /* Suppresses: "Function should return a value" */
#endif /* compiler */

/* Uni-processor */

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        xadd dword ptr [ecx], eax

        /* Since this is pre-increment, we need to inc eax to catch up with the
         * real value
         */
        inc eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        xadd dword ptr [ecx], eax

        /* Since this is pre-decrement, we need to inc eax to catch up with the
         * real value
         */
        dec eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        xadd dword ptr [ecx], eax

        /* Since this is post-increment, we need do nothing, since the previous
         * value is in eax
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        xadd dword ptr [ecx], eax

        /* Since this is post-decrement, we need do nothing, since the previous
         * value is in eax
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_increment_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        add dword ptr [ecx], 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_decrement_up(atomic_int_t volatile* /* pl */)
{
    _asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        sub dword ptr [ecx], 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read_up(atomic_int_t volatile const* /* pl */)
{
    _asm
    {
        mov eax, 0
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */

#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */
        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        /* pop 0 into eax, which can then be atomically added into *pl (held
         * in ecx), leaving the value unchanged.
         */
        xadd dword ptr [ecx], eax

        /* Since it's an xadd it exchanges the previous value into eax, which
         * is exactly what's required
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write_up(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    _asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl, edx is n */

        /* Just exchange *pl and n */
        xchg dword ptr [ecx], edx

        /* The previous value goes into edx, so me move it into eax for return */
        mov eax, edx

        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */
        mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
        mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

        xchg dword ptr [ecx], eax

        ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
    }
}


/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd_up(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    /* Thanks to Eugene Gershnik for the fast-call implementation */
    __asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl, edx is n */

        /* Simply atomically add them, which will leave the previous value
         * in edx
         */
        xadd dword ptr [ecx], edx

        /* Just need to move adx into eax to return it */
        mov eax, edx

        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */

        /* Simply atomically add them, which will leave the previous value
         * in edx
         */
        mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
        mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

        xadd dword ptr [ecx], eax

        /* Just need to move adx into eax to return it */

        ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
    }
}

/* Symmetric multi-processor */

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement_smp(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        lock xadd dword ptr [ecx], eax

        /* Since this is pre-increment, we need to inc eax to catch up with the
         * real value
         */
        inc eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement_smp(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        lock xadd dword ptr [ecx], eax

        /* Since this is pre-decrement, we need to inc eax to catch up with the
         * real value
         */
        dec eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement_smp(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        lock xadd dword ptr [ecx], eax

        /* Since this is post-increment, we need do nothing, since the previous
         * value is in eax
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement_smp(atomic_int_t volatile* /* pl */)
{
    _asm
    {
        /* pop 1 into eax, which can then be atomically added into *pl (held
         * in ecx). Since it's an xadd it exchanges the previous value into eax
         */
        mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */

        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        lock xadd dword ptr [ecx], eax

        /* Since this is post-decrement, we need do nothing, since the previous
         * value is in eax
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read_smp(atomic_int_t volatile const* /* pl */)
{
    _asm
    {
        mov eax, 0
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl */

#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack */
        mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

        /* pop 0 into eax, which can then be atomically added into *pl (held
         * in ecx), leaving the value unchanged.
         */
        lock xadd dword ptr [ecx], eax

        /* Since it's an xadd it exchanges the previous value into eax, which
         * is exactly what's required
         */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        ret 4
#endif /* call-conv */
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write_smp(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    _asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl, edx is n */

        /* Just exchange *pl and n */
        /* lock */ xchg dword ptr [ecx], edx

        /* The previous value goes into edx, so me move it into eax for return */
        mov eax, edx

        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */
        mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
        mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

        /* lock */ xchg dword ptr [ecx], eax

        ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
    }
}


/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd_smp(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    /* Thanks to Eugene Gershnik for the fast-call implementation */
    __asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl, edx is n */

        /* Simply atomically add them, which will leave the previous value
         * in edx
         */
        lock xadd dword ptr [ecx], edx

        /* Just need to move adx into eax to return it */
        mov eax, edx

        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */

        /* Simply atomically add them, which will leave the previous value
         * in edx
         */
        mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
        mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

        lock xadd dword ptr [ecx], eax

        /* Just need to move adx into eax to return it */

        ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
    }
}

/* Processor detection */

namespace
{
    inline ws_bool_t is_host_up()
    {
        /* All these statics are guaranteed to be zero as a result of the module/process loading */
        static atomic_int_t  s_spin; /* The spin variable */
        static ws_bool_t    s_init; /* This is guaranteed to be zero */
        static ws_bool_t    s_up;   /* This is the flag variably, also guaranteed to be zero */

        /* Simple spin lock */
        if(!s_init) /* Low cost pre-test. In the unlikely event that another thread does come in and */
        {           /* also see this as false, the dual initialisation of all three statics is benign */
            spin_mutex  smx(&s_spin);

            smx.lock();
            if(!s_init)
            {
                SYSTEM_INFO sys_info;

                ::GetSystemInfo(&sys_info);

                s_init = true;

                s_up = 1 == sys_info.dwNumberOfProcessors;
            }
            smx.unlock();
        }

        return s_up;
    }

    /* s_up is guaranteed to be zero at load time.
     *
     * There is a race condition with all static variables, since multiple threads
     * can come in and one can have set the hidden flag variable without prior to
     * setting the static variable itself, just at the time that an arbitrary number
     * of other threads pick up the pre-initialised value.
     *
     * However, because the test here is whether to skip the lock, the pathological
     * case is benign. The only cost in the very rare case where it happens is that
     * the thread(s) will use bus locking until such time as the static is fully
     * initialised.
     */
    static ws_bool_t    s_up = is_host_up();
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            xadd dword ptr [ecx], eax

            /* Since this is pre-increment, we need to inc eax to catch up with the
             * real value
             */
            inc eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            lock xadd dword ptr [ecx], eax

            /* Since this is pre-increment, we need to inc eax to catch up with the
             * real value
             */
            inc eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            xadd dword ptr [ecx], eax

            /* Since this is pre-decrement, we need to inc eax to catch up with the
             * real value
             */
            dec eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            lock xadd dword ptr [ecx], eax

            /* Since this is pre-decrement, we need to inc eax to catch up with the
             * real value
             */
            dec eax

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            xadd dword ptr [ecx], eax

            /* Since this is post-increment, we need do nothing, since the previous
             * value is in eax
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            lock xadd dword ptr [ecx], eax

            /* Since this is post-increment, we need do nothing, since the previous
             * value is in eax
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            xadd dword ptr [ecx], eax

            /* Since this is post-decrement, we need do nothing, since the previous
             * value is in eax
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
            /* pop 1 into eax, which can then be atomically added into *pl (held
             * in ecx). Since it's an xadd it exchanges the previous value into eax
             */
            mov eax, -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            lock xadd dword ptr [ecx], eax

            /* Since this is post-decrement, we need do nothing, since the previous
             * value is in eax
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_increment(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            add dword ptr [ecx], 1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            /* The IA-32 Intel Architecture Software Developer's Manual, volume 2
             * states that a LOCK can be prefixed to ADD, but CodePlay VectorC
             * has a problem with it.
             */
#if defined(STLSOFT_COMPILER_IS_VECTORC)
            mov eax, 1
            lock xadd dword ptr [ecx], eax
#else /* ? compiler */
            lock add dword ptr [ecx], 1
#endif /* compiler */


#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_decrement(atomic_int_t volatile* /* pl */)
{
    if(s_up)
    {
        _asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            add dword ptr [ecx], -1

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */

            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

#if defined(STLSOFT_COMPILER_IS_VECTORC)
            mov eax, -1
            lock xadd dword ptr [ecx], eax
#else /* ? compiler */
            /* This might be wrong */
            lock sub dword ptr [ecx], 1
#endif /* compiler */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read(atomic_int_t volatile const* /* pl */)
{
    if(s_up)
    {
        _asm
        {
            mov eax, 0
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */

#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */
            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            /* pop 0 into eax, which can then be atomically added into *pl (held
             * in ecx), leaving the value unchanged.
             */
            xadd dword ptr [ecx], eax

            /* Since it's an xadd it exchanges the previous value into eax, which
             * is exactly what's required
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
    else
    {
        _asm
        {
            mov eax, 0
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl */

#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack */
            mov ecx, dword ptr [esp + 4]
#else
# error Need to define calling convention
#endif /* call-conv */

            /* pop 0 into eax, which can then be atomically added into *pl (held
             * in ecx), leaving the value unchanged.
             */
            lock xadd dword ptr [ecx], eax

            /* Since it's an xadd it exchanges the previous value into eax, which
             * is exactly what's required
             */

#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            ret 4
#endif /* call-conv */
        }
    }
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    _asm
    {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
        /* __fastcall: ecx is pl, edx is n */

        /* Just exchange *pl and n */
        lock xchg dword ptr [ecx], edx

        /* The previous value goes into edx, so me move it into eax for return */
        mov eax, edx

        ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
        /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */
        mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
        mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

        xchg dword ptr [ecx], eax

        ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
    }
}


/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd(atomic_int_t volatile* /* pl */, atomic_int_t /* n */)
{
    /* Thanks to Eugene Gershnik for the fast-call implementation */
    if(s_up)
    {
        __asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl, edx is n */

            /* Simply atomically add them, which will leave the previous value
             * in edx
             */
            xadd dword ptr [ecx], edx

            /* Just need to move adx into eax to return it */
            mov eax, edx

            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */

            /* Simply atomically add them, which will leave the previous value
             * in edx
             */
            mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
            mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

            xadd dword ptr [ecx], eax

            /* Just need to move adx into eax to return it */

            ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
        }
    }
    else
    {
        __asm
        {
#if defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_FASTCALL)
            /* __fastcall: ecx is pl, edx is n */

            /* Simply atomically add them, which will leave the previous value
             * in edx
             */
            lock xadd dword ptr [ecx], edx

            /* Just need to move adx into eax to return it */
            mov eax, edx

            ret
#elif defined(WINSTL_ATOMIC_FNS_CALLCONV_IS_STDCALL)
            /* __stdcall: arguments are on the stack: pl in esp+4, pl in esp+8 */

            /* Simply atomically add them, which will leave the previous value
             * in edx
             */
            mov ecx, dword ptr [esp + 4]    /* Load the address of pl into ecx */
            mov eax, dword ptr [esp + 8]    /* Load the value into eax, so the return value will be there waiting */

            lock xadd dword ptr [ecx], eax

            /* Just need to move adx into eax to return it */

            ret 8
#else
# error Need to define calling convention
#endif /* call-conv */
        }
    }
}

#ifdef STLSOFT_COMPILER_IS_BORLAND
# pragma warn .8070     /* Suppresses: "Function should return a value" */
# pragma warn .8002     /* Suppresses: "Restarting compile using assembly" */
#endif /* compiler */

#  else /* STSLSOFT_INLINE_ASM_SUPPORTED */

/* Non-assembler versions
 *
 * These use the Win32 Interlocked functions. These are not guaranteed to give
 * precise answers on Windows 95.
 */


/* Multi-processor detection variants */
/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement(atomic_int_t volatile* pl)
{
#if defined(WINSTL_OS_IS_WIN32)
    return STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    return STLSOFT_NS_GLOBAL(InterlockedDecrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement(atomic_int_t volatile* pl)
{
#if defined(WINSTL_OS_IS_WIN32)
    return STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    return STLSOFT_NS_GLOBAL(InterlockedDecrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement(atomic_int_t volatile* pl)
{
    atomic_int_t pre = *pl;

#if defined(WINSTL_OS_IS_WIN32)
    STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    STLSOFT_NS_GLOBAL(InterlockedIncrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */

    return pre;
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement(atomic_int_t volatile* pl)
{
    atomic_int_t pre = *pl;

#if defined(WINSTL_OS_IS_WIN32)
    STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    STLSOFT_NS_GLOBAL(InterlockedDecrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */

    return pre;
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_increment(atomic_int_t volatile* pl)
{
#if defined(WINSTL_OS_IS_WIN32)
    STLSOFT_NS_GLOBAL(InterlockedIncrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    STLSOFT_NS_GLOBAL(InterlockedIncrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_decrement(atomic_int_t volatile* pl)
{
#if defined(WINSTL_OS_IS_WIN32)
    STLSOFT_NS_GLOBAL(InterlockedDecrement)((LPLONG)pl);
#elif defined(WINSTL_OS_IS_WIN64)
    STLSOFT_NS_GLOBAL(InterlockedDecrement64)((LONGLONG*)pl);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write(atomic_int_t volatile* pl, atomic_int_t n)
{
#if defined(WINSTL_OS_IS_WIN32)
    return STLSOFT_NS_GLOBAL(InterlockedExchange)((LPLONG)pl, n);
#elif defined(WINSTL_OS_IS_WIN64)
    return STLSOFT_NS_GLOBAL(InterlockedExchange64)((LONGLONG*)pl, n);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read(atomic_int_t volatile const* pl)
{
    return *pl;
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd(atomic_int_t volatile* pl, atomic_int_t n)
{
#if defined(WINSTL_OS_IS_WIN32)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd)((LPLONG)pl, n);
#elif defined(WINSTL_OS_IS_WIN64)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd64)((LONGLONG*)pl, n);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}


/* Uni-processor variants */

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement_up(atomic_int_t volatile* pl)
{
    return atomic_preincrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement_up(atomic_int_t volatile* pl)
{
    return atomic_predecrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement_up(atomic_int_t volatile* pl)
{
    return atomic_postincrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement_up(atomic_int_t volatile* pl)
{
    return atomic_postdecrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_increment_up(atomic_int_t volatile* pl)
{
    atomic_increment(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(void) atomic_decrement_up(atomic_int_t volatile* pl)
{
    atomic_decrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write_up(atomic_int_t volatile* pl, atomic_int_t n)
{
    return atomic_write(pl, n);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read_up(atomic_int_t volatile const* pl)
{
    return *pl;
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd_up(atomic_int_t volatile* pl, atomic_int_t n)
{
#if defined(WINSTL_OS_IS_WIN32)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd)((LPLONG)pl, n);
#elif defined(WINSTL_OS_IS_WIN64)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd64)((LONGLONG*)pl, n);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

/* SMP variants */

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_preincrement_smp(atomic_int_t volatile* pl)
{
    return atomic_preincrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_predecrement_smp(atomic_int_t volatile* pl)
{
    return atomic_predecrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postincrement_smp(atomic_int_t volatile* pl)
{
    return atomic_postincrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postdecrement_smp(atomic_int_t volatile* pl)
{
    return atomic_postdecrement(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_write_smp(atomic_int_t volatile* pl, atomic_int_t n)
{
    return atomic_write(pl, n);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_read_smp(atomic_int_t volatile const* pl)
{
    return *pl;
}

/** \brief
 *
 * \ingroup group__library__synch
 */
WINSTL_ATOMIC_FNS_IMPL_(atomic_int_t) atomic_postadd_smp(atomic_int_t volatile* pl, atomic_int_t n)
{
#if defined(WINSTL_OS_IS_WIN32)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd)((LPLONG)pl, n);
#elif defined(WINSTL_OS_IS_WIN64)
    return (atomic_int_t)STLSOFT_NS_GLOBAL(InterlockedExchangeAdd64)((LONGLONG*)pl, n);
#else /* ? arch */
# error Not valid for processors other than Intel
#endif /* Win32 || Win64 */
}

#  endif /* STSLSOFT_INLINE_ASM_SUPPORTED */

# endif /* !WINSTL_ATOMIC_FNS_DECLARATION_ONLY */

/* /////////////////////////////////////////////////////////////////////////
 * Other inline atomic function
 */

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_preadd_up(atomic_int_t volatile* pl, atomic_int_t n)
{
    return n + atomic_postadd_up(pl, n);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE void atomic_increment_smp(atomic_int_t volatile* pl)
{
    atomic_postincrement_smp(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE void atomic_decrement_smp(atomic_int_t volatile* pl)
{
    atomic_postdecrement_smp(pl);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_preadd_smp(atomic_int_t volatile* pl, atomic_int_t n)
{
    return n + atomic_postadd_smp(pl, n);
}

/** \brief
 *
 * \ingroup group__library__synch
 */
STLSOFT_INLINE atomic_int_t atomic_preadd(atomic_int_t volatile* pl, atomic_int_t n)
{
    return n + atomic_postadd(pl, n);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/atomic_functions_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace winstl */
# else
} /* namespace winstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_SYNCH_H_ATOMIC_FUNCTIONS */

/* ///////////////////////////// end of file //////////////////////////// */
