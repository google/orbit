/* /////////////////////////////////////////////////////////////////////////
 * File:        mfcstl/collections/cstring_adaptors.hpp
 *
 * Purpose:     Contains the definition of the CString_cadaptor and CString_iadaptor
 *              class templates.
 *
 * Created:     1st October 2002
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


/** \file mfcstl/collections/cstring_adaptors.hpp
 *
 * \brief [C++ only] Definition of the mfcstl::CString_cadaptor and
 *   mfcstl::CString_iadaptor class templates
 *   (\ref group__library__collections "Collections" Library).
 */

#ifndef MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS
#define MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS_MAJOR       4
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS_MINOR       1
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS_REVISION    1
# define MFCSTL_VER_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS_EDIT        89
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC: _MSC_VER<1100
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef MFCSTL_INCL_MFCSTL_HPP_MFCSTL
# include <mfcstl/mfcstl.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_HPP_MFCSTL */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1100
# error mfcstl_cstring_veneer.h is not compatible with Visual C++ 4.2 or earlier
#endif /* compiler */

#ifndef MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR
# include <mfcstl/memory/afx_allocator.hpp>
#endif /* !MFCSTL_INCL_MFCSTL_MEMORY_HPP_AFX_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS
# include <stlsoft/util/constraints.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_CONSTRAINTS */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR
# include <stlsoft/util/std/library_discriminator.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_LIBRARY_DISCRIMINATOR */
#ifndef STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD
# include <stlsoft/string/string_traits_fwd.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_STRING_HPP_STRING_TRAITS_FWD */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
/* Define stlsoft::mfcstl_project */

