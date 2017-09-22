/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/meta/n_types.hpp (originally MTBase.h, ::SynesisStl)
 *
 * Purpose:     Basic support meta types.
 *
 * Created:     19th November 1998
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 1998-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/meta/n_types.hpp
 *
 * \brief [C++ only] Definition of the different type size "constants"
 *   (\ref group__library__meta "Template Meta-programming" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES
#define STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_META_HPP_N_TYPES_MAJOR     4
# define STLSOFT_VER_STLSOFT_META_HPP_N_TYPES_MINOR     2
# define STLSOFT_VER_STLSOFT_META_HPP_N_TYPES_REVISION  4
# define STLSOFT_VER_STLSOFT_META_HPP_N_TYPES_EDIT      119
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

/** \defgroup group__library__meta__n_types N-types
 * \ingroup group__library__meta
 * \brief These types are used for making decisions at compile-time. The
 *   sizes of a given type should not be assumed, but each is guaranteed
 *   that each type is a different size than all the others.
 * @{
 */

/** A distinct type for use in meta-programming */  struct one_type         { ss_byte_t ar[1]; };
/** A distinct type for use in meta-programming */  struct two_type         { one_type ar[2];  };
/** A distinct type for use in meta-programming */  struct three_type       { one_type ar[3];  };
/** A distinct type for use in meta-programming */  struct four_type        { one_type ar[4];  };
/** A distinct type for use in meta-programming */  struct five_type        { one_type ar[5];  };
/** A distinct type for use in meta-programming */  struct six_type         { one_type ar[6];  };
/** A distinct type for use in meta-programming */  struct seven_type       { one_type ar[7];  };
/** A distinct type for use in meta-programming */  struct eight_type       { one_type ar[8];  };
/** A distinct type for use in meta-programming */  struct nine_type        { one_type ar[9];  };
/** A distinct type for use in meta-programming */  struct ten_type         { one_type ar[10]; };
/** A distinct type for use in meta-programming */  struct eleven_type      { one_type ar[11]; };
/** A distinct type for use in meta-programming */  struct twelve_type      { one_type ar[12]; };
/** A distinct type for use in meta-programming */  struct thirteen_type    { one_type ar[13]; };
/** A distinct type for use in meta-programming */  struct fourteen_type    { one_type ar[14]; };
/** A distinct type for use in meta-programming */  struct fifteen_type     { one_type ar[15]; };
/** A distinct type for use in meta-programming */  struct sixteen_type     { one_type ar[16]; };
/** A distinct type for use in meta-programming */  struct seventeen_type   { one_type ar[17]; };
/** A distinct type for use in meta-programming */  struct eighteen_type    { one_type ar[18]; };
/** A distinct type for use in meta-programming */  struct nineteen_type    { one_type ar[19]; };
/** A distinct type for use in meta-programming */  struct twenty_type      { one_type ar[20]; };


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# ifdef STLSOFT_UNITTEST
inline int stlsoft_meta_n_types_size_checker()
{
    STLSOFT_STATIC_ASSERT(sizeof(one_type) < sizeof(two_type));
    STLSOFT_STATIC_ASSERT(sizeof(two_type) < sizeof(three_type));
    STLSOFT_STATIC_ASSERT(sizeof(three_type) < sizeof(four_type));
    STLSOFT_STATIC_ASSERT(sizeof(four_type) < sizeof(five_type));
    STLSOFT_STATIC_ASSERT(sizeof(five_type) < sizeof(six_type));
    STLSOFT_STATIC_ASSERT(sizeof(six_type) < sizeof(seven_type));
    STLSOFT_STATIC_ASSERT(sizeof(seven_type) < sizeof(eight_type));
    STLSOFT_STATIC_ASSERT(sizeof(eight_type) < sizeof(nine_type));
    STLSOFT_STATIC_ASSERT(sizeof(nine_type) < sizeof(ten_type));
    STLSOFT_STATIC_ASSERT(sizeof(ten_type) < sizeof(eleven_type));
    STLSOFT_STATIC_ASSERT(sizeof(eleven_type) < sizeof(twelve_type));
    STLSOFT_STATIC_ASSERT(sizeof(twelve_type) < sizeof(thirteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(thirteen_type) < sizeof(fourteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(fourteen_type) < sizeof(fifteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(fifteen_type) < sizeof(sixteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(sixteen_type) < sizeof(seventeen_type));
    STLSOFT_STATIC_ASSERT(sizeof(seventeen_type) < sizeof(eighteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(eighteen_type) < sizeof(nineteen_type));
    STLSOFT_STATIC_ASSERT(sizeof(nineteen_type) < sizeof(twenty_type));

    return 1;
}
# endif /* STLSOFT_UNITTEST */

// Backwards compatiblity
typedef one_type                        one_t;
typedef two_type                        two_t;
typedef three_type                      three_t;
typedef four_type                       four_t;
typedef five_type                       five_t;
typedef six_type                        six_t;
typedef seven_type                      seven_t;
typedef eight_type                      eight_t;
typedef nine_type                       nine_t;
typedef ten_type                        ten_t;
typedef eleven_type                     eleven_t;
typedef twelve_type                     twelve_t;
typedef thirteen_type                   thirteen_t;
typedef fourteen_type                   fourteen_t;
typedef fifteen_type                    fifteen_t;
typedef sixteen_type                    sixteen_t;
typedef seventeen_type                  seventeen_t;
typedef eighteen_type                   eighteen_t;
typedef nineteen_type                   nineteen_t;
typedef twenty_type                     twenty_t;

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** @} */

/** \brief This template is used to synthesise types with specific sizes.
 *
 * \ingroup group__library__meta
 */
template <ss_size_t N>
struct size_type
{
    ss_byte_t ar[N];
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_META_HPP_N_TYPES */

/* ///////////////////////////// end of file //////////////////////////// */
