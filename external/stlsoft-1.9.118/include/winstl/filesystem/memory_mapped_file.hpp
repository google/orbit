/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/memory_mapped_file.hpp (based on MMFile.h, ::SynesisWin)
 *
 * Purpose:     Memory mapped file class.
 *
 * Created:     15th December 1996
 * Updated:     5th June 2011
 *
 * Thanks:      To Pablo Aguilar for requesting multibyte / wide string
 *              ambivalence. To Joe Mariadassou for requesting swap().
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1996-2011, Matthew Wilson and Synesis Software
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


/** \file winstl/filesystem/memory_mapped_file.hpp
 *
 * \brief [C++ only] Definition of the winstl::memory_mapped_file class
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_MAJOR     4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_MINOR     11
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_REVISION  5
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_EDIT      105
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_CF_64BIT_INT_SUPPORT
# error This component no longer supports compilers that do not have native 64-bit integer types
#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
#  include <winstl/error/exceptions.hpp>
# endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP
# include <stlsoft/util/std_swap.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_STD_SWAP */

#ifndef STLSOFT_INCL_H_STRING
# define STLSOFT_INCL_H_STRING
# include <string.h>    // for memcmp()
#endif /* !STLSOFT_INCL_H_STRING */

#ifdef STLSOFT_UNITTEST
# include <winstl/filesystem/file_path_buffer.hpp>
#endif /* STLSOFT_UNITTEST */

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
 * Classes
 */

/** Facade over the Win32 memory mapped file API.
 *
 * \ingroup group__library__filesystem
 */
class memory_mapped_file
{
/// \name Member Types
/// @{
public:
    /// This type
    typedef memory_mapped_file              class_type;
    /// The size type
    ///
    /// This is an unsigned type that is capable of representing any
    /// address on the operating system. On 64-bit systems, it will
    /// be <code>uint64_t</code>; on 32-bit operating systems it will
    /// be <code>uint32_t</code>.
    typedef ws_uintptr_t                    size_type;
    /// The status code type
    typedef ws_dword_t                      status_code_type;
    /// The error type
    ///
    /// \deprecated Instead use \c status_code_type
    typedef ws_dword_t                      error_type;
    /// The offset type
    typedef ws_uint64_t                     offset_type;
    /// The boolean type
    typedef ws_bool_t                       bool_type;
/// @}

/// \name Implementation
/// @{
private:
#if (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1200) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
    static void CloseHandle(HANDLE h)
    {
        ::CloseHandle(h);
    }
#endif /* compiler */

