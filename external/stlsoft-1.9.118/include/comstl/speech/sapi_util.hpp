/* /////////////////////////////////////////////////////////////////////////
 * File:        comstl/speech/sapi_util.hpp (derived from Pantheios)
 *
 * Purpose:     Definition of SAPI types and constants.
 *
 * Created:     31st August 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file comstl/speech/sapi_util.hpp
 *
 * \brief [C++ only; requires COM] Definition of SAPI types and constants.
 *   (\ref group__library__utility__com "COM Utility" Library).
 */

#ifndef COMSTL_INCL_COMSTL_SPEECH_HPP_SAPI_UTIL
#define COMSTL_INCL_COMSTL_SPEECH_HPP_SAPI_UTIL

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define COMSTL_VER_COMSTL_SPEECH_HPP_SAPI_UTIL_MAJOR       1
# define COMSTL_VER_COMSTL_SPEECH_HPP_SAPI_UTIL_MINOR       0
# define COMSTL_VER_COMSTL_SPEECH_HPP_SAPI_UTIL_REVISION    4
# define COMSTL_VER_COMSTL_SPEECH_HPP_SAPI_UTIL_EDIT        6
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_UTIL_HPP_CREATION_FUNCTIONS
# include <comstl/util/creation_functions.hpp>
#endif /* !COMSTL_INCL_UTIL_HPP_CREATION_FUNCTIONS */
#ifndef COMSTL_INCL_UTIL_HPP_INTERFACE_TRAITS
# include <comstl/util/interface_traits.hpp>
#endif /* !COMSTL_INCL_UTIL_HPP_INTERFACE_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS
# include <stlsoft/conversion/char_conversions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_CHAR_CONVERSIONS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_HPP_GET_PTR
# include <stlsoft/shims/attribute/get_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ATTRIBUTE_HPP_GET_PTR */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR
# include <stlsoft/smartptr/ref_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_REF_PTR */

#ifdef COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS
# include <sapi.h>
#endif /* COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if !defined(_COMSTL_NO_NAMESPACE) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# if defined(_STLSOFT_NO_NAMESPACE)
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
 * SAPI
 */

#ifndef COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS

/** Interface that emulates the vtable of ISpNotifySource
 */
struct ISpNotifySource_ : public IUnknown
{
    virtual void ISpNotifySource_1() = 0;
    virtual void ISpNotifySource_2() = 0;
    virtual void ISpNotifySource_3() = 0;
    virtual void ISpNotifySource_4() = 0;
    virtual void ISpNotifySource_5() = 0;
    virtual void ISpNotifySource_6() = 0;
    virtual void ISpNotifySource_7() = 0;
};

/** Interface that emulates the vtable of ISpEventSource
 */
struct ISpEventSource_ : public ISpNotifySource_
{
    virtual void ISpEventSource_1() = 0;
    virtual void ISpEventSource_2() = 0;
    virtual void ISpEventSource_3() = 0;
};

/** Interface that emulates the vtable of ISpVoice
 */
struct ISpVoice_ : public ISpEventSource_
{
    virtual void ISpVoice_1() = 0;
    virtual void ISpVoice_2() = 0;
    virtual void ISpVoice_3() = 0;
    virtual void ISpVoice_4() = 0;
    virtual void ISpVoice_5() = 0;
    virtual void ISpVoice_6() = 0;
    virtual void ISpVoice_7() = 0;

        virtual HRESULT STDMETHODCALLTYPE Speak(
            /* [string][in] */ const WCHAR *pwcs,
            /* [in] */ DWORD dwFlags,
            /* [out] */ ULONG *pulStreamNumber) = 0;

    virtual void ISpVoice_9() = 0;
    virtual void ISpVoice_10() = 0;
    virtual void ISpVoice_11() = 0;
    virtual void ISpVoice_12() = 0;
    virtual void ISpVoice_13() = 0;
    virtual void ISpVoice_14() = 0;
    virtual void ISpVoice_15() = 0;
    virtual void ISpVoice_16() = 0;
    virtual void ISpVoice_17() = 0;
    virtual void ISpVoice_18() = 0;
    virtual void ISpVoice_19() = 0;

