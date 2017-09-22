/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/filesystem/file_path_buffer.hpp
 *
 * Purpose:     Contains the basic_file_path_buffer template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Created:     7th February 2002
 * Updated:     26th February 2011
 *
 * Thanks to:   Pablo Aguilar for discovering the Borland weirdness which is now
 *              addressed with the calc_path_max_() method.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2011, Matthew Wilson and Synesis Software
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


/** \file winstl/filesystem/file_path_buffer.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_file_path_buffer class
 *  template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
#define WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_MAJOR    4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_MINOR    4
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_REVISION 2
# define WINSTL_VER_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER_EDIT     118
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */

#define WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER

#if defined(STLSOFT_COMPILER_IS_MSVC)
# if _MSC_VER < 1100
#  error winstl/filesystem/file_path_buffer.hpp is not compatible with Visual C++ prior to version 5.0
# elif _MSC_VER < 1200
#  undef WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER
# else /* ? _MSC_VER */
   /* Nothing to do */
# endif /* _MSC_VER */
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifdef WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER
# ifndef STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER
#  include <stlsoft/memory/auto_buffer.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_HPP_MEMORY_AUTO_BUFFER */
#else /* ? WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
# include <vector>
#endif /* WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS
# include <stlsoft/string/copy_functions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_COPY_FUNCTIONS */

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

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k C>
struct is_ansi
{
    enum { value = 0 };
};

STLSOFT_TEMPLATE_SPECIALISATION
struct is_ansi<ws_char_a_t>
{
    enum { value = 1 };
};
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

// class basic_file_path_buffer
/** \brief Acts as a buffer with sufficient size for any drive on the host machine
 *
 * \ingroup group__library__filesystem
 *
 * This class is a non-template class primarily so that separate instantiations
 * are not created for each instantiation of the basic_file_path_buffer.
 *
 * This class provides a simple function, which is to provide the maximum path
 * length for the host. This information is then cached due to the static nature
 * of the get_drivesvar_() method, although it can be reset by calling the
 * refresh() method on the buffer class.
 *
 * \param C The character type
 * \param A The allocator type
 * \param CCH The size of the internal member path structure. On translators that support default template arguments this default to (\c 1 + \c WINSTL_CONST_MAX_PATH)
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k A = processheap_allocator<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k A /* = processheap_allocator<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_file_path_buffer
{
/// \name Member Constants
/// @{
private:
    enum
    {
        internalBufferSize = 1 + WINSTL_CONST_MAX_PATH
    };

    enum
    {
            CCH_9x = WINSTL_CONST_MAX_PATH
        ,   CCH_NT = (is_ansi<C>::value ? WINSTL_CONST_MAX_PATH : CONST_NT_MAX_PATH)
    };

    /// \brief The buffer type
#ifdef WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER
    typedef stlsoft_ns_qual(auto_buffer)<   C
# if defined(STLSOFT_COMPILER_IS_BORLAND)
    // This is necessary, since Borland will attempt an auto_buffer with what
    // seems like 0 size, or maybe it just can't define the type. Who can tell?
                                            ,   1 + WINSTL_CONST_MAX_PATH
# else /* ? compiler */
                                            ,   internalBufferSize
# endif /* compiler */
                                            ,   A
                                            >                       buffer_type;
#else /* ? WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
    typedef stlsoft_ns_qual_std(vector)<C>                          buffer_type;
#endif /* WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
/// @}

/// \name Member Types
/// @{
public:
    /// \brief The character type
    typedef C                                                       char_type;
    /// \brief The allocator type
    typedef A                                                       allocator_type;
    /// \brief The current parameterisation of the type
    typedef basic_file_path_buffer<C, A>                            class_type;
    /// \brief The value type
    typedef ss_typename_type_k buffer_type::value_type              value_type;
    /// \brief The reference type
    typedef value_type&                                             reference;
    /// \brief The non-mutating (const) reference type
    typedef value_type const&                                       const_reference;
    /// \brief The size type
    typedef ss_typename_type_k buffer_type::size_type               size_type;
/// @}

/// \name Construction
/// @{
public:
    /// \brief Default constructor
    basic_file_path_buffer()
        : m_buffer(1 + calc_path_max_())
    {
#ifdef _DEBUG
        ::memset(&m_buffer[0], '?', m_buffer.size());
        m_buffer[m_buffer.size() - 1] = '\0';
#endif /* _DEBUG */
    }
    /// \brief Copy constructor
    basic_file_path_buffer(class_type const& rhs)
        : m_buffer(rhs.size())
    {
        stlsoft_ns_qual(pod_copy_n)(data(), rhs.data(), m_buffer.size());
    }
    /// \brief Copy assignment operator
    class_type& operator =(class_type const& rhs)
    {
        m_buffer.resize(rhs.size());
        stlsoft_ns_qual(pod_copy_n)(data(), rhs.data(), m_buffer.size());

        return *this;
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Swaps the contents with those of another instance
    ///
    /// \param rhs The instance whose contents will be swapped with the
    ///  callee
    ///
    /// \note The complexity of this operation is not guaranteed
    ///  to be constant-time. See the documentation for
    ///  \link stlsoft::auto_buffer auto_buffer\endlink for
    ///  further details.
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        m_buffer.swap(rhs.m_buffer);
    }
