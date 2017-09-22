/* /////////////////////////////////////////////////////////////////////////
 * File:        winstl/system/version_info.hpp (originally in MWVerInf.h, ::SynesisWin)
 *
 * Purpose:     Helper for accessing version information.
 *
 * Created:     16th February 1998
 * Updated:     19th May 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2010, Matthew Wilson and Synesis Software
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


/** \file winstl/system/version_info.hpp
 *
 * \brief [C++ only] Definition of the winstl::version_info class
 *  template
 *   (\ref group__library__system "System" Library).
 */

#ifndef WINSTL_INCL_WINSTL_SYSTEM_HPP_VERSION_INFO
#define WINSTL_INCL_WINSTL_SYSTEM_HPP_VERSION_INFO

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define WINSTL_VER_WINSTL_SYSTEM_HPP_VERSION_INFO_MAJOR    5
# define WINSTL_VER_WINSTL_SYSTEM_HPP_VERSION_INFO_MINOR    2
# define WINSTL_VER_WINSTL_SYSTEM_HPP_VERSION_INFO_REVISION 8
# define WINSTL_VER_WINSTL_SYSTEM_HPP_VERSION_INFO_EDIT     126
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef WINSTL_INCL_WINSTL_H_WINSTL
# include <winstl/winstl.h>
#endif /* !WINSTL_INCL_WINSTL_H_WINSTL */
#ifndef WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS
# include <winstl/error/exceptions.hpp>
#endif /* !WINSTL_INCL_WINSTL_ERROR_HPP_WINDOWS_EXCEPTIONS */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# if defined(UNICODE) || \
     defined(_UNICODE)
#  error winstl::version_info is not supported on Visual C++ 5.0 (or previous) with UNICODE compilations
# endif /* Unicode */
# define WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER
#else /* ? compiler */
# ifndef WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER
#  include <winstl/filesystem/file_path_buffer.hpp>
# endif /* !WINSTL_INCL_WINSTL_FILESYSTEM_HPP_FILE_PATH_BUFFER */
#endif /* compiler */
#ifndef WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR
# include <winstl/memory/processheap_allocator.hpp>
#endif /* !WINSTL_INCL_WINSTL_MEMORY_HPP_PROCESSHEAP_ALLOCATOR */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */
#ifndef STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS
# include <stlsoft/collections/util/collections.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_COLLECTIONS_UTIL_HPP_COLLECTIONS */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
# include <stdexcept>                           // for std::exception
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */

#ifdef STLSOFT_UNITTEST
# include <stdio.h>
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
 * Structure headers
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

// The following bizarre construction is solely to avoid an ICE in VC6
// that only raises its ugly head in large, non-trivial, compilation
// contexts.

template<ss_typename_param_k T>
struct hdr
{
    template<int N>
    struct hdr_
    {
        WORD    wLength;
        WORD    wValueLength;
        WORD    wType;
        WCHAR   szKey[1];
    };

}; // namespace hdr

typedef hdr<int>::hdr_<1> VS_VERSIONINFO_hdr;
typedef hdr<int>::hdr_<2> StringFileInfo_hdr;
typedef hdr<int>::hdr_<3> VarFileInfo_hdr;
typedef hdr<int>::hdr_<4> Var_hdr;
typedef hdr<int>::hdr_<5> StringTable_hdr;
typedef hdr<int>::hdr_<6> String_hdr;

template<ss_typename_param_k T>
T* rounded_ptr(T* p, ss_size_t n)
{
    union
    {
        T*          p;
        ss_size_t   cb;
    } u;

    u.p = p;

    u.cb = ((n - 1) + u.cb) & ~(n- 1);

    WINSTL_ASSERT(ptr_byte_diff(u.p, p) >= 0);

    return u.p;
}

