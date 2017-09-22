//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"

namespace Hijacking
{
    bool CreateHook( void* a_FunctionAddress );
    bool CreateZoneStartHook( void* a_FunctionAddress );
    bool CreateZoneStopHook( void* a_FunctionAddress );
    bool CreateOutputDebugStringHook( void* a_FunctionAddress );
    bool CreateSendDataHook( void* a_FunctionAddress );
    bool CreateUnrealActorHook( void* a_FunctionAddress );
    bool CreateAllocHook( void* a_FunctionAddress );
    bool CreateFreeHook( void* a_FunctionAddress );

    bool CreateHook( void* a_FunctionAddress, void* a_PrologCallback, void* a_EpilogCallback );
    bool EnableHook( void* a_FunctionAddress );
    void EnableHooks( DWORD64* a_Addresses, int a_NumAddresses );
    bool DisableHook( void* a_FunctionAddress );
    bool DisableAllHooks();
    bool SuspendBusyLoopThread( OrbitWaitLoop* a_WaitLoop );
    bool ThawMainThread( OrbitWaitLoop* a_WaitLoop );

    void ClearFunctionArguments();
    void SetFunctionArguments( ULONG64 a_FunctionAddress, const FunctionArgInfo & a_Args );
    void TrackCallstack( ULONG64 a_FunctionAddress );
    void SetUnrealInfo( OrbitUnrealInfo & a_UnrealInfo );
}

//-----------------------------------------------------------------------------
struct ReturnAddress
{
    ReturnAddress() : m_AddressOfReturnAddress(nullptr)
                    , m_OriginalReturnAddress(nullptr)
                    , m_EpilogAddress(nullptr) {}

    void** m_AddressOfReturnAddress;
    void*  m_OriginalReturnAddress;
    void*  m_EpilogAddress;
};

