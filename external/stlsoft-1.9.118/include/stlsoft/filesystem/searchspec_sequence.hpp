/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/filesystem/searchspec_sequence.hpp
 *
 * Purpose:     Contains the searchspec_sequence template class, and ANSI
 *              and Unicode specialisations thereof.
 *
 * Thanks to:   Jon Papaioannou for spotting a character encoding error in
 *              one of the typedefs
 *
 * Created:     1st May 2004
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


/** \file stlsoft/filesystem/searchspec_sequence.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::searchspec_sequence class
 *  template
 *   (\ref group__library__filesystem "File System" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE
#define STLSOFT_INCL_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE_MAJOR       4
# define STLSOFT_VER_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE_MINOR       1
# define STLSOFT_VER_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE_REVISION    6
# define STLSOFT_VER_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE_EDIT        59
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/filesystem/searchspec_sequence.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING
# include <stlsoft/string/simple_string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SIMPLE_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TOKENISER
# include <stlsoft/string/string_tokeniser.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TOKENISER */

#ifndef STLSOFT_COMPILER_IS_WATCOM
# ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
#  include <stlsoft/util/std/iterator_helper.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#endif /* compiler */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

/* /////////////////////////////////////////////////////////////////////////
 * Pre-processor
 *
 * Definition of the
 */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

template <ss_typename_param_k T>
inline void call_set_null(T *&pt, void (T::*F)())
{
    STLSOFT_ASSERT(NULL != pt);

    (pt->*F)();

    pt = NULL;
}

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Provides multi-pattern functionality over a file-system search sequence class
 *
 * \ingroup group__library__filesystem
 */