template<ss_typename_param_k T>
T* rounded_ptr(T* p, ss_ptrdiff_t byteOffset, ss_size_t n)
{
    // 1. This has to be done in a ridiculously long-hand fashion because Borland is a *very* stupid compiler!
#if defined(STLSOFT_COMPILER_IS_BORLAND)
    void const* pv  =   &byteOffset[(char*)p];
#else /* ? compiler */
    void const* pv  =   ptr_byte_offset(p, byteOffset);
#endif /* compiler */

    WINSTL_ASSERT(((char*)pv - (char*)p) == byteOffset);

    T*          p_  =   static_cast<T*>(pv);
    T*          r   =   rounded_ptr(p_, n);

#ifdef STLSOFT_COMPILER_IS_BORLAND
    STLSOFT_SUPPRESS_UNUSED(p);
    STLSOFT_SUPPRESS_UNUSED(byteOffset);
#endif /* compiler */

    WINSTL_ASSERT(ptr_byte_diff(r, p_) >= 0);

    return r;
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
/** \brief Exception thrown by the version_info class.
 *
 * \ingroup group__library__system
 */
class version_info_exception
    : public windows_exception
{
/// \name Member Types
/// @{
public:
    typedef windows_exception           parent_class_type;
    typedef version_info_exception      class_type;
/// @}

/// \name Construction
/// @{
public:
    version_info_exception(char const* reason, error_code_type err)
        : parent_class_type(reason, err)
    {}
/// @}
};
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

/** \brief Represents the fixed part of a version information block
 *
 * \ingroup group__library__system
 */
class fixed_file_info
{
    typedef fixed_file_info class_type;

public:
    /// Constructor
    fixed_file_info(VS_FIXEDFILEINFO const* ffi);

public:
    ws_uint16_t ApiVerHigh() const;
    ws_uint16_t ApiVerLow() const;

    /// The major part of the file version
    ws_uint16_t FileVerMajor() const;
    /// The minor part of the file version
    ws_uint16_t FileVerMinor() const;
    /// The revision part of the file version
    ws_uint16_t FileVerRevision() const;
    /// The build increment part of the file version
    ws_uint16_t FileVerBuild() const;

    /// The major part of the product version
    ws_uint16_t ProductVerMajor() const;
    /// The minor part of the product version
    ws_uint16_t ProductVerMinor() const;
    /// The revision part of the product version
    ws_uint16_t ProductVerRevision() const;
    /// The build increment part of the product version
    ws_uint16_t ProductVerBuild() const;

    /// The file flags mask
    ws_uint32_t FileFlagsMask() const;
    /// The file flags
    ws_uint32_t FileFlags() const;

    /// The file operating system
    ws_uint32_t FileOS() const;

    /// The file type
    ws_uint32_t FileType() const;
    /// The file subtype
    ws_uint32_t FileSubtype() const;

    /// The timestamp of the file
    FILETIME const& FileDateTime() const;

private:
    static FILETIME calc_FileDateTime_(VS_FIXEDFILEINFO const* ffi);

private:
    VS_FIXEDFILEINFO const* const   m_ffi;
    FILETIME const                  m_fileDateTime;

private:
    class_type& operator =(class_type const&);
};

/** \brief Represents a variable file part of a version information block
 *
 * \ingroup group__library__system
 */
class VsVar
{
public:
    /// This type
    typedef VsVar   class_type;

    /// Represents a language/code-page pair
    struct LangCodePage
    {
        /// The language
        ss_uint16_t language;
        /// The code-page
        ss_uint16_t codePage;
    };
public:
    /// Constructor
    VsVar(Var_hdr const* p);

    /// The length of the variable
    ss_size_t   length() const;

    /// Subscript operator
    LangCodePage const& operator [](ss_size_t index) const;

private:
    Var_hdr const*      m_p;
    LangCodePage const* m_values;
};

/** \brief Represents a string part of a version information block
 *
 * \ingroup group__library__system
 */
class VsString
{
public:
    /// This type
    typedef VsString    class_type;

public:
    /// Constructor
    VsString(String_hdr const* p);

    /// The name of the variable
    wchar_t const* name() const;

    /// The value of the variable
    wchar_t const* value() const;

private:
    wchar_t const*  m_name;
    wchar_t const*  m_value;
};

/** \brief Represents a string table
 *
 * \ingroup group__library__system
 */
class VsStringTable
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// This type
    typedef VsStringTable   class_type;
    /// The value type
    typedef VsString        value_type;

public:
    /// Constructor
    VsStringTable(StringTable_hdr const* p);

    /// The key
    wchar_t const* Key() const;

    /// The non-mutating (const) iterator
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(forward_iterator_tag)
                                            ,   value_type
                                            ,   ws_ptrdiff_t
                                            ,   void        // By-Value Temporary reference
                                            ,   value_type  // By-Value Temporary reference
                                            >
    {
    public:
        /// This type
        typedef const_iterator  class_type;
        /// The value type
        typedef VsString        value_type;

    public:
        /// Constructor
        const_iterator(void const* p);

        /// Pre-increment operator
        class_type& operator ++();

        /// Post-increment operator
        class_type operator ++(int);

        value_type operator *() const;

        ws_bool_t operator ==(class_type const& rhs) const;

        ws_bool_t operator !=(class_type const& rhs) const;

    private:
        void const* m_p;
    };

    const_iterator begin() const;

    const_iterator end() const;

private:
    StringTable_hdr const*  m_p;
    void const*             m_strings;
};

