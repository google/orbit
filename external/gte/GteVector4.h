// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.0 (2014/08/11)

#pragma once

#include "GteVector3.h"

namespace gte
{

template <typename Real>
class Vector4 : public Vector<4,Real>
{
public:
    // Construction and destruction.  The destructor hides the base-class
    // destructor, but the latter has no side effects.  Vector4 is designed
    // to provide specialized constructors and geometric operations.  The
    // default constructor does not initialize its data.
    ~Vector4();
    Vector4();
    Vector4(Vector4 const& vec);
    Vector4(Vector<4,Real> const& vec);
    Vector4(Real x0, Real x1, Real x2, Real x3);

    // Assignment.
    Vector4& operator=(Vector4 const& vec);
    Vector4& operator=(Vector<4,Real> const& vec);

    // Special geometric vectors.
    static Vector4 Origin();   // (0,0,0;1) for 3D embedded in 4D
    static Vector4 Basis0();   // (1,0,0;0)
    static Vector4 Basis1();   // (0,1,0;0)
    static Vector4 Basis2();   // (0,0,1;0)
    static Vector4 Basis3();   // (0,0,0;1)
};

// The Vector3 Cross, UnitCross, and DotCross have a template parameter N that
// should be 3 or 4.  The latter case supports affine vectors in 4D (last
// component w = 0) when you want to use 4-tuples and 4x4 matrices for affine
// algebra.  Thus, you may use those template functions for Vector4.

// Compute the hypercross product using the formal determinant:
//   hcross = det{{e0,e1,e2,e3},{x0,x1,x2,x3},{y0,y1,y2,y3},{z0,z1,z2,z3}}
// where e0 = (1,0,0,0), e1 = (0,1,0,0), e2 = (0,0,1,0), e3 = (0,0,0,1),
// v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3), and v2 = (z0,z1,z2,z3).
template <typename Real>
Vector4<Real> HyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1,
    Vector4<Real> const& v2);

// Compute the normalized hypercross product.
template <typename Real>
Vector4<Real> UnitHyperCross(Vector4<Real> const& v0,
    Vector4<Real> const& v1, Vector4<Real> const& v2);

// Compute Dot(HyperCross((x0,x1,x2,x3),(y0,y1,y2,y3),(z0,z1,z2,z3)),
// (w0,w1,w2,w3)), where v0 = (x0,x1,x2,x3), v1 = (y0,y1,y2,y3),
// v2 = (z0,z1,z2,z3), and v3 = (w0,w1,w2,w3).
template <typename Real>
Real DotHyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1,
    Vector4<Real> const& v2, Vector4<Real> const& v3);

// Compute a right-handed orthonormal basis for the orthogonal complement
// of the input vectors.  The function returns the smallest length of the
// unnormalized vectors computed during the process.  If this value is nearly
// zero, it is possible that the inputs are linearly dependent (within
// numerical round-off errors).  On input, numInputs must be 1, 2, or 3 and
// v[0] through v[numInputs-1] must be initialized.  On output, the vectors
// v[0] through v[3] form an orthonormal set.
template <typename Real>
Real ComputeOrthogonalComplement (int numInputs, Vector4<Real>* v);

#include "GteVector4.inl"

}