namespace stlsoft
{

namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Applies standard string (std::basic_string) interface to the CString class
 *
 * \ingroup group__library__collections
 *
 * This class adapts the MFC CString type to express a standard String-like interface
 *
 * \ingroup concepts_veneer
 */
template<ss_typename_param_k I>
class CString_adaptor_base
    : public stlsoft_ns_qual(stl_collection_tag)
{
/// \name Member Types
/// @{
private:
    typedef I                                               interface_type;
    typedef CString_adaptor_base<I>                         class_type;
public:
    /// The value type
    typedef TCHAR                                           value_type;
    /// The allocator type
    typedef afx_allocator<value_type>                       allocator_type;
    /// The pointer type
    typedef LPTSTR                                          pointer;
    /// The pointer-to-const type
    typedef LPCTSTR                                         const_pointer;
    /// The reference type
    typedef TCHAR&                                          reference;
    /// The reference-to-const type
    typedef TCHAR const&                                    const_reference;
    /// The size type
    typedef ms_size_t                                       size_type;
    /// The difference type
    typedef ms_ptrdiff_t                                    difference_type;
#if !defined(STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT)
    /// The mutating iterator type
    typedef LPTSTR                                          iterator;
    /// The non-mutating (const) iterator type
    typedef LPCTSTR                                         const_iterator;
#else /* ? !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
           ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type
                                        ,   pointer
                                        ,   reference
                                        >::type             iterator;
    /// The non-mutating (const) iterator type
    typedef
# if !defined(STLSOFT_COMPILER_IS_BORLAND)
         ss_typename_type_k
# endif /* compiler */
                       pointer_iterator <   value_type const
                                        ,   const_pointer
                                        ,   const_reference
                                        >::type             const_iterator;

    /// The mutating (non-const) reverse iterator type
    typedef reverse_iterator_base       <   iterator
                                        ,   value_type
                                        ,   reference
                                        ,   pointer
                                        ,   difference_type
                                        >                   reverse_iterator;

    /// The non-mutating (const) reverse iterator type
    typedef const_reverse_iterator_base <   const_iterator
                                        ,   value_type const
                                        ,   const_reference
                                        ,   const_pointer
                                        ,   difference_type
                                        >                   const_reverse_iterator;
#endif /* !STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
    /// The Boolean type
    typedef ms_bool_t                                       bool_type;
/// @}

/// \name Construction
/// @{
public:
    allocator_type  get_allocator() const;
/// @}

/// \name Underlying container
/// @{
public:
    /// \brief Returns a mutating (non-const) reference to the underlying CString
    CString         &get_CString()
    {
        return static_cast<interface_type*>(this)->get_actual_string();
    }
    /// \brief Returns a non-mutating (const) reference to the underlying CString
    CString const   &get_CString() const
    {
        return static_cast<interface_type const*>(this)->get_actual_string();
    }
/// @}

/// \name Assignment
/// @{
public:
    /// \brief Replaces the string contents with \c s
    class_type  &assign(LPCSTR s);
    /// \brief Replaces the string contents with \c s
    class_type  &assign(LPCWSTR s);
    /// \brief Replaces the string contents with \c s
    class_type  &assign(unsigned char const* s);
    /// \brief Replaces the string contents with the first \c n characters from \c s
    class_type  &assign(LPCSTR s, size_type n);
    /// \brief Replaces the string contents with the first \c n characters from \c s
    class_type  &assign(LPCWSTR s, size_type n);
    /// \brief Replaces the string contents with the first \c n characters from \c s
    class_type  &assign(unsigned char const* s, size_type n);
    /// \brief Replaces the string contents with \c s
    class_type  &assign(class_type const& s);
    /// \brief Replaces the string contents with \c n characters of \c s, starting from offset \c pos
    class_type  &assign(class_type const& str, size_type pos, size_type n);
    /// \brief Replaces the string contents with \c n elements of \c ch
    class_type  &assign(size_type n, value_type ch);
    /// \brief Replaces the string contents with the contents of the range [first, last)
#if defined(STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0
# pragma message("TODO: Fix this up before 1.9.1 proper")
    class_type  &assign(LPCTSTR first, LPCTSTR last);
#else /* ? library */
    class_type  &assign(const_iterator first, const_iterator last);
#endif /* library */
/// @}

/// \name Iteration
/// @{
public:
    /// Begins the iteration
    ///
    /// \return A mutating (non-const) iterator representing the start of the sequence
    iterator                begin();
    /// Ends the iteration
    ///
    /// \return A mutating (non-const) iterator representing the end of the sequence
    iterator                end();
    /// Begins the iteration
    ///
    /// \return A non-mutating (const) iterator representing the start of the sequence
    const_iterator          begin() const;
    /// Ends the iteration
    ///
    /// \return A non-mutating (const) iterator representing the end of the sequence
    const_iterator          end() const;
#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
    /// Begins the iteration
    ///
    /// \return A mutating (non-const) reverse iterator representing the start of the sequence
    reverse_iterator        rbegin();
    /// Ends the iteration
    ///
    /// \return A mutating (non-const) reverse iterator representing the end of the sequence
    reverse_iterator        rend();
    /// Begins the iteration
    ///
    /// \return A non-mutating (const) reverse iterator representing the start of the sequence
    const_reverse_iterator  rbegin() const;
    /// Ends the iteration
    ///
    /// \return A non-mutating (const) reverse iterator representing the end of the sequence
    const_reverse_iterator  rend() const;
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */
/// @}

/// \name Element Access
/// @{
protected:
    reference       subscript_(size_type index);
    const_reference subscript_(size_type index) const;
public:
    /// Returns a mutable (non-const) reference to the element at \c index
    ///
    /// \note Throws an instance of std::out_of_range if index >= size()
    reference       at(size_type index);
    /// Returns a non-mutable (const) reference to the element at \c index
    ///
    /// \note Throws an instance of std::out_of_range if index >= size()
    const_reference at(size_type index) const;
/// @}

/// \name Attributes
/// @{
public:
    /// Returns the number of elements in the sequence
    size_type       length() const;
    /// Returns the number of elements in the sequence
    size_type       size() const;
    /// \brief Indicates whether the sequence is empty
    bool_type       empty() const;
    /// Returns a pointer to constant data representing the managed string
    const_pointer   c_str() const;
    /// Returns a possibly unterminated pointer to constant data representing the managed string
    const_pointer   data() const;
/// @}
};

/** \brief Adaptor class, representing a Class Adaptor over the MFC CString type
 *
 * \ingroup group__library__collections
 *
 * It can be used wherever a CString is, or it can be used via the std::basic_string-like interface
 */
class CString_cadaptor
    : public CString
    , public CString_adaptor_base<CString_cadaptor>
{
/// \name Member Types
/// @{
private:
    typedef CString_adaptor_base<CString_cadaptor>  parent_class_type;
public:
    /// The class type
    typedef CString_cadaptor                        class_type;
/// @}

public:
    /// Default constructor
    CString_cadaptor();
    /// Copy constructor
    CString_cadaptor(class_type const& rhs);
    /// Copy constructor
    ss_explicit_k CString_cadaptor(CString const& rhs);
    /// Construct from an ANSI string
    ss_explicit_k CString_cadaptor(LPCSTR s);
    /// Construct from a Unicode string
    ss_explicit_k CString_cadaptor(LPCWSTR s);
    /// Construct from an MBCS string
    ss_explicit_k CString_cadaptor(unsigned char const* s);
    /// Construct from a range
    CString_cadaptor(LPCTSTR from, LPCTSTR to);
    /// Construct from a range
    CString_cadaptor(LPCTSTR from, size_type length);
    /// \brief Constructs from str the range identified by [pos, pos + n)
    CString_cadaptor(class_type const& str, size_type pos, size_type n);
    ///
    CString_cadaptor(ms_size_t cch, TCHAR ch);

    /// Copy assignment operator
    class_type const& operator =(class_type const& rhs);
    /// Copy assignment operator
    class_type const& operator =(CString const& rhs);
    /// Assignment operator
    class_type const& operator =(LPCSTR s);
    /// Assignment operator
    class_type const& operator =(LPCWSTR s);
    /// Assignment operator
    class_type const& operator =(unsigned char const* s);

private:
    friend class CString_adaptor_base<CString_cadaptor>;

    CString         &get_actual_string()
    {
        return *this;
    }
    CString const   &get_actual_string() const
    {
        return *this;
    }
};

/** \brief Adaptor class, representing an Instance Adaptor over the MFC CString type
 *
 * \ingroup group__library__collections
 *
 * It can be used via the std::basic_string-like interface
 */
class CString_iadaptor
    : public CString_adaptor_base<CString_iadaptor>
{
/// \name Member Types
/// @{
private:
    typedef CString_adaptor_base<CString_iadaptor>  parent_class_type;
public:
    /// The class type
    typedef CString_iadaptor                        class_type;
/// @}

/// \name Construction
/// @{
public:
    CString_iadaptor(CString &str);
    CString_iadaptor(CString *str);
/// @}

    /// Copy assignment operator
    class_type const& operator =(class_type const& rhs);
    /// Copy assignment operator
    class_type const& operator =(CString const& rhs);
    /// Assignment operator
    class_type const& operator =(LPCSTR s);
    /// Assignment operator
    class_type const& operator =(LPCWSTR s);
    /// Assignment operator
    class_type const& operator =(unsigned char const* s);

/// \name Element Access
/// @{
public:
    /// Returns a mutable (non-const) reference to the element at \c index
    ///
    /// \note The behaviour is undefined if index >= size()
    reference       operator [](size_type index);
    /// Returns a non-mutable (const) reference to the element at \c index
    ///
    /// \note The behaviour is undefined if index >= size()
    const_reference operator [](size_type index) const;
/// @}

/// \name Implementation
/// @{
private:
    friend class CString_adaptor_base<CString_iadaptor>;

    CString         &get_actual_string()
    {
        return *m_str;
    }
    CString const   &get_actual_string() const
    {
        return *m_str;
    }
/// @}

/// \name Members
/// @{
private:
    CString *m_str;
/// @}
};

/** \brief Non-mutable Adaptor class, representing an Instance Adaptor over the MFC CString type
 *
 * \ingroup group__library__collections
 *
 * It can be used via the std::basic_string-like interface
 */
class const_CString_iadaptor
    : public CString_adaptor_base<const_CString_iadaptor>
{
/// \name Member Types
/// @{
private:
    typedef CString_adaptor_base<const_CString_iadaptor>    parent_class_type;
public:
    /// The class type
    typedef CString_iadaptor                                class_type;
/// @}

/// \name Construction
/// @{
public:
    const_CString_iadaptor(CString const& str);
    const_CString_iadaptor(CString const* str);
/// @}

/// \name Element Access
/// @{
public:
    /// Returns a non-mutable (const) reference to the element at \c index
    ///
    /// \note The behaviour is undefined if index >= size()
    const_reference operator [](size_type index) const;
/// @}

/// \name Implementation
/// @{
private:
    friend class CString_adaptor_base<const_CString_iadaptor>;

    CString const   &get_actual_string() const
    {
        return *m_str;
    }
/// @}

/// \name Members
/// @{
private:
    CString const   *m_str;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Operators
 */

// operator ==

inline ms_bool_t operator ==(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}

inline ms_bool_t operator ==(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}
inline ms_bool_t operator ==(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}

inline ms_bool_t operator ==(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() == rhs.get_CString();
}
inline ms_bool_t operator ==(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() == rhs;
}
inline ms_bool_t operator ==(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs == rhs.get_CString();
}
inline ms_bool_t operator ==(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}
inline ms_bool_t operator ==(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() == CString(rhs);
}
inline ms_bool_t operator ==(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) == rhs.get_CString();
}

// operator !=

inline ms_bool_t operator !=(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}

inline ms_bool_t operator !=(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}
inline ms_bool_t operator !=(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}

inline ms_bool_t operator !=(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() != rhs.get_CString();
}
inline ms_bool_t operator !=(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() != rhs;
}
inline ms_bool_t operator !=(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs != rhs.get_CString();
}
inline ms_bool_t operator !=(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}
inline ms_bool_t operator !=(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() != CString(rhs);
}
inline ms_bool_t operator !=(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) != rhs.get_CString();
}

