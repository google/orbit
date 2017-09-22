/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/performance/performance_counter_scope.hpp
 *
 * Purpose:     Performance counter scope class.
 *
 * Created:     20th July 2002
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


/** \file stlsoft/performance/performance_counter_scope.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::performance_counter_scope
 *  class template
 *   (\ref group__library__performance "Performance" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE
#define STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE_MAJOR    4
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE_MINOR    0
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE_REVISION 1
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE_EDIT     41
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

// class performance_counter_scope
/** \brief Scopes the measurement period for a performance counter
 *
 * \ingroup group__library__performance
 *
 * \param C The performance counter type
 */
template <ss_typename_param_k C>
class performance_counter_scope
{
public:
    /// \brief The counter type
    typedef C                               counter_type;
    /// \brief The class type
    typedef performance_counter_scope<C>    class_type;

public:
    /// \brief Constructs with the performance counter instance to manage, on which start() is called
    ss_explicit_k performance_counter_scope(counter_type &counter)
        : m_counter(counter)
    {
        m_counter.start();
    }
    /// \brief Calls stop() on the managed counter
    ~performance_counter_scope() stlsoft_throw_0()
    {
        m_counter.stop();
    }

    /// \brief Calls stop() on the managed counter
    void stop()
    {
        m_counter.stop();
    }

    // This method is const, to ensure that only the stop operation (via
    // performance_counter_scope::stop()) is accessible on the managed counter.
    /// \brief Returns a non-mutable (const) reference to the managed counter
    counter_type const& get_counter() const
    {
        return m_counter;
    }

// Members
private:
    C   &m_counter;

// Not to be implemented
private:
    performance_counter_scope(class_type const& rhs);
    class_type const& operator =(class_type const& rhs);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_SCOPE */

/* ///////////////////////////// end of file //////////////////////////// */
