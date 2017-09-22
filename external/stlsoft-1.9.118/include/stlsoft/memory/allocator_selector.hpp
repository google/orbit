/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/memory/allocator_selector.hpp
 *
 * Purpose:     Selects the most appropriate allocator.
 *
 * Created:     20th August 2005
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


/** \file stlsoft/memory/allocator_selector.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::allocator_selector class
 *   (\ref group__library__memory "Memory" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR
#define STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR_MAJOR    2
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR_MINOR    1
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR_REVISION 3
# define STLSOFT_VER_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR_EDIT     26
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Auto-generation and compatibility
 */

/*
[<[STLSOFT-AUTO:NO-UNITTEST]>]
[DocumentationStatus:Ready]
*/

/* /////////////////////////////////////////////////////////////////////////
 * Includes - 1
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES
# include <stlsoft/memory/allocator_features.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_FEATURES */

/* Chosen allocator
 */

#if defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_MALLOC_ALLOCATOR) && \
    defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR)
  /* Prevent use of malloc_allocator. */
# undef STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR
#endif /* allocator */

#if defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_NEW_ALLOCATOR) && \
    defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR)
  /* Prevent use of new_allocator. */
# undef STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR
#endif /* allocator */

#if defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STD_ALLOCATOR) && \
    defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR)
  /* Prevent use of std::allocator. */
# undef STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR
#endif /* allocator */



#if defined(STLSOFT_COMPILER_IS_BORLAND) || \
    (   defined(STLSOFT_COMPILER_IS_DMC) && \
        __DMC__ < 0x0845) || \
    (   defined(STLSOFT_COMPILER_IS_GCC) && \
        __GNUC__ < 3)
 // Something's wrong with Borland - big shock! - that causes crashes in deallocation
 //
 // Same for GCC 2.95
 //
 // With DMC++, it crashes the compiler!! ;-(
# define STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR
#elif defined(STLSOFT_COMPILER_IS_WATCOM) || \
      (   defined(STLSOFT_COMPILER_IS_MSVC) && \
          _MSC_VER >= 1400)
 // Watcom's got major problems, so we use malloc_allocator
 //
 // VC++ 8 & 9 has a bug that results in a definition of
 // std::allocator<>::allocate() and std::allocator<>::deallocate(),
 // which breaks the linker with LNK2005 (multiple definitions)
# define STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR
#else /* ? compiler */
 // Now we work out which to select by default for each
# if !defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR) && \
     !defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR) && \
     !defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR)
#  if !defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STD_ALLOCATOR)
#   define STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR
#  elif !defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_NEW_ALLOCATOR)
#   define STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR
#  elif !defined(STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_MALLOC_ALLOCATOR)
#   define STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR
#  else /* ? NO_USE_??? */
#   error All allocator types disabled. You must enable one or more, by disabling one or more of
#   error symbols STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STD_ALLOCATOR,
#   error STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_NEW_ALLOCATOR
#   error or STLSOFT_ALLOCATOR_SELECTOR_NO_USE_STLSOFT_MALLOC_ALLOCATOR.
#  endif /* NO_USE_??? */
# endif /* USE_??? */
#endif /* allocator */

/* /////////////////////////////////////////////////////////////////////////
 * Includes - 2
 */

#if defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR)
# ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_MALLOC_ALLOCATOR
#  include <stlsoft/memory/malloc_allocator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_MALLOC_ALLOCATOR */
#elif defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR)
# ifndef STLSOFT_INCL_STLSOFT_MEMORY_HPP_NEW_ALLOCATOR
#  include <stlsoft/memory/new_allocator.hpp>
# endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_NEW_ALLOCATOR */
#elif defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR)
# include <memory>
#else /* _USE_??? */
# error Error in discrimination. allocator_selector must select either std::allocator, stlsoft::malloc_allocator or stlsoft::new_allocator
#endif /* STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR */

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

/** \brief \ref group__pattern__type_generator "Type generator" class
 *   template that selects an appropriate allocator specialisation for the
 *   given type.
 *
 * It is used in several components in the STLSoft libraries, performing the
 * function of selecting a specialisation of <code>std::allocator</code>
 * for compilers/libraries where that is defined and compatible, otherwise
 * selecting a specialisation of one of the allocator classes provided by
 * STLSoft.
 *
 * Although not likely to be necessary, it is available for use in user's
 * class templates, as in:
\code
template< typename T
        , typename A = typename stlsoft::allocator_selector<T>::allocator_type
        >
class my_buffer
{
public: // Member types
  typedef T   value_type;
  typedef A   allocator_type;

  . . .
};
\endcode
 *
 * \ingroup group__library__memory
 *
 * \see stlsoft::auto_buffer
 */
template <ss_typename_param_k T>
struct allocator_selector
{
#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
    /// The actual allocator specialisation
    ///
    /// This will be one of:
    ///  stlsoft::malloc_allocator<T>,
    ///  stlsoft::new_allocator<T>
    /// or
    ///  stlsoft::std::allocator<T>.
    typedef _some_allocator_<T>                 allocator_type;
#elif defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_MALLOC_ALLOCATOR)
    typedef malloc_allocator<T>                 allocator_type;
#elif defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STLSOFT_NEW_ALLOCATOR)
    typedef new_allocator<T>                    allocator_type;
#elif defined(STLSOFT_ALLOCATOR_SELECTOR_USE_STD_ALLOCATOR)
    typedef stlsoft_ns_qual_std(allocator)<T>   allocator_type;
#else /* USE_??? */
# error Error in discrimination. allocator_selector must select either std::allocator, stlsoft::malloc_allocator or stlsoft::new_allocator
#endif /* allocator */
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_MEMORY_HPP_ALLOCATOR_SELECTOR */

/* ///////////////////////////// end of file //////////////////////////// */
