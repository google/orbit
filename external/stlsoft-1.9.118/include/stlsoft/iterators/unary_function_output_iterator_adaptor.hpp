/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/unary_function_output_iterator_adaptor.hpp
 *
 * Purpose:     Class that adapts a unary function to an output iterator.
 *
 * Created:     20th July 2005
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2005-2009, Matthew Wilson and Synesis Software
 * Copyright (c) 2005, Pablo Aguilar
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


/** \file stlsoft/iterators/unary_function_output_iterator_adaptor.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::unary_function_output_iterator_adaptor
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR_MAJOR     2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR_MINOR     0
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR_REVISION  1
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR_EDIT      14
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER
# include <stlsoft/util/std/iterator_helper.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_UTIL_STD_HPP_ITERATOR_HELPER */

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

/** \brief This class adapts a Unary Function into an OutputIterator, using
 *     CRTP/SCTP.
 *
 * \ingroup group__library__iterators
 *
 * \param F The Unary Function class type
 */
template <ss_typename_param_k F>
struct unary_function_output_iterator_adaptor
{
/// \name Types
/// @{
public:
    typedef F                                           function_class_type;
    typedef unary_function_output_iterator_adaptor<F>   class_type;

public:
    typedef stlsoft_ns_qual_std(output_iterator_tag)    iterator_category;
    typedef void                                        value_type;
    typedef void                                        pointer;
    typedef void                                        reference;
    typedef ss_ptrdiff_t                                difference_type;
private:
    class deref_proxy;
    friend class deref_proxy;
/// @}

/// \name Implementation
/// @{
private:
    class deref_proxy
    {
    public:
        deref_proxy(unary_function_output_iterator_adaptor<F> *it)
            : m_it(it)
        {}

    public:
        template <ss_typename_param_k A>
        void operator =(A const& arg)
        {
            m_it->invoke_(arg);
        }

    private:
        unary_function_output_iterator_adaptor<F>  *const m_it;

    // Not to be implemented
    private:
        void operator =(deref_proxy const&);
    };
/// @}

/// \name Iterator methods
/// @{
public:
    /// \brief Pre-increment operation
    class_type& operator ++()
    {
        return *this;
    }
    /// \brief Post-increment operation
    class_type operator ++(int)
    {
        return *this;
    }

    /// \brief Dereference operator
    deref_proxy operator *()
    {
        return deref_proxy(this);
    }

/// @}

/// \name Implementation
/// @{
private:
    template <ss_typename_param_k A>
    void invoke_(A const& arg)
    {
        (*static_cast<function_class_type*>(this))(arg);
    }
/// @}

private:
    void operator =(class_type const&);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_UNARY_FUNCTION_OUTPUT_ITERATOR_ADAPTOR */

/* ///////////////////////////// end of file //////////////////////////// */
