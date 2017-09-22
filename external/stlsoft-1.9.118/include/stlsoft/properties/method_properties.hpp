/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/properties/method_properties.hpp
 *
 * Purpose:     Method-based properties.
 *
 * Created:     6th October 2003
 * Updated:     10th August 2009
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2003-2009, Matthew Wilson and Synesis Software
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


/** \file stlsoft/properties/method_properties.hpp
 *
 * \brief [C++ only] Definition of the method property implementation
 *  class templates:
 * stlsoft::method_property_get,
 * stlsoft::method_property_set,
 * stlsoft::method_property_getset,
 * stlsoft::method_property_get_external,
 * stlsoft::method_property_set_external,
 * stlsoft::method_property_getset_external,
 * stlsoft::static_method_property_get,
 * stlsoft::static_method_property_set
 * stlsoft::static_method_property_getset,
 * stlsoft::static_method_property_get_external,
 * stlsoft::static_method_property_set_external
 * and
 * stlsoft::static_method_property_getset_external
 *   (\ref group__library__properties "Properties" Library).
 */

#ifndef STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES
#define STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES_MAJOR     4
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES_MINOR     0
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES_REVISION  3
# define STLSOFT_VER_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES_EDIT      57
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/*
[Incompatibilies-start]
STLSOFT_COMPILER_IS_GCC:    __GNUC__ == 3 && defined(__APPLE__)
STLSOFT_COMPILER_IS_MSVC:   _MSC_VER<1200
STLSOFT_COMPILER_IS_WATCOM:
[Incompatibilies-end]
 */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
# include <stlsoft/stlsoft.h>
#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER < 1200
# error stlsoft/properties/method_properties.hpp is not compatible with Visual C++ 5.0 or earlier
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Compatibility
 */

/* Don't want all the nasty crud for handling backwards compilers included in
 * the documentation
 */
#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* Certain compilers are too hopeless to help in any conceivable way. */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
 /* Compatible */
#elif ( defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1200)
# error Compiler is not compatible with method properties
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 */

#ifndef _STLSOFT_NO_NAMESPACE
namespace stlsoft
{
#endif /* _STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#define STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)   \
                                                    \
    P##_prop_offset_##C


/** \def STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET
 *
 * \ingroup group__library__properties
 *
 * \param C The containing class type
 * \param P The property name
 */

#define STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)                                     \
                                                                                        \
    static stlsoft_ns_qual(ss_ptrdiff_t) STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)()    \
    {                                                                                   \
        return STLSOFT_RAW_OFFSETOF(C, P);                                              \
    }


/** \def STLSOFT_METHOD_PROPERTY_GET
 *
 * \ingroup group__library__properties
 *
 * \param V The value type
 * \param R The value reference type
 * \param C The containing class type
 * \param GM The property get accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_GET(V, R, C, GM, P)        \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
                                                            \
    stlsoft_ns_qual(method_property_get)<   V               \
                                        ,   R               \
                                        ,   C               \
                                        ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                        ,   &C::GM          \
                                        >           P

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

# define STLSOFT_METHOD_PROPERTY_GET(V, R, C, GM, P)        \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
                                                            \
    stlsoft_ns_qual(method_property_get)<   V               \
                                        ,   R               \
                                        ,   C               \
                                        ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                        >           P

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */



/** \def STLSOFT_METHOD_PROPERTY_SET
 *
 * \ingroup group__library__properties
 *
 * \param V The value type
 * \param R The value reference type
 * \param C The containing class type
 * \param SM The property set accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_SET(V, R, C, SM, P)        \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
                                                            \
    stlsoft_ns_qual(method_property_set)<   V               \
                                        ,   R               \
                                        ,   C               \
                                        ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                        ,   &C::SM          \
                                        >           P

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */



/** \def STLSOFT_METHOD_PROPERTY_GETSET
 *
 * \ingroup group__library__properties
 *
 * \param V The value type
 * \param RG The get reference type
 * \param RS The set reference type
 * \param C The containing class type
 * \param GM The property get accessor method
 * \param SM The property set accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_GETSET(V, RG, RS, C, GM, SM, P)    \
                                                                    \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)                     \
                                                                    \
    stlsoft_ns_qual(method_property_getset)<    V                   \
                                            ,   RG                  \
                                            ,   RS                  \
                                            ,   C                   \
                                            ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                            ,   &C::GM              \
                                            ,   &C::SM              \
                                            >       P

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

# define STLSOFT_METHOD_PROPERTY_GETSET(V, RG, RS, C, GM, SM, P)    \
                                                                    \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)                     \
                                                                    \
    stlsoft_ns_qual(method_property_getset)<    V                   \
                                            ,   RG                  \
                                            ,   RS                  \
                                            ,   C                   \
                                            ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                            >       P

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */



/** \def STLSOFT_METHOD_PROPERTY_GET_EXTERNAL
 *
 * \ingroup group__library__properties
 *
 * \param R The value reference type
 * \param C The containing class type
 * \param GM The property get accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_GET_EXTERNAL_PROP(R, C, GM, P) \
                                                                \
    stlsoft_ns_qual(method_property_get_external)<  R           \
                                                ,   C           \
                                                ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                                ,   &C::GM      \
                                                >       P

# define STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(R, C, GM, P)  \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
    STLSOFT_METHOD_PROPERTY_GET_EXTERNAL_PROP(R, C, GM, P)

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

# define STLSOFT_METHOD_PROPERTY_GET_EXTERNAL(R, C, GM, P)  \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
                                                            \
    stlsoft_ns_qual(method_property_get_external)<  R       \
                                                ,   C       \
                                                ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                                >       P

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */



/** \def STLSOFT_METHOD_PROPERTY_SET_EXTERNAL
 *
 * \ingroup group__library__properties
 *
 * \param R The value reference type
 * \param C The containing class type
 * \param SM The property set accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_SET_EXTERNAL_PROP(R, C, SM, P) \
                                                                \
    stlsoft_ns_qual(method_property_set_external)<  R           \
                                                ,   C           \
                                                ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)    \
                                                ,   &C::SM      \
                                                >           P

# define STLSOFT_METHOD_PROPERTY_SET_EXTERNAL(R, C, SM, P)  \
                                                            \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)             \
    STLSOFT_METHOD_PROPERTY_SET_EXTERNAL_PROP(R, C, SM, P)

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */



/** \def STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL
 *
 * \ingroup group__library__properties
 *
 * \param RG The get reference type
 * \param RS The set reference type
 * \param C The containing class type
 * \param GM The property get accessor method
 * \param SM The property set accessor method
 * \param P The property name
 *
 * TODO: EXAMPLE HERE
 */

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT

# define STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL_PROP(RG, RS, C, GM, SM, P) \
                                                                            \
    stlsoft_ns_qual(method_property_getset_external)<   RG                  \
                                                    ,   RS                  \
                                                    ,   C                   \
                                                    ,   &C::STLSOFT_METHOD_PROPERTY_OFFSET_NAME(C, P)  \
                                                    ,   &C::GM              \
                                                    ,   &C::SM              \
                                                    >       P

# define STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL(RG, RS, C, GM, SM, P)  \
                                                                        \
    STLSOFT_METHOD_PROPERTY_DEFINE_OFFSET(C, P)                         \
    STLSOFT_METHOD_PROPERTY_GETSET_EXTERNAL_PROP(RG, RS, C, GM, SM, P)

#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */


/* /////////////////////////////////////////////////////////////////////////
 * Classes
 */

/* Designates a property
 */
struct property_tag
{};

struct internal_property_tag
    : public property_tag
{
    enum { is_internal  =   1       };
    enum { is_external  =   0       };
};

struct external_property_tag
    : public property_tag
{
    enum { is_internal  =   0       };
    enum { is_external  =   1       };
};

/** Designates an internal property
 */
template<   int R
        ,   int W
        ,   int S
        >
struct internal_property
    : public internal_property_tag
{
    enum { is_read      =   R       };
    enum { is_write     =   W       };
    enum { is_static    =   S       };
};

/** Designates an external property
 */
template<   int R
        ,   int W
        ,   int S
        >
struct external_property
    : public external_property_tag
{
    enum { is_read      =   R       };
    enum { is_write     =   W       };
    enum { is_static    =   S       };
};