template <ss_typename_param_k S>
class searchspec_sequence
    : public stl_collection_tag
{
/// \name Member Types
/// @{
public:
    /// The underlying find sequence type
    typedef S                                                           find_sequence_type;
    /// The current parameterisation of the type
    typedef searchspec_sequence<S>                                      class_type;
private:
    typedef searchspec_sequence<S>                                      outer_class_type;
    typedef ss_typename_type_k find_sequence_type::traits_type          traits_type;
public:
    /// The character type
    typedef ss_typename_type_k find_sequence_type::char_type            char_type;
    /// The value type
    typedef ss_typename_type_k find_sequence_type::value_type           value_type;
    /// The size type
    typedef ss_typename_type_k find_sequence_type::size_type            size_type;
    /// The non-mutable (const) reference type
    typedef ss_typename_type_k find_sequence_type::const_reference      const_reference;
    /// The non-mutating (const) iterator type
    class                                                               const_iterator;
private:
    typedef basic_simple_string<char_type>                              string_type;
    // TODO: Have all filesystem_traits that have a fixed length derive from a tag type, so that we can parameterise
    // based on that. Then reinstate the string type discrimination based on the traits
//    typedef basic_static_string<char_type, traits_type::maxPathLength>  string_type;
    typedef string_tokeniser<string_type, char_type>                    tokeniser_type;
/// @}

/// \name Construction
/// @{
public:
    /// Creates a search sequence for the given search specification and delimiter
    ///
    /// \param searchSpec The multi-filter search specification, e.g. "*.cpp", "*.d:makefile:*.java"
    /// \param delimiter The delimiter to use, e.g. ':'
    ///
    /// \note Assumes the current directory
    searchspec_sequence(char_type const* searchSpec, char_type delimiter)
        : m_rootDir(get_root_dir())
        , m_searchSpec(searchSpec)
        , m_delimiter(delimiter)
        , m_flags(0)
    {}
    /// Creates a search sequence for the given search specification, delimiter and flags
    ///
    /// \param searchSpec The multi-filter search specification, e.g. "*.cpp", "*.d:makefile:*.java"
    /// \param delimiter The delimiter to use, e.g. ':'
    /// \param flags Flags specific to the underlying sequence
    ///
    /// \note Assumes the current directory
    searchspec_sequence(char_type const* searchSpec, char_type delimiter, ss_int_t flags)
        : m_rootDir(get_root_dir())
        , m_searchSpec(searchSpec)
        , m_delimiter(delimiter)
        , m_flags(flags)
    {}
    /// Creates a search sequence for the given root directory, search specification and delimiter
    ///
    /// \param rootDir The root directory of the search
    /// \param searchSpec The multi-filter search specification, e.g. "*.cpp", "*.d:makefile:*.java"
    /// \param delimiter The delimiter to use, e.g. ':'
    searchspec_sequence(char_type const* rootDir, char_type const* searchSpec, char_type delimiter)
        : m_rootDir(rootDir)
        , m_searchSpec(searchSpec)
        , m_delimiter(delimiter)
        , m_flags(0)
    {}
    /// Creates a search sequence for the given root directory, search specification, delimiter and flags
    ///
    /// \param rootDir The root directory of the search
    /// \param searchSpec The multi-filter search specification, e.g. "*.cpp", "*.d:makefile:*.java"
    /// \param delimiter The delimiter to use, e.g. ':'
    /// \param flags Flags specific to the underlying sequence
    searchspec_sequence(char_type const* rootDir, char_type const* searchSpec, char_type delimiter, ss_int_t flags)
        : m_rootDir(rootDir)
        , m_searchSpec(searchSpec)
        , m_delimiter(delimiter)
        , m_flags(flags)
    {}
/// @}

/// \name Implementation
/// @{
private:
    struct search_state
    {
        string_type                                             m_rootDir;
        ss_int_t                                                m_flags;
        tokeniser_type                                          m_tokens;
        ss_typename_type_k tokeniser_type::const_iterator       m_tokensNext;
        ss_typename_type_k tokeniser_type::const_iterator       m_tokensEnd;
        find_sequence_type                                      *m_entries;
        ss_typename_type_k find_sequence_type::const_iterator   m_entriesNext;
        ss_typename_type_k find_sequence_type::const_iterator   m_entriesEnd;
        ss_sint32_t                                             m_cRefs;

    private:
        search_state(char_type const* rootDir, char_type const* searchSpec, char_type delimiter, ss_int_t flags)
            : m_rootDir(rootDir)
            , m_flags(flags)
            , m_tokens(searchSpec, delimiter)
            , m_tokensNext(m_tokens.begin())
            , m_tokensEnd(m_tokens.end())
#if defined(STLSOFT_COMPILER_IS_VECTORC)
            , m_entries(new find_sequence_type(m_rootDir.c_str(), (*m_tokensNext).c_str(), m_flags))
#else /* ? compiler */
            , m_entries(new find_sequence_type(c_str_ptr(m_rootDir), c_str_ptr(*m_tokensNext), m_flags))
#endif /* compiler */
            , m_entriesNext(m_entries->begin())
            , m_entriesEnd(m_entries->end())
            , m_cRefs(1)
        {
            while(m_entriesNext == m_entriesEnd)
            {
                ++m_tokensNext;

                if(m_tokensNext == m_tokensEnd)
                {
                    break;
                }
                else
                {
                    STLSOFT_ASSERT(NULL != m_entries);

                    stlsoft_destroy_instance_fn(m_entries);
#if defined(STLSOFT_COMPILER_IS_VECTORC)
                    m_entries = new(m_entries) find_sequence_type(m_rootDir.c_str(), (*m_tokensNext).c_str(), m_flags);
#else /* ? compiler */
                    m_entries = new(m_entries) find_sequence_type(c_str_ptr(m_rootDir), c_str_ptr(*m_tokensNext), m_flags);
#endif /* compiler */

                    m_entriesEnd    =   m_entries->end();
                    m_entriesNext   =   m_entries->begin();
                }
            }
        }

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1300
        friend class const_iterator;
#endif /* compiler */

    public:
        static search_state *create(char_type const* rootDir, char_type const* searchSpec, char_type delimiter, ss_int_t flags)
        {
            search_state *ss = new search_state(rootDir, searchSpec, delimiter, flags);

            if(ss->m_tokensNext == ss->m_tokensEnd)
            {
                delete ss;

                ss = NULL;
            }

            return ss;
        }

    public:
        bool next()
        {
            if(m_tokensNext == m_tokensEnd)
            {
                return false;
            }

            STLSOFT_ASSERT(m_tokensNext != m_tokensEnd);
            STLSOFT_ASSERT(NULL != m_entries || m_tokensNext == m_tokensEnd);

            // We are doing two enumerations here.
            //
            // The outer enumeration is over the tokens, the
            // inner is over the file entries.

            ++m_entriesNext;

            while(m_entriesNext == m_entriesEnd)
            {
                ++m_tokensNext;

                if(m_tokensNext == m_tokensEnd)
                {
                    return false;
                }
                else
                {
                    STLSOFT_ASSERT(NULL != m_entries);

                    stlsoft_destroy_instance_fn(m_entries);
#if defined(STLSOFT_COMPILER_IS_VECTORC)
                    m_entries = new(m_entries) find_sequence_type(m_rootDir.c_str(), (*m_tokensNext).c_str(), m_flags);
#else /* ? compiler */
                    m_entries = new(m_entries) find_sequence_type(c_str_ptr(m_rootDir), c_str_ptr(*m_tokensNext), m_flags);
#endif /* compiler */

                    m_entriesEnd    =   m_entries->end();
                    m_entriesNext   =   m_entries->begin();
                }
            }

            return true;
        }

        void Release()
        {
            if(0 == --m_cRefs)
            {
                delete this;
            }
        }
#if defined(STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR)
protected:
#else /* ? STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
private:
#endif /* STLSOFT_CF_COMPILER_WARNS_NO_PUBLIC_DTOR */
        ~search_state() stlsoft_throw_0()
        {
            delete m_entries;
        }
    };
/// @}

/// \name Iteration
/// @{
public:
    /// The const_iterator type for the searchspec_sequence
    class const_iterator
    {
    private:
        typedef const_iterator      class_type;

    private:
        friend class searchspec_sequence<S>;

        const_iterator(char_type const* rootDir, char_type const* searchSpec, char_type delimiter, ss_int_t flags)
            : m_searchState(search_state::create(rootDir, searchSpec, delimiter, flags))
        {}

    public:
        /// Default constructor
        const_iterator()
            : m_searchState(NULL)
        {}
        /// Destructor
        ~const_iterator() stlsoft_throw_0()
        {
            if(NULL != m_searchState)
            {
                m_searchState->Release();
            }
        }

        /// Copy constructor
        const_iterator(class_type const& rhs)
            : m_searchState(rhs.m_searchState)
        {
            if(NULL != m_searchState)
            {
                ++m_searchState->m_cRefs;
            }
        }

        class_type& operator =(class_type const& rhs)
        {
            if(NULL != m_searchState)
            {
                m_searchState->Release();
            }

            m_searchState = rhs.m_searchState;

            if(NULL != m_searchState)
            {
                ++m_searchState->m_cRefs;
            }

            return *this;
        }

    public:
        class_type& operator ++()
        {
            STLSOFT_ASSERT(NULL != m_searchState);

            if(!m_searchState->next())
            {
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1300
                m_searchState->Release();

                m_searchState = NULL;
#else /* ? compiler */
                call_set_null(m_searchState, &search_state::Release);
#endif /* compiler */
            }

            return *this;
        }

        class_type operator ++(int)
        {
            class_type  ret(*this);

            operator ++();

            return ret;
        }

        /// Dereference to return the value at the current position
        const value_type operator *() const
        {
            STLSOFT_ASSERT(NULL != m_searchState);

            return *m_searchState->m_entriesNext;
        }

        ss_bool_t operator ==(class_type const& rhs) const
        {
            // Only evaluate the same when both are at their ends
            return (NULL == m_searchState) && (NULL == rhs.m_searchState);
        }
        ss_bool_t operator !=(class_type const& rhs) const
        {
            return !operator ==(rhs);
        }

    private:
        search_state    *m_searchState;
    };
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return An iterator representing the start of the sequence
    const_iterator begin() const
    {
#if defined(STLSOFT_COMPILER_IS_VECTORC)
        return const_iterator(m_rootDir.c_str(), (m_searchSpec).c_str(), m_delimiter, m_flags);
#else /* ? compiler */
        return const_iterator(c_str_ptr(m_rootDir), c_str_ptr(m_searchSpec), m_delimiter, m_flags);
#endif /* compiler */
    }
    /// Ends the iteration
    ///
    /// \return An iterator representing the end of the sequence
    const_iterator end() const
    {
        return const_iterator();
    }
/// @}

/// \name Attributes
/// @{
public:
    /// \brief Indicates whether the sequence is empty
    ss_bool_t empty() const
    {
        return begin() == end();
    }
/// @}

/// \name Implementation
/// @{
private:
    static char_type const* get_root_dir()
    {
        static char_type s_rootDir[2] = { '.', '\0' };

        return s_rootDir;
    }
/// @}

/// \name Members
/// @{
private:
    string_type m_rootDir;
    string_type m_searchSpec;
    char_type   m_delimiter;
    ss_int_t    m_flags;
/// @}
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/searchspec_sequence_unittest_.h"
#endif /* STLSOFT_UNITTEST */

////////////////////////////////////////////////////////////////////////////
// Implementation

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION


#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_FILESYSTEM_HPP_SEARCHSPEC_SEQUENCE */

/* ///////////////////////////// end of file //////////////////////////// */
