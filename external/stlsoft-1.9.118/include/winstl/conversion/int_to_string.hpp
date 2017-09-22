/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/conversion/int_to_string.hpp
 *
 * Purpose:     WinSTL integer to string conversions.
 *
 * Created:     31st July 2002
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


/** \file winstl/conversion/int_to_string.hpp
 *
 * \brief [C++ only] Very efficient integer to string conversion functions
 *   for the Windows platform
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef WINSTL_INCL_WINSTL_CONVERSION_HPP_INT_TO_STRING
#define WINSTL_INCL_WINSTL_CONVERSION_HPP_INT_TO_STRING

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _WINSTL_VER_WINSTL_CONVERSION_HPP_INT_TO_STRING_MAJOR      2
# define _WINSTL_VER_WINSTL_CONVERSION_HPP_INT_TO_STRING_MINOR      1
# define _WINSTL_VER_WINSTL_CONVERSION_HPP_INT_TO_STRING_REVISION   4
# define _WINSTL_VER_WINSTL_CONVERSION_HPP_INT_TO_STRING_EDIT       41
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_BORLAND:  __BORLANDC__<0x0560
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_INT_TO_STRING
# include <stlsoft/conversion/integer_to_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_INT_TO_STRING */
#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_THREAD_MUTEX
# include <winstl/synch/thread_mutex.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_THREAD_MUTEX */
#ifndef WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX
# include <winstl/synch/spin_mutex.hpp>
#endif /* !WINSTL_INCL_WINSTL_SYNCH_HPP_SPIN_MUTEX */
#ifndef STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE
# include <stlsoft/synch/lock_scope.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SYNCH_HPP_LOCK_SCOPE */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */

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
 * Pre-processor options
 */

#if defined(_WINSTL_INT_TO_STRING_USE_DECLSPECTHREAD_FOR_EXES)
# if defined(_DLL) || \
     defined(__DLL__) || \
     defined(_WINDLL) || \
     defined(_USRDLL) || \
     defined(_AFXDLL)
#  pragma message("Using _WINSTL_INT_TO_STRING_USE_DECLSPECTHREAD_FOR_EXES when building DLLs will result in their not being loadable dynamically (via LoadLibrary())")
# endif /* dll */
#endif /* _WINSTL_INT_TO_STRING_USE_DECLSPECTHREAD_FOR_EXES */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
namespace int_to_string_tls
{
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

// NOTE: This class had to be moved out of line to prevent VC++
// from emitting multiple definitions. Silly billy!

struct thread_mx_
    : public thread_mutex
{
public:
    thread_mx_()
    {}

    void* operator new(ws_size_t , void* p)
    {
        return p;
    }

#if !defined(STLSOFT_COMPILER_IS_BORLAND) && \
    (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
        _MSC_VER >= 1200)
    void operator delete(void* , void* )
    {}
#endif /* compiler */
    void operator delete(void*)
    {}
};


template< ss_typename_param_k C
        , ws_size_t           CCH
        >
struct Slot
{
    Slot(Slot* next)
        : next(next)
    {}
    ~Slot() stlsoft_throw_0()
    {
        delete next;
    }

    // Use the process heap because:
    //
    // 1. Don't want to worry about thread-specificity, since
    //   deallocation will occur in a different thread to allocation
    // 2. Don't want to worry about linkage to any specific CRT or
    //      other library
    // 3. Doesn't matter how fast it is
    // 4. Want it to be *highly* unlikely that allocation will fail,
    //    which is indeed pretty unheard of when using the Win32
    //    process heap.
    // 5. Want a C++-exception free solution, so use the Win32-system
    //    out-of-memory exception, and not have to worry about any
    //    linkage pains.
    void* operator new(ws_size_t cb)
    {
        return ::HeapAlloc(::GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cb);
    }
    void operator delete(void* pv)
    {
        ::HeapFree(::GetProcessHeap(), 0, pv);
    }

    C       buff[CCH];
    Slot*   next;
};

template< ss_typename_param_k C
        , ws_size_t           CCH
        >
struct Key
{
    typedef Slot<C, CCH>    Slot;

    // This is admittedly totally gross, but it works and will be portable
    // across different compilers. The reason it works is that s_index is
    // static, and therefore all its members will be 0. This facilitates
    // using interlocking and spin-locks in order to manage the lifetime
    // correctly, and control access to the instance's constructor.
    //
    // Note that no members are initialised in a constructor member
    // initialisation list (MIL). They are initialised within the
    // constructor body

    Key()
    {
        // Since multiple threads could get into here before they
        // are all eventually blocked out by the static controller
        // variable, we must guard against such multiple
        // construction. Hence, if this is the only thread currently
        // engaged in construction, we can proceed to initialisation.
        // otherwise we must spin.
        //
        // We can start off by incrementing m_ctor, since it will have
        // been initialised to 0 by the load-time initialisation of
        // all static memory

        // Wrap the constructor count in a spin-mutex, and then lock it
        spin_mutex                                                      smx(&m_ctor);
        stlsoft_ns_qual(lock_scope)<spin_mutex, spin_mutex_lock_traits> lock(smx);

        if(0 == m_init++) // The test on this variable is always guarded by m_ctor
        {
            // Initialisation.

            // The construction thread could be suspended after a
            // successful completion of the constructor, but before
            // the hidden boolean managing static creation was
            // updated. Therefore, we need to reference-count the
            // construction, which is done via another interlocked
            // count, this time on m_init.

            // The constructor has not yet been called through
            // to this point

            new (&mx()) thread_mx_();

            m_index =   ::TlsAlloc();

            // Use Win32 exception because:
            //
            // 1. Process cannot recover from this error in any
            // meaningful way
            // 2. Do not want to couple to C++ exception-handling
            // and there is no graceful way to allow this to be
            // parameterisable. (May allow a pp-discriminated
            // mechanism in next version.)
            if(TLS_OUT_OF_INDEXES == m_index)
            {
                ::RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, 0);
            }
        }
    }

