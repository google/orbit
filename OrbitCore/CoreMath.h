//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "../external/gte/GteVector2.h"
#include "../external/gte/GteVector3.h"
#include "../external/gte/GteVector4.h"

typedef gte::Vector2<float> Vec2;
typedef gte::Vector3<float> Vec3;
typedef gte::Vector4<float> Vec4;
typedef gte::Vector2<int>   Vec2i;

typedef gte::Vector4<unsigned char> Color;

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    return min( max(v, lo), hi );
}