// operator <

inline ms_bool_t operator <(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() < rhs.get_CString();
}
inline ms_bool_t operator <(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}
inline ms_bool_t operator <(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}

inline ms_bool_t operator <(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() < rhs.get_CString();
}
inline ms_bool_t operator <(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}
inline ms_bool_t operator <(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}

inline ms_bool_t operator <(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() < rhs.get_CString();
}
inline ms_bool_t operator <(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() < rhs;
}
inline ms_bool_t operator <(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs < rhs.get_CString();
}
inline ms_bool_t operator <(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}
inline ms_bool_t operator <(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() < CString(rhs);
}
inline ms_bool_t operator <(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) < rhs.get_CString();
}

// operator >

inline ms_bool_t operator >(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() > rhs.get_CString();
}
inline ms_bool_t operator >(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}
inline ms_bool_t operator >(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}

inline ms_bool_t operator >(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() > rhs.get_CString();
}
inline ms_bool_t operator >(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}
inline ms_bool_t operator >(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}

inline ms_bool_t operator >(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() > rhs.get_CString();
}
inline ms_bool_t operator >(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() > rhs;
}
inline ms_bool_t operator >(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs > rhs.get_CString();
}
inline ms_bool_t operator >(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}
inline ms_bool_t operator >(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() > CString(rhs);
}
inline ms_bool_t operator >(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) > rhs.get_CString();
}