/** \brief Represents a variable file info part of a version information block
 *
 * \ingroup group__library__system
 */
class VsVarFileInfo
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// This type
    typedef VsVarFileInfo   class_type;
    /// The value type
    typedef VsVar           value_type;

public:
    /// Constructor
    ///
    /// \param p The header of the block for which the instance will act
    VsVarFileInfo(VarFileInfo_hdr const* p);

    /// The Key property
    wchar_t const* Key() const;

    /// Iterator class
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(forward_iterator_tag)
                                            ,   value_type
                                            ,   ws_ptrdiff_t
                                            ,   void        // By-Value Temporary reference
                                            ,   value_type  // By-Value Temporary reference
                                            >
    {
    public:
        /// This type
        typedef const_iterator  class_type;

    public:
        /// Constructor
        const_iterator(void const* p);

        /// Pre-increment operator
        class_type& operator ++();

        /// Post-increment operator
        class_type operator ++(int);

        value_type operator *() const;

        ws_bool_t operator ==(class_type const& rhs) const;

        ws_bool_t operator !=(class_type const& rhs) const;

    private:
        void const* m_p;
    };

    const_iterator begin() const;

    const_iterator end() const;

private:
    VarFileInfo_hdr const*  m_p;
    void const*             m_vars;
};

/** \brief Represents a variable string part of a version information block
 *
 * \ingroup group__library__system
 */
class VsStringFileInfo
    : public stlsoft_ns_qual(stl_collection_tag)
{
public:
    /// This type
    typedef VsStringFileInfo    class_type;
    /// The value type
    typedef VsStringTable       value_type;
public:
    /// Constructor
    VsStringFileInfo(StringFileInfo_hdr const* p);

    /// The key of the StringFileInfo block
    wchar_t const* Key() const;

    /// Non-mutating (const) iterator type for the StringFileInfo block
    ///
    /// \note The value type is \c VsStringTable
    class const_iterator
        : public stlsoft_ns_qual(iterator_base)<winstl_ns_qual_std(forward_iterator_tag)
                                            ,   value_type
                                            ,   ws_ptrdiff_t
                                            ,   void        // By-Value Temporary reference
                                            ,   value_type  // By-Value Temporary reference
                                            >
    {
    public:
        /// The class type
        typedef const_iterator  class_type;
        /// The value type
        typedef VsStringTable   value_type;

    public:
        /// Constructor
        const_iterator(void const* p);

        /// Pre-increment operator
        class_type& operator ++();

        /// Post-increment operator
        class_type operator ++(int);

        value_type operator *() const;

        ws_bool_t operator ==(class_type const& rhs) const;

        ws_bool_t operator !=(class_type const& rhs) const;

    private:
        void const* m_p;
    };

    const_iterator begin() const;

    const_iterator end() const;

private:
    StringFileInfo_hdr const*   m_p;
    void const*                 m_vars;
};


/** \brief Provides convenient access to aspects of a module's version information
 *
 * \ingroup group__library__system
 */
class version_info
{
private:
    typedef processheap_allocator<ws_byte_t>    allocator_type;
public:
    /// This type
    typedef version_info                        class_type;

/// \name Construction
/// @{
public:
    /// Creates an instance corresponding to the version information from the given module
    ///
    /// \param moduleName The name of the module (.exe, .dll, etc.) to load
    ss_explicit_k version_info(ws_char_a_t const* moduleName);

    /// Creates an instance corresponding to the version information from the given module
    ///
    /// \param moduleName The name of the module (.exe, .dll, etc.) to load
    ss_explicit_k version_info(ws_char_w_t const* moduleName);

    /// Releases any allocated resources
    ~version_info() stlsoft_throw_0();
/// @}

/// \name Properties
/// @{
public:
    /// The length of the version information
    ws_size_t Length() const;

    /// The length of the value part of the version block
    ws_size_t ValueLength() const;

    /// The type field in the version block
    ws_size_t Type() const;

    /// The key of the version block
    wchar_t const* Key() const;

    /// The FixedFileInfo part of the block
    fixed_file_info FixedFileInfo() const;

    /// Indicates whether the module contains a VarFileInfo block
    ws_bool_t HasVarFileInfo() const;

    /// The VarFileInfo part of the block
    VsVarFileInfo VarFileInfo() const;

