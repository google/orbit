/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/unused_return_value_monitor.hpp
 *
 * Purpose:     Basic functionals.
 *
 * Created:     8th June 2002
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


/** \file stlsoft/util/unused_return_value_monitor.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::unused_return_value_monitor
 *   class template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR_MAJOR        4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR_MINOR        0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR_REVISION     1
# define STLSOFT_VER_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR_EDIT         43
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/** \brief Return value adaptor for monitoring whether return values are used.
 *
 * \ingroup group__library__utility
 *
 * \param V The value type. This is the type that is to be returned by the function whose return type is to be monitored
 * \param M The monitor function type. If the return value is not used, the monitor function instance will be called.
 * \param R The reference type. This is the reference type, used in the constructor of the monitor, and the type of the member variable storing the value
 *
 * If the value type is a simple type, you can just allow the reference type R to be defaulted, as in:
 *
\code
struct int_monitor
{
public:
  void operator ()(void const* instance, int value) const
  {
    printf("Unused return value %d from object instance %p\n", value, instance);
  }
};

class X
{
public:
  unused_return_value_monitor<int, int_monitor>   fn()
  {
    return 10; // A copy is taken, but copies of ints usually cost the same as references to ints
  }
};
\endcode
 *
 *
 * Where the return type is a complex type, you can make efficiency savings by using a (C++) reference for the reference type. However,
 * this must only be done when the returned value will persist for the duration of the calling expression.
 *
\code
struct string_monitor
{
public:
  void operator ()(void const* instance, string const& value) const
  {
    printf("Unused return value %s from object instance %p\n", value.c_str(), instance);
  }
};

class Y1
{
public:
  Y2()
    : m_str("What's new, Pussycat?")
  {}

public:
  // Must store return value by value, since the returned value will not persist for the duration of the calling expression
  unused_return_value_monitor<string, string_monitor> fn_by_value()
  {
    return "Hello sailor";
  }

  // Must store return value by (const) reference, since the returned value will persist for the duration of the calling expression
  unused_return_value_monitor<string, string_monitor, string const&> fn_by_ref()
  {
    return m_str;
  }

private:
  string const  m_str;
};
\endcode
 *
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k M
        ,   ss_typename_param_k R = V // Could be V const&, if you know the value is to persist
        >
// class unused_return_value_monitor
class unused_return_value_monitor
{
public:
    typedef V                                       value_type;
    typedef M                                       monitor_function;
    typedef R                                       reference_type;
    typedef unused_return_value_monitor<V, M, R>    class_type;

/// \name Construction
/// @{
public:
    /* ss_explicit_k */ unused_return_value_monitor(reference_type value)
        : m_value(value)
        , m_monitorFn()
        , m_bUsed(false)
    {}
    unused_return_value_monitor(reference_type value, monitor_function monitorFn)
        : m_value(value)
        , m_monitorFn(monitorFn)
        , m_bUsed(false)
    {}
    unused_return_value_monitor(class_type const& rhs)
        : m_value(rhs.m_value)
        , m_monitorFn(rhs.m_monitorFn)
        , m_bUsed(rhs.m_bUsed)
    {
        rhs.m_bUsed = false;
    }
    /// Destructor
    ///
    /// \note The destructor for unused_return_value_monitor deliberately does not
    /// declare a throw() clause because it may indeed throw an exception.
    ///
    /// \note This requires that unused_return_value_monitor is *never* used
    /// inside a catch block.
    ~unused_return_value_monitor() stlsoft_throw_0()
    {
        if(!m_bUsed)
        {
            m_monitorFn(this, m_value);
        }
    }
/// @}

/// \name Conversion
/// @{
public:
    operator value_type() const
    {
        const_cast<class_type*>(this)->m_bUsed = true;

        return m_value;
    }
/// @}

/** \brief Members
 *
 * \ingroup group__library__utility
 */
private:
    reference_type      m_value;
    monitor_function    m_monitorFn;
    ss_bool_t           m_bUsed;

// Not to be implemented
private:
    unused_return_value_monitor& operator =(class_type const& rhs);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_UNUSED_RETURN_VALUE_MONITOR */

/* ///////////////////////////// end of file //////////////////////////// */
