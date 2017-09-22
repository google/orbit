/* /////////////////////////////////////////////////////////////////////////
 * File:        unixstl/filesystem/memory_mapped_file.hpp (based on MMFile.h, ::SynesisWin)
 *
 * Purpose:     Memory mapped file class.
 *
 * Created:     15th December 1996
 * Updated:     21st January 2011
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


/** \file unixstl/filesystem/memory_mapped_file.hpp
 *
 * \brief [C++ only] Definition of the unixstl::memory_mapped_file class
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE
#define UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_MAJOR       4
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_MINOR       5
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_REVISION    1
# define UNIXSTL_VER_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE_EDIT        94
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef UNIXSTL_INCL_UNIXSTL_H_UNIXSTL
# include <unixstl/unixstl.h>
#endif /* !UNIXSTL_INCL_UNIXSTL_H_UNIXSTL */
#ifndef UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <unixstl/filesystem/filesystem_traits.hpp>
#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# ifndef UNIXSTL_INCL_UNIXSTL_HPP_ERROR_UNIX_EXCEPTIONS
#  include <unixstl/error/exceptions.hpp>
# endif /* !UNIXSTL_INCL_UNIXSTL_ERROR_HPP_UNIX_EXCEPTIONS */
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD
# include <stlsoft/shims/access/string/fwd.h>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_STRING_H_FWD */

#ifndef STLSOFT_INCL_SYS_H_MMAN
# define STLSOFT_INCL_SYS_H_MMAN
# include <sys/mman.h>
#endif /* !STLSOFT_INCL_SYS_H_MMAN */
#ifndef STLSOFT_INCL_SYS_H_STAT
# define STLSOFT_INCL_SYS_H_STAT
# include <sys/stat.h>
#endif /* !STLSOFT_INCL_SYS_H_STAT */

#ifdef STLSOFT_UNITTEST
# include <unixstl/filesystem/file_path_buffer.hpp>
#endif /* STLSOFT_UNITTEST */

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

/** \brief Facade over the UNIX memory mapped file API.
 *
 * \ingroup group__library__filesystem
 */
class memory_mapped_file
{
/// \name Member Types
/// @{
private:
    /// \brief The character type
    typedef us_char_a_t                     char_type;
    /// \brief The traits type
    typedef filesystem_traits<us_char_a_t>  traits_type;
public:
    /// \brief This type
    typedef memory_mapped_file              class_type;
    /// \brief The size type
    typedef us_size_t                       size_type;
    /// \brief The error type
    typedef int                             error_type;
    /// \brief The offset type
    typedef off_t                           offset_type;
    /// The boolean type
    typedef us_bool_t                       bool_type;
/// @}

/// \name Implementation
/// @{
private:
    void open_(
        char_type const*    fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
    {
        scoped_handle<int>  hfile(  traits_type::open(  fileName
                                                    ,   O_RDONLY
                                                    ,   PROT_READ)
                                ,   &traits_type::close
                                ,   -1);

        if(hfile.empty())
        {
            on_error_("Failed to open file for mapping");
        }
        else
        {
            struct stat st;

            if(0 != ::fstat(hfile.get(), &st))
            {
                on_error_("Failed to determine mapped file size");
            }
            else if(0 == st.st_size)
            {
                m_memory    =   NULL;
                m_cb        =   0;
            }
            else
            {
                if(0 == requestSize)
                {
                    requestSize = static_cast<size_type>(st.st_size);
                }
                else if(requestSize + offset > static_cast<size_type>(st.st_size))
                {
                    requestSize = static_cast<size_type>(st.st_size) - offset;
                }

                void* memory = ::mmap(NULL, static_cast<size_t>(requestSize), PROT_READ, MAP_PRIVATE, hfile.get(), offset);

                if(MAP_FAILED == memory)
                {
                    on_error_("Failed to map view of file");
                }
                else
                {
                    m_memory    =   memory;
                    m_cb        =   requestSize;
                }
            }
        }
    }
/// @}

/// \name Construction
/// @{
public:
    ss_explicit_k memory_mapped_file(char_type const* fileName)
        : m_cb(0)
        , m_memory(NULL)
    {
        open_(fileName, 0, 0);
    }
    template <ss_typename_param_k S>
    ss_explicit_k memory_mapped_file(S const& fileName)
        : m_cb(0)
        , m_memory(NULL)
    {
        open_(stlsoft_ns_qual(c_str_ptr)(fileName), 0, 0);
    }
    memory_mapped_file(
        char_type const*    fileName
    ,   offset_type         offset
    ,   size_type           requestSize
    )
        : m_cb(0)
        , m_memory(NULL)
    {
        open_(fileName, offset, requestSize);
    }
    template <ss_typename_param_k S>
    memory_mapped_file(
        S const&    fileName
    ,   offset_type offset
    ,   size_type   requestSize
    )
        : m_cb(0)
        , m_memory(NULL)
    {
        open_(stlsoft_ns_qual(c_str_ptr)(fileName), offset, requestSize);
    }

    /// Closes the view on the mapped file
    ~memory_mapped_file() stlsoft_throw_0()
    {
        UNIXSTL_ASSERT(is_valid());

        if(NULL != m_memory)
        {
            ::munmap(m_memory, static_cast<us_size_t>(m_cb));
        }
    }

    /// Swaps the state of this instance with another
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        UNIXSTL_ASSERT(is_valid());

        std_swap(m_cb, rhs.m_cb);
        std_swap(m_memory, rhs.m_memory);
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
        std_swap(m_lastError, rhs.m_lastError);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

        UNIXSTL_ASSERT(is_valid());
    }
/// @}

/// \name Accessors
/// @{
public:
    /// \brief Non-mutating (const) pointer to the start of the mapped
    ///  region.
    void const* memory() const
    {
        return m_memory;
    }
    /// \brief The number of bytes in the mapped region
    size_type size() const
    {
        return m_cb;
    }

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
    error_type lastError() const
    {
        return m_lastError;
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
    void on_error_(
        char const* message
    ,   error_type  error = errno
    )
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(unix_exception(message, error));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        m_lastError = error;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    bool_type is_valid() const
    {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        if((NULL != m_memory) != (0 != m_cb))
        {
            return false;
        }
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

        return true;
    }
/// @}

/// \name Members
/// @{
private:
    size_type   m_cb;
    void*       m_memory;
#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
    error_type  m_lastError;
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

/** Swaps the state of two \link unixstl::memory_mapped_file memory_mapped_file\endlink
 * instances.
 */
inline void swap(
    memory_mapped_file& lhs
,   memory_mapped_file& rhs
)
{
    lhs.swap(rhs);
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/memory_mapped_file_unittest_.h"
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

namespace std
{

    inline void swap(
        unixstl_ns_qual(memory_mapped_file)& lhs
    ,   unixstl_ns_qual(memory_mapped_file)& rhs
    )
    {
        lhs.swap(rhs);
    }

} /* namespace std */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !UNIXSTL_INCL_UNIXSTL_FILESYSTEM_HPP_MEMORY_MAPPED_FILE */

/* ///////////////////////////// end of file //////////////////////////// */
