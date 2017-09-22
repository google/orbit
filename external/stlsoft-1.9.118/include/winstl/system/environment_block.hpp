/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/environment_block.hpp
 *
 * Purpose:     Contains the basic_environment_block class.
 *
 * Created:     25th June 2004
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


/** \file winstl/system/environment_block.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_environment_block class
 *  template
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK_MAJOR       4
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK_MINOR       0
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK_REVISION    2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK_EDIT        54
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING
# include <winstl/shims/access/string.hpp>
#endif /* !WINSTL_INCL_WINSTL_SHIMS_ACCESS_HPP_STRING */

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

/** \brief Class used for preparing environment blocks compatible with the Windows
 * CreateProcess() function.
 *
 * \ingroup group__library__system
 *
 * It is used as follows:
\code
winstl::environment_block   env;

env.push_back("Name1", "Value1"); // Insert separate name and value
env.push_back("Name2=Value2");    // Insert composite name and value

::CreateProcess(  . . . // application name
               ,  . . . // command line
               ,  . . . // process attributes
               ,  . . . // thread attributes
               ,  . . . // handle inherit boolean
               ,  . . . // creation flags
               ,  const_cast<void*>(env.base()) // The environment
               ,  . . . // current directory
               ,  . . . // statup info
               ,  . . . // process info);
\endcode
 *
 */
template<   ss_typename_param_k C
#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT
        ,   ss_typename_param_k T = char_traits<C>
        ,   ss_typename_param_k A = processheap_allocator<C>
#else /* ? STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        ,   ss_typename_param_k T /* = char_traits<C> */
        ,   ss_typename_param_k A /* = processheap_allocator<C> */
#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */
        >
class basic_environment_block
{
/// \name Types
/// @{
public:
    /// The value type
    typedef C                                   value_type;
    /// The char type
    typedef C                                   char_type;
    /// The traits type
    typedef T                                   traits_type;
    /// The allocator type
    typedef A                                   allocator_type;
    /// The current parameterisation of the type
    typedef basic_environment_block<C, T, A>    class_type;
    /// The size type
    typedef ws_size_t                           size_type;
/// @}

/// \name Construction
/// @{
public:
    /// Constructs an empty block
    basic_environment_block()
        : m_buffer(2)
    {
        m_buffer[0] = '\0';
        m_buffer[1] = '\0';
    }
    /// Constructs the block with a copy of the given instance
    basic_environment_block(class_type const& rhs)
        : m_buffer(rhs.m_buffer.size())
    {
        pod_copy_n(&m_buffer.data()[0], &rhs.m_buffer.data()[0], m_buffer.size());
    }