    void open_(
        ws_char_a_t const*  fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
    {
        scoped_handle<HANDLE>   hfile(  ::CreateFileA(  fileName
                                                    ,   GENERIC_READ
                                                    ,   FILE_SHARE_READ
                                                    ,   NULL
                                                    ,   OPEN_EXISTING
                                                    ,   FILE_FLAG_RANDOM_ACCESS
                                                    ,   NULL)
                            ,   CloseHandle
                            ,   INVALID_HANDLE_VALUE);


        open_helper_(hfile.get(), offset, requestSize);
    }

    void open_(
        ws_char_w_t const*  fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
    {
        scoped_handle<HANDLE>   hfile(  ::CreateFileW(  fileName
                                                    ,   GENERIC_READ
                                                    ,   FILE_SHARE_READ
                                                    ,   NULL
                                                    ,   OPEN_EXISTING
                                                    ,   FILE_FLAG_RANDOM_ACCESS
                                                    ,   NULL)
                            ,   CloseHandle
                            ,   INVALID_HANDLE_VALUE);


        open_helper_(hfile.get(), offset, requestSize);
    }

    void open_helper_(
        HANDLE      hFile
    ,   offset_type offset
    ,   size_type   requestSize
    )
    {
        if(INVALID_HANDLE_VALUE == hFile)
        {
            on_failure_("Failed to open file for mapping");

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
            return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            DWORD   fileSizeHigh;
            DWORD   fileSizeLow =   ::GetFileSize(hFile, &fileSizeHigh);
            DWORD   scode       =   ::GetLastError();

            if( INVALID_FILE_SIZE == fileSizeLow &&
                ERROR_SUCCESS != scode)
            {
                on_failure_("Failed to determine mapped file size", scode);

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
            }
            else
            {
                ws_uint64_t fileSize    =   (stlsoft_static_cast(ws_uint64_t, fileSizeHigh) << 32) | fileSizeLow;
                ws_uint64_t mapSize     =   offset + requestSize;

                if(mapSize < offset) // Overflow?
                {
                    on_failure_("Requested region exceeds the available address space", ERROR_INVALID_PARAMETER);

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                    return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
                }

                if(offset > fileSize)
                {
                    if(0 == requestSize)
                    {
                        on_failure_("Region out of range", ERROR_INVALID_PARAMETER);

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                        return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
                    }
                    else
                    {
                        // Do nothing, because MapViewOfFile() will fail for us
                    }
                }
                else if(0 == requestSize)
                {
#ifdef WINSTL_OS_IS_WIN64

                    // all 64-bit

                    requestSize = fileSize - offset;

#else /* ? WINSTL_OS_IS_WIN64 */

                    // offset is 64-bit; file-size is 64-bit; request size is 32-bit

                    ws_uint64_t requestSize2 = fileSize - offset;

                    if(requestSize2 > stlsoft_static_cast(ws_uint64_t, 0xffffffff))
                    {
                        on_failure_("Region size too large", ERROR_NOT_ENOUGH_MEMORY);

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                        return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
                    }

                    requestSize =   stlsoft_static_cast(ws_uint32_t, requestSize2);
#endif /* WINSTL_OS_IS_WIN64 */
                    mapSize     =   offset + requestSize;
                }
                else
                {
#ifndef WINSTL_MMF_DONT_TRIM_REQUEST_SIZE
                    // Work out how large the file mapping object has to be
                    // to cater to all the requested region. Also check if
                    // have asked for too much, and reduce request if
                    // necessary.
                    //
                    // NOTE: This request trimming is only appropriate
                    // because we are providing only a read-only view. Even
                    // so, it is possible that the map will cause the file
                    // to increase in size. But that's caveat emptor for
                    // the user of MMFs, and nothing per se to do with this
                    // component.

                    if(mapSize > fileSize)
                    {
                        WINSTL_ASSERT((mapSize - fileSize) <= stlsoft_static_cast(ws_uint64_t, 0xffffffff));
                        WINSTL_ASSERT(offset <= fileSize);

                        requestSize -= stlsoft_static_cast(size_type, (mapSize - fileSize));
                        mapSize = 0;
                    }
#endif /* !WINSTL_MMF_DONT_TRIM_REQUEST_SIZE */
                }

                if(0 == requestSize)
                {
                    // Windows CreateFileMapping() does not support mapping
                    // zero-length files, so we catch this condition here
                    m_memory    =   NULL;
                    m_cb        =   0;
                }
                else
                {
                    DWORD   mapSizeHi   =   stlsoft_static_cast(DWORD, mapSize >> 32);
                    DWORD   mapSizeLo   =   stlsoft_static_cast(DWORD, mapSize);

                    HANDLE hMap = create_map_(
                                        hFile
                                    ,   PAGE_READONLY
                                    ,   mapSizeHi
                                    ,   mapSizeLo
                                    );

                    if(NULL == hMap)
                    {
                        on_failure_("Failed to open file mapping");

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                        return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
                    }
                    else
                    {
                        scoped_handle<HANDLE> scoper2(hMap, ::CloseHandle, NULL);

                        void* memory = create_view_(
                                            hMap
                                        ,   FILE_MAP_READ
                                        ,   offset
                                        ,   requestSize
                                        );

                        if(NULL == memory)
                        {
                            // The following block of code attempts to provide
                            // improved precision in status code, by using
                            // ERROR_NOT_ENOUGH_MEMORY in the case where a too-large
                            // view size is requested, instead of the usual
                            // ERROR_INVALID_PARAMETER.

                            status_code_type scode2 = ::GetLastError();

#ifdef WINSTL_MEMORY_MAPPED_FILE_TRANSLATE_SC_EINVAL_2_EMEM

                            if(ERROR_INVALID_PARAMETER == scode2)
                            {
                                SYSTEM_INFO si;

                                ::GetSystemInfo(&si);

                                if(0 == (offset % si.dwAllocationGranularity))
                                {
# ifndef WINSTL_OS_IS_WIN64
                                    if(requestSize >= 0x7ffe0000)
# endif /* !WINSTL_OS_IS_WIN64 */
                                    {
                                        scode2 = ERROR_NOT_ENOUGH_MEMORY;
                                    }
                                }
                            }

                            on_failure_("Failed to map view of file", scode2);

#else /* ? WINSTL_MEMORY_MAPPED_FILE_TRANSLATE_SC_EINVAL_2_EMEM */

                            STLSOFT_SUPPRESS_UNUSED(scode2);

                            on_failure_("Failed to map view of file");

#endif /* WINSTL_MEMORY_MAPPED_FILE_TRANSLATE_SC_EINVAL_2_EMEM */

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
                            return;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
                        }
                        else
                        {
                            m_memory    =   memory;
                            m_cb        =   requestSize;
                        }
                    }
                }
            }
        }
    }
/// @}

/// \name Construction
/// @{
public:
    /// Maps an entire file into memory
    ///
    /// \param fileName The name of the file to map into memory
    ///
    /// \exception winstl::windows_exception Thrown if the map cannot be
    ///   created. May be any value returned by the Windows API. Known
    ///   values include:
    ///   - ERROR_NOT_ENOUGH_MEMORY, when the map size is too large to fit
    ///     into memory
    ///   - ERROR_INVALID_PARAMETER, when the allocated size is too large to
    ///     be valid
    ///   - ERROR_MAPPED_ALIGNMENT, when the offset is not a multiple of the
    //      system allocation granularity
    ss_explicit_k memory_mapped_file(ws_char_a_t const* fileName)
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(fileName, 0, 0);
    }
    /// Maps an entire file into memory
    ss_explicit_k memory_mapped_file(ws_char_w_t const* fileName)
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(fileName, 0, 0);
    }
    /// Maps an entire file into memory
    template <ss_typename_param_k S>
    ss_explicit_k memory_mapped_file(S const& fileName)
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(stlsoft_ns_qual(c_str_ptr)(fileName), 0, 0);
    }

    /// Maps a portion of a file into memory
    ///
    /// \param fileName The name of the file to map into memory
    /// \param offset The offset into the file where the mapping
    ///   begins. Must be a multiple of the system allocation
    ///   granularity
    /// \param requestSize The size of the portion of the file
    ///   to map into memory. If 0, all (of the remaining portion)
    ///   of the file is loaded
    memory_mapped_file(
        ws_char_a_t const*  fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(fileName, offset, requestSize);
    }
    /// Maps a portion of a file into memory
    ///
    /// \param fileName The name of the file to map into memory
    /// \param offset The offset into the file where the mapping
    ///   begins. Must be a multiple of the system allocation
    ///   granularity
    /// \param requestSize The size of the portion of the file
    ///   to map into memory. If 0, all (of the remaining portion)
    ///   of the file is loaded
    memory_mapped_file(
        ws_char_w_t const*  fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(fileName, offset, requestSize);
    }
    /// Maps a portion of a file into memory
    ///
    /// \param fileName The name of the file to map into memory
    /// \param offset The offset into the file where the mapping
    ///   begins. Must be a multiple of the system allocation
    ///   granularity
    /// \param requestSize The size of the portion of the file
    ///   to map into memory. If 0, all (of the remaining portion)
    ///   of the file is loaded
    template <ss_typename_param_k S>
    memory_mapped_file(
        S const&    fileName
    ,   offset_type offset
    ,   size_type   requestSize
    )
        : m_cb(0)
        , m_memory(NULL)
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        , m_lastStatusCode(ERROR_SUCCESS)
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
    {
        open_(stlsoft_ns_qual(c_str_ptr)(fileName), offset, requestSize);
    }

    /// Closes the view on the mapped file
    ~memory_mapped_file() stlsoft_throw_0()
    {
        WINSTL_ASSERT(is_valid());

        if(NULL != m_memory)
        {
            ::UnmapViewOfFile(m_memory);
        }
    }

    /// Swaps the state of this instance with another
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        WINSTL_ASSERT(is_valid());

        std_swap(m_cb, rhs.m_cb);
        std_swap(m_memory, rhs.m_memory);
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        std_swap(m_lastStatusCode, rhs.m_lastStatusCode);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

        WINSTL_ASSERT(is_valid());
    }