/* /////////////////////////////////////////////////////////////////////////
 * Internal method property classes
 */

// Some compilers store member functions as very large quantities, e.g Borland
// uses 12-bytes on Win32, so the pointer to member is stored in a static method
// of a unique type, whose users are required to call its constructor before
// accessing its static method

/** \brief Provides static storage and access to a get member function of a
 *   given type.
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k T /* The outer class, used to provide uniqueness */
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C   /* The enclosing class */
        >
struct member_get_pointer
{
private:
    member_get_pointer(R (C::*pfn)() const)
    {
        function(pfn, NULL, NULL);
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T);

    static R get(C *pC)
    {
        R   r;

        function(NULL, pC, &r);

        return r;
    }

private:
    static void function(R (C::*pfn)() const , C *pC, R *pR)
    {
        static R (C::*s_pfn)() const    =   pfn;

        STLSOFT_MESSAGE_ASSERT("member_get_pointer called before being initialised!", NULL != s_pfn);

        if(NULL != pC)
        {
            *pR = (pC->*s_pfn)();
        }
    }
};


/** \brief Provides static storage and access to a set member function of a
 *   given type.
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k T /* The outer class, used to provide uniqueness */
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C   /* The enclosing class */
        >
struct member_set_pointer
{
private:
    member_set_pointer(void (C::*pfn)(R ))
    {
        function(pfn, NULL, NULL);
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T);

    static void set(C *pC, R *pR)
    {
        function(NULL, pC, pR);
    }

private:
    static void function(void (C::*pfn)(R ) , C *pC, R *pR)
    {
        static void (C::*s_pfn)(R ) =   pfn;

        STLSOFT_MESSAGE_ASSERT("member_set_pointer called before being initialised!", NULL != s_pfn);

        if(NULL != pC)
        {
            (pC->*s_pfn)(*pR);
        }
    }
};

/** \brief This class provides method-based read-only property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines a get method. It also defines a static method
 * that contains the offset of the given property from within the container.
 * Then the template is parameterised with the value type, the reference type,
 * the container type, the member function and the offset function.
 */
template<   ss_typename_param_k V       /* The actual property value type */
        ,   ss_typename_param_k R       /* The reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   R (C::*PFnGet)() const  /* Pointer to a const member function returning R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_get
    : public internal_property<1, 0, 0>
{
/// \name Member Types
/// @{
public:
    typedef V                                               value_type;
    typedef R                                               reference_type;
    typedef C                                               container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_get<V, R, C, PFnOff, PFnGet>    class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_get<V, R, C, PFnOff>            class_type;
    typedef member_get_pointer<class_type, R, C>            member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

# if defined(STLSOFT_COMPILER_IS_DMC)
/// @}

/// \name Construction
/// @{
public:
# else
private:
# endif /* compiler */
    method_property_get()
    {}
private:
    ss_explicit_k method_property_get(reference_type value)
        : m_value(value)
    {}

#ifndef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
private:
    typedef member_pointer_type PFnGet;

private:
    method_property_get(reference_type (C::*pfn)() const)
    {
        PFnGet  f(pfn);
    }
#endif /* !STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
/// @}

/// \name Accessors
/// @{
public:
    /// Provides read-only access to the property
    operator reference_type() const
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        return (pC->*PFnGet)();
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        return PFnGet::get(pC);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}

/// \name Not to be implemented
/// @{
private:
    /// This method is hidden in order to prevent users of this class from
    /// becoming familiar with using operator = on the property instances
    /// from within the containing class, since doing so with
    /// method_property_getset<> would result in an infinite loop.
    class_type& operator =(reference_type value);
/// @}
};


/** \brief This class provides method-based write-only property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines a set method. It also defines a static method
 * that contains the offset of the given property from within the container.
 * Then the template is parameterised with the value type, the reference type,
 * the container type, the member function and the offset function.
 */
