/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/commandline_parser.hpp (originally MLCmdArg.h, ::SynesisStd)
 *
 * Purpose:     commandline_parser class.
 *
 * Created:     20th May 2000
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2000-2009, Matthew Wilson and Synesis Software
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


/** \file winstl/system/commandline_parser.hpp
 *
 * \brief [C++ only] Definition of the winstl::basic_commandline_parser
 *  class template
 *   (\ref group__library__system "System" Library).
 */


#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER_MAJOR      2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER_MINOR      1
# define WINSTL_VER_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER_REVISION   2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER_EDIT       36
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER
# include <stlsoft/memory/auto_buffer.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_AUTO_BUFFER */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS
# include <stlsoft/string/char_traits.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_CHAR_TRAITS */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_H_CTYPE
# define STLSOFT_INCL_H_CTYPE
# include <ctype.h>      // for isspace()
#endif /* !STLSOFT_INCL_H_CTYPE */

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

/** Parses a Windows (<code>WinMain()</code>) command line into
 *   parts, and provides array semantics for their access.
 *
\code
winstl::commandline_parser  cp("abc \"d e f\" ghi");

assert(3 == cp.size());
assert(0 == ::strcmp("abc",   cp[0]));
assert(0 == ::strcmp("d e f", cp[1]));
assert(0 == ::strcmp("ghi",   cp[2]));
\endcode
 *
 * \ingroup group__library__system
 */
template<   ss_typename_param_k C
        ,   ss_typename_param_k T = stlsoft_ns_qual(stlsoft_char_traits)<C>
        >
class basic_commandline_parser
{
/// \name Member Types
/// @{
public:
    /// The character type
    typedef C                                                   char_type;
    /// The traits type
    typedef T                                                   traits_type;
    /// The current instantiation of the type
    typedef basic_commandline_parser<C, T>                      class_type;
private:
    typedef char_type*                                          pointer_type_;
    typedef processheap_allocator<char_type>                    allocator_type_;
    typedef processheap_allocator<pointer_type_>                pointers_allocator_type_;
    typedef stlsoft_ns_qual(auto_buffer)<
        char_type
    ,   256
    ,   allocator_type_
    >                                                           buffer_type_;
    typedef stlsoft_ns_qual(auto_buffer)<
        pointer_type_
    ,   20
    ,   pointers_allocator_type_
    >                                                           pointers_type_;
    typedef ss_typename_type_k buffer_type_::iterator           iterator;
public:
    /// The value type
    typedef ss_typename_type_k pointers_type_::value_type       value_type;
    /// The non-mutating (const) iterator type
    typedef ss_typename_type_k pointers_type_::const_iterator   const_iterator;
    /// The size type
    typedef ss_size_t                                           size_type;
/// @}

/// \name Construction
/// @{
public:
    /** Parses the given command-line and creates an internal array
     *   of pointers to the arguments.
     */
    ss_explicit_k basic_commandline_parser(char_type const* cmdLine)
        : m_buffer(1 + stlsoft_ns_qual(c_str_len)(cmdLine))
        , m_pointers(0)
    {
        init_(cmdLine, m_buffer.size() - 1);
    }
    /** Parses the given command-line and creates an internal array
     *   of pointers to the arguments.
     */
    ss_explicit_k basic_commandline_parser(char_type const* cmdLine, size_type len)
        : m_buffer(1 + len)
        , m_pointers(0)
    {
        init_(cmdLine, len);
    }

private:
    void init_(char_type const* cmdLine, size_type len)
    {
        WINSTL_MESSAGE_ASSERT("command-line may not be NULL, unless length is 0", (0 == len) || (NULL != cmdLine));

        traits_type::copy(&m_buffer[0], cmdLine, len);
        m_buffer[len] = '\0';

        // Here's the algorithm:
        //
        // Walk the string, mindful of quotes, remembering the start of an
        // argument, and writing the nul-character into spaces.

        enum state_t
        {
                argument
            ,   quotedArgumentStart
            ,   quotedArgument
            ,   space
        };

        state_t         state   =   space;
        iterator        b       =   m_buffer.begin();
        const iterator  e       =   m_buffer.end() - 1;

        for(; b != e; ++b)
        {
            const char_type ch = *b;

            WINSTL_ASSERT('\0' != ch);

            if('"' == ch)
            {
                if(quotedArgumentStart == state)
                {
                    state   =   space;
                }
                else if(quotedArgument == state)
                {
                    *b      =   '\0';
                    state   =   space;
                }
                else if(space == state)
                {
                    state   =   quotedArgumentStart;
                }
                else
                {
                }
            }
            else if(isspace(ch))
            {
                if(quotedArgumentStart == state)
                {
                    state   =   quotedArgument;

                    add_pointer(&*b);
                }
                else if(quotedArgument == state)
                {
                }
                else if(space == state)
                {
                }
                else
                {
                    WINSTL_ASSERT(argument == state);

                    *b = '\0';

                    state = space;
                }
            }
            else
            {
                if(quotedArgumentStart == state)
                {
                    state   =   quotedArgument;

                    add_pointer(&*b);
                }
                else if(space == state)
                {
                    state   =   argument;

                    add_pointer(&*b);
                }
            }
        }
    }
/// @}

/// \name Accessors
/// @{
public:
    /// The number of arguments
    size_type           size() const
    {
        return m_pointers.size();
    }

    /** Returns a non-mutating (const) pointer to each argument
     *   string.
     *
     * \param index The index of the argument
     *
     * \note The behaviour is undefined if <code>index</code> is greater
     *  than <code>size()</code>. If <code>index</code> is equal to
     *  <code>size()</code>, then the returned reference may not be used,
     *  other than to take its address (which is the address of the
     *  <a href="http://www.extendedstl.com/glossary.html#end-element">"end-element"</a>).
     */
    value_type const&   operator [](size_type index) const
    {
        WINSTL_ASSERT(index <= size());

        return m_pointers[index];
    }
/// @}

/// \name Iteration
/// @{
public:
    /// An iterator representing the start of the sequence
    const_iterator  begin() const
    {
        return m_pointers.begin();
    }
    /// An iterator representing the end of the sequence
    const_iterator  end() const
    {
        return m_pointers.end();
    }
/// @}

/// \name Implementation
/// @{
private:
    ws_bool_t add_pointer(pointer_type_ p)
    {
        if(!m_pointers.resize(1 + m_pointers.size()))
        {
            return false;
        }

        m_pointers[m_pointers.size() - 1] = p;

        return true;
    }
/// @}

/// \name Members
/// @{
private:
    buffer_type_    m_buffer;
    pointers_type_  m_pointers;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs for commonly encountered types
 */

/** Specialisation of the basic_commandline_parser template for the ANSI character type \c char
 *
 * \ingroup group__library__system
 */
typedef basic_commandline_parser<ws_char_a_t>   commandline_parser_a;
/** Specialisation of the basic_commandline_parser template for the Unicode character type \c wchar_t
 *
 * \ingroup group__library__system
 */
typedef basic_commandline_parser<ws_char_w_t>   commandline_parser_w;
/** Specialisation of the basic_commandline_parser template for the Win32 character type \c TCHAR
 *
 * \ingroup group__library__system
 */
typedef basic_commandline_parser<TCHAR>         commandline_parser;

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/commandline_parser_unittest_.h"
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

/* ////////////////////////////////////////////////////////////////////// */

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_COMMANDLINE_PARSER */

/* ///////////////////////////// end of file //////////////////////////// */