/// @}

/// \name Accessors
/// @{
public:
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
    value_type const* data() const
    {
#ifdef WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER
        return m_buffer.data();
#else /* ? WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
        return &m_buffer[0];
#endif /* WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
    }
    value_type* data()
    {
#ifdef WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER
        return m_buffer.data();
#else /* ? WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
        return &m_buffer[0];
#endif /* WINSTL_FILE_PATH_BUFFER_USE_AUTO_BUFFER */
    }
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

    /// \brief Returns a pointer to a nul-terminated string
    value_type const* c_str() const
    {
        return this->data();
    }

    /// \brief Returns a mutable (non-const) pointer to the internal buffer
    reference operator [](ws_size_t index)
    {
        buffer_type& this_ = m_buffer;

        return this_[index];
    }
#if !defined(STLSOFT_COMPILER_IS_COMO) || \
    __COMO_VERSION__ >= 4303
    /// \brief Returns a non-mutable (const) pointer to the internal buffer
    const_reference operator [](ws_size_t index) const
    {
        WINSTL_MESSAGE_ASSERT("Index out of range", !(size() < index));

        return data()[index];
    }
#else /* ? compiler */
#endif /* compiler */

    /// \brief Returns the size of the internal buffer
    size_type size() const
    {
        return m_buffer.size();
    }

    /// \brief Returns the maximum size of the internal buffer
    static size_type max_size()
    {
        return calc_path_max_();
    }

    /// \brief Copies the contents into a caller supplied buffer
    ///
    /// \param buffer Pointer to character buffer to receive the contents.
    ///  May be NULL, in which case the method returns size().
    /// \param cchBuffer Number of characters of available space in \c buffer.
    size_type copy(char_type *buffer, size_type cchBuffer) const
    {
        return stlsoft_ns_qual(copy_contents)(buffer, cchBuffer, m_buffer.data(), m_buffer.size());
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Causes the drives to be examined again for the next instance.
    ///
    /// \deprecated
    static void refresh() stlsoft_throw_0()
    {}
/// @}

/// \name Implementation
/// @{
private:
    // Have to do it this way, as an inline, in-MIL tertiary operator causes
    // Borland C++ to think that the answer's 5! 'tis a mysterious beastie,
    // the old Borland compiler, is it not?
    static size_type calc_path_max_()
    {
        size_type   n;

        if(::GetVersion() & 0x80000000)
        {
            n = CCH_9x;
        }
        else
        {
            n = CCH_NT;
        }

        return n;
    }
/// @}

/// \name Members
/// @{
private:
    buffer_type m_buffer;
/// @}
};

/* Typedefs to commonly encountered types. */
/** \brief Specialisation of the basic_file_path_buffer template for the ANSI character type \c char
 *
 * \ingroup group__library__filesystem
 */
typedef basic_file_path_buffer<ws_char_a_t, processheap_allocator<ws_char_a_t> >    file_path_buffer_a;
/** \brief Specialisation of the basic_file_path_buffer template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__filesystem
 */
typedef basic_file_path_buffer<ws_char_w_t, processheap_allocator<ws_char_w_t> >    file_path_buffer_w;
/** \brief Specialisation of the basic_file_path_buffer template for the ambeint character type \c TCHAR
 *
 * \ingroup group__library__filesystem
 */
typedef basic_file_path_buffer<TCHAR, processheap_allocator<TCHAR> >                file_path_buffer;

/* /////////////////////////////////////////////////////////////////////////
 * Support for PlatformSTL redefinition by inheritance+namespace, for confused
 * compilers (e.g. VC++ 6)
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

    template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
            ,   ss_typename_param_k A = winstl_ns_qual(processheap_allocator)<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            ,   ss_typename_param_k A /* = processheap_allocator<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
            >
    class basic_file_path_buffer__
        : public winstl_ns_qual(basic_file_path_buffer)<C, A>
    {
    private:
        typedef winstl_ns_qual(basic_file_path_buffer)<C, A>            parent_class_type;
    public:
        typedef basic_file_path_buffer__<C, A>                          class_type;
        typedef ss_typename_type_k parent_class_type::value_type        value_type;
        typedef ss_typename_type_k parent_class_type::reference         reference;
        typedef ss_typename_type_k parent_class_type::const_reference   const_reference;
        typedef ss_typename_type_k parent_class_type::size_type         size_type;
    };

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * swapping
 */

