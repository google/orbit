// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.0 (2014/08/11)

#pragma once

#include "GteVector.h"

namespace gte
{

template <typename Real>
class Vector3 : public Vector<3,Real>
{
public:
    // Construction and destruction.  The destructor hides the base-class
    // destructor, but the latter has no side effects.  Vector3 is designed
    // to provide specialized constructors and geometric operations.  The
    // default constructor does not initialize its data.
    ~Vector3();
    Vector3();
    Vector3(Vector3 const& vec);
    Vector3(Vector<3,Real> const& vec);
    Vector3(Real x0, Real x1, Real x2);

    // Assignment.
    Vector3& operator=(Vector3 const& vec);
    Vector3& operator=(Vector<3,Real> const& vec);

    // Special geometric vectors.
    static Vector3 Origin();   // (0,0,0)
    static Vector3 Basis0();   // (1,0,0)
    static Vector3 Basis1();   // (0,1,0)
    static Vector3 Basis2();   // (0,0,1)
};

// Cross, UnitCross, and DotCross have a template parameter N that should
// be 3 or 4.  The latter case supports affine vectors in 4D (last component
// w = 0) when you want to use 4-tuples and 4x4 matrices for affine algebra.

// Compute the cross product using the formal determinant:
//   cross = det{{e0,e1,e2},{x0,x1,x2},{y0,y1,y2}}
//         = (x1*y2-x2*y1, x2*y0-x0*y2, x0*y1-x1*y0)
// where e0 = (1,0,0), e1 = (0,1,0), e2 = (0,0,1), v0 = (x0,x1,x2), and
// v1 = (y0,y1,y2).
template <int N, typename Real>
Vector<N,Real> Cross (Vector<N,Real> const& v0, Vector<N,Real> const& v1);

// Compute the normalized cross product.
template <int N, typename Real>
Vector<N, Real> UnitCross(Vector<N,Real> const& v0, Vector<N,Real> const& v1);

// Compute Dot((x0,x1,x2),Cross((y0,y1,y2),(z0,z1,z2)), the triple scalar
// product of three vectors, where v0 = (x0,x1,x2), v1 = (y0,y1,y2), and
// v2 is (z0,z1,z2).
template <int N, typename Real>
Real DotCross(Vector<N,Real> const& v0, Vector<N,Real> const& v1,
    Vector<N,Real> const& v2);

// Compute a right-handed orthonormal basis for the orthogonal complement
// of the input vectors.  The function returns the smallest length of the
// unnormalized vectors computed during the process.  If this value is nearly
// zero, it is possible that the inputs are linearly dependent (within
// numerical round-off errors).  On input, numInputs must be 1 or 2 and
// v[0] through v[numInputs-1] must be initialized.  On output, the
// vectors v[0] through v[2] form an orthonormal set.
template <typename Real>
Real ComputeOrthogonalComplement(int numInputs, Vector3<Real>* v);

// Compute the barycentric coordinates of the point P with respect to the
// tetrahedron <V0,V1,V2,V3>, P = b0*V0 + b1*V1 + b2*V2 + b3*V3, where
// b0 + b1 + b2 + b3 = 1.  The return value is 'true' iff {V0,V1,V2,V3} is
// a linearly independent set.  Numerically, this is measured by
// |det[V0 V1 V2 V3]| <= epsilon.  The values bary[] are valid only when the
// return value is 'true' but set to zero when the return value is 'false'.
template <typename Real>
bool ComputeBarycentrics(Vector3<Real> const& p, Vector3<Real> const& v0,
    Vector3<Real> const& v1, Vector3<Real> const& v2, Vector3<Real> const& v3,
    Real bary[4], Real epsilon = (Real)0);

// Get intrinsic information about the input array of vectors.  The return
// value is 'true' iff the inputs are valid (numVectors > 0, v is not null,
// and epsilon >= 0), in which case the class members are valid.
template <typename Real>
class IntrinsicsVector3
{
public:
    // The constructor sets the class members based on the input set.
    IntrinsicsVector3(int numVectors, Vector3<Real> const* v, Real inEpsilon);

    // A nonnegative tolerance that is used to determine the intrinsic
    // dimension of the set.
    Real epsilon;

    // The intrinsic dimension of the input set, computed based on the
    // nonnegative tolerance mEpsilon.
    int dimension;

    // Axis-aligned bounding box of the input set.  The maximum range is
    // the larger of max[0]-min[0], max[1]-min[1], and max[2]-min[2].
    Real min[3], max[3];
    Real maxRange;

    // Coordinate system.  The origin is valid for any dimension d.  The
    // unit-length direction vector is valid only for 0 <= i < d.  The
    // extreme index is relative to the array of input points, and is also
    // valid only for 0 <= i < d.  If d = 0, all points are effectively
    // the same, but the use of an epsilon may lead to an extreme index
    // that is not zero.  If d = 1, all points effectively lie on a line
    // segment.  If d = 2, all points effectively line on a plane.  If
    // d = 3, the points are not coplanar.
    Vector3<Real> origin;
    Vector3<Real> direction[3];

    // The indices that define the maximum dimensional extents.  The
    // values extreme[0] and extreme[1] are the indices for the points
    // that define the largest extent in one of the coordinate axis
    // directions.  If the dimension is 2, then extreme[2] is the index
    // for the point that generates the largest extent in the direction
    // perpendicular to the line through the points corresponding to
    // extreme[0] and extreme[1].  Furthermore, if the dimension is 3,
    // then extreme[3] is the index for the point that generates the
    // largest extent in the direction perpendicular to the triangle
    // defined by the other extreme points.  The tetrahedron formed by the
    // points V[extreme[0]], V[extreme[1]], V[extreme[2]], and
    // V[extreme[3]] is clockwise or counterclockwise, the condition
    // stored in extremeCCW.
    int extreme[4];
    bool extremeCCW;
};

#include "GteVector3.inl"

}