template<   ss_typename_param_k V       /* The actual property value type */
        ,   ss_typename_param_k R       /* The reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   void (C::*PFnSet)(R )   /* Pointer to a member function taking R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_set
    : public internal_property<0, 1, 0>
{
/// \name Member Types
/// @{
private:
    typedef V                                               value_type;
    typedef R                                               reference_type;
    typedef C                                               container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_set<V, R, C, PFnOff, PFnSet>    class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_set<V, R, C, PFnOff>            class_type;
    typedef member_set_pointer<class_type, R, C>            member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

/// \name Construction
/// @{
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
# if defined(STLSOFT_COMPILER_IS_DMC)
public:
# else /* ? compiler */
private:
# endif /* compiler */
    method_property_set()
    {}

private:
    ss_explicit_k method_property_set(reference_type value)
        : m_value(value)
    {}
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

private:
    typedef member_pointer_type PFnSet;

private:
    ss_explicit_k method_property_set(void (C::*pfn)(reference_type ))
    {
        PFnSet f(pfn);
    }
    method_property_set(void (C::*pfn)(reference_type ), reference_type value)
        : m_value(value)
    {
        PFnSet f(pfn);
    }

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
/// @}

/// \name Accessors
/// @{
public:
    /// Provides write-only access to the property
    class_type& operator =(reference_type value)
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        (pC->*PFnSet)(value);
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        PFnSet::set(pC, &value);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

        return *this;
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}
};


/** \brief This class provides method-based read/write property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines get and set methods. It also defines a static
 * method that contains the offset of the given property from within the container.
 * Then the template is parameterised with the value type, the set reference type,
 * the get reference type, the container type, the member functions and the offset
 * function.
 */
template<   ss_typename_param_k V       /* The actual property value type */
        ,   ss_typename_param_k RG      /* The get reference type */
        ,   ss_typename_param_k RS      /* The set reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   RG (C::*PFnGet)() const /* Pointer to a const member function returning R */
        ,   void (C::*PFnSet)(RS)   /* Pointer to a member function taking R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_getset
    : public internal_property<1, 1, 0>
{
/// \name Member Types
/// @{
private:
    typedef V                                                               value_type;
    typedef RG                                                              get_reference_type;
    typedef RS                                                              set_reference_type;
    typedef C                                                               container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_getset<V, RG, RS, C, PFnOff, PFnGet, PFnSet>    class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_getset<V, RG, RS, C, PFnOff>                    class_type;
    typedef member_get_pointer<class_type, RG, C>                           get_member_pointer_type;
    typedef member_set_pointer<class_type, RS, C>                           set_member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

/// \name Construction
/// @{
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
# if defined(STLSOFT_COMPILER_IS_DMC)
public:
# else /* ? compiler */
private:
# endif /* compiler */
    method_property_getset()
    {}
private:
    ss_explicit_k method_property_getset(set_reference_type value)
        : m_value(value)
    {}
#else
private:
    typedef get_member_pointer_type PFnGet;
    typedef set_member_pointer_type PFnSet;

private:
    method_property_getset(get_reference_type (C::*pfnGet)() const, void (C::*pfnSet)(set_reference_type ))
    {
        PFnGet  fg(pfnGet);
        PFnSet  fs(pfnSet);
    }
    method_property_getset(get_reference_type (C::*pfnGet)() const, void (C::*pfnSet)(set_reference_type ), set_reference_type value)
        : m_value(value)
    {
        PFnGet  fg(pfnGet);
        PFnSet  fs(pfnSet);
    }
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

/// \name Accessors
/// @{
public:
    /// Provides read-only access to the property
    operator get_reference_type () const
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        return (pC->*PFnGet)();
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        return PFnGet::get(pC);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    }

    /// Provides write-only access to the property
    class_type& operator =(set_reference_type value)
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        (pC->*PFnSet)(value);
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        PFnSet::set(pC, &value);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

        return *this;
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * External method property classes
 */

/**\brief This class provides indirect method-based read-only property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines a get method. It also defines a static method
 * that contains the offset of the given property from within the container.
 * Then the template is parameterised with the the reference type, the
 * container type, the member function and the offset function.
 */