template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline void swap(basic_file_path_buffer<C, A>& lhs, basic_file_path_buffer<C, A>& rhs)
{
    lhs.swap(rhs);
}

/* /////////////////////////////////////////////////////////////////////////
 * Shims
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k A>
inline ws_char_a_t const* c_str_data_a(winstl_ns_qual(basic_file_path_buffer)<ws_char_a_t, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k A>
inline ws_char_w_t const* c_str_data_w(winstl_ns_qual(basic_file_path_buffer)<ws_char_w_t, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_data for winstl::basic_file_path_buffer
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C const* c_str_data(winstl_ns_qual(basic_file_path_buffer)<C, A> const& b)
{
    return b.c_str();
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k A>
inline ws_size_t c_str_len_a(winstl_ns_qual(basic_file_path_buffer)<ws_char_a_t, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_a)(b.c_str());
}
template <ss_typename_param_k A>
inline ws_size_t c_str_len_w(winstl_ns_qual(basic_file_path_buffer)<ws_char_w_t, A> const& b)
{
    return stlsoft_ns_qual(c_str_len_w)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_len for winstl::basic_file_path_buffer
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline ws_size_t c_str_len(winstl_ns_qual(basic_file_path_buffer)<C, A> const& b)
{
    return stlsoft_ns_qual(c_str_len)(b.c_str());
}


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k A>
inline ws_char_a_t const* c_str_ptr_a(winstl_ns_qual(basic_file_path_buffer)<ws_char_a_t, A> const& b)
{
    return b.c_str();
}
template <ss_typename_param_k A>
inline ws_char_w_t const* c_str_ptr_w(winstl_ns_qual(basic_file_path_buffer)<ws_char_w_t, A> const& b)
{
    return b.c_str();
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr for winstl::basic_file_path_buffer
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr(winstl_ns_qual(basic_file_path_buffer)<C, A> const& b)
{
    return b.c_str();
}



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <ss_typename_param_k A>
inline ws_char_a_t const* c_str_ptr_null_a(winstl_ns_qual(basic_file_path_buffer)<ws_char_a_t, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}
template <ss_typename_param_k A>
inline ws_char_w_t const* c_str_ptr_null_w(winstl_ns_qual(basic_file_path_buffer)<ws_char_w_t, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief \ref group__concept__shim__string_access__c_str_ptr_null for winstl::basic_file_path_buffer
 *
 * \ingroup group__concept__shim__string_access
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline C const* c_str_ptr_null(winstl_ns_qual(basic_file_path_buffer)<C, A> const& b)
{
    return stlsoft_ns_qual(c_str_ptr_null)(b.c_str());
}




/** \brief \ref group__concept__shim__stream_insertion "stream insertion shim" for winstl::basic_file_path_buffer
 *
 * \ingroup group__concept__shim__stream_insertion
 */
template<   ss_typename_param_k S
        ,   ss_typename_param_k C
        ,   ss_typename_param_k A
        >
inline S& operator <<(S& s, winstl_ns_qual(basic_file_path_buffer)<C, A> const& b)
{
    s << b.c_str();

    return s;
}

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/file_path_buffer_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _WINSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace winstl
# else
} // namespace winstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* In the special case of Intel behaving as VC++ 7.0 or earlier on Win32, we
 * illegally insert into the std namespace.
 */
#if defined(STLSOFT_CF_std_NAMESPACE)
# if ( ( defined(STLSOFT_COMPILER_IS_INTEL) && \
         defined(_MSC_VER))) && \
     _MSC_VER < 1310
namespace std
{
    template<   ss_typename_param_k C
            ,   ss_typename_param_k A
            >
    inline void swap(winstl_ns_qual(basic_file_path_buffer)<C, A>& lhs, winstl_ns_qual(basic_file_path_buffer)<C, A>& rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
# endif /* INTEL && _MSC_VER < 1310 */
#endif /* STLSOFT_CF_std_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The string access shims exist either in the stlsoft namespace, or in the
 * global namespace. This is required by the lookup rules.
 *
 */

#ifndef _WINSTL_NO_NAMESPACE
# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
namespace stlsoft
{
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */

using ::winstl::c_str_ptr_null;
using ::winstl::c_str_ptr_null_a;
using ::winstl::c_str_ptr_null_w;

using ::winstl::c_str_ptr;
using ::winstl::c_str_ptr_a;
using ::winstl::c_str_ptr_w;

using ::winstl::c_str_data;
using ::winstl::c_str_data_a;
using ::winstl::c_str_data_w;

using ::winstl::c_str_len;

# if !defined(_STLSOFT_NO_NAMESPACE) && \
     !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace stlsoft
# else /* ? _STLSOFT_NO_NAMESPACE */
/* There is no stlsoft namespace, so must define in the global namespace */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !_WINSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */

/* ///////////////////////////// end of file //////////////////////////// */
