/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/util/options_verifier.hpp
 *
 * Purpose:     Options verification.
 *
 * Created:     9th November 2004
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


/** \file stlsoft/util/options_verifier.hpp
 *
 * \brief [C++ only] Definition of the stlsoft::options_verifier class
 *   template
 *   (\ref group__library__utility "Utility" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER
#define STLSOFT_INCL_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER_MAJOR     2
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER_MINOR     0
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER_REVISION  4
# define STLSOFT_VER_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER_EDIT      43
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_MSVC:  _MSC_VER==1300
[Incompatibilies-end]
 */

#if defined(STLSOFT_COMPILER_IS_MWERKS) || \
    (   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER == 1200)
# define STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */
#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER == 1300
# pragma warning("Inclusion of this header can lead to ambiguities with the sequence operator (comma) with Visual C++ 7.0")
#endif /* compiler */
#ifndef STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING
# include <stlsoft/shims/access/string.hpp>
#endif /* !STLSOFT_INCL_STLSOFT_SHIMS_ACCESS_HPP_STRING */

#ifndef STLSOFT_INCL_EXCEPTION
# define STLSOFT_INCL_EXCEPTION
# include <exception>                    // for uncaught_exception
#endif /* !STLSOFT_INCL_EXCEPTION */
#ifndef STLSOFT_INCL_STDEXCEPT
# define STLSOFT_INCL_STDEXCEPT
# include <stdexcept>                    // for std::runtime_error
#endif /* !STLSOFT_INCL_STDEXCEPT */

#ifdef STLSOFT_UNITTEST
# include <string>
#endif /* STLSOFT_UNITTEST */

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

/** \brief Exception thrown by the options_verifier class template
 *
 * \ingroup group__library__utility
 *
 */
class option_verification_exception
    : public stlsoft_ns_qual_std(runtime_error)
{
public:
    typedef stlsoft_ns_qual_std(runtime_error)  parent_class_type;
    typedef option_verification_exception       class_type;

public:
    option_verification_exception(char const* message)
        : parent_class_type(message)
    {}
#ifdef STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT
    template <ss_typename_param_k S>
    option_verification_exception(S const& message)
        : parent_class_type(stlsoft_ns_qual(c_str_ptr)(message))
    {}
#endif /* STLSOFT_CF_MEMBER_TEMPLATE_CTOR_SUPPORT */
    /// Copy constructor
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    option_verification_exception(class_type const& rhs)
        : parent_class_type(rhs)
    {}

    /// Copy assignment operator
    ///
    /// \note This has to be provided, to avoid precipitating C4217 with Visual C++
    class_type& operator =(class_type const& rhs)
    {
        parent_class_type::operator =(rhs);

        return *this;
    }
};

/** \brief Exception policy for options verification
 *
 * \ingroup group__library__utility
 *
 */
// [[synesis:class:exception-policy: option_verification_policy]]
struct option_verification_policy
{
public:
    /// The thrown type
    typedef option_verification_exception   thrown_type;

public:
    void operator ()(char const* message)
    {
        STLSOFT_THROW_X(option_verification_exception(message));
    }
};