template<   ss_typename_param_k R       /* The reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   R (C::*PFnGet)() const  /* Pointer to a const member function returning R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_get_external
    : public external_property<1, 0, 0>
{
/// \name Member Types
/// @{
public:
    typedef R                                                   reference_type;
    typedef C                                                   container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_get_external<R, C, PFnOff, PFnGet>  class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_get_external<R, C, PFnOff>          class_type;
    typedef member_get_pointer<class_type, R, C>                member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

#ifndef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
/// @}

/// \name Construction
/// @{
private:
    method_property_get_external(R (C::*pfn)() const)
    {
        PFnGet  f(pfn);
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

private:
    typedef member_pointer_type PFnGet;

#endif /* !STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

/// \name Operators
/// @{
public:
    /// Provides read-only access to the property
    operator reference_type () const
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        return (pC->*PFnGet)();
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        return PFnGet::get(pC);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    }
/// @}

/// \name Not to be implemented
/// @{
private:
    /// This method is hidden in order to prevent users of this class from
    /// becoming familiar with using operator = on the property instances
    /// from within the containing class, since doing so with
    /// method_property_getset<> would result in an infinite loop.
    class_type& operator =(reference_type value);
/// @}
};


/** \brief This class provides indirect method-based write-only property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines a set method. It also defines a static method
 * that contains the offset of the given property from within the container.
 * Then the template is parameterised with the reference type, the container
 * type, the member function and the offset function.
 */
template<   ss_typename_param_k R       /* The reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   void (C::*PFnSet)(R )   /* Pointer to a member function taking R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_set_external
    : public external_property<0, 1, 0>
{
/// \name Member Types
/// @{
public:
    typedef R                                                   reference_type;
    typedef C                                                   container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_set_external<R, C, PFnOff, PFnSet>  class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_set_external<R, C, PFnOff>          class_type;
    typedef member_set_pointer<class_type, R, C>                member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

#ifndef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
/// @}

/// \name Construction
/// @{
private:
    method_property_set_external(void (C::*pfn)(R ))
    {
        PFnSet  f(pfn);
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

private:
    typedef member_pointer_type PFnSet;

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

/// \name Operators
/// @{
public:
    /// Provides read-only access to the property
    method_property_set_external& operator =(reference_type value)
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        (pC->*PFnSet)(value);
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        PFnSet::set(pC, &value);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

        return *this;
    }
/// @}
};



/** \brief This class provides indirect method-based read/write property access
 *
 * \ingroup group__library__properties
 *
 * The containing class defines get and set methods. It also defines a static
 * method that contains the offset of the given property from within the container.
 * Then the template is parameterised with the set reference type, the get
 * reference type, the container type, the member functions and the offset function.
 */
template<   ss_typename_param_k RG      /* The reference type */
        ,   ss_typename_param_k RS      /* The reference type */
        ,   ss_typename_param_k C       /* The enclosing class */
        ,   ss_ptrdiff_t (*PFnOff)()    /* Pointer to function providing offset of property within container */
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   RG (C::*PFnGet)() const /* Pointer to a const member function returning R */
        ,   void (C::*PFnSet)(RS )  /* Pointer to a member function taking R */
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        >
class method_property_getset_external
    : public external_property<1, 1, 0>
{
/// \name Member Types
/// @{
public:
    typedef RG                                                                  get_reference_type;
    typedef RS                                                                  set_reference_type;
    typedef C                                                                   container_type;
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
    typedef method_property_getset_external<RG, RS, C, PFnOff, PFnGet, PFnSet>  class_type;
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    typedef method_property_getset_external<RG, RS, C, PFnOff>                  class_type;
    typedef member_get_pointer<class_type, RG, C>                               get_member_pointer_type;
    typedef member_set_pointer<class_type, RS, C>                               set_member_pointer_type;
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */


#ifndef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
/// @}

/// \name Construction
/// @{
private:
    method_property_getset_external(get_reference_type (C::*pfnGet)() const, void (C::*pfnSet)(set_reference_type ))
    {
        PFnGet  fg(pfnGet);
        PFnSet  fs(pfnSet);
    }

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);

private:
    typedef get_member_pointer_type PFnGet;
    typedef set_member_pointer_type PFnSet;

#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
/// @}