    /// Indicates whether the module contains a StringFileInfo block
    ws_bool_t HasStringFileInfo() const;

    /// The StringFileInfo part of the block
    VsStringFileInfo    StringFileInfo() const;
/// @}

private:
    static VS_VERSIONINFO_hdr const* retrieve_module_info_block_(ws_char_a_t const* moduleName);

    static VS_VERSIONINFO_hdr const* retrieve_module_info_block_(ws_char_w_t const* moduleName);

    static wchar_t const* calc_key_(void const* pv);

    static VS_FIXEDFILEINFO const* calc_ffi_(wchar_t const* key);

    static WORD const* calc_children_(VS_FIXEDFILEINFO const* ffi);

private:
    void init_();

private:
    VS_VERSIONINFO_hdr const*   const   m_hdr;
    wchar_t const*              const   m_key;
    VS_FIXEDFILEINFO const*     const   m_ffi;
    WORD const*                 const   m_children;
    StringFileInfo_hdr const*           m_sfi;
    VarFileInfo_hdr const*              m_vfi;

// Not to be implemented
private:
    version_info(class_type const& rhs);
    class_type& operator =(class_type const& rhs);
};

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/version_info_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* /////////////////////////////////////////////////////////////////////////
 * Implementation
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

inline /* static */ FILETIME fixed_file_info::calc_FileDateTime_(VS_FIXEDFILEINFO const* ffi)
{
    FILETIME    ft = {  ffi->dwFileDateLS, ffi->dwFileDateMS };

    return ft;
}

inline fixed_file_info::fixed_file_info(VS_FIXEDFILEINFO const* ffi)
    : m_ffi(ffi)
    , m_fileDateTime(calc_FileDateTime_(ffi))
{}

inline ws_uint16_t fixed_file_info::ApiVerHigh() const
{
    return HIWORD(m_ffi->dwStrucVersion);
}

inline ws_uint16_t fixed_file_info::ApiVerLow() const
{
    return LOWORD(m_ffi->dwStrucVersion);
}

inline ws_uint16_t fixed_file_info::FileVerMajor() const
{
    return HIWORD(m_ffi->dwFileVersionMS);
}

inline ws_uint16_t fixed_file_info::FileVerMinor() const
{
    return LOWORD(m_ffi->dwFileVersionMS);
}

inline ws_uint16_t fixed_file_info::FileVerRevision() const
{
    return HIWORD(m_ffi->dwFileVersionLS);
}

inline ws_uint16_t fixed_file_info::FileVerBuild() const
{
    return LOWORD(m_ffi->dwFileVersionLS);
}

inline ws_uint16_t fixed_file_info::ProductVerMajor() const
{
    return HIWORD(m_ffi->dwProductVersionMS);
}

inline ws_uint16_t fixed_file_info::ProductVerMinor() const
{
    return LOWORD(m_ffi->dwProductVersionMS);
}

inline ws_uint16_t fixed_file_info::ProductVerRevision() const
{
    return HIWORD(m_ffi->dwProductVersionLS);
}

inline ws_uint16_t fixed_file_info::ProductVerBuild() const
{
    return LOWORD(m_ffi->dwProductVersionLS);
}

inline ws_uint32_t fixed_file_info::FileFlagsMask() const
{
    return m_ffi->dwFileFlagsMask;
}

inline ws_uint32_t fixed_file_info::FileFlags() const
{
    return m_ffi->dwFileFlags;
}

inline ws_uint32_t fixed_file_info::FileOS() const
{
    return m_ffi->dwFileOS;
}

inline ws_uint32_t fixed_file_info::FileType() const
{
    return m_ffi->dwFileType;
}

inline ws_uint32_t fixed_file_info::FileSubtype() const
{
    return m_ffi->dwFileSubtype;
}

inline FILETIME const& fixed_file_info::FileDateTime() const
{
    return m_fileDateTime;
}

inline VsVar::VsVar(Var_hdr const* p)
    : m_p(p)
{
    WINSTL_ASSERT(0 == ::wcsncmp(p->szKey, L"Translation", 12));

    m_values = sap_cast<LangCodePage const*>(rounded_ptr(&p->szKey[1 + ::wcslen(p->szKey)], 4));
}

inline ss_size_t VsVar::length() const
{
    return m_p->wValueLength / sizeof(LangCodePage);
}

inline VsVar::LangCodePage const& VsVar::operator [](ss_size_t index) const
{
    return m_values[index];
}

