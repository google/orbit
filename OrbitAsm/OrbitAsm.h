//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include "OrbitAsmC.h"

//-----------------------------------------------------------------------------
struct OrbitProlog
{
    OrbitProlog(){ memset( this, 0, sizeof( OrbitProlog ) ); }
    void Init();
    Prolog m_Data;
};

//-----------------------------------------------------------------------------
struct OrbitEpilog
{
    OrbitEpilog(){ memset( this, 0, sizeof( OrbitEpilog ) ); }
    void Init();
    Epilog m_Data;
};

#pragma pack(push, 1)
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
extern "C" void OrbitGetSSEContext( OrbitSSEContext * a_Context );
extern "C" void OrbitSetSSEContext( OrbitSSEContext * a_Context );
extern "C" void OrbitPrologAsm();
extern "C" void OrbitEpilogAsm();
#else
//-----------------------------------------------------------------------------
void OrbitPrologAsm();
void OrbitEpilogAsm();
#endif