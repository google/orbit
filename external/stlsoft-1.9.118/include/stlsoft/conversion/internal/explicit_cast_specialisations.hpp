/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/conversion/internal/explicit_cast_specialisations.hpp
 *
 * Purpose:     Specialisations of explicit_cast
 *
 * Created:     13th August 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/conversion/internal/explicit_cast_specialisations.hpp
 *
 * \brief [C++ only] Explicit specialisations of stlsoft::explicit_cast
 *   (\ref group__library__conversion "Conversion" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST
# error This file is included from within stlsoft/conversion/explicit_cast.hpp, and cannot be included directly
#else

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_CONVERSION_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS_MAJOR    4
# define STLSOFT_VER_STLSOFT_CONVERSION_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS_MINOR    0
# define STLSOFT_VER_STLSOFT_CONVERSION_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS_REVISION 1
# define STLSOFT_VER_STLSOFT_CONVERSION_INTERNAL_HPP_EXPLICIT_CAST_SPECIALISATIONS_EDIT     21
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Specialisations
 */

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<char const&>
{
public:
    typedef char                value_type;
    typedef explicit_cast<char> class_type;

// Construction
public:
    explicit_cast(char const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator char const& () const
    {
        return m_t;
    }

// Members
private:
    char const& m_t;
};

#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<wchar_t const&>
{
public:
    typedef wchar_t                value_type;
    typedef explicit_cast<wchar_t> class_type;

// Construction
public:
    explicit_cast(wchar_t const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator wchar_t const& () const
    {
        return m_t;
    }

// Members
private:
    wchar_t const& m_t;
};

#endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned char const&>
{
public:
    typedef unsigned char                value_type;
    typedef explicit_cast<unsigned char> class_type;

// Construction
public:
    explicit_cast(unsigned char const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned char const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned char const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed char const&>
{
public:
    typedef signed char                value_type;
    typedef explicit_cast<signed char> class_type;

// Construction
public:
    explicit_cast(signed char const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed char const& () const
    {
        return m_t;
    }

// Members
private:
    signed char const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed short const&>
{
public:
    typedef signed short                value_type;
    typedef explicit_cast<signed short> class_type;

// Construction
public:
    explicit_cast(signed short const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed short const& () const
    {
        return m_t;
    }

// Members
private:
    signed short const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned short const&>
{
public:
    typedef unsigned short                value_type;
    typedef explicit_cast<unsigned short> class_type;

// Construction
public:
    explicit_cast(unsigned short const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned short const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned short const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed int const&>
{
public:
    typedef signed int                value_type;
    typedef explicit_cast<signed int> class_type;

// Construction
public:
    explicit_cast(signed int const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed int const& () const
    {
        return m_t;
    }

// Members
private:
    signed int const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned int const&>
{
public:
    typedef unsigned int                value_type;
    typedef explicit_cast<unsigned int> class_type;

// Construction
public:
    explicit_cast(unsigned int const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned int const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned int const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed long const&>
{
public:
    typedef signed long                value_type;
    typedef explicit_cast<signed long> class_type;

// Construction
public:
    explicit_cast(signed long const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed long const& () const
    {
        return m_t;
    }

// Members
private:
    signed long const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned long const&>
{
public:
    typedef unsigned long                value_type;
    typedef explicit_cast<unsigned long> class_type;

// Construction
public:
    explicit_cast(unsigned long const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned long const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned long const& m_t;
};

#ifdef STLSOFT_CF_64BIT_INT_IS_long_long

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed long long const&>
{
public:
    typedef signed long long                value_type;
    typedef explicit_cast<signed long long> class_type;

// Construction
public:
    explicit_cast(signed long long const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed long long const& () const
    {
        return m_t;
    }

// Members
private:
    signed long long const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned long long const&>
{
public:
    typedef unsigned long long                value_type;
    typedef explicit_cast<unsigned long long> class_type;

// Construction
public:
    explicit_cast(unsigned long long const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned long long const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned long long const& m_t;
};

#elif defined(STLSOFT_CF_64BIT_INT_IS___int64)

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<signed __int64 const&>
{
public:
    typedef signed __int64                value_type;
    typedef explicit_cast<signed __int64> class_type;

// Construction
public:
    explicit_cast(signed __int64 const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator signed __int64 const& () const
    {
        return m_t;
    }

// Members
private:
    signed __int64 const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<unsigned __int64 const&>
{
public:
    typedef unsigned __int64                value_type;
    typedef explicit_cast<unsigned __int64> class_type;

// Construction
public:
    explicit_cast(unsigned __int64 const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator unsigned __int64 const& () const
    {
        return m_t;
    }

// Members
private:
    unsigned __int64 const& m_t;
};

#else

# error 64-bit discrimination not handled correctly

#endif /* 64-bit */

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<float const&>
{
public:
    typedef float                value_type;
    typedef explicit_cast<float> class_type;

// Construction
public:
    explicit_cast(float const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator float const& () const
    {
        return m_t;
    }

// Members
private:
    float const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<double const&>
{
public:
    typedef double                value_type;
    typedef explicit_cast<double> class_type;

// Construction
public:
    explicit_cast(double const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator double const& () const
    {
        return m_t;
    }

// Members
private:
    double const& m_t;
};

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<long double const&>
{
public:
    typedef long double                value_type;
    typedef explicit_cast<long double> class_type;

// Construction
public:
    explicit_cast(long double const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator long double const& () const
    {
        return m_t;
    }

// Members
private:
    long double const& m_t;
};

#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT

STLSOFT_TEMPLATE_SPECIALISATION
class explicit_cast<bool const&>
{
public:
    typedef bool                value_type;
    typedef explicit_cast<bool> class_type;

// Construction
public:
    explicit_cast(bool const& t)
        : m_t(t)
    {}

// Conversions
public:
    operator bool const& () const
    {
        return m_t;
    }

// Members
private:
    bool const& m_t;
};

#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_EXPLICIT_CAST */

/* ///////////////////////////// end of file //////////////////////////// */