    ~Key() stlsoft_throw_0()
    {
        if(0 == ::InterlockedDecrement((LPLONG)&m_init))
        {
            // Walk the slot list and free. This can be as slow as
            // you like, since performance is not important here
            delete m_top;

            // Now release the index
            ::TlsFree(m_index);

            // Need to explicitly destroy the mutex.
            mx().~thread_mutex();
        }
    }

    Slot* GetSlot()
    {
        // NOTE: This does not need to be thread-safe
        return sap_cast<Slot*>(::TlsGetValue(m_index));
    }

    Slot* AllocSlot()
    {
        Slot* next;

        { // Protect linked-list manipulation
            stlsoft_ns_qual(lock_scope)<thread_mutex, thread_mutex_lock_traits>  lock(mx());

            m_top = next = new Slot(m_top);
        }

        ::TlsSetValue(m_index, next);

        return next;
    }

// Implementation
private:
    thread_mutex &mx()
    {
        return *static_cast<thread_mutex*>(static_cast<void*>(&m__mx.bytes[0]));
    }

private:
#if 0
// In an ideal world the member layout would be as follows:
    ws_dword_t      m_index;
    Slot*           m_top;
    thread_mutex    m_mx;
#else /* ? 0 */
// But we're not in an ideal world, so it is like this
    ws_dword_t      m_index;
    Slot*           m_top;
    union
    {
        ws_byte_t   bytes[sizeof(thread_mutex)];
        long double ld;
    }               m__mx;
    ws_sint32_t     m_init; // Construction count
    ws_sint32_t     m_ctor; // Ctor entry count
#endif /* 0 */
};

#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
} /* namespace int_to_string_tls */
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */


template< ss_typename_param_k C
        , ws_size_t           CCH
        >
inline C* i2str_get_tss_buffer()
{
#if defined(_WINSTL_INT_TO_STRING_USE_DECLSPECTHREAD_FOR_EXES)
    __declspec(thread) static C s_buffer[CCH];

    return s_buffer;
#else

#ifdef STLSOFT_CF_NAMESPACE_SUPPORT
    typedef int_to_string_tls::Key<C, CCH>      Key;
    typedef int_to_string_tls::Slot<C, CCH>     Slot;
#else
    typedef Key<C, CCH>                         Key;
    typedef Slot<C, CCH>                        Slot;
#endif /* STLSOFT_CF_NAMESPACE_SUPPORT */

    static Key  s_index;
    Slot*       slot = s_index.GetSlot();

    if(NULL == slot)
    {
        slot = s_index.AllocSlot();
    }

    return slot->buff;
#endif /* dll */
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** Converts a signed 8-bit integer to a character string
 *
 * For example:
\code
signed char   v = 13;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "13"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"13"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_sint8_t value)
{
    const ws_size_t CCH     = 21; // 5 fits 8-bit + sign
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a unsigned 8-bit integer to a character string
 *
 * For example:
\code
unsigned char   v = 14;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "14"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"14"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_uint8_t value)
{
    const ws_size_t CCH     = 21; // 4 fits 8-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a signed 16-bit integer to a character string
 *
 * For example:
\code
signed char   v = 15;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "15"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"15"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_sint16_t value)
{
    const ws_size_t CCH     = 21; // 7 fits 16-bit + sign
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a unsigned 16-bit integer to a character string
 *
 * For example:
\code
unsigned char   v = 16;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "16"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"16"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_uint16_t value)
{
    const ws_size_t CCH     = 21; // 6 fits 16-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a signed 32-bit integer to a character string
 *
 * For example:
\code
signed char   v = 17;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "17"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"17"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_sint32_t value)
{
    const ws_size_t CCH     = 21; // 12 fits 32-bit + sign
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a unsigned 32-bit integer to a character string
 *
 * For example:
\code
unsigned char   v = 18;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "18"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"18"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_uint32_t value)
{
    const ws_size_t CCH     = 21; // 11 fits 32-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a signed 64-bit integer to a character string
 *
 * For example:
\code
signed char   v = 19;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "19"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"19"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_sint64_t const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit + sign
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

/** Converts a unsigned 64-bit integer to a character string
 *
 * For example:
\code
unsigned char   v = 20;

assert(0 == ::strcmp(winstl::int_to_string<char>(v), "20"));
assert(0 == ::wcscmp(winstl::int_to_string<wchar_t>(v), L"20"));
\endcode
 *
 * \ingroup group__library__conversion
 *
 * \warning This function is *not* re-entrant. You must ensure that
 *   it is only invoked once in a statement. This includes possible
 *   invocations by other functions in the same statement.
 */
template<ss_typename_param_k C>
inline C const* int_to_string(ws_uint64_t const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}


#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE

template<ss_typename_param_k C>
inline C const* int_to_string(int const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

template<ss_typename_param_k C>
inline C const* int_to_string(unsigned int const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

#endif /* !STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE

template<ss_typename_param_k C>
inline C const* int_to_string(long const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

template<ss_typename_param_k C>
inline C const* int_to_string(unsigned long const& value)
{
    const ws_size_t CCH     = 21; // fits 64-bit
    C*              buffer  = i2str_get_tss_buffer<C, CCH>();

    return stlsoft::integer_to_string(buffer, CCH, value);
}

#endif /* !STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/int_to_string_unittest_.h"
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

#endif /* !WINSTL_INCL_WINSTL_CONVERSION_HPP_INT_TO_STRING */

/* ///////////////////////////// end of file //////////////////////////// */
