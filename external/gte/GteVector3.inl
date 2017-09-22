// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.2 (2014/08/19)

//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>::~Vector3()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>::Vector3()
{
    // Uninitialized.
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>::Vector3(Vector3 const& vec)
    :
    Vector<3,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>::Vector3(Vector<3,Real> const& vec)
    :
    Vector<3,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>::Vector3(Real x0, Real x1, Real x2)
{
    this->mTuple[0] = x0;
    this->mTuple[1] = x1;
    this->mTuple[2] = x2;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>& Vector3<Real>::operator=(Vector3 const& vec)
{
    Vector<3,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real>& Vector3<Real>::operator=(Vector<3,Real> const& vec)
{
    Vector<3,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real> Vector3<Real>::Origin()
{
    return Vector3((Real)0, (Real)0, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real> Vector3<Real>::Basis0()
{
    return Vector3((Real)1, (Real)0, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real> Vector3<Real>::Basis1()
{
    return Vector3((Real)0, (Real)1, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real> Vector3<Real>::Basis2()
{
    return Vector3((Real)0, (Real)0, (Real)1);
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> Cross(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N,Real> result;
    result.MakeZero();
    result[0] = v0[1]*v1[2] - v0[2]*v1[1];
    result[1] = v0[2]*v1[0] - v0[0]*v1[2];
    result[2] = v0[0]*v1[1] - v0[1]*v1[0];
    return result;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> UnitCross(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N,Real> unitCross = Cross(v0, v1);
    Normalize(unitCross);
    return unitCross;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real DotCross(Vector<N,Real> const& v0, Vector<N,Real> const& v1,
    Vector<N,Real> const& v2)
{
    return Dot(v0, Cross(v1, v2));
}
//----------------------------------------------------------------------------
template <typename Real>
Real ComputeOrthogonalComplement(int numInputs, Vector3<Real>* v)
{
    if (numInputs == 1)
    {
        if (std::abs(v[0][0]) > std::abs(v[0][1]))
        {
            v[1] = Vector3<Real>(-v[0][2], (Real)0, +v[0][0]);
        }
        else
        {
            v[1] = Vector3<Real>((Real)0, +v[0][2], -v[0][1]);
        }
        numInputs = 2;
    }

    if (numInputs == 2)
    {
        v[2] = Cross(v[0], v[1]);
        return Orthonormalize<3,Real>(3, v);
    }

    return (Real)0;
}
//----------------------------------------------------------------------------
template <typename Real>
bool ComputeBarycentrics(Vector3<Real> const& p, Vector3<Real> const& v0,
    Vector3<Real> const& v1, Vector3<Real> const& v2, Vector3<Real> const& v3,
    Real bary[4], Real epsilon)
{
    // Compute the vectors relative to V3 of the tetrahedron.
    Vector3<Real> diff[4] = { v0 - v3, v1 - v3, v2 - v3, p - v3 };

    Real det = DotCross(diff[0], diff[1], diff[2]);
    if (det < -epsilon || det > epsilon)
    {
        Real invDet = ((Real)1)/det;
        bary[0] = DotCross(diff[3], diff[1], diff[2])*invDet;
        bary[1] = DotCross(diff[3], diff[2], diff[0])*invDet;
        bary[2] = DotCross(diff[3], diff[0], diff[1])*invDet;
        bary[3] = (Real)1 - bary[0] - bary[1] - bary[2];
        return true;
    }

    for (int i = 0; i < 4; ++i)
    {
        bary[i] = (Real)0;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename Real>
IntrinsicsVector3<Real>::IntrinsicsVector3(int numVectors,
    Vector3<Real> const* v, Real inEpsilon)
    :
    epsilon(inEpsilon),
    dimension(0),
    maxRange((Real)0),
    origin(Vector3<Real>::Zero()),
    extremeCCW(false)
{
    min[0] = (Real)0;
    min[1] = (Real)0;
    min[2] = (Real)0;
    direction[0] = Vector3<Real>::Zero();
    direction[1] = Vector3<Real>::Zero();
    direction[2] = Vector3<Real>::Zero();
    extreme[0] = 0;
    extreme[1] = 0;
    extreme[2] = 0;
    extreme[3] = 0;

    if (numVectors > 0 && v && epsilon >= (Real)0)
    {
        // Compute the axis-aligned bounding box for the input vectors.  Keep
        // track of the indices into 'vectors' for the current min and max.
        int j, indexMin[3], indexMax[3];
        for (j = 0; j < 3; ++j)
        {
            min[j] = v[0][j];
            max[j] = min[j];
            indexMin[j] = 0;
            indexMax[j] = 0;
        }

        int i;
        for (i = 1; i < numVectors; ++i)
        {
            for (j = 0; j < 3; ++j)
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
        range = max[2] - min[2];
        if (range > maxRange)
        {
            maxRange = range;
            extreme[0] = indexMin[2];
            extreme[1] = indexMax[2];
        }

        // The origin is either the vector of minimum x0-value, vector of
        // minimum x1-value, or vector of minimum x2-value.
        origin = v[extreme[0]];

        // Test whether the vector set is (nearly) a vector.
        if (maxRange <= epsilon)
        {
            dimension = 0;
            for (j = 0; j < 3; ++j)
            {
                extreme[j + 1] = extreme[0];
            }
            return;
        }

        // Test whether the vector set is (nearly) a line segment.  We need
        // {direction[2],direction[3]} to span the orthogonal complement of
        // direction[0].
        direction[0] = v[extreme[1]] - origin;
        Normalize(direction[0]);
        if (std::abs(direction[0][0]) > std::abs(direction[0][1]))
        {
            direction[1][0] = -direction[0][2];
            direction[1][1] = (Real)0;
            direction[1][2] = +direction[0][0];
        }
        else
        {
            direction[1][0] = (Real)0;
            direction[1][1] = +direction[0][2];
            direction[1][2] = -direction[0][1];
        }
        Normalize(direction[1]);
        direction[2] = Cross(direction[0], direction[1]);

        // Compute the maximum distance of the points from the line
        // origin+t*direction[0].
        Real maxDistance = (Real)0;
        Real distance, dot;
        extreme[2] = extreme[0];
        for (i = 0; i < numVectors; ++i)
        {
            Vector3<Real> diff = v[i] - origin;
            dot = Dot(direction[0], diff);
            Vector3<Real> proj = diff - dot*direction[0];
            distance = Length(proj);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                extreme[2] = i;
            }
        }

        if (maxDistance <= epsilon*maxRange)
        {
            // The points are (nearly) on the line origin+t*direction[0].
            dimension = 1;
            extreme[2] = extreme[1];
            extreme[3] = extreme[1];
            return;
        }

        // Test whether the vector set is (nearly) a planar polygon.  The
        // point v[extreme[2]] is farthest from the line: origin + 
        // t*direction[0].  The vector v[extreme[2]]-origin is not
        // necessarily perpendicular to direction[0], so project out the
        // direction[0] component so that the result is perpendicular to
        // direction[0].
        direction[1] = v[extreme[2]] - origin;
        dot = Dot(direction[0], direction[1]);
        direction[1] -= dot*direction[0];
        Normalize(direction[1]);

        // We need direction[2] to span the orthogonal complement of
        // {direction[0],direction[1]}.
        direction[2] = Cross(direction[0], direction[1]);

        // Compute the maximum distance of the points from the plane
        // origin+t0*direction[0]+t1*direction[1].
        maxDistance = (Real)0;
        Real maxSign = (Real)0;
        extreme[3] = extreme[0];
        for (i = 0; i < numVectors; ++i)
        {
            Vector3<Real> diff = v[i] - origin;
            distance = Dot(direction[2], diff);
            Real sign = (distance >(Real)0 ? (Real)1 :
                (distance < (Real)0 ? (Real)-1 : (Real)0));
            distance = std::abs(distance);
            if (distance > maxDistance)
            {
                maxDistance = distance;
                maxSign = sign;
                extreme[3] = i;
            }
        }

        if (maxDistance <= epsilon*maxRange)
        {
            // The points are (nearly) on the plane origin+t0*direction[0]
            // +t1*direction[1].
            dimension = 2;
            extreme[3] = extreme[2];
            return;
        }

        dimension = 3;
        extremeCCW = (maxSign > (Real)0);
        return;
    }
}
//----------------------------------------------------------------------------
