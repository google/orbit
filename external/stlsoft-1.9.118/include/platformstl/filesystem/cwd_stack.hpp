/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/filesystem/cwd_stack.hpp
 *
 * Purpose:     Platform header for the filesystem_traits components.
 *
 * Created:     16th July 2006
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2006-2009, Matthew Wilson and Synesis Software
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


/** \file platformstl/filesystem/cwd_stack.hpp
 *
 * \brief [C++ only] Definition of the platformstl::cwd_stack
 *   class template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK
#define PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK_MAJOR     2
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK_MINOR     1
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK_REVISION  5
# define PLATFORMSTL_VER_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK_EDIT      25
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_PLATFORMSTL */
#ifndef PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_EXCEPTIONS
# include <platformstl/error/exceptions.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_ERROR_HPP_EXCEPTIONS */
#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY
# include <platformstl/filesystem/current_directory.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CURRENT_DIRECTORY */
#ifndef PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS
# include <platformstl/filesystem/filesystem_traits.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_FILESYSTEM_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
# include <stlsoft/string/simple_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */

#ifndef STLSOFT_INCL_STACK
# define STLSOFT_INCL_STACK
# include <stack>
#endif /* !STLSOFT_INCL_STACK */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
namespace platformstl
{
#else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Acts as a stack for current working directory changes, setting
 *    the current working directory with <code>push()</code>, and resetting
 *    to its previous value with <code>pop()</code>.
 *
 * \ingroup group__library__filesystem
 */
template<   typename    C
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        ,   typename    XP = platformstl::platform_exception_policy
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        ,   typename    XP = stlsoft::nothrow_exception_policy
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        >
class basic_cwd_stack
{
public:
    typedef C                                   char_type;
    typedef XP                                  exception_policy_type;
    typedef stlsoft::basic_simple_string<C>     string_type;
    typedef bool                                bool_type;
    typedef stlsoft_ns_qual(ss_size_t)          size_type;
private:
    typedef filesystem_traits<C>                traits_type;
    typedef std::stack<string_type>             stack_type;

public:
    string_type const   &top() const;

    void                push(string_type const& directory);
    ///
    /// \exception platformstl::platform_exception Thrown if the directory
    void                pop();

    ///
    ///
    /// \remarks If fails, then platformstl::system_traits<C>::get_last_error() gives the error
    bool_type           try_pop();

public:
    /// Indicates whether the stack is empty
    bool_type   empty() const;
    /// [DEPRECATED] Indicates whether the stack is empty
    ///
    /// \deprecated Use empty() instead
    bool_type   is_empty() const;
    /// Indicates the number of directories in the stack
    size_type   size() const;

private:
    string_type const   &translate_environment_(string_type const& directory, string_type &trueDirectory);

private:
    stack_type  m_stack;

// TODO: if throws, then implies that the class keeps a lock on the directory, if poss.

#if 0
    struct directory
    {
        string_type     path;
        file_handle     handle; // This keeps the directory from being removed
    };
#endif /* 0 */
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 */

typedef basic_cwd_stack<ss_char_a_t>            cwd_stack_a;
typedef basic_cwd_stack<ss_char_w_t>            cwd_stack_w;
typedef basic_cwd_stack<ss_char_a_t>            cwd_stack;

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

template <typename C, typename XP>
inline ss_typename_type_ret_k basic_cwd_stack<C, XP>::string_type const& basic_cwd_stack<C, XP>::translate_environment_(ss_typename_type_k basic_cwd_stack<C, XP>::string_type const& directory, ss_typename_type_k basic_cwd_stack<C, XP>::string_type &trueDirectory)
{
    if(directory.end() != std::find(directory.begin(), directory.end(), '%'))
    {
        stlsoft::auto_buffer<char_type>     buffer(1 + traits_type::expand_environment_strings(directory.c_str(), NULL, 0));
        const stlsoft_ns_qual(ss_size_t)    cch = traits_type::expand_environment_strings(directory.c_str(), &buffer[0], buffer.size());

        trueDirectory.assign(buffer.data(), cch);

        return trueDirectory;
    }

    return directory;
}


template <typename C, typename XP>
inline ss_typename_type_ret_k basic_cwd_stack<C, XP>::size_type basic_cwd_stack<C, XP>::size() const
{
    return m_stack.size();
}

template <typename C, typename XP>
inline ss_typename_type_ret_k basic_cwd_stack<C, XP>::bool_type basic_cwd_stack<C, XP>::empty() const
{
    return 0 == size();
}

template <typename C, typename XP>
inline ss_typename_type_ret_k basic_cwd_stack<C, XP>::bool_type basic_cwd_stack<C, XP>::is_empty() const
{
    return empty();
}

template <typename C, typename XP>
inline void basic_cwd_stack<C, XP>::push(ss_typename_type_k basic_cwd_stack<C, XP>::string_type const& directory)
{
    stlsoft::auto_buffer<char_type>     buffer(1 + traits_type::get_current_directory(static_cast<char_type*>(NULL), 0));
    const stlsoft_ns_qual(ss_size_t)    cch = traits_type::get_current_directory(&buffer[0], buffer.size());

    string_type                         cwd(buffer.data(), cch);
    string_type                         trueDirectory;

    m_stack.push(cwd);

    if(!traits_type::set_current_directory(translate_environment_(directory, trueDirectory).c_str()))
    {
        m_stack.pop();

        exception_policy_type   xp;

        xp("Failed to change directory", traits_type::get_last_error());
    }
}

template <typename C, typename XP>
inline void basic_cwd_stack<C, XP>::pop()
{
    STLSOFT_ASSERT(!empty());

    const string_type directory =   m_stack.top();

    m_stack.pop();

    if(!traits_type::set_current_directory(directory.c_str()))
    {
        exception_policy_type   xp;

        xp("Failed to restore directory", traits_type::get_last_error());
    }
}

template <typename C, typename XP>
inline ss_typename_type_ret_k basic_cwd_stack<C, XP>::bool_type basic_cwd_stack<C, XP>::try_pop()
{
    STLSOFT_ASSERT(!empty());

    string_type const   &directory  =   m_stack.top();

    if(!traits_type::set_current_directory(directory.c_str()))
    {
        return false;
    }
    else
    {
        m_stack.pop();

        return true;
    }
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#if defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
#else
} // namespace platformstl_project
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_FILESYSTEM_HPP_CWD_STACK */

/* ///////////////////////////// end of file //////////////////////////// */