// operator <=

inline ms_bool_t operator <=(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}

inline ms_bool_t operator <=(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}
inline ms_bool_t operator <=(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}

inline ms_bool_t operator <=(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() <= rhs.get_CString();
}
inline ms_bool_t operator <=(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() <= rhs;
}
inline ms_bool_t operator <=(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs <= rhs.get_CString();
}
inline ms_bool_t operator <=(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}
inline ms_bool_t operator <=(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() <= CString(rhs);
}
inline ms_bool_t operator <=(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) <= rhs.get_CString();
}

// operator >=

inline ms_bool_t operator >=(CString_cadaptor const& lhs, CString_cadaptor const& rhs)
{
    return lhs.get_CString() >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_cadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(CString const& lhs, CString_cadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_cadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(LPCSTR lhs, CString_cadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_cadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(LPCWSTR lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_cadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(unsigned char const* lhs, CString_cadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}

inline ms_bool_t operator >=(CString_iadaptor const& lhs, CString_iadaptor const& rhs)
{
    return lhs.get_CString() >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(CString const& lhs, CString_iadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(LPCSTR lhs, CString_iadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(LPCWSTR lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}
inline ms_bool_t operator >=(CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(unsigned char const* lhs, CString_iadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}

inline ms_bool_t operator >=(const_CString_iadaptor const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs.get_CString() >= rhs.get_CString();
}
inline ms_bool_t operator >=(const_CString_iadaptor const& lhs, CString const& rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(CString const& lhs, const_CString_iadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(const_CString_iadaptor const& lhs, LPCSTR rhs)
{
    return lhs.get_CString() >= rhs;
}
inline ms_bool_t operator >=(LPCSTR lhs, const_CString_iadaptor const& rhs)
{
    return lhs >= rhs.get_CString();
}
inline ms_bool_t operator >=(const_CString_iadaptor const& lhs, LPCWSTR rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(LPCWSTR lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}
inline ms_bool_t operator >=(const_CString_iadaptor const& lhs, unsigned char const* rhs)
{
    return lhs.get_CString() >= CString(rhs);
}
inline ms_bool_t operator >=(unsigned char const* lhs, const_CString_iadaptor const& rhs)
{
    return CString(lhs) >= rhs.get_CString();
}

////////////////////////////////////////////////////////////////////////////
// string traits

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/** Specialisation for mfcstl::CString_cadaptor
 */
STLSOFT_TEMPLATE_SPECIALISATION
struct string_traits<mfcstl_ns_qual(CString_cadaptor)>
{
    typedef mfcstl_ns_qual(CString_cadaptor)    value_type;
    typedef TCHAR                               char_type;
    typedef ss_size_t                           size_type;
    typedef char_type const                     const_char_type;
    typedef value_type                          string_type;
    typedef LPCSTR                              pointer;
    typedef LPCTSTR                             const_pointer;
    typedef LPCSTR                              iterator;
    typedef LPCTSTR                             const_iterator;
    typedef value_type::reverse_iterator        reverse_iterator;
    typedef value_type::const_reverse_iterator  const_reverse_iterator;
    enum
    {
            is_pointer          =   false
        ,   is_pointer_to_const =   false
        ,   char_type_size      =   sizeof(char_type)
    };

    static string_type empty_string()
    {
        return string_type();
    }
    static string_type construct(string_type const& src, size_type pos, size_type len)
    {
        return string_type(src, pos, len);
    }
    static string_type &assign_inplace(string_type &str, const_iterator first, const_iterator last)
    {
        return (str = string_type(first, last), str);
    }
};

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
/* There is no stlsoft namespace, so must define ::mfcstl */
namespace mfcstl
{
# else
namespace mfcstl_project
{

# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/cstring_adaptors_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// CString_adaptor_base

template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::allocator_type CString_adaptor_base<I>::get_allocator() const
{
    return allocator_type();
}

template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(LPCSTR s)
{
    get_CString() = s;

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(LPCWSTR s)
{
    get_CString() = s;

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(unsigned char const* s)
{
    get_CString() = s;

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(LPCSTR s, size_type n)
{
#ifdef UNICODE
    get_CString() = CString(s).Left(n);
#else /* ? UNICODE */
    get_CString() = CString(s, n);
#endif /* UNICODE */

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(LPCWSTR s, size_type n)
{
#ifdef UNICODE
    get_CString() = CString(s, n);
#else /* ? UNICODE */
    get_CString() = CString(s).Left(n);
#endif /* UNICODE */

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(unsigned char const* s, size_type n)
{
    get_CString() = CString(s).Left(n);

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(class_type const& s)
{
    get_CString() = s.get_CString();

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(class_type const& s, size_type pos, size_type index)
{
    MFCSTL_MESSAGE_ASSERT("invalid index", index + pos <= s.size());

    get_CString() = CString(s.c_str() + pos, index);

    return *this;
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(size_type n, value_type ch)
{
    get_CString() = CString(ch, static_cast<int>(n));

    return *this;
}

template<ss_typename_param_k I>
#if defined(STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION) && \
    STLSOFT_CF_STD_LIBRARY_DINKUMWARE_VC_VERSION == STLSOFT_CF_DINKUMWARE_VC_VERSION_7_0
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(LPCTSTR first, LPCTSTR last)
#else /* ? library */
inline ss_typename_type_ret_k CString_adaptor_base<I>::class_type& CString_adaptor_base<I>::assign(const_iterator first, const_iterator last)
#endif /* library */
{
    get_CString() = CString(first, static_cast<int>(last - first));

    return *this;
}

template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::iterator CString_adaptor_base<I>::begin()
{
    return const_cast<pointer>(this->c_str());
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::iterator CString_adaptor_base<I>::end()
{
    return this->begin() + this->size();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_iterator CString_adaptor_base<I>::begin() const
{
    return this->c_str();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_iterator CString_adaptor_base<I>::end() const
{
    return this->begin() + this->size();
}

#ifdef STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::reverse_iterator CString_adaptor_base<I>::rbegin()
{
    return reverse_iterator(end());
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::reverse_iterator CString_adaptor_base<I>::rend()
{
    return reverse_iterator(begin());
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_reverse_iterator  CString_adaptor_base<I>::rbegin() const
{
    return const_reverse_iterator(end());
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_reverse_iterator CString_adaptor_base<I>::rend() const
{
    return const_reverse_iterator(begin());
}
#endif /* STLSOFT_LF_BIDIRECTIONAL_ITERATOR_SUPPORT */



template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::reference CString_adaptor_base<I>::subscript_(size_type index)
{
    MFCSTL_MESSAGE_ASSERT("invalid index", index < size());

    return const_cast<pointer>(this->data())[index];
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_reference CString_adaptor_base<I>::subscript_(size_type index) const
{
    MFCSTL_MESSAGE_ASSERT("invalid index", index < size());

    return this->data()[index];
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::reference CString_adaptor_base<I>::at(size_type index)
{
    if(index >= size())
    {
        STLSOFT_THROW_X(mfcstl_ns_qual_std(out_of_range)("invalid index"));
    }

    return subscript_(index);
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_reference CString_adaptor_base<I>::at(size_type index) const
{
    if(index >= size())
    {
        STLSOFT_THROW_X(mfcstl_ns_qual_std(out_of_range)("invalid index"));
    }

    return subscript_(index);
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::size_type CString_adaptor_base<I>::length() const
{
    return get_CString().GetLength();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::size_type CString_adaptor_base<I>::size() const
{
    return this->length();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::bool_type CString_adaptor_base<I>::empty() const
{
    return 0 == this->length();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_pointer CString_adaptor_base<I>::c_str() const
{
//      return empty() ? _T("") : static_cast<const_pointer>(get_CString());
    return get_CString();
}
template<ss_typename_param_k I>
inline ss_typename_type_ret_k CString_adaptor_base<I>::const_pointer CString_adaptor_base<I>::data() const
{
    return this->c_str();
}


// CString_cadaptor

inline CString_cadaptor::CString_cadaptor()
{}
inline CString_cadaptor::CString_cadaptor(class_type const& rhs)
    : CString(rhs)
{}
inline /* ss_explicit_k */ CString_cadaptor::CString_cadaptor(CString const& rhs)
    : CString(rhs)
{}
inline /* ss_explicit_k */ CString_cadaptor::CString_cadaptor(LPCSTR s)
    : CString(s)
{}
inline /* ss_explicit_k */ CString_cadaptor::CString_cadaptor(LPCWSTR s)
    : CString(s)
{}
inline /* ss_explicit_k */ CString_cadaptor::CString_cadaptor(unsigned char const* s)
    : CString(s)
{}
inline CString_cadaptor::CString_cadaptor(LPCTSTR from, LPCTSTR to)
    : CString(from, static_cast<int>(to - from))
{}
inline CString_cadaptor::CString_cadaptor(LPCTSTR from, size_type length)
    : CString(from, length)
{}
inline CString_cadaptor::CString_cadaptor(class_type const& str, size_type pos, size_type n)
    : CString(static_cast<LPCTSTR>(str) + pos, n)
{}
inline CString_cadaptor::CString_cadaptor(ms_size_t cch, TCHAR ch)
    : CString(ch, static_cast<int>(cch))
{}
inline CString_cadaptor::class_type const& CString_cadaptor::operator =(CString_cadaptor::class_type const& rhs)
{
    assign(rhs);

    return *this;
}
inline CString_cadaptor::class_type const& CString_cadaptor::operator =(CString const& rhs)
{
    assign(rhs);

    return *this;
}
inline CString_cadaptor::class_type const& CString_cadaptor::operator =(LPCSTR rhs)
{
    assign(rhs);

    return *this;
}
inline CString_cadaptor::class_type const& CString_cadaptor::operator =(LPCWSTR rhs)
{
    assign(rhs);

    return *this;
}
inline CString_cadaptor::class_type const& CString_cadaptor::operator =(unsigned char const* rhs)
{
    assign(rhs);

    return *this;
}


// CString_iadaptor

inline CString_iadaptor::CString_iadaptor(CString &str)
    : m_str(&str)
{}
inline CString_iadaptor::CString_iadaptor(CString *str)
    : m_str(str)
{}
inline CString_iadaptor::class_type const& CString_iadaptor::operator =(CString_iadaptor::class_type const& rhs)
{
    assign(rhs);

    return *this;
}
inline CString_iadaptor::class_type const& CString_iadaptor::operator =(CString const& rhs)
{
    assign(rhs);

    return *this;
}
inline CString_iadaptor::class_type const& CString_iadaptor::operator =(LPCSTR rhs)
{
    assign(rhs);

    return *this;
}
inline CString_iadaptor::class_type const& CString_iadaptor::operator =(LPCWSTR rhs)
{
    assign(rhs);

    return *this;
}
inline CString_iadaptor::class_type const& CString_iadaptor::operator =(unsigned char const* rhs)
{
    assign(rhs);

    return *this;
}
inline CString_iadaptor::reference CString_iadaptor::operator [](CString_iadaptor::size_type index)
{
    return this->subscript_(index);
}
inline CString_iadaptor::const_reference CString_iadaptor::operator [](CString_iadaptor::size_type index) const
{
    return this->subscript_(index);
}


// const_CString_iadaptor

inline const_CString_iadaptor::const_CString_iadaptor(CString const& str)
    : m_str(&str)
{}
inline const_CString_iadaptor::const_CString_iadaptor(CString const* str)
    : m_str(str)
{}
inline const_CString_iadaptor::const_reference const_CString_iadaptor::operator [](const_CString_iadaptor::size_type index) const
{
    return this->subscript_(index);
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _MFCSTL_NO_NAMESPACE
# if defined(_STLSOFT_NO_NAMESPACE) || \
     defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
} // namespace mfcstl
# else
} // namespace mfcstl_project
} // namespace stlsoft
# endif /* _STLSOFT_NO_NAMESPACE */
#endif /* !_MFCSTL_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !MFCSTL_INCL_MFCSTL_COLLECTIONS_HPP_CSTRING_ADAPTORS */

/* ///////////////////////////// end of file //////////////////////////// */