inline VsString::VsString(String_hdr const* p)
    : m_name(p->szKey)
{
    m_value =   sap_cast<wchar_t const*>(rounded_ptr(&p->szKey[1 + ::wcslen(p->szKey)], 4));
}

inline wchar_t const* VsString::name() const
{
    return m_name;
}

inline wchar_t const* VsString::value() const
{
    return m_value;
}

inline VsStringTable::VsStringTable(StringTable_hdr const* p)
    : m_p(p)
{
    m_strings = rounded_ptr(&p->szKey[1 + ::wcslen(p->szKey)], 4);
}

inline wchar_t const* VsStringTable::Key() const
{
    WINSTL_ASSERT(NULL != m_p);

    return m_p->szKey;
}

inline VsStringTable::const_iterator::const_iterator(void const* p)
    : m_p(p)
{}

inline VsStringTable::const_iterator::class_type& VsStringTable::const_iterator::operator ++()
{
    String_hdr const* str = static_cast<String_hdr const*>(m_p);

    m_p = rounded_ptr(m_p, str->wLength, 4);

    return *this;
}

inline VsStringTable::const_iterator::class_type VsStringTable::const_iterator::operator ++(int)
{
    const_iterator  ret(*this);

    operator ++();

    return ret;
}

inline VsString VsStringTable::const_iterator::operator *() const
{
    String_hdr const* str = static_cast<String_hdr const*>(m_p);

    return VsString(str);
}

inline ws_bool_t VsStringTable::const_iterator::operator ==(VsStringTable::const_iterator::class_type const& rhs) const
{
    return m_p == rhs.m_p;
}

inline ws_bool_t VsStringTable::const_iterator::operator !=(VsStringTable::const_iterator::class_type const& rhs) const
{
    return !operator ==(rhs);
}

inline VsStringTable::const_iterator VsStringTable::begin() const
{
    return const_iterator(m_strings);
}

inline VsStringTable::const_iterator VsStringTable::end() const
{
    return const_iterator(rounded_ptr(m_p, m_p->wLength, 4));
}

inline VsVarFileInfo::VsVarFileInfo(VarFileInfo_hdr const* p)
    : m_p(p)
{
    WINSTL_ASSERT(0 == ::wcsncmp(p->szKey, L"VarFileInfo", 12));

    m_vars = rounded_ptr(&p->szKey[1 + ::wcslen(p->szKey)], 4);
}

inline wchar_t const* VsVarFileInfo::Key() const
{
    WINSTL_ASSERT(NULL != m_p);

    return m_p->szKey;
}

inline VsVarFileInfo::const_iterator::const_iterator(void const* p)
    : m_p(p)
{}

inline VsVarFileInfo::const_iterator::class_type& VsVarFileInfo::const_iterator::operator ++()
{
    Var_hdr const* var = static_cast<Var_hdr const*>(m_p);

    m_p = rounded_ptr(m_p, var->wLength, 4);

    return *this;
}

inline VsVarFileInfo::const_iterator::class_type VsVarFileInfo::const_iterator::operator ++(int)
{
    const_iterator  ret(*this);

    operator ++();

    return ret;
}

inline VsVar VsVarFileInfo::const_iterator::operator *() const
{
    Var_hdr const* var = static_cast<Var_hdr const*>(m_p);

    return VsVar(var);
}

inline ws_bool_t VsVarFileInfo::const_iterator::operator ==(class_type const& rhs) const
{
    return m_p == rhs.m_p;
}

inline ws_bool_t VsVarFileInfo::const_iterator::operator !=(class_type const& rhs) const
{
    return !operator ==(rhs);
}

inline VsVarFileInfo::const_iterator VsVarFileInfo::begin() const
{
    return const_iterator(m_vars);
}

inline VsVarFileInfo::const_iterator VsVarFileInfo::end() const
{
    return const_iterator(rounded_ptr(m_p, m_p->wLength, 4));
}

inline VsStringFileInfo::VsStringFileInfo(StringFileInfo_hdr const* p)
    : m_p(p)
{
    WINSTL_ASSERT(0 == ::wcsncmp(p->szKey, L"StringFileInfo", 15));

    m_vars = rounded_ptr(&p->szKey[1 + ::wcslen(p->szKey)], 4);
}

inline wchar_t const* VsStringFileInfo::Key() const
{
    WINSTL_ASSERT(NULL != m_p);

    return m_p->szKey;
}

inline VsStringFileInfo::const_iterator::const_iterator(void const* p)
    : m_p(p)
{}

