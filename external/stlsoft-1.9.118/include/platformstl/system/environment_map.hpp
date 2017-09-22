/* /////////////////////////////////////////////////////////////////////////
 * File:        platformstl/system/environment_map.hpp
 *
 * Purpose:     Definition of the environment_map class.
 *
 * Created:     14th November 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
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


/** \file platformstl/system/environment_map.hpp
 *
 * \brief [C++ only] Definition of the platformstl::environment_map class
 *   (\ref group__library__system "System" Library).
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP
#define PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP_MAJOR       2
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP_MINOR       3
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP_REVISION    1
# define PLATFORMSTL_VER_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP_EDIT        56
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL
# include <platformstl/platformstl.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_HPP_PLATFORMSTL */
#ifndef PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_VARIABLE_TRAITS
# include <platformstl/system/environment_variable_traits.hpp>
#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_VARIABLE_TRAITS */

#if defined(PLATFORMSTL_OS_IS_UNIX)

#elif defined(PLATFORMSTL_OS_IS_WINDOWS)

#else /* ? operating system */
# error Operating system not discriminated
#endif /* operating system */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR
# include <stlsoft/util/std/library_discriminator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE
# include <stlsoft/smartptr/scoped_handle.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SCOPED_HANDLE */
#ifndef STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR
# include <stlsoft/smartptr/shared_ptr.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SMARTPTR_HPP_SHARED_PTR */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_SPLIT_FUNCTIONS
# include <stlsoft/string/split_functions.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_SPLIT_FUNCTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifndef STLSOFT_INCL_MAP
# define STLSOFT_INCL_MAP
# include <map>
#endif /* !STLSOFT_INCL_MAP */
#ifndef STLSOFT_INCL_UTILITY
# define STLSOFT_INCL_UTILITY
# include <utility>
#endif /* !STLSOFT_INCL_UTILITY */

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

/** \brief Provides an associative STL-collection interface to the current
 *   process's system environment.
 *
 * \note The design and implementation of this class is documented in Part 2
 *  of the forthcoming book
 *  <a href = "http://extendedstl.com">Extended STL</a>.
 */
class environment_map
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef environment_variable_traits                     traits_type;
    typedef stlsoft_ns_qual_std(string)                     string_type;
public:
    /// \brief The string type used for variable name, and lookup
    ///
    /// \note This is the association "key" type
    typedef string_type                                     first_type;
    /// \brief The string type used for variable value, and retrieval
    ///
    /// \note This is the association "value" type
    typedef string_type                                     second_type;
    /// \brief Value type of the class: a pair of first_type and second_type
    typedef std::pair<  const first_type
                    ,   second_type
                    >                                       value_type;
    /// \brief The size type
    typedef ss_size_t                                       size_type;
    /// \brief The difference type
    typedef ss_ptrdiff_t                                    difference_type;
    /// \brief The non-mutating (const) reference type
    typedef const value_type                                const_reference;    // BVT
    /// \brief The non-mutating (const) iterator type
    class                                                   const_iterator;
#if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT) && \
    !defined(STLSOFT_COMPILER_IS_BORLAND)
    typedef const_reverse_bidirectional_iterator_base<  const_iterator
                                                    ,   value_type
                                                    ,   const_reference
                                                    ,   void            // By-Value Temporary reference category
                                                    ,   difference_type
                                                    >       const_reverse_iterator;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// \brief The type of the class
    typedef environment_map                                 class_type;

private:
    friend class const_iterator;
    class snapshot
    {
    public: // Member Types
        typedef stlsoft_ns_qual(shared_ptr)<snapshot>   ref_type;
#if defined(STLSOFT_CF_STD_LIBRARY_IS_DINKUMWARE_VC) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0
        // VC7 libs get confused with const key type here
        typedef stlsoft_ns_qual_std(map)<         first_type
#else /* ? library */
        typedef stlsoft_ns_qual_std(map)<   const first_type
#endif /* library */
                                        ,   second_type
                                        >               variables_type_;
        typedef variables_type_::iterator               iterator;

    public: // Construction
        snapshot();

    public: // Operations
        ss_bool_t erase(    first_type const&   name) throw();
        void erase(         iterator            it) throw();
        void insert(        first_type const    &name
                        ,   second_type const&  value);
        void set(           first_type const&   name
                        ,   second_type const&  value);
        ss_bool_t lookup(   first_type const&   name
                        ,   second_type*&       pvalue) throw();

    public: // Iteration
        iterator    begin();
        iterator    end();

    private: // Members
        variables_type_   m_variables;
    };
/// @}