#ifdef STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS
namespace options_verifier_internal_ns
{
#endif /* STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS */


/** \brief Verifies that a given variable is equal to one of a set of options
 *
 * \ingroup group__library__utility
 *
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k XP  =   option_verification_policy
        >
class options_verifier
#if !defined(STLSOFT_COMPILER_IS_DMC)
    : private XP // Inherit, so we can utilise EBO
#else /* ? compiler */
    : public XP // Inherit, so we can utilise EBO
#endif /* compiler */
{
public:
    typedef T                       value_type;
    typedef XP                      exception_policy_type;
    typedef XP                      parent_class_type;
    typedef options_verifier<T, XP> class_type;

public:
    options_verifier(T const& value, char const* failureMessage)
        : parent_class_type()
        , m_value(value)
        , m_failureMessage(failureMessage)
        , m_bMatched(false)
    {}
    options_verifier(T const& value, exception_policy_type policy, char const* failureMessage)
        : parent_class_type(policy)
        , m_value(value)
        , m_failureMessage(failureMessage)
        , m_bMatched(false)
    {}
    options_verifier(class_type const& rhs)
        : parent_class_type(rhs)
        , m_value(rhs.m_value)
        , m_failureMessage(rhs.m_failureMessage)
        , m_bMatched(rhs.m_bMatched)
    {
        rhs.m_bMatched = true;
    }
    /// Destructor
    ///
    /// \note The destructor for options_verifier deliberately does not
    /// declare a throw() clause because it does indeed throw an exception.
    ///
    /// If we've not had a match, and we've not currently unwinding
    /// from another exception, then we report the failure.
    ///
    /// \note This requires that options_verifier is *never* used
    /// inside a catch block.
    ///
    /// \note CodeWarrior doesn't support uncaught_exception, so we
    /// have a potentially nasty situation whereby an options_verifier
    /// could be being destroyed as part of the cleanup of another
    /// exception. However, since options_verifier should never be
    /// created other than by calling verify_options(), this means that
    /// instances will always be temporary, which means that they'll
    /// never exist for long enough for an external exception to enter
    /// their lifetime. This is another reason why the failure message
    /// is char const*, and not string, and why m_value is a reference,
    /// not a value.
    ~options_verifier()
    {
        if( !m_bMatched &&
# if defined(STLSOFT_COMPILER_IS_MWERKS)
            1)
# else /* ? compiler */
            !::std::uncaught_exception())
# endif /* compiler */
        {
            exception_policy_type   &policy =   *this;

            policy(m_failureMessage);
        }
    }

public:
    template <ss_typename_param_k U>
    class_type& test(U const& u)
    {
        if( !m_bMatched &&
            m_value == u)
        {
            m_bMatched = true;
        }

        return *this;
    }

private:
    T const             &m_value;
    char const* const   m_failureMessage;
    ss_mutable_k bool   m_bMatched;

private:
    class_type& operator =(class_type const&);
};

#ifdef STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS
} // namespace options_verifier_internal_ns

using options_verifier_internal_ns::options_verifier;
#endif /* STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS */


template<ss_typename_param_k T>
options_verifier<T> verify_options(T const& value, char const* failureMessage)
{
    return options_verifier<T>(value, failureMessage);
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k XP
        >
options_verifier<T, XP> verify_options(T const& value, XP const& policy, char const* failureMessage)
{
    return options_verifier<T, XP>(value, policy, failureMessage);
}

#ifdef STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS
namespace options_verifier_internal_ns
{
#endif /* STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS */

template<   ss_typename_param_k T
        ,   ss_typename_param_k XP
        ,   ss_typename_param_k U
        >
inline options_verifier<T, XP>& operator ,(options_verifier<T, XP> &ov, U const& u)
{
    return ov.test(u);
}

#if 1
/** \brief Acts as a temporary reference to the options_verifier
 *
 * \ingroup group__library__utility
 *
 */
template<   ss_typename_param_k T
        ,   ss_typename_param_k U
        >
class options_verifier_comparison_ref
{
public:
    typedef options_verifier<T>                     verifier_type;
    typedef options_verifier_comparison_ref<T, U>   class_type;

public:
    options_verifier_comparison_ref(verifier_type *verifier)
        : m_verifier(verifier)
    {}

    options_verifier_comparison_ref(class_type const& rhs)
        : m_verifier(rhs.m_verifier)
    {}

public:
    class_type& test(U const& u)
    {
        m_verifier->test(u);

        return *this;
    }

private:
    verifier_type   *m_verifier;
};

template<   ss_typename_param_k T
        ,   ss_typename_param_k U
        >
inline options_verifier_comparison_ref<T, U>& operator ==(options_verifier<T> const& ov, U const& u)
{
    // NOTE: Have to declare the ov as const, and const_cast it. That's valid because
    // an options_verifier will only be used in one way, but it's still a bit yuck.
    options_verifier_comparison_ref<T, U>   ovcr(const_cast<options_verifier<T>*>(&ov));

    return ovcr.test(u);
}

template<   ss_typename_param_k T
        ,   ss_typename_param_k U1
        ,   ss_typename_param_k U2
        >
inline options_verifier_comparison_ref<T, U1>& operator ||(options_verifier_comparison_ref<T, U1> &ov, U2 const& u)
{
    return ov.test(u);
}
#endif /* 0 */

#ifdef STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS
} // namespace options_verifier_internal_ns
#endif /* STLSOFT_OPTIONS_VERIFIER_REQUIRES_SEPARATE_NS */

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/options_verifier_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_UTIL_HPP_OPTIONS_VERIFIER */

/* ///////////////////////////// end of file //////////////////////////// */