inline VsStringFileInfo::const_iterator::class_type& VsStringFileInfo::const_iterator::operator ++()
{
    StringTable_hdr const* strtbl = static_cast<StringTable_hdr const*>(m_p);

    m_p = rounded_ptr(m_p, strtbl->wLength, 4);

    return *this;
}

inline VsStringFileInfo::const_iterator::class_type VsStringFileInfo::const_iterator::operator ++(int)
{
    const_iterator  ret(*this);

    operator ++();

    return ret;
}

inline VsStringTable VsStringFileInfo::const_iterator::operator *() const
{
    StringTable_hdr const* strtbl = static_cast<StringTable_hdr const*>(m_p);

    return VsStringTable(strtbl);
}

inline ws_bool_t VsStringFileInfo::const_iterator::operator ==(class_type const& rhs) const
{
    return m_p == rhs.m_p;
}

inline ws_bool_t VsStringFileInfo::const_iterator::operator !=(class_type const& rhs) const
{
    return !operator ==(rhs);
}

inline VsStringFileInfo::const_iterator VsStringFileInfo::begin() const
{
    return const_iterator(m_vars);
}

inline VsStringFileInfo::const_iterator VsStringFileInfo::end() const
{
    return const_iterator(rounded_ptr(m_p, m_p->wLength, 4));
}

inline /* ss_explicit_k */ version_info::version_info(ws_char_a_t const* moduleName)
    : m_hdr(retrieve_module_info_block_(moduleName))
    , m_key(calc_key_(m_hdr))
    , m_ffi(calc_ffi_(m_key))
    , m_children(calc_children_(m_ffi))
    , m_sfi(NULL)
    , m_vfi(NULL)
{
    init_();
}

inline /* ss_explicit_k */ version_info::version_info(ws_char_w_t const* moduleName)
    : m_hdr(retrieve_module_info_block_(moduleName))
    , m_key(calc_key_(m_hdr))
    , m_ffi(calc_ffi_(m_key))
    , m_children(calc_children_(m_ffi))
    , m_sfi(NULL)
    , m_vfi(NULL)
{
    init_();
}

inline version_info::~version_info() stlsoft_throw_0()
{
    allocator_type  allocator;

    allocator.deallocate(const_cast<ws_byte_t*>(sap_cast<ws_byte_t const*>(m_hdr)));
}

inline ws_size_t version_info::Length() const
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == m_hdr)
    {
        return 0;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != m_hdr);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

    return *(sap_cast<WORD const*>(m_hdr) + 0);
}

inline ws_size_t version_info::ValueLength() const
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == m_hdr)
    {
        return 0;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != m_hdr);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

    return *(sap_cast<WORD const*>(m_hdr) + 1);
}

inline ws_size_t version_info::Type() const
{
    WINSTL_ASSERT(NULL != m_hdr);

    return *(sap_cast<WORD const*>(m_hdr) + 2);
}

inline wchar_t const* version_info::Key() const
{
    WINSTL_ASSERT(NULL != m_hdr);

    return m_key;
}

inline fixed_file_info version_info::FixedFileInfo() const
{
    WINSTL_ASSERT(NULL != m_hdr);

    return fixed_file_info(m_ffi);
}

inline ws_bool_t version_info::HasVarFileInfo() const
{
    return NULL != m_vfi;
}

inline VsVarFileInfo version_info::VarFileInfo() const
{
    WINSTL_ASSERT(NULL != m_vfi);

    return VsVarFileInfo(m_vfi);
}

inline ws_bool_t version_info::HasStringFileInfo() const
{
    return NULL != m_sfi;
}

inline VsStringFileInfo version_info::StringFileInfo() const
{
    WINSTL_ASSERT(NULL != m_sfi);

    return VsStringFileInfo(m_sfi);
}

inline /* static */ VS_VERSIONINFO_hdr const* version_info::retrieve_module_info_block_(ws_char_a_t const* moduleName)
{
#ifdef WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER
    ws_char_a_t                         buffer[1 + WINSTL_CONST_MAX_PATH];
#else /* ?WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
    basic_file_path_buffer<ws_char_a_t> buffer;
#endif /* WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */

    if( NULL == moduleName &&
#ifdef WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER
        0 != ::GetModuleFileNameA(NULL, &buffer[0], STLSOFT_NUM_ELEMENTS(buffer)))
#else /* ?WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
        0 != ::GetModuleFileNameA(NULL, &buffer[0], DWORD(buffer.size())))
