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
class Vector2 : public Vector<2,Real>
{
public:
    // Construction and destruction.  The destructor hides the base-class
    // destructor, but the latter has no side effects.  Vector2 is designed
    // to provide specialized constructors and geometric operations.  The
    // default constructor does not initialize its data.
    ~Vector2();
    Vector2();
    Vector2(Vector2 const& vec);
    Vector2(Vector<2,Real> const& vec);
    Vector2(Real x0, Real x1);

    // Assignment.
    Vector2& operator=(Vector2 const& vec);
    Vector2& operator=(Vector<2,Real> const& vec);

    // Special geometric vectors.
    static Vector2 Origin();   // (0,0)
    static Vector2 Basis0();   // (1,0)
    static Vector2 Basis1();   // (0,1)
};

// Compute the perpendicular using the formal determinant,
//   perp = det{{e0,e1},{x0,x1}} = (x1,-x0)
// where e0 = (1,0), e1 = (0,1), and v = (x0,x1).
template <typename Real>
Vector2<Real> Perp(Vector2<Real> const& v);

// Compute the normalized perpendicular.
template <typename Real>
Vector2<Real> UnitPerp(Vector2<Real> const& v);

// Compute Dot((x0,x1),Perp(y0,y1)) = x0*y1 - x1*y0, where v0 = (x0,x1)
// and v1 = (y0,y1).
template <typename Real>
Real DotPerp(Vector2<Real> const& v0, Vector2<Real> const& v1);

// Compute a right-handed orthonormal basis for the orthogonal complement
// of the input vectors.  The function returns the smallest length of the
// unnormalized vectors computed during the process.  If this value is nearly
// zero, it is possible that the inputs are linearly dependent (within
// numerical round-off errors).  On input, numInputs must be 1 and v[0]
// must be initialized.  On output, the vectors v[0] and v[1] form an
// orthonormal set.
template <typename Real>
Real ComputeOrthogonalComplement(int numInputs, Vector2<Real>* v);

// Compute the barycentric coordinates of the point P with respect to the
// triangle <V0,V1,V2>, P = b0*V0 + b1*V1 + b2*V2, where b0 + b1 + b2 = 1.
// The return value is 'true' iff {V0,V1,V2} is a linearly independent set.
// Numerically, this is measured by |det[V0 V1 V2]| <= epsilon.  The values
// bary[] are valid only when the return value is 'true' but set to zero when
// the return value is 'false'.
template <typename Real>
bool ComputeBarycentrics(Vector2<Real> const& p, Vector2<Real> const& v0,
    Vector2<Real> const& v1, Vector2<Real> const& v2, Real bary[3],
    Real epsilon = (Real)0);

// Get intrinsic information about the input array of vectors.  The return
// value is 'true' iff the inputs are valid (numVectors > 0, v is not null,
// and epsilon >= 0), in which case the class members are valid.
template <typename Real>
class IntrinsicsVector2
{
public:
    // The constructor sets the class members based on the input set.
    IntrinsicsVector2(int numVectors, Vector2<Real> const* v, Real inEpsilon);

    // A nonnegative tolerance that is used to determine the intrinsic
    // dimension of the set.
    Real epsilon;

    // The intrinsic dimension of the input set, computed based on the
    // nonnegative tolerance mEpsilon.
    int dimension;

    // Axis-aligned bounding box of the input set.  The maximum range is
    // the larger of max[0]-min[0] and max[1]-min[1].
    Real min[2], max[2];
    Real maxRange;

    // Coordinate system.  The origin is valid for any dimension d.  The
    // unit-length direction vector is valid only for 0 <= i < d.  The
    // extreme index is relative to the array of input points, and is also
    // valid only for 0 <= i < d.  If d = 0, all points are effectively
    // the same, but the use of an epsilon may lead to an extreme index
    // that is not zero.  If d = 1, all points effectively lie on a line
    // segment.  If d = 2, the points are not collinear.
    Vector2<Real> origin;
    Vector2<Real> direction[2];

    // The indices that define the maximum dimensional extents.  The
    // values extreme[0] and extreme[1] are the indices for the points
    // that define the largest extent in one of the coordinate axis
    // directions.  If the dimension is 2, then extreme[2] is the index
    // for the point that generates the largest extent in the direction
    // perpendicular to the line through the points corresponding to
    // extreme[0] and extreme[1].  The triangle formed by the points
    // V[extreme[0]], V[extreme[1]], and V[extreme[2]] is clockwise or
    // counterclockwise, the condition stored in extremeCCW.
    int extreme[3];
    bool extremeCCW;
};

#include "GteVector2.inl"

}
