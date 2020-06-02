// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.1 (2014/08/19)

//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>::~Vector4()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>::Vector4()
{
    // Uninitialized.
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>::Vector4(Vector4 const& vec)
    :
    Vector<4,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>::Vector4(Vector<4,Real> const& vec)
    :
    Vector<4,Real>(vec)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>::Vector4(Real x0, Real x1, Real x2, Real x3)
{
    this->mTuple[0] = x0;
    this->mTuple[1] = x1;
    this->mTuple[2] = x2;
    this->mTuple[3] = x3;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>& Vector4<Real>::operator=(Vector4 const& vec)
{
    Vector<4,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real>& Vector4<Real>::operator=(Vector<4,Real> const& vec)
{
    Vector<4,Real>::operator=(vec);
    return *this;
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> Vector4<Real>::Origin()
{
    return Vector4((Real)0, (Real)0, (Real)0, (Real)1);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> Vector4<Real>::Basis0()
{
    return Vector4((Real)1, (Real)0, (Real)0, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> Vector4<Real>::Basis1()
{
    return Vector4((Real)0, (Real)1, (Real)0, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> Vector4<Real>::Basis2()
{
    return Vector4((Real)0, (Real)0, (Real)1, (Real)0);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> Vector4<Real>::Basis3()
{
    return Vector4((Real)0, (Real)0, (Real)0, (Real)1);
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> HyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1,
    Vector4<Real> const& v2)
{
    Real m01 = v0[0]*v1[1] - v0[1]*v1[0];  // x0*y1 - y0*x1
    Real m02 = v0[0]*v1[2] - v0[2]*v1[0];  // x0*z1 - z0*x1
    Real m03 = v0[0]*v1[3] - v0[3]*v1[0];  // x0*w1 - w0*x1
    Real m12 = v0[1]*v1[2] - v0[2]*v1[1];  // y0*z1 - z0*y1
    Real m13 = v0[1]*v1[3] - v0[3]*v1[1];  // y0*w1 - w0*y1
    Real m23 = v0[2]*v1[3] - v0[3]*v1[2];  // z0*w1 - w0*z1
    return Vector4<Real>
    (
        +m23*v2[1] - m13*v2[2] + m12*v2[3],  // +m23*y2 - m13*z2 + m12*w2
        -m23*v2[0] + m03*v2[2] - m02*v2[3],  // -m23*x2 + m03*z2 - m02*w2
        +m13*v2[0] - m03*v2[1] + m01*v2[3],  // +m13*x2 - m03*y2 + m01*w2
        -m12*v2[0] + m02*v2[1] - m01*v2[2]   // -m12*x2 + m02*y2 - m01*z2
    );
}
//----------------------------------------------------------------------------
template <typename Real>
Vector4<Real> UnitHyperCross(Vector4<Real> const& v0,
    Vector4<Real> const& v1, Vector4<Real> const& v2)
{
    Vector4<Real> unitHyperCross = HyperCross(v0, v1, v2);
    Normalize(unitHyperCross);
    return unitHyperCross;
}
//----------------------------------------------------------------------------
template <typename Real>
Real DotHyperCross(Vector4<Real> const& v0, Vector4<Real> const& v1,
    Vector4<Real> const& v2, Vector4<Real> const& v3)
{
    return Dot(HyperCross(v0, v1, v2), v3);
}
//----------------------------------------------------------------------------
template <typename Real>
Real ComputeOrthogonalComplement(int numInputs, Vector4<Real> v[4])
{
    if (numInputs == 1)
    {
        int maxIndex = 0;
        Real maxAbsValue = std::abs(v[0][0]);
        for (int i = 1; i < 4; ++i)
        {
            Real absValue = std::abs(v[0][i]);
            if (absValue > maxAbsValue)
            {
                maxIndex = i;
                maxAbsValue = absValue;
            }
        }

        if (maxIndex < 2)
        {
            v[1][0] = -v[0][1];
            v[1][1] = +v[0][0];
            v[1][2] = (Real)0;
            v[1][3] = (Real)0;
        }
        else if (maxIndex == 3)
        {
            // Generally, you can skip this clause and swap the last two
            // components.  However, by swapping 2 and 3 in this case, we
            // allow the function to work properly when the inputs are 3D
            // vectors represented as 4D affine vectors (w = 0).
            v[1][0] = (Real)0;
            v[1][1] = +v[0][2];
            v[1][2] = -v[0][1];
            v[1][3] = (Real)0;
        }
        else
        {
            v[1][0] = (Real)0;
            v[1][1] = (Real)0;
            v[1][2] = -v[0][3];
            v[1][3] = +v[0][2];
        }

        numInputs = 2;
    }

    if (numInputs == 2)
    {
        Real det[6] =
        {
            v[0][0]*v[1][1] - v[1][0]*v[0][1],
            v[0][0]*v[1][2] - v[1][0]*v[0][2],
            v[0][0]*v[1][3] - v[1][0]*v[0][3],
            v[0][1]*v[1][2] - v[1][1]*v[0][2],
            v[0][1]*v[1][3] - v[1][1]*v[0][3],
            v[0][2]*v[1][3] - v[1][2]*v[0][3]
        };

        int maxIndex = 0;
        Real maxAbsValue = std::abs(det[0]);
        for (int i = 1; i < 6; ++i)
        {
            Real absValue = std::abs(det[i]);
            if (absValue > maxAbsValue)
            {
                maxIndex = i;
                maxAbsValue = absValue;
            }
        }

        if (maxIndex == 0)
        {
            v[2] = Vector4<Real>(-det[4], +det[2], (Real)0, -det[0]);
        }
        else if (maxIndex <= 2)
        {
            v[2] = Vector4<Real>(+det[5], (Real)0, -det[2], +det[1]);
        }
        else
        {
            v[2] = Vector4<Real>((Real)0, -det[5], +det[4], -det[3]);
        }

        numInputs = 3;
    }

    if (numInputs == 3)
    {
        v[3] = HyperCross(v[0], v[1], v[2]);
        return Orthonormalize<4,Real>(4, v);
    }

    return (Real)0;
}
//----------------------------------------------------------------------------