#endif /* WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
    {
        moduleName = stlsoft_ns_qual(c_str_ptr)(buffer);
    }
    else
    {
        // Must verify it can be loaded, i.e. is a 32-bit resource
        //
        // TODO: Work out how to support 16-bit versions
        HINSTANCE   hinst   =   ::LoadLibraryExA(moduleName, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if(NULL == hinst)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(version_info_exception("Could not elicit version information from module", ::GetLastError()));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            return NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            ::FreeLibrary(hinst);
        }
    }

    allocator_type      allocator;
    ws_dword_t          cb  =   ::GetFileVersionInfoSizeA(const_cast<ws_char_a_t*>(moduleName), NULL);
    void                *pv =   (0 == cb) ? NULL : allocator.allocate(cb);

#if !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    // If bad_alloc will not be thrown, then we need to check for NULL, but only act on it
    // if cb is non-zero
    if( 0 != cb &&
        pv == NULL)
    {
        ::GetLastError();

        return NULL;
    }
#endif /* !STLSOFT_CF_THROW_BAD_ALLOC */

    WINSTL_ASSERT(0 == cb || pv != NULL);

    if( 0 == cb ||
        !::GetFileVersionInfoA(const_cast<ws_char_a_t*>(moduleName), 0, cb, pv))
    {
        allocator.deallocate(static_cast<ws_byte_t*>(pv), cb);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X(version_info_exception("Could not elicit version information from module", ::GetLastError()));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        return NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    WINSTL_ASSERT(pv != NULL);

    return static_cast<VS_VERSIONINFO_hdr*>(pv);
}

inline /* static */ VS_VERSIONINFO_hdr const* version_info::retrieve_module_info_block_(ws_char_w_t const* moduleName)
{
#ifdef WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER
    ws_char_w_t                         buffer[1 + WINSTL_CONST_MAX_PATH];
#else /* ?WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
    basic_file_path_buffer<ws_char_w_t> buffer;
#endif /* WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */

    if( NULL == moduleName &&
#ifdef WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER
        0 != ::GetModuleFileNameW(NULL, &buffer[0], STLSOFT_NUM_ELEMENTS(buffer)))
#else /* ?WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
        0 != ::GetModuleFileNameW(NULL, &buffer[0], DWORD(buffer.size())))
#endif /* WINSTL_VERSION_INFO_NO_USE_FILE_PATH_BUFFER */
    {
        moduleName = stlsoft_ns_qual(c_str_ptr)(buffer);
    }
    else
    {
        // Must verify it can be loaded, i.e. is a 32-bit resource
        //
        // TODO: Work out how to support 16-bit versions
        HINSTANCE   hinst   =   ::LoadLibraryExW(moduleName, NULL, LOAD_LIBRARY_AS_DATAFILE);

        if(NULL == hinst)
        {
#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
            STLSOFT_THROW_X(version_info_exception("Could not elicit version information from module", ::GetLastError()));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
            return NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
        }
        else
        {
            ::FreeLibrary(hinst);
        }
    }

    allocator_type  allocator;
    ws_dword_t      cb  =   ::GetFileVersionInfoSizeW(const_cast<ws_char_w_t*>(moduleName), NULL);
    void            *pv =   (0 == cb) ? NULL : allocator.allocate(cb);

#ifndef STLSOFT_CF_THROW_BAD_ALLOC
    if( 0 != cb &&
        pv == NULL)
    {
        return NULL;
    }
#endif /* !STLSOFT_CF_THROW_BAD_ALLOC */

    if( 0 == cb ||
        !::GetFileVersionInfoW(const_cast<ws_char_w_t*>(moduleName), 0, cb, pv))
    {
        allocator.deallocate(static_cast<ws_byte_t*>(pv), cb);

#ifdef STLSOFT_CF_EXCEPTION_SUPPORT
        STLSOFT_THROW_X((version_info_exception("Could not elicit version information from module", ::GetLastError())));
#else /* ? STLSOFT_CF_EXCEPTION_SUPPORT */
        pv = NULL;
#endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
    }

    return static_cast<VS_VERSIONINFO_hdr*>(pv);
}

inline /* static */ wchar_t const* version_info::calc_key_(void const* pv)
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == pv)
    {
        return NULL;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != pv);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

#ifdef _DEBUG
    // Bit of 16-bit resource code here
    //
    // This is reasonably safe, because if it is unicode, then the n-limited string comparison
    // will simply return non-0, rather than potentially going off and crashing
    {
        char const* keyA = reinterpret_cast<char const*>(static_cast<WORD const*>(pv) + 2);

        if(0 == ::strncmp("VS_VERSION_INFO", keyA, 16))
        {
            keyA = NULL;
        }
    }
