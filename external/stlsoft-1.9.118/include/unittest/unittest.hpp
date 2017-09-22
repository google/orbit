/* /////////////////////////////////////////////////////////////////////////
 * File:        unittest/unittest.hpp
 *
 * Purpose:     Defines various classes and functions used by the STLSoft
 *              unit-test framework.
 *
 * Created:     26th February 2004
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


/** \file unittest/unittest.hpp
 *
 * \brief [C++ only] Defines various classes and functions used by the
 *    STLSoft unit-test framework
 *   (\ref group__library__unittest "Unit Test" Library).
 */

#ifndef STLSOFT_INCL_UNITTEST_HPP_UNITTEST
#define STLSOFT_INCL_UNITTEST_HPP_UNITTEST

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_UNITTEST_HPP_UNITTEST_MAJOR    4
# define STLSOFT_VER_UNITTEST_HPP_UNITTEST_MINOR    2
# define STLSOFT_VER_UNITTEST_HPP_UNITTEST_REVISION 2
# define STLSOFT_VER_UNITTEST_HPP_UNITTEST_EDIT     38
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[<[STLSOFT-AUTO:NO-DOCFILELABEL]>]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#ifndef STLSOFT_UNITTEST
# error This file should not be used when STLSOFT_UNITTEST is not defined. You should probably refrain from including it directly, since it is an internal implementation feature, and subject to change at any time
#endif /* !STLSOFT_UNITTEST */

#ifndef STLSOFT_INCL_H_STDIO
# define STLSOFT_INCL_H_STDIO
# include <stdio.h>
#endif /* !STLSOFT_INCL_H_STDIO */
#ifndef STLSOFT_INCL_H_WCHAR
# define STLSOFT_INCL_H_WCHAR
# include <wchar.h>
#endif /* !STLSOFT_INCL_H_WCHAR */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{

namespace unittest
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Globals
 */

extern FILE *err;
extern FILE *out;

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */

using ::fprintf;
using ::fwprintf;

/* /////////////////////////////////////////////////////////////////////////
 * Unit test interfaces
 */

#ifndef __STLSOFT_UNITTEST_BASE_FILE
# define __STLSOFT_UNITTEST_BASE_FILE   NULL
#endif /* !__STLSOFT_UNITTEST_BASE_FILE */

/* /////////////////////////////////////////////////////////////////////////
 * Unit test interfaces
 */

/** \brief Interface for the unit test reporter
 *
 * \ingroup group__library__unittest
 */
struct unittest_reporter
{
public:
    virtual void set_project(char const* projectName) = 0;

    virtual void set_component(char const* componentName) = 0;

    virtual void set_file(char const* fileName) = 0;

    virtual void start() = 0;

    virtual void report(char const* message, int line = 0, char const* baseFile = __STLSOFT_UNITTEST_BASE_FILE) = 0;

    virtual void start_section(char const* message, int line = 0, char const* baseFile = __STLSOFT_UNITTEST_BASE_FILE) = 0;

    virtual void end_section(char const* message, int line = 0, char const* baseFile = __STLSOFT_UNITTEST_BASE_FILE) = 0;

    virtual void stop() = 0;

    virtual int  verbosity() const = 0;
};

typedef ss_bool_t (*unittest_function)(unittest_reporter *);

/** \brief Interface for the unit test host
 *
 * \ingroup group__library__unittest
 */
struct unittest_host
{
public:
    virtual ss_uint32_t register_unittest_fn(unittest_function ) = 0;

    virtual void deregister_unittest_fn(ss_uint32_t unittestId) = 0;
};

/* /////////////////////////////////////////////////////////////////////////
 * Unit test functions
 */

/** \brief This function must be defined by the test program, and should return
 * an instance of a class implementing the unittest_host interface (protocol)
 *
 * \ingroup group__library__unittest
 *
 * \note Because the STLSoft components use a scoping initialiser (Schwarz
 * counter) it is important that the implementing object is a heap instance, to
 * ensure that it is not destroyed prior to the destructors for the unit test
 * initialisers using it to deregister themselves.
 */
extern "C" unittest_host *get_unittest_host(void);

/* /////////////////////////////////////////////////////////////////////////
 * Unit test classes
 */

/** \brief This class is used to automatically register the unit-tests for a
 * given compilation unit with the unit test host.
 *
 * \ingroup group__library__unittest
 *
 * To use it you simply declare an instance of it, inside a
 */
class unittest_registrar
{
    typedef unittest_registrar  class_type;

public:
    unittest_registrar(unittest_function pfn)
        : m_host(get_unittest_host())
    {
        m_key = m_host->register_unittest_fn(pfn);
    }
    ~unittest_registrar() stlsoft_throw_0()
    {
        if(0 != m_key)
        {
            m_host->deregister_unittest_fn(m_key);
        }
    }

private:
    unittest_host *const    m_host;
    ss_uint32_t             m_key;

// Not to be implemented
private:
    unittest_registrar(class_type const& rhs);
    unittest_registrar& operator =(class_type const& rhs);
};

/** \brief This class is used within unit tests to simplify the
 * process of registering the test information, and calling
 * \c start() and \c stop() on the reporter.
 *
 * \ingroup group__library__unittest
 *
 */
class unittest_initialiser
{
    typedef unittest_initialiser    class_type;

public:
    /// Constructor
    ///
    /// \param reporter The reporter to be used
    /// \param project The project name
    /// \param component The component name
    /// \param file The file name
    unittest_initialiser(   unittest_reporter*  reporter
                        ,   char const*         project
                        ,   char const*         component
                        ,   char const*         file)
        : m_reporter(reporter)
    {
        STLSOFT_ASSERT(NULL != reporter);
        STLSOFT_ASSERT(NULL != project);
        STLSOFT_ASSERT(NULL != component);
        STLSOFT_ASSERT(NULL != file);

        m_reporter->set_project(project);
        m_reporter->set_file(file);
        m_reporter->set_component(component);

        m_reporter->start();
    }
    ~unittest_initialiser() stlsoft_throw_0()
    {
        m_reporter->stop();
    }

private:
    unittest_reporter *const    m_reporter;

// Not to be implemented
private:
    unittest_initialiser(class_type const& rhs);
    unittest_initialiser& operator =(class_type const& rhs);
};

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace unittest

using unittest::unittest_reporter;
using unittest::unittest_registrar;
#if 0
using unittest::fprintf;
using unittest::fwprintf;
#endif /* 0 */
using unittest::err;
using unittest::out;

} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_UNITTEST_HPP_UNITTEST */

/* ///////////////////////////// end of file //////////////////////////// */
