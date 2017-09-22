//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"

#pragma pack(push, 1)

// Integer register
//-----------------------------------------------------------------------------
struct Reg32 { DWORD Low; DWORD High; };
struct RegF  { float LowF; float HighF; };
struct Reg8  { char A; char B; char C; char D; char E; char F; char G; char H; };
struct IntReg
{
    union
    {
        DWORD64 m_Reg64;
        Reg32   m_Reg32;
        RegF    m_RegF;
        Reg8    m_Reg8;
        void*   m_Ptr; // TODO: void* is not the same size on 32 and 64 bits...
    };

    IntReg & operator=(void* a_Ptr) { m_Ptr = a_Ptr; return *this; }
};
static_assert(sizeof(IntReg) == 8, "IntReg must be 64 bits");

// Floating point register
//-----------------------------------------------------------------------------
struct XmmReg64 { DWORD64 Low; DWORD64 High; };
struct XmmRegFloat{ float m_F0; float m_F1; float m_F2; float m_F3; };
struct XmmRegDouble{ double m_D0; double m_D1; };
struct XmmReg
{
    union
    {
        XmmReg64     m_Reg64;
        XmmRegFloat  m_RegFloat;
        XmmRegDouble m_RegDouble;
    };
};
static_assert(sizeof(XmmReg) == 16, "XmmReg must be 128 bits");

// Return value
//-----------------------------------------------------------------------------
struct RetValue
{
    union
    {
        XmmReg m_FloatVal;
        IntReg m_IntVal;
    };
};

//-----------------------------------------------------------------------------
// NOTE:  Context structs are used to cast a stack address and interpret its content.
//        Arguments must match what is being pushed by assembly (orbitasm.h).
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
struct Context64
{
    // Has to match prologue in orbitasm.asm...

    /*
    ...
    STACK	HIGH ADDR
    STACK
    STACK
    STACK
    RSP ->	(a_ReturnAddressLocation)
    RCX	    8   --|
    RDX	    16    |
    R8	    24    |
    R9	    32    |
    RBX	    40    |
    XMM0	48    |
    XMM0	56    |-> Context on stack
    XMM1	64    |
    XMM1	72    |
    XMM2	80    |
    XMM2	88    |
    XMM3	96    |
    XMM3	104   |
    RSP	    112 --|
    RSP	    120 --| // Dummy for stack alignment
    SHADOW
    SHADOW
    SHADOW
    SHADOW	LOW ADDR
    return addr
*/
    IntReg m_Dummy;
    IntReg m_RSP;
    XmmReg m_XMM3;
    XmmReg m_XMM2;
    XmmReg m_XMM1;
    XmmReg m_XMM0;
    IntReg m_RBX;
    IntReg m_R9;
    IntReg m_R8;
    IntReg m_RDX;
    IntReg m_RCX;
    IntReg m_RET;

    enum { MaxStackBytes = 128
         , StackDataSize = MaxStackBytes + sizeof(int) };
    char m_Stack[MaxStackBytes]; // Arguments passed on the stack
    int  m_StackSize;
    
#ifdef _WIN64
    void* GetRet(){ return m_RET.m_Ptr; }
    void* GetThis(){ return m_RCX.m_Ptr; }
#endif
    static int GetFixedDataSize() { return sizeof(Context64) - StackDataSize; }
};

//-----------------------------------------------------------------------------
struct Context32
{
    Context32() : m_StackSize(0) {}
    /*
    ...             HIGH ADDRESS                --|
    function arguments                            |
    ...                                           |
    return addr                                   |
    eax                                           |
    ecx                                           |
    edx                                           |-> Context32
    xmm3                                          |
    xmm2                                          |
    xmm1                                          |
    xmm0                                          |
    eax/esp :address of return address (arg1)     |
    ecx     :address of original function (arg0)--|
    return addr
                    LOW ADDRESS
    */

    DWORD  m_Arg0;
    DWORD  m_Arg1;
    XmmReg m_XMM3;
    XmmReg m_XMM2;
    XmmReg m_XMM1;
    XmmReg m_XMM0;
    DWORD  m_EDX;
    DWORD  m_ECX;
    DWORD  m_EAX;
    DWORD  m_EBP;
    DWORD  m_RET;
    
    enum { MaxStackBytes = 128
         , StackDataSize = MaxStackBytes + sizeof(int)
    };
    char m_Stack[MaxStackBytes]; // Arguments passed on the stack
    int  m_StackSize;

#ifndef _WIN64
    void* GetRet(){ return (void*)m_RET; }
    void* GetThis(){ return (void*)m_ECX; }
#endif
    static int GetFixedDataSize() { return sizeof(Context32) - StackDataSize; }
};

//-----------------------------------------------------------------------------
struct EpilogContext64
{
    DWORD64 GetReturnValue(){ return m_RAX.m_Reg64; }
    XmmReg  m_XMM0;
    IntReg  m_R9;
    IntReg  m_R8;
    IntReg  m_RDX;
    IntReg  m_RCX;
    IntReg  m_RBX;
    IntReg  m_RAX;
};

//-----------------------------------------------------------------------------
struct EpilogContext32
{
    DWORD GetReturnValue(){ return m_EAX; }
    // TODO: push ST0
    XmmReg  m_XMM0;
    DWORD   m_EAX;
};

//-----------------------------------------------------------------------------
struct SavedContext32
{
    Context32       m_Context;
    EpilogContext32 m_EpilogContext;
};

//-----------------------------------------------------------------------------
struct SavedContext64
{
    Context64       m_Context;
    EpilogContext64 m_EpilogContext;
};

#ifdef _WIN64
typedef Context64       Context;
typedef EpilogContext64 EpilogContext;
typedef SavedContext64  SavedContext;
typedef void*           AddressType;
#else
typedef Context32       Context;
typedef EpilogContext32 EpilogContext;
typedef SavedContext32  SavedContext;
typedef DWORD           AddressType;
#endif

#pragma pack(pop)
