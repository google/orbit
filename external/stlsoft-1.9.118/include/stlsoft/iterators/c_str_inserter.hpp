/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/iterators/c_str_inserter.hpp
 *
 * Purpose:     Contains the c_str_ptr_extract_iterator template class and c_str_inserter creator function.
 *
 * Created:     12th October 2004
 * Updated:     28th January 2011
 *
 * Thanks to:   Pablo Aguilar for spotting missing inclusions.
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2004-2011, Matthew Wilson and Synesis Software
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


/** \file stlsoft/iterators/c_str_inserter.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::c_str_ptr_extract_iterator
 *   and its creator function
 *   class template
 *   (\ref group__library__iterators "Iterators" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_ITERATORS_HPP_C_STR_INSERTER
#define STLSOFT_INCL_STLSOFT_ITERATORS_HPP_C_STR_INSERTER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_C_STR_INSERTER_MAJOR     2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_C_STR_INSERTER_MINOR     0
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_C_STR_INSERTER_REVISION  2
# define STLSOFT_VER_STLSOFT_ITERATORS_HPP_C_STR_INSERTER_EDIT      30
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */
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

/** \brief This iterator adaptor translates the values assigned to it via the
 * c_str_ptr access shim, and passes the result to the function on which it's
 * parameterised.
 *
 * \ingroup group__library__iterators
 *
 * It would be used as shown in the following example, which copies all files
 * found in the local directory to the vector of strings:
 *
\code
  std::vector&lt;std::string>  c1;
  unixstl::readdir_sequence files(".");

  std::copy(files.begin(), files.end(), c_str_inserter(std::back_inserter(c1)));

  std::cout &lt;&lt; "Files:" &lt;&lt; std::endl;
  std::copy(c1.begin(), c1.end(), stlsoft::ostream_iterator&lt;std::string>(std::cout, "\t", "\n"));
\endcode
 */

template <ss_typename_param_k F>
struct c_str_ptr_extract_iterator
    : public stlsoft_ns_qual(iterator_base)<stlsoft_ns_qual_std(output_iterator_tag), void, void, void, void>
{
public:
    typedef F                               function_type;
    typedef c_str_ptr_extract_iterator<F>   class_type;
private:
    class deref_proxy;
    friend class deref_proxy;

public:
    ss_explicit_k c_str_ptr_extract_iterator(F &f)
        : m_f(f)
    {}

    deref_proxy operator *()
    {
        return deref_proxy(this);
    }
    class_type& operator ++()
    {
        return *this;
    }
    class_type operator ++(int)
    {
        return class_type(m_f);
    }

/// \name Implementation
/// @{
private:
    class deref_proxy
    {
    public:
        deref_proxy(c_str_ptr_extract_iterator *it)
            : m_it(it)
        {}

    public:
        template <ss_typename_param_k S>
        void operator =(S const& s)
        {
            m_it->invoke_(s);
        }

    private:
        c_str_ptr_extract_iterator  *const m_it;

    // Not to be implemented
    private:
        void operator =(deref_proxy const&);
    };
/// @}

/// \name Implementation
/// @{
private:
    template <ss_typename_param_k S>
    void invoke_(S const& s)
    {
        // Double application to ensure the function understands
        // the C-style string.
        m_f = stlsoft_ns_qual(c_str_ptr)(stlsoft_ns_qual(c_str_ptr)(s));
    }
/// @}

private:
    F   m_f;
};

/** Creates an instance of the c_str insert iterator.
 *
 * \ingroup group__library__iterators
 */
template <ss_typename_param_k F>
inline c_str_ptr_extract_iterator<F> c_str_inserter(F &f)
{
    return c_str_ptr_extract_iterator<F>(f);
}

/** \brief [DEPRECATED] Creates an instance of the c_str insert iterator.
 *
 * \ingroup group__library__iterators
 *
 * \deprecated Use stlsoft::c_str_inserter()
 */
template <ss_typename_param_k F>
inline c_str_ptr_extract_iterator<F> c_str_ptr_inserter(F &f)
{
    return c_str_ptr_extract_iterator<F>(f);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_ITERATORS_HPP_C_STR_INSERTER */

/* ///////////////////////////// end of file //////////////////////////// */