        virtual HRESULT STDMETHODCALLTYPE WaitUntilDone(
            /* [in] */ ULONG msTimeout) = 0;

    virtual void ISpVoice_21() = 0;
    virtual void ISpVoice_22() = 0;

        virtual /* [local] */ HANDLE STDMETHODCALLTYPE SpeakCompleteEvent( void) = 0;

    virtual void ISpVoice_24() = 0;
    virtual void ISpVoice_25() = 0;
};

/** Flags that may be passed to comstl::sapi_speak()
 */
enum SPEAKFLAGS
{
    SPF_DEFAULT = 0,
    SPF_ASYNC = 1L << 0,
    SPF_PURGEBEFORESPEAK = 1L << 1,
    SPF_IS_FILENAME = 1L << 2,
    SPF_IS_XML = 1L << 3,
    SPF_IS_NOT_XML = 1L << 4,
    SPF_PERSIST_XML = 1L << 5,
    SPF_NLP_SPEAK_PUNC = 1L << 6,
    SPF_NLP_MASK = SPF_NLP_SPEAK_PUNC,
    SPF_VOICE_MASK = SPF_ASYNC | SPF_PURGEBEFORESPEAK | SPF_IS_FILENAME | SPF_IS_XML | SPF_IS_NOT_XML | SPF_NLP_MASK | SPF_PERSIST_XML,
    SPF_UNUSED_FLAGS = ~SPF_VOICE_MASK
};

# define CLSID_SpVoice  comstl_ns_qual(CLSID_SpVoice_)
# define ISpVoice       comstl_ns_qual(ISpVoice_)

namespace
{

    const CLSID CLSID_SpVoice_  =   { 0x96749377, 0x3391, 0x11D2, { 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96 } };
    const IID   IID_ISpVoice_   =   { 0x6C44DF74, 0x72B9, 0x4992, { 0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4 } };

} // anonymous namespace


COMSTL_IID_TRAITS_DEFINE__(ISpVoice_, ISpVoice_)


#endif /* !COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

namespace impl
{
    inline HRESULT sapi_speak_(ISpVoice* voice, wchar_t const* words, DWORD flags)
    {
        COMSTL_ASSERT(NULL != voice);

        ULONG   streamNumber;

        return voice->Speak(words, flags, &streamNumber);
    }
    inline HRESULT sapi_speak_(ISpVoice* voice, char const* words, DWORD flags)
    {
        return sapi_speak_(voice, stlsoft_ns_qual(a2w)(words), flags);
    }

} // namespace impl

template<   ss_typename_param_k V
        ,   ss_typename_param_k S
        >
inline HRESULT sapi_speak(V& voice, S const& words, DWORD flags)
{
    return impl::sapi_speak_(stlsoft_ns_qual(get_ptr)(voice), stlsoft_ns_qual(c_str_ptr)(words), flags);
}

inline HRESULT sapi_create(stlsoft_ns_qual(ref_ptr)<ISpVoice>& voice)
{
    return comstl::co_create_instance(CLSID_SpVoice, voice);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _COMSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} /* namespace comstl */
# else
} /* namespace comstl_project */
} /* namespace stlsoft */
# endif /* _STLSOFT_NO_NAMESPACE */

# ifndef COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS

using ::comstl::SPF_DEFAULT;
using ::comstl::SPF_ASYNC;
using ::comstl::SPF_PURGEBEFORESPEAK;
using ::comstl::SPF_IS_FILENAME;
using ::comstl::SPF_IS_XML;
using ::comstl::SPF_IS_NOT_XML;
using ::comstl::SPF_PERSIST_XML;
using ::comstl::SPF_NLP_SPEAK_PUNC;
using ::comstl::SPF_NLP_MASK;
using ::comstl::SPF_VOICE_MASK;
using ::comstl::SPF_UNUSED_FLAGS;

# endif /* !COMSTL_SPEECH_SAPI_UTIL_USE_MS_SAPI_HEADERS */

#endif /* !_COMSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* COMSTL_INCL_COMSTL_SPEECH_HPP_SAPI_UTIL */

/* ///////////////////////////// end of file //////////////////////////// */