/// @}

/// \name Accessors
/// @{
public:
    /// Non-mutating (const) pointer to the start of the mapped region
    void const* memory() const
    {
        return m_memory;
    }
    /// The number of bytes in the mapped region
    size_type size() const
    {
        return m_cb;
    }

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
    /// The status code associated with the last attempted operation
    status_code_type lastStatusCode() const
    {
        return m_lastStatusCode;
    }

    /// The status code associated with the last attempted operation
    ///
    /// \deprecated Instead use \c status_code_type
    status_code_type lastError() const
    {
        return m_lastStatusCode;
    }
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
/// @}

/// \name Comparison
/// @{
public:
    /** Determines whether the given instance is the same size and has
     * identical contents to the calling instance.
     *
     * \param rhs The instance against whose contents will be compared those
     *   of the calling instance.
     *
     * \retval true \c rhs is the same size and has identical contents to
     *   the calling instance.
     * \retval false \c rhs is a different size and/or has different
     *   contents to the calling instance.
     */
    bool equal(class_type const& rhs) const
    {
        class_type const& lhs = *this;

        if(lhs.size() != rhs.size())
        {
            return false;
        }
        if(0 != ::memcmp(lhs.memory(), rhs.memory(), lhs.size()))
        {
            return false;
        }
        return true;
    }