/// \name Operators
/// @{
public:
    /// Provides read-only access to the property
    operator get_reference_type () const
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        return (pC->*PFnGet)();
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        return PFnGet::get(pC);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
    }

    /// Provides read-only access to the property
    class_type& operator =(set_reference_type value)
    {
        ss_ptrdiff_t    offset  =   (*PFnOff)();
        container_type  *pC     =   (container_type*)((ss_byte_t*)this - offset);

#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        (pC->*PFnSet)(value);
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        PFnSet::set(pC, &value);
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */

        return *this;
    }
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * Internal static method property classes
 */

/** \brief Implements static read-only Method Property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C
        ,   R (*PFn)(void)
        >
class static_method_property_get
    : public internal_property<1, 0, 1>
{
/// \name Member Types
/// @{
public:
    typedef V                                           value_type;
    typedef R                                           reference_type;
    typedef C                                           container_type;
    typedef static_method_property_get<V, R, C, PFn>    class_type;
/// @}

/// \name Construction
/// @{
public:
    static_method_property_get()
    {}
    ss_explicit_k static_method_property_get(reference_type value)
        : m_value(value)
    {}

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
/// @}

/// \name Operators
/// @{
public:
    operator reference_type() const
    {
        return (*PFn)();
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}

/// \name Not to be implemented
/// @{
private:
    /// This method is hidden in order to prevent users of this class from
    /// becoming familiar with using operator = on the property instances
    /// from within the containing class, since doing so with
    /// method_property_getset<> would result in an infinite loop.
    class_type& operator =(reference_type value);
/// @}
};

/** \brief Implements static write-only Method Property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k R   /* The reference type */
        ,   ss_typename_param_k C
        ,   void (*PFn)(R )
        >
class static_method_property_set
    : public internal_property<0, 1, 1>
{
/// \name Member Types
/// @{
public:
    typedef V                                               value_type;
    typedef R                                               reference_type;
    typedef C                                               container_type;
    typedef static_method_property_set<V, R, C, PFn>        class_type;
/// @}

/// \name Construction
/// @{
public:
    static_method_property_set()
    {}
    ss_explicit_k static_method_property_set(reference_type value)
        : m_value(value)
    {}

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
/// @}

/// \name Operators
/// @{
public:
    static_method_property_set& operator =(reference_type value)
    {
        (*PFn)(value);

        return *this;
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}
};

/** \brief Implements static read-write Method Property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k V
        ,   ss_typename_param_k RG
        ,   ss_typename_param_k RS
        ,   ss_typename_param_k C
        ,   RG (*PFnGet)(void)
        ,   void (*PFnSet)(RS )
        >
class static_method_property_getset
    : public internal_property<1, 1, 1>
{
/// \name Member Types
/// @{
public:
    typedef V                                                               value_type;
    typedef RG                                                              get_reference_type;
    typedef RS                                                              set_reference_type;
    typedef C                                                               container_type;
    typedef static_method_property_getset<V, RG, RS, C, PFnGet, PFnSet>     class_type;

    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
/// @}

/// \name Construction
/// @{
public:
    static_method_property_getset()
    {}
    ss_explicit_k static_method_property_getset(set_reference_type value)
        : m_value(value)
    {}
/// @}

/// \name Operators
/// @{
public:
    operator get_reference_type() const
    {
        return (*PFnGet)();
    }
    static_method_property_getset& operator =(set_reference_type value)
    {
        (*PFnSet)(value);

        return *this;
    }
/// @}

/// \name Members
/// @{
private:
    value_type  m_value;
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * External static method property classes
 */

/** \brief Implements External static read-only Method Property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k R   /* The reference type */
        ,   R (*PFn)(void)
        >
class static_method_property_get_external
    : public external_property<1, 0, 1>
{
/// \name Member Types
/// @{
public:
    typedef R                                               reference_type;
    typedef static_method_property_get_external<R, PFn>     class_type;
/// @}

/// \name Operators
/// @{
public:
#if !defined(STLSOFT_COMPILER_IS_MSVC) || \
    _MSC_VER > 1200
    operator reference_type() const
    {
        return (*PFn)();
    }
#else /* ? compiler */
    operator R() const
    {
        R   (*pfn)()    =   PFn;

        return (*pfn)();
    }
#endif /* compiler */
/// @}

/// \name Not to be implemented
/// @{
private:
    /// This method is hidden in order to prevent users of this class from
    /// becoming familiar with using operator = on the property instances
    /// from within the containing class, since doing so with
    /// method_property_getset<> would result in an infinite loop.
    class_type& operator =(reference_type value);
/// @}
};