    /// Copies the contents of the given instance
    ///
    /// \exception std::bad_alloc When compiling with exception support, this will throw
    /// std::bad_alloc if memory cannot be acquired. When compiling absent
    /// exception support, failure to acquire memory will leave the
    /// instance unchanged.
    class_type& operator =(class_type const& rhs)
    {
        if(m_buffer.resize(rhs.m_buffer.size()))
        {
            pod_copy_n(&m_buffer.data()[0], &rhs.m_buffer.data()[0], m_buffer.size());
        }

        return *this;
    }
/// @}

/// \name Operations
/// @{
public:
    /// \brief Append a full NAME=VALUE environment variable
    ///
    /// \param variable The variable
    /// \param cch The length of the variable
    ///
    /// \return An indication of success. This will always return true when
    /// compiling with exception support.
    ///
    /// \note The variable must contain an equal sign ('=')
    ///
    /// \exception std::bad_alloc When compiling with exception support, this will throw
    /// std::bad_alloc if memory cannot be acquired. When compiling absent
    /// exception support, failure to acquire memory will cause the method
    /// to return false.
    ws_bool_t push_back(char_type const* variable, ws_size_t cch)
    {
        WINSTL_ASSERT(NULL != variable);
        WINSTL_ASSERT(cch >= 3);
        WINSTL_ASSERT(NULL != traits_type::find(variable, cch, '='));

        size_type   oldSize = m_buffer.size();

        WINSTL_ASSERT(m_buffer.size() > 1);
        WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 1]);
        WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 2]);

        if(!m_buffer.resize(oldSize + cch + 1))
        {
            return false;
        }
        else
        {
            traits_type::copy(&m_buffer[oldSize - 1], variable, cch);
            m_buffer[m_buffer.size() - 2] = '\0';
            m_buffer[m_buffer.size() - 1] = '\0';

            WINSTL_ASSERT(m_buffer.size() > 1);
            WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 1]);
            WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 2]);

            return true;
        }
    }
    /// \brief Append a NAME= environment variable
    ///
    /// \param variable The variable
    ///
    /// \return An indication of success. This will always return true when
    /// compiling with exception support.
    ///
    /// \note The variable must contain an equal sign ('=')
    ///
    /// \exception std::bad_alloc When compiling with exception support, this will throw
    /// std::bad_alloc if memory cannot be acquired. When compiling absent
    /// exception support, failure to acquire memory will cause the method
    /// to return false.
    template <ss_typename_param_k S>
    ws_bool_t push_back(S const& variable)
    {
        return push_back(stlsoft_ns_qual(c_str_data)(variable), stlsoft_ns_qual(c_str_len)(variable));
    }
    /// \brief Append a full NAME=VALUE environment pair
    ///
    /// \param name The variable name
    /// \param cchName The length of the variable name
    /// \param value The variable value
    /// \param cchValue The length of the variable value
    ///
    /// \return An indication of success. This will always return true when
    /// compiling with exception support.
    ///
    /// \exception std::bad_alloc When compiling with exception support, this will throw
    /// std::bad_alloc if memory cannot be acquired. When compiling absent
    /// exception support, failure to acquire memory will cause the method
    /// to return false.
    ws_bool_t push_back(char_type const* name, ws_size_t cchName, char_type const* value, ws_size_t cchValue)
    {
        WINSTL_ASSERT(NULL != name);
        WINSTL_ASSERT(NULL != value);
        WINSTL_ASSERT(cchName > 1);
//        WINSTL_ASSERT(cchValue > 1);

        size_type   oldSize = m_buffer.size();

        WINSTL_ASSERT(m_buffer.size() > 1);
        WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 1]);
        WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 2]);

        if(!m_buffer.resize(oldSize + cchName + 1 + cchValue + 1))
        {
            return false;
        }
        else
        {
            traits_type::copy(&m_buffer[oldSize - 2], name, cchName);
            m_buffer[oldSize - 2 + cchName] = '=';
            traits_type::copy(&m_buffer[oldSize - 2 + cchName + 1], value, cchValue);
            m_buffer[oldSize - 2 + cchName + 1 + cchValue] = '\0';
            m_buffer[m_buffer.size() - 2] = '\0';
            m_buffer[m_buffer.size() - 1] = '\0';

            WINSTL_ASSERT(m_buffer.size() > 1);
            WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 1]);
            WINSTL_ASSERT('\0' == m_buffer[m_buffer.size() - 2]);

            return true;
        }
    }
    /// \brief Append a full NAME=VALUE environment pair
    ///
    /// \param name The variable name
    /// \param value The variable value
    ///
    /// \return An indication of success. This will always return true when
    /// compiling with exception support.
    ///
    /// \exception std::bad_alloc When compiling with exception support, this will throw
    /// std::bad_alloc if memory cannot be acquired. When compiling absent
    /// exception support, failure to acquire memory will cause the method
    /// to return false.
    template<   ss_typename_param_k S1
            ,   ss_typename_param_k S2
            >
    ws_bool_t push_back(S1 const& name, S2 const& value)
    {
        return push_back(stlsoft_ns_qual(c_str_data)(name), stlsoft_ns_qual(c_str_len)(name), stlsoft_ns_qual(c_str_data)(value), stlsoft_ns_qual(c_str_len)(value));
    }

    /// Empties the block of all variables
    void clear()
    {
        m_buffer.resize(2);

        m_buffer[0] = '\0';
        m_buffer[1] = '\0';
    }

    /// Swaps the contents of the two instances
    void swap(class_type& rhs) stlsoft_throw_0()
    {
        m_buffer.swap(rhs.m_buffer);
    }
/// @}

/// \name Accessors
/// @{
public:
    /// Returns a pointer to the block contents
    void const  *base() const
    {
        return m_buffer.data();
    }
    /// The number of characters in the block
    size_type size() const
    {
        return m_buffer.size();
    }
    /// The number of characters in the block
    ///
    /// \note This method is a synonym for size()
    size_type length() const
    {
        return size();
    }
/// @}

/** \brief Members
 *
 * \ingroup group__library__system
 */
private:
    typedef stlsoft_ns_qual(auto_buffer_old)<   char_type
                                            ,   allocator_type
                                            ,   1024
                                            >       buffer_type_;

    buffer_type_    m_buffer;
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

#ifdef STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT

 /// Specialisation of the basic_path template for the ANSI character type \c char
typedef basic_environment_block<ws_char_a_t>    environment_block_a;
/** \brief Specialisation of the basic_environment_block template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__system
 */
typedef basic_environment_block<ws_char_w_t>    environment_block_w;
/** \brief Specialisation of the basic_environment_block template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__system
 */
typedef basic_environment_block<TCHAR>          environment_block;

#endif /* STLSOFT_CF_TEMPLATE_CLASS_DEFAULT_CLASS_ARGUMENT_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST
# include "./unittest/environment_block_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

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

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_ENVIRONMENT_BLOCK */

/* ///////////////////////////// end of file //////////////////////////// */
