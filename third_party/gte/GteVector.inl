// Geometric Tools LLC, Redmond WA 98052
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 1.0.1 (2014/08/30)

//----------------------------------------------------------------------------
template <int N, typename Real>
void Vector<N,Real>::MakeZero()
{
    std::fill(mTuple.begin(), mTuple.end(), (Real)0);
}
//----------------------------------------------------------------------------
template <int N, typename Real>
void Vector<N,Real>::MakeUnit(int d)
{
    std::fill(mTuple.begin(), mTuple.end(), (Real)0);
    mTuple[d] = (Real)1;
}
//----------------------------------------------------------------------------
template <int N, typename Real> inline
int Vector<N,Real>::GetSize() const
{
    return N;
}
//----------------------------------------------------------------------------
template <int N, typename Real> inline
Real const& Vector<N,Real>::operator[](int i) const
{
    return mTuple[i];
}
//----------------------------------------------------------------------------
template <int N, typename Real> inline
Real& Vector<N,Real>::operator[](int i)
{
    return mTuple[i];
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator==(Vector const& vec) const
{
    return std::equal(mTuple.begin(), mTuple.end(), vec.mTuple.begin());
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator!=(Vector const& vec) const
{
    return !operator==(vec);
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator<(Vector const& vec) const
{
    for (int i = 0; i < N; ++i)
    {
        if (mTuple[i] < vec.mTuple[i])
        {
            return true;
        }

        if (mTuple[i] > vec.mTuple[i])
        {
            return false;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator<=(Vector const& vec) const
{
    for (int i = 0; i < N; ++i)
    {
        if (mTuple[i] < vec.mTuple[i])
        {
            return true;
        }

        if (mTuple[i] > vec.mTuple[i])
        {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator>(Vector const& vec) const
{
    for (int i = 0; i < N; ++i)
    {
        if (mTuple[i] > vec.mTuple[i])
        {
            return true;
        }

        if (mTuple[i] < vec.mTuple[i])
        {
            return false;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool Vector<N,Real>::operator>=(Vector const& vec) const
{
    for (int i = 0; i < N; ++i)
    {
        if (mTuple[i] > vec.mTuple[i])
        {
            return true;
        }

        if (mTuple[i] < vec.mTuple[i])
        {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> Vector<N,Real>::Zero()
{
    Vector<N,Real> v;
    v.MakeZero();
    return v;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> Vector<N,Real>::Unit(int d)
{
    Vector<N,Real> v;
    v.MakeUnit(d);
    return v;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator+(Vector<N,Real> const& v)
{
    return v;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator-(Vector<N,Real> const& v)
{
    Vector<N,Real> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = -v[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator+(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N,Real> result = v0;
    return result += v1;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator-(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N,Real> result = v0;
    return result -= v1;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator*(Vector<N,Real> const& v, Real scalar)
{
    Vector<N,Real> result = v;
    return result *= scalar;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator*(Real scalar, Vector<N,Real> const& v)
{
    Vector<N,Real> result = v;
    return result *= scalar;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator/(Vector<N,Real> const& v, Real scalar)
{
    Vector<N,Real> result = v;
    return result /= scalar;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator+=(Vector<N,Real>& v0, Vector<N,Real> const& v1)
{
    for (int i = 0; i < N; ++i)
    {
        v0[i] += v1[i];
    }
    return v0;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator-=(Vector<N,Real>& v0, Vector<N,Real> const& v1)
{
    for (int i = 0; i < N; ++i)
    {
        v0[i] -= v1[i];
    }
    return v0;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator*=(Vector<N,Real>& v, Real scalar)
{
    for (int i = 0; i < N; ++i)
    {
        v[i] *= scalar;
    }
    return v;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator/=(Vector<N,Real>& v, Real scalar)
{
    if (scalar != (Real)0)
    {
        Real invScalar = ((Real)1)/scalar;
        for (int i = 0; i < N; ++i)
        {
            v[i] *= invScalar;
        }
    }
    else
    {
        for (int i = 0; i < N; ++i)
        {
            v[i] = (Real)0;
        }
    }
    return v;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator*(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N, Real> result = v0;
    return result *= v1;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real> operator/(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Vector<N, Real> result = v0;
    return result /= v1;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator*=(Vector<N,Real>& v0, Vector<N,Real> const& v1)
{
    for (int i = 0; i < N; ++i)
    {
        v0[i] *= v1[i];
    }
    return v0;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N,Real>& operator/=(Vector<N,Real>& v0, Vector<N,Real> const& v1)
{
    for (int i = 0; i < N; ++i)
    {
        v0[i] /= v1[i];
    }
    return v0;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real Dot(Vector<N,Real> const& v0, Vector<N,Real> const& v1)
{
    Real dot = v0[0]*v1[0];
    for (int i = 1; i < N; ++i)
    {
        dot += v0[i]*v1[i];
    }
    return dot;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real Length(Vector<N,Real> const& v)
{
    return sqrt(Dot(v, v));
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real LengthRobust(Vector<N,Real> const& v)
{
    Real maxAbsComp = std::abs(v[0]);
    for (int i = 1; i < N; ++i)
    {
        Real absComp = std::abs(v[i]);
        if (absComp > maxAbsComp)
        {
            maxAbsComp = absComp;
        }
    }

    Real length;
    if (maxAbsComp > (Real)0)
    {
        Vector<N,Real> scaled = v/maxAbsComp;
        length = maxAbsComp*Length(scaled);
    }
    else
    {
        length = (Real)0;
    }
    return length;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real Normalize(Vector<N,Real>& v)
{
    Real length = Length(v);
    v /= length;
    return length;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real NormalizeRobust(Vector<N,Real>& v)
{
    Real maxAbsComp = std::abs(v[0]);
    for (int i = 1; i < N; ++i)
    {
        Real absComp = std::abs(v[i]);
        if (absComp > maxAbsComp)
        {
            maxAbsComp = absComp;
        }
    }

    Real length;
    if (maxAbsComp > (Real)0)
    {
        v /= maxAbsComp;
        length = Length(v);
        v /= length;
        length *= maxAbsComp;
    }
    else
    {
        length = (Real)0;
        for (int i = 0; i < N; ++i)
        {
            v[i] = (Real)0;
        }
    }
    return length;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Real Orthonormalize(int numInputs, Vector<N,Real>* v)
{
    if (v && 1 <= numInputs && numInputs <= N)
    {
        Real minLength = Normalize(v[0]);
        for (int i = 1; i < numInputs; ++i)
        {
            for (int j = 0; j < i; ++j)
            {
                Real dot = Dot(v[i], v[j]);
                v[i] -= v[j]*dot;
            }
            Real length = Normalize(v[i]);
            if (length < minLength)
            {
                minLength = length;
            }
        }
        return minLength;
    }

    return (Real)0;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
bool ComputeExtremes (int numVectors, Vector<N,Real> const* v,
    Vector<N,Real>& vmin, Vector<N,Real>& vmax)
{
    if (v && numVectors > 0)
    {
        vmin = v[0];
        vmax = vmin;
        for (int j = 1; j < numVectors; ++j)
        {
            Vector<N,Real> const& vec = v[j];
            for (int i = 0; i < N; ++i)
            {
                if (vec[i] < vmin[i])
                {
                    vmin[i] = vec[i];
                }
                else if (vec[i] > vmax[i])
                {
                    vmax[i] = vec[i];
                }
            }
        }
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N + 1, Real> HLift(Vector<N, Real> const& v, Real last)
{
    Vector<N + 1, Real> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = v[i];
    }
    result[N] = last;
    return result;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N - 1, Real> HProject(Vector<N, Real> const& v)
{
    static_assert(N >= 2, "Invalid dimension.");
    Vector<N - 1, Real> result;
    for (int i = 0; i < N - 1; ++i)
    {
        result[i] = v[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N + 1, Real> Lift(Vector<N, Real> const& v, int inject, Real value)
{
    Vector<N + 1, Real> result;
    int i;
    for (i = 0; i < inject; ++i)
    {
        result[i] = v[i];
    }
    result[i] = value;
    int j = i;
    for (++j; i < N; ++i, ++j)
    {
        result[j] = v[i];
    }
    return result;
}
//----------------------------------------------------------------------------
template <int N, typename Real>
Vector<N - 1, Real> Project(Vector<N, Real> const& v, int reject)
{
    static_assert(N >= 2, "Invalid dimension.");
    Vector<N - 1, Real> result;
    for (int i = 0, j = 0; i < N - 1; ++i, ++j)
    {
        if (j == reject)
        {
            ++j;
        }
        result[i] = v[j];
    }
    return result;
}
//----------------------------------------------------------------------------