/** \brief Implements External static write-only Method Property.
 */
template<   ss_typename_param_k R   /* The reference type */
        ,   void (*PFn)(R )
        >
class static_method_property_set_external
    : public external_property<0, 1, 1>
{
/// \name Member Types
/// @{
public:
    typedef R                                               reference_type;
    typedef static_method_property_set_external<R, PFn>     class_type;
/// @}

/// \name Operators
/// @{
public:
    static_method_property_set_external& operator =(reference_type value)
    {
        (*PFn)(value);

        return *this;
    }
/// @}
};

/** \brief Implements External static read-write Method Property
 *
 * \ingroup group__library__properties
 */
template<   ss_typename_param_k RG
        ,   ss_typename_param_k RS
        ,   RG (*PFnGet)(void)
        ,   void (*PFnSet)(RS )
        >
class static_method_property_getset_external
    : public external_property<1, 1, 1>
{
/// \name Member Types
/// @{
public:
    typedef RG                                                                  get_reference_type;
    typedef RS                                                                  set_reference_type;
    typedef static_method_property_getset_external<RG, RS, PFnGet, PFnSet>      class_type;
/// @}

/// \name Operators
/// @{
public:
    operator get_reference_type() const
    {
        return (*PFnGet)();
    }
    static_method_property_getset_external& operator =(set_reference_type value)
    {
        (*PFnSet)(value);

        return *this;
    }
/// @}
};

/* /////////////////////////////////////////////////////////////////////////
 * IOStream compatibility
 */

// method_property_getset

template<   ss_typename_param_k V
        ,   ss_typename_param_k RG
        ,   ss_typename_param_k RS
        ,   ss_typename_param_k C
        ,   ss_ptrdiff_t (*PFnOff)()
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   RG (C::*PFnGet)() const
        ,   void (C::*PFnSet)(RS)
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
                    ,   method_property_getset<V, RG, RS, C, PFnOff, PFnGet, PFnSet> const& prop)
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
                    ,   method_property_getset<V, RG, RS, C, PFnOff> const&                 prop)
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
{
    s << static_cast<RG>(prop);

    return s;
}

// method_property_get

template<   ss_typename_param_k V
        ,   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_ptrdiff_t (*PFnOff)()
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   R (C::*PFnGet)() const
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
                    ,   method_property_get<V, R, C, PFnOff, PFnGet> const& prop)
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
                    ,   method_property_get<V, R, C, PFnOff> const&         prop)
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
{
    s << static_cast<R>(prop);

    return s;
}

// method_property_getset_external

template<   ss_typename_param_k RG
        ,   ss_typename_param_k RS
        ,   ss_typename_param_k C
        ,   ss_ptrdiff_t (*PFnOff)()
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   RG (C::*PFnGet)() const
        ,   void (C::*PFnSet)(RS )
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
                    ,   method_property_getset_external<RG, RS, C, PFnOff, PFnGet, PFnSet> const&   prop)
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
                    ,   method_property_getset_external<RG, RS, C, PFnOff> const&                   prop)
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
{
    s << static_cast<RG>(prop);

    return s;
}

// method_property_get_external

template<   ss_typename_param_k R
        ,   ss_typename_param_k C
        ,   ss_ptrdiff_t (*PFnOff)()
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
        ,   R (C::*PFnGet)() const
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
        ,   ss_typename_param_k S
        >
inline S& operator <<(  S& s
#ifdef STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT
                    ,   method_property_get_external<R, C, PFnOff, PFnGet> const&   prop)
#else /* ? STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
                    ,   method_property_get_external<R, C, PFnOff> const&           prop)
#endif /* STLSOFT_CF_MEM_FUNC_AS_TEMPLATE_PARAM_SUPPORT */
{
    s << static_cast<R>(prop);

    return s;
}

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} // namespace stlsoft
#endif /* _STLSOFT_NO_NAMESPACE */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_PROPERTIES_HPP_METHOD_PROPERTIES */

/* ///////////////////////////// end of file //////////////////////////// */