#endif /* _DEBUG */

    wchar_t const* key = reinterpret_cast<wchar_t const*>(static_cast<WORD const*>(pv) + 3);

    WINSTL_ASSERT(0 == ::wcsncmp(L"VS_VERSION_INFO", key, 16));

    return key;
}

inline /* static */ VS_FIXEDFILEINFO const* version_info::calc_ffi_(wchar_t const* key)
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == key)
    {
        return NULL;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != key);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

    return sap_cast<VS_FIXEDFILEINFO const*>(rounded_ptr(&key[1 + ::wcslen(key)], 4));
}

inline /* static */ WORD const* version_info::calc_children_(VS_FIXEDFILEINFO const* ffi)
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == ffi)
    {
        return NULL;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != ffi);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

    return sap_cast<WORD const*>(rounded_ptr(&ffi[1], 4));
}

inline void version_info::init_()
{
#if !defined(STLSOFT_CF_EXCEPTION_SUPPORT) || \
    !defined(STLSOFT_CF_THROW_BAD_ALLOC)
    if(NULL == m_hdr)
    {
        return;
    }
#else /* ? exceptions */
    WINSTL_ASSERT(NULL != m_hdr);
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT || !STLSOFT_CF_THROW_BAD_ALLOC */

#ifdef _DEBUG
    // Check that ffi is the same as the pointer returned from VerQueryValue("\\");
    VS_FIXEDFILEINFO    *ffi    =   NULL;
    UINT                cchInfo =   0;

    WINSTL_ASSERT(::VerQueryValueA(const_cast<VS_VERSIONINFO_hdr*>(m_hdr), "\\", reinterpret_cast<void**>(&ffi), &cchInfo));
    WINSTL_ASSERT(ffi == m_ffi);
#endif /* _DEBUG */

    // Now we must parse the children.

    void const  *       pv  =   m_children;
    void const  *const  end =   rounded_ptr(m_hdr, m_hdr->wLength, 4);

    WINSTL_ASSERT(ptr_byte_diff(end, pv) >= 0);

    for(; pv != end; )
    {
        union
        {
            void const                  *pv_;
            StringFileInfo_hdr const    *psfi;
            VarFileInfo_hdr const       *pvfi;
        } u;

        u.pv_ = pv;

        WINSTL_ASSERT(ptr_byte_diff(pv, m_hdr) < m_hdr->wLength);

        if(0 == ::wcsncmp(u.psfi->szKey, L"StringFileInfo", 15))
        {
            WINSTL_ASSERT(NULL == m_sfi);

            m_sfi = u.psfi;

            pv = rounded_ptr(pv, u.psfi->wLength, 4);
        }
        else if(0 == ::wcsncmp(u.psfi->szKey, L"VarFileInfo", 12))
        {
            WINSTL_ASSERT(NULL == m_vfi);

            m_vfi = u.pvfi;

            pv = rounded_ptr(pv, u.pvfi->wLength, 4);
        }
        else
        {
#ifdef STLSOFT_UNITTEST
            ::wprintf(L"Unexpected contents of VS_VERSIONINFO children. pv: 0x%08x; Key: %.*s\n", pv, 20, u.psfi->szKey);
#endif /* STLSOFT_UNITTEST */

            WINSTL_MESSAGE_ASSERT("Unexpected contents of VS_VERSIONINFO children", NULL == m_vfi);

            break;
        }

        WINSTL_ASSERT(ptr_byte_diff(pv, end) <= 0);
    }

    WINSTL_ASSERT(ptr_byte_diff(pv, m_hdr) == m_hdr->wLength);

#ifdef _DEBUG
    fixed_file_info fixedInfo = FixedFileInfo();

    ws_uint16_t    j   =   fixedInfo.FileVerMajor();
    ws_uint16_t    n   =   fixedInfo.FileVerMinor();
    ws_uint16_t    r   =   fixedInfo.FileVerRevision();
    ws_uint16_t    b   =   fixedInfo.FileVerBuild();

    STLSOFT_SUPPRESS_UNUSED(j);
    STLSOFT_SUPPRESS_UNUSED(n);
    STLSOFT_SUPPRESS_UNUSED(r);
    STLSOFT_SUPPRESS_UNUSED(b);
#endif /* _DEBUG */
}

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

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

#endif /* WINSTL_INCL_WINSTL_SYSTEM_HPP_VERSION_INFO */

/* ///////////////////////////// end of file //////////////////////////// */
