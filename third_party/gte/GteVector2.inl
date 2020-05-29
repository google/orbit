// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.1 (2014/08/19)

//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>::~Vector2()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>::Vector2()
{
    // Uninitialized.
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>::Vector2(Vector2 const& vec)
    :
    Vector<2,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>::Vector2(Vector<2,Real> const& vec)
    :
    Vector<2,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>::Vector2(Real x0, Real x1)
{
    this->mTuple[0] = x0;
    this->mTuple[1] = x1;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>& Vector2<Real>::operator=(Vector2 const& vec)
{
    Vector<2,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real>& Vector2<Real>::operator=(Vector<2,Real> const& vec)
{
    Vector<2,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real> Vector2<Real>::Origin()
{
    return Vector2((Real)0, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real> Vector2<Real>::Basis0()
{
    return Vector2((Real)1, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real> Vector2<Real>::Basis1()
{
    return Vector2((Real)0, (Real)1);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real> Perp(Vector2<Real> const& v)
{
    return Vector2<Real>(v[1], -v[0]);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector2<Real> UnitPerp(Vector2<Real> const& v)
{
    Vector2<Real> unitPerp(v[1], -v[0]);
    Normalize(unitPerp);
    return unitPerp;
}
//----------------------------------------------------------------------------
template <typename Real>
Real DotPerp(Vector2<Real> const& v0, Vector2<Real> const& v1)
{
    return Dot(v0, Perp(v1));
}
//----------------------------------------------------------------------------
template <typename Real>
Real ComputeOrthogonalComplement(int numInputs, Vector2<Real>* v)
{
    if (numInputs == 1)
    {
        v[1] = -Perp(v[0]);
        return Orthonormalize<2,Real>(2, v);
    }

    return (Real)0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool ComputeBarycentrics(Vector2<Real> const& p, Vector2<Real> const& v0,
    Vector2<Real> const& v1, Vector2<Real> const& v2, Real bary[3],
    Real epsilon)
{
    // Compute the vectors relative to V2 of the triangle.
    Vector2<Real> diff[3] = { v0 - v2, v1 - v2, p - v2 };

    Real det = DotPerp(diff[0], diff[1]);
    if (det < -epsilon || det > epsilon)
    {
        Real invDet = ((Real)1)/det;
        bary[0] = DotPerp(diff[2], diff[1])*invDet;
        bary[1] = DotPerp(diff[0], diff[2])*invDet;
        bary[2] = (Real)1 - bary[0] - bary[1];
        return true;
    }

    for (int i = 0; i < 3; ++i)
    {
        bary[i] = (Real)0;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename Real>
IntrinsicsVector2<Real>::IntrinsicsVector2(int numVectors,
    Vector2<Real> const* v, Real inEpsilon)
    :
    epsilon(inEpsilon),
    dimension(0),
    maxRange((Real)0),
    origin(Vector2<Real>::Zero()),
    extremeCCW(false)
{
    min[0] = (Real)0;
    min[1] = (Real)0;
    direction[0] = Vector2<Real>::Zero();
    direction[1] = Vector2<Real>::Zero();
    extreme[0] = 0;
    extreme[1] = 0;
    extreme[2] = 0;

    if (numVectors > 0 && v && epsilon >= (Real)0)
    {
        // Compute the axis-aligned bounding box for the input vectors.  Keep
        // track of the indices into 'vectors' for the current min and max.
        int j, indexMin[2], indexMax[2];
        for (j = 0; j < 2; ++j)
        {
            min[j] = v[0][j];
            max[j] = min[j];
            indexMin[j] = 0;
            indexMax[j] = 0;
        }

        int i;
        for (i = 1; i < numVectors; ++i)
        {
            for (j = 0; j < 2; ++j)
            {
                if (v[i][j] < min[j])
                {
                    min[j] = v[i][j];
                    indexMin[j] = i;
                }
                else if (v[i][j] > max[j])
                {
                    max[j] = v[i][j];
                    indexMax[j] = i;
                }
            }
        }

        // Determine the maximum range for the bounding box.
        maxRange = max[0] - min[0];
        extreme[0] = indexMin[0];
        extreme[1] = indexMax[0];
        Real range = max[1] - min[1];
        if (range > maxRange)
        {
            maxRange = range;
            extreme[0] = indexMin[1];
            extreme[1] = indexMax[1];
        }

        // The origin is either the vector of minimum x0-value or vector of
        // minimum x1-value.
        origin = v[extreme[0]];

        // Test whether the vector set is (nearly) a vector.
        if (maxRange <= epsilon)
        {
            dimension = 0;
            for (j = 0; j < 2; ++j)
            {
                extreme[j + 1] = extreme[0];
            }
            return;
        }

        // Test whether the vector set is (nearly) a line segment.  We need
        // direction[1] to span the orthogonal complement of direction[0].
        direction[0] = v[extreme[1]] - origin;
        Normalize(direction[0]);
        direction[1] = -Perp(direction[0]);

        // Compute the maximum distance of the points from the line
        // origin+t*direction[0].
        Real maxDistance = (Real)0;
        Real maxSign = (Real)0;
        extreme[2] = extreme[0];
        for (i = 0; i < numVectors; ++i)
        {
            Vector2<Real> diff = v[i] - origin;
            Real distance = Dot(direction[1], diff);
            Real sign = (distance >(Real)0 ? (Real)1 :
                (distance < (Real)0 ? (Real)-1 : (Real)0));
            distance = std::abs(distance);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                maxSign = sign;
                extreme[2] = i;
            }
        }

        if (maxDistance <= epsilon*maxRange)
        {
            // The points are (nearly) on the line origin+t*direction[0].
            dimension = 1;
            extreme[2] = extreme[1];
            return;
        }

        dimension = 2;
        extremeCCW = (maxSign > (Real)0);
        return;
    }
}
//----------------------------------------------------------------------------
