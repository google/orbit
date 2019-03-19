//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "CallstackTypes.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
struct CallStackPOD
{
    CallStackPOD(){ memset( this, 0, sizeof( CallStackPOD ) ); }
    static CallStackPOD Walk( DWORD64 a_Rip, DWORD64 a_Rsp );
    size_t GetSizeInBytes(){ return offsetof(CallStackPOD, m_Data) + m_Depth*sizeof(m_Data[0]); }

    inline CallstackID Hash()
    {
        m_Hash = XXH64( &m_Data[0], m_Depth * sizeof( DWORD64 ), 0xca1157ac ); 
        return m_Hash;
    }

    // Callstack needs to be POD
    CallstackID m_Hash;
    int         m_Depth;
    ThreadID    m_ThreadId;
    DWORD64     m_Data[ORBIT_STACK_SIZE]; // Needs to be last member
};

//-----------------------------------------------------------------------------
struct CallStack
{
    CallStack(){ memset( this, 0, sizeof( CallStack ) ); }
    CallStack( CallStackPOD a_CS );
    inline CallstackID Hash() 
    { 
        m_Hash = XXH64( m_Data.data(), m_Depth * sizeof( DWORD64 ), 0xca1157ac );
        return m_Hash; 
    }
    void Print();
    std::wstring GetString();

    CallstackID m_Hash;
    int         m_Depth;
    ThreadID    m_ThreadId;
    std::vector<DWORD64> m_Data;

    ORBIT_SERIALIZABLE;
};

#ifdef _WIN32
//-----------------------------------------------------------------------------
struct StackFrame
{
    StackFrame( HANDLE a_Thread );
    CONTEXT       m_Context;
    STACKFRAME64  m_StackFrame;
    DWORD         m_ImageType;
    CallStack     m_Callstack;
};

//-----------------------------------------------------------------------------
inline CallStack GetCallstackRtl()
{
    //SCOPE_TIMER_LOG( "GetCallstack()" );
    CallStack cs;
    void* stack[ORBIT_STACK_SIZE];
    ULONG hash;
    USHORT numFrames = RtlCaptureStackBackTrace( 2, ORBIT_STACK_SIZE, stack, &hash );

    for( USHORT i = 0; i < numFrames; ++i )
    {
        cs.m_Data[i] = (DWORD64)stack[i];
    }

    cs.m_Hash = hash;
    cs.m_Depth = numFrames;
    cs.m_ThreadId = GetCurrentThreadId();

    return cs;
}

#ifndef _WIN64
//-----------------------------------------------------------------------------
inline CallStack GetCallstack( DWORD64 a_ProgramCounter, DWORD64 a_AddressOfReturnAddress )
{
    unsigned int depth = 0;

    SetLastError(0);
    PRINT_VAR( GetLastErrorAsString() );

    HANDLE procHandle = GetCurrentProcess();
    HANDLE threadHandle = GetCurrentThread();

    StackFrame frame( threadHandle );

	frame.m_Context.Eip = (DWORD)a_ProgramCounter;
	frame.m_Context.Ebp = (DWORD)a_AddressOfReturnAddress - 4;

    PRINT_VAR( GetLastErrorAsString() );
    while( true )
    {
        bool success = StackWalk64( frame.m_ImageType
                                  , procHandle
                                  , threadHandle
                                  , &frame.m_StackFrame
                                  , &frame.m_Context
                                  , nullptr
                                  , &SymFunctionTableAccess64
                                  , &SymGetModuleBase64
                                  , nullptr ) != 0;
        if( !success )
        {
            PRINT_VAR( GetLastErrorAsString() );
            break;
        }

        if( frame.m_StackFrame.AddrPC.Offset && depth < ORBIT_STACK_SIZE )
        {
            frame.m_Callstack.m_Data[depth++] = frame.m_StackFrame.AddrPC.Offset;
        }
        else
        {
            break;
        }
    }

    if( depth > 0 )
    {
        frame.m_Callstack.m_Depth = depth;
        frame.m_Callstack.m_ThreadId = GetCurrentThreadId();
    }

    return frame.m_Callstack;
}

//-----------------------------------------------------------------------------
inline CallStackPOD GetCallstackManual( DWORD64 a_ProgramCounter, DWORD64 a_AddressOfReturnAddress )
{
    CallStackPOD CS;

    CS.m_Data[CS.m_Depth++] = a_ProgramCounter;

    DWORD* Ebp = (DWORD*)((DWORD)a_AddressOfReturnAddress - 4);
    DWORD returnAddress = *(Ebp+1);

    while( returnAddress && CS.m_Depth < ORBIT_STACK_SIZE )
    {
        CS.m_Data[CS.m_Depth++] = returnAddress;
        Ebp = reinterpret_cast<DWORD*>(*Ebp);
        returnAddress = Ebp ? *(Ebp+1) : NULL;
    }

    return CS;
}

//-----------------------------------------------------------------------------
inline CallStack GetCallStackAsm()
{
    CONTEXT c;
    memset( &c, 0, sizeof( CONTEXT ) );
    
    __asm    call x
    __asm    x: pop eax
    __asm    mov c.Eip, eax
    __asm    mov c.Ebp, ebp
    __asm    mov c.Esp, esp

    return GetCallstackManual( c.Eip, c.Ebp + 4 );
}

#endif

#endif
