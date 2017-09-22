/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/performance/performance_counter_init.hpp
 *
 * Purpose:     Performance counter initialising class.
 *
 * Created:     30th July 2002
 * Updated:     13th August 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2010, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


/** \file stlsoft/performance/performance_counter_init.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::performance_counter_initialiser
 *  class template
 *   (\ref group__library__performance "Performance" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT
#define STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT_MAJOR     4
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT_MINOR     1
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT_REVISION  1
# define STLSOFT_VER_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT_EDIT      37
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

// class performance_counter_initialiser
/** A \ref group__pattern__class_adaptor "class adaptor" that
 *   initialises a performance counter.
 *
 * \ingroup group__library__performance
 *
 * \param C The performance counter type.
 */
template <ss_typename_param_k C>
class performance_counter_initialiser
    : public C
{
/// \name Member Types
/// @{
public:
    /// The counter type
    typedef C                                               counter_type;
    /// The class type
    typedef performance_counter_initialiser<C>              class_type;
    /// The interval type
    typedef ss_typename_type_k counter_type::interval_type  interval_type;
/// @}

/// \name Construction
/// @{
public:
    /// Initialises the counter by calling start() and stop() on it.
    performance_counter_initialiser()
    {
        counter_type::start();
        counter_type::stop();
    }
/// @}

/// \name Operations
/// @{
public:
    /// Terminates and measures the current sampling period, and commences
    /// another.
    interval_type get_period_and_restart(interval_type (class_type::*pmfn)(void) const)
    {
        counter_type::stop();
        interval_type r = (this->*pmfn)();
        counter_type::start();
        return r;
    }
/// @}
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_HPP_PERFORMANCE_COUNTER_INIT */

/* ///////////////////////////// end of file //////////////////////////// */