/// \name Construction
/// @{
public:
    /// \brief Constructs an instance of the type
    ///
    /// \note This instance does <b>not</b> store a snapshot of the
    ///  environment at the time of its construction. All lookup and
    ///  iteration is carried out 'live' at the time of invocation.
    environment_map();
/// @}

/// \name Element Access
/// @{
public:
    /// \brief Returns the value of the given environment variable, or throws
    ///  std::out_of_range if it does not exist.
    ///
    /// \param name The name of the environment variable whose value is to be
    ///  retrieved
    ///
    /// \exception std::out_of_range If no variable exists with the given name
    second_type operator [](char const* name) const;
    /// \brief Returns the value of the given environment variable, or throws
    ///  std::out_of_range if it does not exist.
    ///
    /// \param name The name of the environment variable whose value is to be
    ///  retrieved
    ///
    /// \exception std::out_of_range If no variable exists with the given name
    second_type operator [](first_type const& name) const;

    /// \brief Looks for the variable of the given name in the current
    ///  process environment.
    ///
    /// \return A Boolean value indicating whether the variable was found
    ss_bool_t   lookup(char const* name, second_type& value) const;
    /// \brief Looks for the variable of the given name in the current
    ///  process environment.
    ///
    /// \return A Boolean value indicating whether the variable was found
    ss_bool_t   lookup(first_type const& name, second_type& value) const;
/// @}

/// \name Operations
/// @{
public:
    /// \brief Discard any current enumeration snapshots.
    ///
    /// Used to force the collection instance to discard any currently snapshotd
    /// snapshot it may be holding on behalf of extant iterator instances, so
    /// that new iterator instances will receive a refreshed view of the
    /// underlying environment.
    void refresh();
/// @}

/// \name Modifiers
/// @{
public:
#ifdef PLATFORMSTL_ENVVAR_SET_SUPPORTED
    /// \brief Inserts or updates and environment variable
    ///
    /// \note This method is strongly exception-safe. The insertion into the
    /// snapshot is done first. If that does not throw an exception, but the
    /// insertion into the process' environment fails, it is removed from
    /// the snapshot. The only way that could fail would be if the element
    /// already exists, in which case
    void insert(first_type const& name, second_type const& value);

    /// \brief The semantics of this function are identical to the string object overload
    ///
    /// \param name The name of the variable to insert/update
    /// \param value The new value of the variable
    void insert(char const* name, char const* value);
#endif /* PLATFORMSTL_ENVVAR_SET_SUPPORTED */
#ifdef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
    /// \brief Removes the entry of the given name
    ///
    /// \note If the given entry does not exist
    size_type erase(first_type const& name);

    /// \brief Removes the entry of the given name
    ///
    /// \note If the given entry does not exist
    size_type erase(char const* name);

    /// \brief Removes the entry corresponding to the given iterator
    void erase(const_iterator it);
#endif /* PLATFORMSTL_ENVVAR_ERASE_SUPPORTED */
/// @}

/// \name Iteration
/// @{
public:
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    /// \brief Begins the iteration
    ///
    /// \return A non-mutating (const) iterator representing the start of the sequence
    const_iterator  begin() const;
    /// \brief Ends the iteration
    ///
    /// \return A non-mutating (const) iterator representing (one past) the end of the sequence
    const_iterator  end() const;
# if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT) && \
     !defined(STLSOFT_COMPILER_IS_BORLAND)
    /// \brief Begins the reverse iteration
    ///
    /// \return A non-mutating (const) iterator representing the start of the reverse sequence
    const_reverse_iterator  rbegin() const;
    /// \brief Ends the reverse iteration
    ///
    /// \return A non-mutating (const) iterator representing (one past) the end of the reverse sequence
    const_reverse_iterator  rend() const;
# endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */
/// @}

/// \name Iteration
/// @{
public:
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    /// \brief const_iterator class
    ///
    /// \note Even though the const_iterator class, in and of itself, supports Invalidatable
    /// References, the collection as a whole supports only By-Value Temporary (BVT) References
    /// because that is the highest model that the subscript operations can support. (See XSTL
    /// for details.)
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<stlsoft_ns_qual_std(bidirectional_iterator_tag)
                                            ,   value_type
                                            ,   ss_ptrdiff_t
                                            ,   void                // By-Value Temporary reference
                                            ,   const value_type    // By-Value Temporary reference
                                            >
    {
    /// \name Member Types
    /// @{
    public:
        typedef const_iterator  class_type;
    /// @}

    /// \name Construction
    /// @{
    private:
        friend class environment_map;
        const_iterator(snapshot::iterator it, snapshot::ref_type snapshot);
    public:
        const_iterator();
        const_iterator(class_type const& rhs);
    /// @}

    /// \name Forward Iterator Operations
    /// @{
    public:
        class_type& operator ++();
        class_type operator ++(int);
        const_reference operator *() const;
    /// @}

    /// \name BiDirectional Iterator Operations
    /// @{
    public:
        class_type& operator --();
        class_type operator --(int);
    /// @}

    /// \name Comparison
    /// @{
    public:
        ss_bool_t equal(class_type const& rhs) const;
    /// @}

    /// \name Members
    /// @{
    private:
        snapshot::iterator     m_it;
        snapshot::ref_type     m_snapshot;
    /// @}
    };
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */
/// @}

