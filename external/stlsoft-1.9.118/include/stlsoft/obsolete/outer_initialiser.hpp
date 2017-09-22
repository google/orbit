/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/obsolete/outer_initialiser.hpp
 *
 * Purpose:     Contains the outer_initialiser template class.
 *
 * Created:     9th July 2004
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


/// \file stlsoft/obsolete/outer_initialiser.hpp
///
/// Contains the outer_initialiser template class.

#ifndef STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER
#define STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER_MAJOR       3
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER_MINOR       2
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER_REVISION    1
# define STLSOFT_VER_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER_EDIT        24
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:TO-BE-OBSOLETE]>]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST
# include <stlsoft/conversion/sap_cast.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_CONVERSION_HPP_SAP_CAST */

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

// class outer_initialiser

/** \brief Initialises
 *
 * \ingroup group__library__obsolete
 */
template<ss_typename_param_k O>
class outer_initialiser
{
public:
    typedef O                       outer_type;
    typedef outer_initialiser<O>    class_type;

private:
    /// Ensures packing is correct
    struct pack_tester
    {
        /// Used to assert structure alignment
        struct large_type
        {
            int         i;
            long double ld;
        };

        large_type  lrg;
        int         sml;
    };

public:
    outer_initialiser(outer_type *outer, void (O::*fn)())
        : m_outer(outer)
        , m_fn(fn)
    {
        // Ensure that there's a good chance that outer_initialiser
        // is in the right place in the outer type
        STLSOFT_ASSERT(sap_cast<char*>(outer) < sap_cast<char*>(this));

        (m_outer->*m_fn)();
    }

    outer_initialiser(class_type const&)
    {
        (m_outer->*m_fn)();
    }

private:
    outer_type *const   m_outer;
    void (O::*m_fn)();
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_OBSOLETE_HPP_OUTER_INITIALISER */

/* ///////////////////////////// end of file //////////////////////////// */
