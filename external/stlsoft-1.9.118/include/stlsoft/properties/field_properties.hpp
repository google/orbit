/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/properties/field_properties.hpp
 *
 * Purpose:     Field-based properties.
 *
 * Created:     6th October 2003
 * Updated:     3rd March 2010
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2010, Matthew Wilson and Synesis Software
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


/** \file stlsoft/properties/field_properties.hpp
 *
 * \brief [C++ only] Definition of the field property implementation
 *  class templates:
 *  stlsoft::field_property_get,
 *  stlsoft::field_property_set,
 *  stlsoft::field_property_get_external
 *  and
 *  stlsoft::field_property_set_external
 *   (\ref group__library__properties "Properties" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES
#define STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES_MAJOR      4
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES_MINOR      0
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES_REVISION   3
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES_EDIT       32
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:     __GNUC__ == 3 && defined(__APPLE__)
[Incompatibilies-end]
 */

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

/** \brief Class template used to define a read-only internal field
 *   property.
 *
\code
class ClassWithReadOnlyProp
{
public: // Member Types
  typedef std::string   string_type;
  typedef int           index_type;

public: // Construction
  ClassWithReadOnlyProp(char const* instanceName, index_type instanceIndex)
    : name(instanceName)  // Property value can be initialised inside enclosing class
    , index(1)            // Property value can be initialised inside enclosing class
  {}

public: // Operations
  void ReBadge(char const* newInstanceName, index_type newInstanceIndex)
  {
    name  = newInstanceName;  // Property value can be assigned inside enclosing class
    index = newInstanceIndex; // Property value can be assigned inside enclosing class
  }

public: // Properties
  stlsoft::field_property_get<index_type, index_type, ClassWithReadOnlyProp>          index;
  stlsoft::field_property_get<string_type, string_type const&, ClassWithReadOnlyProp> name;
};

. . .

ClassWithReadOnlyProp   c("Object-#1", 1);

int                 index   =   c.index;    // Ok: access the value of the read-only property
std::string const   &name   =   c.name;     // Ok: access the value of the read-only property

c.index =   2;          // Compile error: cannnot write to the read-only property
c.name  =   "new name"; // Compile error: cannnot write to the read-only property
\endcode
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V   /* The actual property value type */
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C   /* The enclosing class */
        >
class field_property_get
{
public:
    typedef V                           value_type;
    typedef R                           reference_type;
    typedef C                           container_type;
    typedef field_property_get<V, R, C> class_type;

# if defined(STLSOFT_COMPILER_IS_DMC)
public:
# else /* ? compiler */
private:
# endif /* compiler */
    field_property_get()
    {}
private:
    ss_explicit_k field_property_get(reference_type value)
        : m_value(value)
    {}

    class_type& operator =(reference_type value)
    {
        m_value = value;

        return *this;
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

public:
    /// Provides read-only access to the property
    operator reference_type () const
    {
        return m_value;
    }

// Members
private:
    value_type  m_value;

// Not to be implemented
private:
    field_property_get(class_type const&);
    class_type& operator =(class_type const&);
};


/** \brief Class template used to define a write-only internal field
 *   property.
 *
\code
class ClassWithWriteOnlyProp
{
public:
  stlsoft::field_property_set<int, int, ClassWithWriteOnlyProp>  prop2;
};

ClassWithWriteOnlyProp  c;
int                     i;

c.prop2 = i;  // Ok: Write to the write-only property

i = c.prop2;  // Compile error: Cannot access the value of the write-only property
\endcode
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V   /* The actual property value type */
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C   /* The enclosing class */
        >
class field_property_set
{
public:
    typedef V                           value_type;
    typedef R                           reference_type;
    typedef C                           container_type;
    typedef field_property_set<V, R, C> class_type;

# if defined(STLSOFT_COMPILER_IS_DMC)
public:
# else /* ? compiler */
private:
# endif /* compiler */
    field_property_set()
    {}
private:
    ss_explicit_k field_property_set(reference_type value)
        : m_value(value)
    {}

    operator reference_type () const
    {
        return m_value;
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

public:
    /// Provides write-only access to the property
    class_type& operator =(reference_type value)
    {
        m_value = value;

        return *this;
    }

private:
    value_type  m_value;

// Not to be implemented
private:
    field_property_set(class_type const&);
    class_type& operator =(class_type const&);
};


#if 1
/** Class template that defines a read-only external property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V   /* The actual property value type */
        ,   ss_typename_param_k R   /* The reference type */
        >
class field_property_get_external
{
public:
    typedef V                                   value_type;
    typedef R                                   reference_type;
    typedef field_property_get_external<V, R>   class_type;

public:
    field_property_get_external(value_type& value)
        : m_value(value)
    {}

// Accessors
public:
    /// Provides read-only access to the property
    operator reference_type() const
    {
        return m_value;
    }

// Members
private:
    value_type  &m_value;
};


/** Class template that defines a write-only external property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V   /* The actual property value type */
        ,   ss_typename_param_k R   /* The reference type */
        >
class field_property_set_external
{
public:
    typedef V                                   value_type;
    typedef R                                   reference_type;
    typedef field_property_get_external<V, R>   class_type;

public:
    field_property_set_external(value_type& value)
        : m_value(value)
    {}

// Accessors
public:
    /// Provides write-only access to the property
    field_property_set_external& operator =(reference_type value)
    {
        m_value = value;

        return *this;
    }

// Members
private:
    value_type  &m_value;
};
#else /* ? 1 */

template<   ss_typename_param_k V       /* The actual property value type */
        ,   ss_typename_param_k R       /* The reference type */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset from value to property */
        >
class field_property_get_external
{
public:
    typedef V                                           value_type;
    typedef R                                           reference_type;
    typedef field_property_get_external<V, R, PFnOff>   class_type;

// Accessors
public:
    /// Provides read-only access to the property
    operator reference_type() const
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        value_type      *pV     =   (value_type*)((ss_byte_t*)this - offset);

        return *pV;
    }
};


template<   ss_typename_param_k V       /* The actual property value type */
        ,   ss_typename_param_k R       /* The reference type */
        ,   ss_ptrdiff_t   (*PFnOff)()  /* Pointer to function providing offset from value to property */
        >
class field_property_set_external
{
public:
    typedef V                                           value_type;
    typedef R                                           reference_type;
    typedef field_property_get_external<V, R, PFnOff>   class_type;

// Accessors
public:
    /// Provides write-only access to the property
    field_property_set_external& operator =(reference_type value)
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        value_type      *pV     =   (value_type*)((ss_byte_t*)this - offset);

        *pV = value;

        return *this;
    }
};
#endif /* 0 */

/* /////////////////////////////////////////////////////////////////////////
 * IOStream compatibility
 */

// field_property_get

template<   ss_typename_param_k V
        ,   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
                    ,   field_property_get<V, R, C> const&  prop)
{
    s << static_cast<R>(prop);

    return s;
}

// field_property_get_external

template<   ss_typename_param_k V
        ,   ss_typename_param_k R
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
                    ,   field_property_get_external<V, R> const& prop)
{
    s << static_cast<R>(prop);

    return s;
}

////////////////////////////////////////////////////////////////////////////
// Unit-testing

#ifdef STLSOFT_UNITTEST
# include "./unittest/field_properties_unittest_.h"
#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_FIELD_PROPERTIES */

/* ///////////////////////////// end of file //////////////////////////// */