/// \name Implementation
/// @{
private:
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    void check_refresh_snapshot_() const;
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */
/// @}

/// \name Members
/// @{
private:
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    mutable snapshot::ref_type     m_snapshot;
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */
/// @}

/// \name Not to be defined
/// @{
private:
    environment_map(environment_map const& rhs);
    environment_map& operator =(environment_map const& rhs);
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

// NOTE: Neither Borland (5.6+) nor DMC++ correctly handle ADL for (comparison)
//       operators of types within a namespace, so we temporarily skip out into
//       the global namespace.

#if (   defined(STLSOFT_COMPILER_IS_BORLAND) && \
        __BORLANDC__ >= 0x0560) || \
    defined(STLSOFT_COMPILER_IS_DMC)

# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace platformstl
# else
} // namespace platformstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */

#endif /* compiler */

inline stlsoft_ns_qual(ss_bool_t) operator ==(  platformstl_ns_qual(environment_map)::const_iterator const&  lhs
                                            ,   platformstl_ns_qual(environment_map)::const_iterator const&  rhs)
{
    return lhs.equal(rhs);
}
inline stlsoft_ns_qual(ss_bool_t) operator !=(  platformstl_ns_qual(environment_map)::const_iterator const&  lhs
                                            ,   platformstl_ns_qual(environment_map)::const_iterator const&  rhs)
{
    return !lhs.equal(rhs);
}

// TODO: Make this a discriminated feature declared in the cccap files
#if (   defined(STLSOFT_COMPILER_IS_BORLAND) && \
        __BORLANDC__ >= 0x0560) || \
    defined(STLSOFT_COMPILER_IS_DMC)

# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::platformstl */
namespace platformstl
{
# else
/* Define stlsoft::platformstl_project */

namespace stlsoft
{

namespace platformstl_project
{
# endif /* _STLSOFT_NO_NAMESPACE */

#endif /* compiler */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/environment_map_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// environment_map::const_iterator

#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
inline environment_map::const_iterator::const_iterator()
    : m_it()
{}

inline environment_map::const_iterator::const_iterator(environment_map::snapshot::iterator it, environment_map::snapshot::ref_type snapshot)
    : m_it(it)
    , m_snapshot(snapshot)
{}

inline environment_map::const_iterator::const_iterator(environment_map::const_iterator::class_type const& rhs)
    : m_it(rhs.m_it)
    , m_snapshot(rhs.m_snapshot)
{}

inline environment_map::const_iterator::class_type& environment_map::const_iterator::operator ++()
{
    ++m_it;

    return *this;
}

inline environment_map::const_iterator::class_type environment_map::const_iterator::operator ++(int)
{
    class_type  r(*this);

    operator ++();

    return r;
}

inline environment_map::const_reference environment_map::const_iterator::operator *() const
{
    return *m_it;
}

inline environment_map::const_iterator::class_type& environment_map::const_iterator::operator --()
{
    --m_it;

    return *this;
}

inline environment_map::const_iterator::class_type environment_map::const_iterator::operator --(int)
{
    class_type  r(*this);

    operator --();

    return r;
}

inline ss_bool_t environment_map::const_iterator::equal(environment_map::const_iterator::class_type const& rhs) const
{
    return m_it == rhs.m_it;
}
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

// environment_map

inline environment_map::environment_map()
{}

inline environment_map::second_type environment_map::operator [](char const* name) const
{
    char const  *value  =   traits_type::get_variable(name);

    if(NULL == value)
    {
        STLSOFT_THROW_X(stlsoft_ns_qual_std(out_of_range)("variable does not exist"));
    }

    return value;
}

inline environment_map::second_type environment_map::operator [](environment_map::first_type const& name) const
{
    return operator [](name.c_str());
}

inline ss_bool_t environment_map::lookup(char const* name, environment_map::second_type& value) const
{
    char const  *value_ =   traits_type::get_variable(name);

    return (NULL == value_) ? false : (value = value_, true);
}

inline ss_bool_t environment_map::lookup(environment_map::first_type const& name, environment_map::second_type& value) const
{
    return lookup(name.c_str(), value);
}

inline void environment_map::refresh()
{
#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
    m_snapshot.close();
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */
}

#ifdef PLATFORMSTL_ENVVAR_SET_SUPPORTED
inline void environment_map::insert(environment_map::first_type const& name, environment_map::second_type const& value)
{
    // Preconditions
    STLSOFT_MESSAGE_ASSERT("Name may not be empty", !name.empty());
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", name.end() == std::find(name.begin(), name.end(), '='));
    STLSOFT_MESSAGE_ASSERT("Empty value not allowed in insertion", !value.empty());

    second_type *pstr   =   NULL;

    if( 1 < m_snapshot.use_count() &&
        m_snapshot->lookup(name, pstr))
    {
        // If it exists, then:

        // 1. Reserve the appropriate amount of storage (which may throw,
        // but doesn't need to be rolled back). If this succeeds, then
        // it means that the insert() call cannot throw an exception.
        pstr->reserve(value.size());

        // 2. Insert into the host environment
        if(0 != traits_type::set_variable(name.c_str(), value.c_str()))
        {
            STLSOFT_THROW_X(stlsoft_ns_qual_std(runtime_error)("Cannot set environment variable"));
        }

        // 3. Update the snapshot
        m_snapshot->set(name, value);
    }
    else
    {
        // If it does not exist, then we add it first, and remove
        // again if the set_variable() call fails to also put it
        // in the host environment
        if(1 < m_snapshot.use_count())
        {
            m_snapshot->insert(name, value);
        }

        if(0 != traits_type::set_variable(name.c_str(), value.c_str()))
        {
            if(1 < m_snapshot.use_count())
            {
                m_snapshot->erase(name);
            }

            STLSOFT_THROW_X(stlsoft_ns_qual_std(runtime_error)("Cannot set environment variable"));
        }
    }
}

inline void environment_map::insert(char const* name, char const* value)
{
    // Preconditions
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_MESSAGE_ASSERT("Name may not be empty", 0 != ::strlen(name));
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", NULL == ::strchr(name, '='));
    STLSOFT_MESSAGE_ASSERT("Null value not allowed in insertion", NULL != value);
    STLSOFT_MESSAGE_ASSERT("Empty value not allowed in insertion", 0 != ::strlen(value));

    insert(first_type(name), second_type(value));
}
#endif /* PLATFORMSTL_ENVVAR_SET_SUPPORTED */

#ifdef PLATFORMSTL_ENVVAR_ERASE_SUPPORTED
inline environment_map::size_type environment_map::erase(environment_map::first_type const& name)
{
    // Preconditions
    STLSOFT_MESSAGE_ASSERT("Name may not be empty", !name.empty());
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", name.end() == std::find(name.begin(), name.end(), '='));

    size_type   b   =   0;

    if(0 != traits_type::erase_variable(name.c_str()))
    {
#if 0
        // Failure to erase might be if some external part of the
        // process has already erased it.
        //
        // Hence, the somewhat crude measure to checking whether it
        // still exists (rather than knowing what value(s) to check
        // the return value of erase_variable() for).

        if(NULL != traits_type::get_variable(name.c_str()))
#endif /* 0 */
        {
            STLSOFT_THROW_X(stlsoft_ns_qual_std(runtime_error)("Cannot erase environment variable"));
        }
    }
    else
    {
        b = 1;
    }

    if(1 < m_snapshot.use_count())
    {
        if(m_snapshot->erase(name))
        {
            b = 1;
        }
    }

    return b;
}

inline environment_map::size_type environment_map::erase(char const* name)
{
    // Preconditions
    STLSOFT_ASSERT(NULL != name);
    STLSOFT_MESSAGE_ASSERT("Name may not be empty", 0 != ::strlen(name));
    STLSOFT_MESSAGE_ASSERT("Name may not contain '='", NULL == ::strchr(name, '='));

    return erase(first_type(name));
}

inline void environment_map::erase(environment_map::const_iterator it)
{
    STLSOFT_MESSAGE_ASSERT("No snapshot assigned, so erase() is inappropriate; maybe premature call to clear()", 1 < m_snapshot.use_count());

#if 0
    first_type const    &name   =   (*it).first;
#else /* ? 0 */
    first_type const    &name   =   (*it.m_it).first;   // Avoid CUR
#endif /* 0 */

    if(0 != traits_type::erase_variable(name.c_str()))
    {
#if 0
        // Failure to erase might be if some external part of the
        // process has already erased it.
        //
        // Hence, the somewhat crude measure to checking whether it
        // still exists (rather than knowing what value(s) to check
        // the return value of erase_variable() for).

        if(NULL != traits_type::get_variable(name.c_str()))
#endif /* 0 */
        {
            STLSOFT_THROW_X(stlsoft_ns_qual_std(runtime_error)("Cannot erase environment variable"));
        }
    }

    m_snapshot->erase(it.m_it);
}
#endif /* PLATFORMSTL_ENVVAR_ERASE_SUPPORTED */

#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
inline environment_map::const_iterator environment_map::begin() const
{
    check_refresh_snapshot_();

#if 0
    snapshot::ref_type     snapshot =   m_snapshot;  // Avoid a const_cast

    return const_iterator(snapshot->begin(), m_snapshot);
#else /* ? 0 */
    return const_iterator(m_snapshot->begin(), m_snapshot);
#endif /* 0 */
}

inline environment_map::const_iterator environment_map::end() const
{
    check_refresh_snapshot_();

#if 0
    snapshot::ref_type     snapshot =   m_snapshot;  // Avoid a const_cast

    return const_iterator(snapshot->end(), m_snapshot);
#else /* ? 0 */
    return const_iterator(m_snapshot->end(), m_snapshot);
#endif /* 0 */
}

# if defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT) && \
     !defined(STLSOFT_COMPILER_IS_BORLAND)
inline environment_map::const_reverse_iterator environment_map::rbegin() const
{
    return const_reverse_iterator(end());
}

inline environment_map::const_reverse_iterator environment_map::rend() const
{
    return const_reverse_iterator(begin());
}
# endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

#ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON
inline void environment_map::check_refresh_snapshot_() const
{
    if(m_snapshot.use_count() < 2)
    {
        m_snapshot = snapshot::ref_type(new snapshot());
    }
}
#endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

// environment_map::snapshot

# ifdef PLATFORMSTL_ENVVAR_HAS_ENVIRON

inline environment_map::snapshot::snapshot()
{
    first_type  name;
    second_type value;

    stlsoft::scoped_handle<char const**>    env(    traits_type::get_environ()
                                                ,   &traits_type::release_environ);

    { for(char const** p = env.get(); NULL != *p; ++p)
    {
        stlsoft::split(*p, '=', name, value);

        m_variables[name] = value;
    }}
}

inline environment_map::snapshot::iterator environment_map::snapshot::begin()
{
    return m_variables.begin();
}

inline environment_map::snapshot::iterator environment_map::snapshot::end()
{
    return m_variables.end();
}

inline ss_bool_t environment_map::snapshot::erase(first_type const& name) throw()
{
    variables_type_::iterator it  =   m_variables.find(name);

    if(m_variables.end() != it)
    {
        m_variables.erase(it);

        return true;
    }

    return false;
}

inline void environment_map::snapshot::erase(environment_map::snapshot::iterator it) throw()
{
    m_variables.erase(it);
}

inline void environment_map::snapshot::insert(first_type const& name, second_type const& value)
{
#  if 0
    /// This is not strongly exception safe, ...
    m_variables[name] = value;
#  else /* ? 0 */
    /// ... but this is.
    m_variables.insert(value_type(name, value));
#  endif /* 0 */
}

inline void environment_map::snapshot::set(first_type const& name, second_type const& value)
{
    variables_type_::iterator it  =   m_variables.find(name);

    STLSOFT_ASSERT(m_variables.end() != it);
    STLSOFT_ASSERT((*it).second.capacity() >= value.size());

#  ifdef _DEBUG
    try
#  endif /* _DEBUG */
    {
        (*it).second.assign(value);
    }
#  ifdef _DEBUG
    catch(...)
    {
        STLSOFT_MESSAGE_ASSERT("Should never happen", 0);

        throw; // Might as well pass on to precipitate unexpected()
    }
#  endif /* _DEBUG */

}

inline ss_bool_t environment_map::snapshot::lookup(first_type const& name, second_type *&pvalue) throw()
{
    variables_type_::iterator it  =   m_variables.find(name);

    if(m_variables.end() == it)
    {
        return false;
    }
    else
    {
        pvalue = &(*it).second;

        return true;
    }
}

# endif /* PLATFORMSTL_ENVVAR_HAS_ENVIRON */

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

#endif /* !PLATFORMSTL_INCL_PLATFORMSTL_SYSTEM_HPP_ENVIRONMENT_MAP */

/* ///////////////////////////// end of file //////////////////////////// */
