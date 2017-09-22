//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <windows.h>

#pragma pack(push, 1)
struct OrbitSSEContext
{
    _M128A xmm0;
    _M128A xmm1;
    _M128A xmm2;
    _M128A xmm3;
    _M128A xmm4;
    _M128A xmm5;
    _M128A xmm6;
    _M128A xmm7;
    _M128A xmm8;
    _M128A xmm9;
    _M128A xmm10;
    _M128A xmm11;
    _M128A xmm12;
    _M128A xmm13;
    _M128A xmm14;
    _M128A xmm15;
};
#pragma pack(pop)

#ifdef _WIN64
extern "C" void OrbitGetSSEContext( OrbitSSEContext * a_Context );
extern "C" void OrbitSetSSEContext( OrbitSSEContext * a_Context );
#endif