/// @}

/// \name Implementation
/// @{
private:
    HANDLE create_map_(
        HANDLE  hFile
    ,   DWORD   protection
    ,   DWORD   mapSizeHi
    ,   DWORD   mapSizeLo
    )
    {
        return ::CreateFileMappingA(
                    hFile
                ,   NULL
                ,   protection
                ,   mapSizeHi
                ,   mapSizeLo
                ,   NULL
                );
    }

    void* create_view_(
        HANDLE      hMap
    ,   DWORD       access
    ,   offset_type offset
    ,   size_type   requestSize
    )
    {
        return ::MapViewOfFile(
                    hMap
                ,   access
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
                ,   stlsoft_static_cast(DWORD, offset >> 32)
#else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
                ,   0
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
                ,   stlsoft_static_cast(DWORD, offset)
                ,   requestSize
                );
    }

    void on_failure_(
        char const*         message
    ,   status_code_type    scode = ::GetLastError()
    )
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        // The exception policy is used because VC++ 5 has a cow when it is
        // asked to throw a windows_exception, and this special case is
        // handled by windows_exception_policy
        windows_exception_policy    xp;

        xp(message, scode);
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */

        STLSOFT_SUPPRESS_UNUSED(message);

        m_lastStatusCode = scode;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    bool_type is_valid() const
    {
        if((NULL != m_memory) != (0 != m_cb))
        {
            return false;
        }
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        if((0 != m_cb) && (0 != m_lastStatusCode))
        {
          return false;
        }
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

        return true;
    }
/// @}

/// \name Members
/// @{
private:
    size_type           m_cb;
    void*               m_memory;
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
    status_code_type    m_lastStatusCode;
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */
/// @}

/// \name Not to be implemented
/// @{
private:
    memory_mapped_file(class_type const&);
    class_type& operator =(class_type const&);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Comparison operators
 */

inline bool operator ==(
    memory_mapped_file const&   lhs
,   memory_mapped_file const&   rhs
)
{
    return lhs.equal(rhs);
}

inline bool operator !=(
    memory_mapped_file const&   lhs
,   memory_mapped_file const&   rhs
)
{
    return !lhs.equal(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Swapping
 */

/** Swaps the state of two \link winstl::memory_mapped_file memory_mapped_file\endlink
 * instances.
 */
inline void swap(
    memory_mapped_file& lhs
,   memory_mapped_file& rhs
)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/memory_mapped_file_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

#ifdef STLSOFT_CF_std_NAMESPACE
namespace std
{

    inline void swap(
        winstl_ns_qual(memory_mapped_file)& lhs
    ,   winstl_ns_qual(memory_mapped_file)& rhs
    )
    {
        lhs.swap(rhs);
    }

} /* namespace std */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE */

/* ///////////////////////////// end of file //////////////////////////// */
