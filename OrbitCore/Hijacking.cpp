//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "Hijacking.h"
#include "TcpClient.h"
#include "MinHook.h"
#include "OrbitLib.h"
#include "ScopeTimer.h"
#include "Context.h"
#include "Message.h"
#include "OrbitType.h"
#include "TimerManager.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <intrin.h>
#include "Callstack.h"
#include "../OrbitAsm/OrbitAsm.h"
#include "../external/minhook/src/buffer.h"
#include "../external/minhook/src/trampoline.h"

const unsigned int MAX_DEPTH = 64;

//-----------------------------------------------------------------------------
struct ContextScope
{
#ifdef _WIN64
    // NOTE:  To support Whole Program Optimization, we need more conservative 
    // register preserving.  Tested on oqpi and Unity, works much better now.
    ContextScope() { OrbitGetSSEContext( &m_Context ); }
    ~ContextScope(){ OrbitSetSSEContext( &m_Context ); }
    OrbitSSEContext m_Context;
#endif
};

#ifdef _WIN64
#define SSE_SCOPE ContextScope scope;
#else
#define SSE_SCOPE
#endif

//-----------------------------------------------------------------------------
struct ThreadLocalData
{
    ThreadLocalData()
    {
        m_Timers.reserve( MAX_DEPTH );
        m_ReturnAdresses.reserve( MAX_DEPTH );
        m_Contexts.reserve( MAX_DEPTH );
        m_SentCallstacks.reserve( 1024 );
        m_SessionID = -1;
        m_ThreadID = GetCurrentThreadId();
        m_ThreadName = GetCurrentThreadName();
        m_ZoneStack = 0;
        SendThreadInfo();
    }

    __forceinline void CheckSessionId()
    {
        if( m_SessionID != Message::GSessionID )
        {
            m_SentCallstacks.clear();
            m_SentLiterals.clear();
            m_SentActorNames.clear();
            m_SessionID = Message::GSessionID;
            Timer::ClearThreadDepthTLS();
            m_ZoneStack = 0;
        }
    }

    void SendThreadInfo()
    {
        if (!m_ThreadName.empty())
        {
            Message msg(Msg_ThreadInfo, (m_ThreadName.size()+1) * sizeof(m_ThreadName[0]), (char*)m_ThreadName.data());
            msg.m_ThreadId = m_ThreadID;
            GTcpClient->Send(msg);
        }
    }

    std::wstring                    m_ThreadName;
    std::vector<ReturnAddress>      m_ReturnAdresses;
    std::vector<Timer>              m_Timers;
    std::vector<const Context*>     m_Contexts;
    std::unordered_set<CallstackID> m_SentCallstacks;
    std::unordered_set<char*>       m_SentLiterals;
    std::unordered_set<char*>       m_SentActorNames;
    int                             m_SessionID;
    DWORD                           m_ThreadID;
    int                             m_ZoneStack;
};

//-----------------------------------------------------------------------------
namespace Hijacking
{
    bool  Initialize();
    bool  Deinitialize();
    
    void  Prolog            ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologZoneStart   ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologZoneStop    ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologOutputDbg   ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologSendData    ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologUnrealActor ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    void  PrologFree        ( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize );
    
    void* Epilog();
    void* EpilogEmpty();
    void* EpilogZoneStop();
    void* EpilogAlloc();

    __forceinline CallstackID SendCallstack( void* a_OriginalFunctionAddress, void** a_ReturnAddressLocation );
    __forceinline void PushReturnAddress( void** a_ReturnAddress );
    __forceinline void PopReturnAddress();
    __forceinline void* GetReturnAddress();
    __forceinline FunctionArgInfo* GetArgInfo( void* a_Address );

    thread_local ThreadLocalData* TlsData;

    __forceinline void CheckTls(){ if( !TlsData ){ TlsData = new ThreadLocalData(); } TlsData->CheckSessionId(); }
    __forceinline void PushContext( const Context* a_Context, void* a_OriginalFunction );
    __forceinline void PushZoneContext( const Context* a_Context, void* a_OriginalFunction );
    __forceinline void PopContext();
    __forceinline void SendContext( const Context* a_Context, EpilogContext* a_EpilogContext );
    __forceinline void SetOriginalReturnAddresses();
    __forceinline void SetOverridenReturnAddresses();
    __forceinline void SendUObjectName( void* a_UnrealActor );
    
    std::unordered_map< ULONG64, FunctionArgInfo > m_FunctionArgsMap;
    std::unordered_set< ULONG64 >                  m_SendCallstacks;
    OrbitUnrealInfo                                m_UnrealInfo;
    
    // On Win64, epilog context is at 40 bytes: 8 bytes (return address) + 32 bytes (shadow space)
    // On Win32, epilog context is at 4 bytes (return address)
    int s_StackOffset = sizeof(void*) > 4 ? 40 : 4;

    struct HijackManager
    {
        HijackManager(){}
        ~HijackManager(){ Deinitialize(); }
    };
    HijackManager GHijackManager;
}

//-----------------------------------------------------------------------------
__forceinline CallstackID Hijacking::SendCallstack( void* a_OriginalFunctionAddress, void** a_ReturnAddressLocation )
{
    /*bool needsCallstack = m_SendCallstacks.find( reinterpret_cast<ULONG64>( a_OriginalFunctionAddress ) ) != m_SendCallstacks.end();
    if( needsCallstack )*/
    {
        SetOriginalReturnAddresses();
        CallStackPOD cs = CallStackPOD::Walk( (DWORD64)a_OriginalFunctionAddress, (DWORD64)a_ReturnAddressLocation );
        SetOverridenReturnAddresses();

        // Send callstack once (*per thread* for now, we should have a concurrent set or hashmap...)
        if( TlsData->m_SentCallstacks.find( cs.m_Hash ) == TlsData->m_SentCallstacks.end() )
        {
            TlsData->m_SentCallstacks.insert( cs.m_Hash );
            GTcpClient->Send( Msg_Callstack, (void*)&cs, cs.GetSizeInBytes() );
        }

        return cs.m_Hash;
    }

   // return 0;
}

//-----------------------------------------------------------------------------
void Hijacking::Prolog( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();

    PushContext( a_Context, a_OriginalFunctionAddress );
    PushReturnAddress( &a_Context->m_RET.m_Ptr );

    TlsData->m_Timers.push_back( Timer() );
    TlsData->m_Timers.back().m_FunctionAddress = reinterpret_cast<ULONG64>( a_OriginalFunctionAddress );
    TlsData->m_Timers.back().m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
    TlsData->m_Timers.back().Start();
}

//-----------------------------------------------------------------------------
void Hijacking::PrologZoneStart( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();

    PushZoneContext( a_Context, a_OriginalFunctionAddress );
    PushReturnAddress( &a_Context->m_RET.m_Ptr );

    ++TlsData->m_ZoneStack;

    TlsData->m_Timers.push_back( Timer() );
    TlsData->m_Timers.back().m_FunctionAddress = reinterpret_cast<ULONG64>( a_OriginalFunctionAddress );
    TlsData->m_Timers.back().m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
    TlsData->m_Timers.back().Start();
}

//-----------------------------------------------------------------------------
void Hijacking::PrologZoneStop( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();
    PushReturnAddress( &a_Context->m_RET.m_Ptr );
}

//-----------------------------------------------------------------------------
void Hijacking::PrologOutputDbg( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();
    OrbitLogEntry entry;
    entry.m_Time = OrbitTicks();

#ifdef _WIN64
    entry.m_Text = (char*)a_Context->m_RCX.m_Ptr;
#else
    entry.m_Text = *((char**)&a_Context->m_Stack[0]);
#endif

    entry.m_ThreadId = TlsData->m_ThreadID;

    PushReturnAddress( &a_Context->m_RET.m_Ptr );
    
    entry.m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
    GTcpClient->Send( entry );
}

//-----------------------------------------------------------------------------
void Hijacking::PrologSendData( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();
    Orbit::UserData entry;
    entry.m_Time = OrbitTicks();

    // __declspec( noinline ) inline void OrbitSendData( void*, int)  { ORBIT_NOOP; }

#ifdef _WIN64
    entry.m_Data = (char*)a_Context->m_RCX.m_Ptr;
    entry.m_NumBytes = (int)a_Context->m_RDX.m_Reg64;
#else
    entry.m_Data = *( (char**)&a_Context->m_Stack[0] );
    entry.m_NumBytes = *( (int*)&a_Context->m_Stack[4] ); //order ??
#endif
    entry.m_ThreadId = TlsData->m_ThreadID;

    PushReturnAddress( &a_Context->m_RET.m_Ptr );

    entry.m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
    GTcpClient->Send( entry );
}


typedef void* (*GetDisplayNameEntryFunction)( void* );
GetDisplayNameEntryFunction GetDisplayNameEntry;

//-----------------------------------------------------------------------------
void Hijacking::PrologUnrealActor( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();
    
    PushContext( a_Context, a_OriginalFunctionAddress );
    PushReturnAddress( &a_Context->m_RET.m_Ptr );

#ifdef _WIN64
    void* uobject = a_Context->m_RCX.m_Ptr;
#else
    void* uobject = nullptr;
#endif
    SendUObjectName( uobject );

    TlsData->m_Timers.push_back( Timer() );
    Timer & timer = TlsData->m_Timers.back();
    timer.m_Type = Timer::UNREAL_OBJECT;
    timer.m_FunctionAddress = reinterpret_cast<ULONG64>( a_OriginalFunctionAddress );
    timer.m_UserData[0] = (DWORD64)uobject;
    timer.m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
    timer.Start();
}

#define NAME_WIDE_MASK 0x1
//-----------------------------------------------------------------------------
__forceinline void Hijacking::SendUObjectName( void* a_UObject )
{
    if( a_UObject )
    {
        void* FName = (char*)a_UObject + m_UnrealInfo.m_UobjectNameOffset;
        void* Entry = GetDisplayNameEntry( FName );
        char* actorName = (char*)Entry + m_UnrealInfo.m_EntryNameOffset;

        if( TlsData->m_SentActorNames.find( actorName ) == TlsData->m_SentActorNames.end() )
        {
            int   Index = *(int*)( (char*)Entry + m_UnrealInfo.m_EntryIndexOffset );
            bool  IsWide = ( Index & NAME_WIDE_MASK );

            if( IsWide )
            {
                wchar_t* actorNameW = (wchar_t*)actorName;
                size_t numChars = wcslen( actorNameW ) + 1;
                Message msg( Msg_OrbitUnrealObject, (int)( numChars * sizeof( wchar_t ) ), (char*)actorNameW );
                msg.m_Header.m_UnrealObjectHeader.m_WideStr = true;
                msg.m_Header.m_UnrealObjectHeader.m_Ptr = (DWORD64)a_UObject;
                msg.m_Header.m_UnrealObjectHeader.m_StrSize = numChars;
                GTcpClient->Send( msg );
            }
            else
            {
                size_t numChars = strlen( actorName ) + 1;
                Message msg( Msg_OrbitUnrealObject, (int)numChars, actorName );
                msg.m_Header.m_UnrealObjectHeader.m_WideStr = false;
                msg.m_Header.m_UnrealObjectHeader.m_Ptr = (DWORD64)a_UObject;
                msg.m_Header.m_UnrealObjectHeader.m_StrSize = numChars;
                GTcpClient->Send( msg );
            }

            TlsData->m_SentActorNames.insert( actorName );
        }
    }
}

//-----------------------------------------------------------------------------
void  Hijacking::PrologFree( void* a_OriginalFunctionAddress, Context* a_Context, unsigned a_ContextSize )
{
    SSE_SCOPE;
    CheckTls();

    PushContext( a_Context, a_OriginalFunctionAddress );
    PushReturnAddress( &a_Context->m_RET.m_Ptr );

    TlsData->m_Timers.push_back( Timer() );
    Timer & timer = TlsData->m_Timers.back();
    timer.m_FunctionAddress = reinterpret_cast<ULONG64>( a_OriginalFunctionAddress );
    timer.m_CallstackHash = SendCallstack( a_OriginalFunctionAddress, &a_Context->m_RET.m_Ptr );
#ifdef _WIN64
    timer.m_UserData[0] = a_Context->m_RCX.m_Reg64;
#else
    timer.m_UserData[0] = 0;
#endif
    timer.SetType(Timer::FREE);
    timer.Start();
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::SetOriginalReturnAddresses()
{
    // In some cases, a hooked function might be a small stub that redirects to 
    // another hooked function using a jmp instruction and not a proper call.
    // In that case, the return address location is the same for both functions.  
    // Make sure we don't interpret the last written address as an overwritten address.

    void* lastWritten = nullptr;

    for( ReturnAddress & ret : TlsData->m_ReturnAdresses )
    {
        ret.m_EpilogAddress = *ret.m_AddressOfReturnAddress;
        
        if( ret.m_AddressOfReturnAddress != lastWritten )
        {
            *ret.m_AddressOfReturnAddress = ret.m_OriginalReturnAddress;
            lastWritten = ret.m_AddressOfReturnAddress;
        }
    }
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::SetOverridenReturnAddresses()
{
    void* lastWritten = nullptr;

    for (ReturnAddress & ret : TlsData->m_ReturnAdresses)
    {
        if( ret.m_AddressOfReturnAddress != lastWritten )
        {
            *ret.m_AddressOfReturnAddress = ret.m_EpilogAddress;
            lastWritten = ret.m_AddressOfReturnAddress;
        }
    }
}

//-----------------------------------------------------------------------------
void* Hijacking::Epilog()
{
    SSE_SCOPE;

    Timer & timer = TlsData->m_Timers.back();
    timer.Stop();
    
    // Send timer
    GTimerManager->Add( timer );

    // Pop timer
    TlsData->m_Timers.pop_back();

    // Get stack context
    void * stackAddress = _AddressOfReturnAddress();
    EpilogContext* epilogContext = (EpilogContext*)((char*)stackAddress + s_StackOffset );
    
    SendContext(TlsData->m_Contexts.back(), epilogContext);
    PopContext();

    void* ReturnAddress = TlsData->m_ReturnAdresses.back().m_OriginalReturnAddress;
    TlsData->m_ReturnAdresses.pop_back();
    return ReturnAddress;
}

//-----------------------------------------------------------------------------
void* Hijacking::EpilogEmpty()
{
    SSE_SCOPE;
    void* ReturnAddress = TlsData->m_ReturnAdresses.back().m_OriginalReturnAddress;
    TlsData->m_ReturnAdresses.pop_back();
    return ReturnAddress;
}

//-----------------------------------------------------------------------------
void* Hijacking::EpilogZoneStop()
{
    SSE_SCOPE;
    if( TlsData->m_ZoneStack > 0 )
    {
        Timer & timer = TlsData->m_Timers.back();
        timer.Stop();

        // Send string literal address as function address
        const Context* context = TlsData->m_Contexts.back();
#ifdef _WIN64
        char* zoneName = (char*)context->m_RCX.m_Ptr;
        timer.m_FunctionAddress = context->m_RCX.m_Reg64;
#else
        char* zoneName = *((char**)&context->m_Stack[0]);
        timer.m_FunctionAddress = (DWORD)zoneName;
#endif
        timer.SetType(Timer::ZONE);

        // Send string once (per thread)
        if( TlsData->m_SentLiterals.find( zoneName ) == TlsData->m_SentLiterals.end() )
        {
            size_t numChars = std::min( strlen( zoneName ), size_t( OrbitZoneName::NUM_CHAR - 1 ) );
            OrbitZoneName zone;
#ifdef _WIN64
            zone.m_Address = context->m_RCX.m_Reg64;
#else
            zone.m_Address = *((DWORD*)&context->m_Stack[0]);
#endif
            memcpy( zone.m_Data, zoneName, numChars );
            zone.m_Data[numChars] = 0;

            GTcpClient->Send( Msg_OrbitZoneName, zone );
            TlsData->m_SentLiterals.insert( zoneName );
        }

        // Send timer
        GTimerManager->Add( timer );

        // Pop timer
        TlsData->m_Timers.pop_back();

        PopContext();
        --TlsData->m_ZoneStack;
    }

    void* ReturnAddress = TlsData->m_ReturnAdresses.back().m_OriginalReturnAddress;
    TlsData->m_ReturnAdresses.pop_back();
    return ReturnAddress;
}

//-----------------------------------------------------------------------------
void* Hijacking::EpilogAlloc()
{
    SSE_SCOPE;

    // Get stack context
    void * stackAddress = _AddressOfReturnAddress();
    EpilogContext* epilogContext = (EpilogContext*)( (char*)stackAddress + s_StackOffset );

    Timer & timer = TlsData->m_Timers.back();
    timer.Stop();

    const Context* prologContext = TlsData->m_Contexts.back();

    timer.m_UserData[0] = epilogContext->GetReturnValue(); // Pointer

#ifdef _WIN64
    timer.m_UserData[1] = prologContext->m_RCX.m_Reg64; // Size
#else
    timer.m_UserData[1] = 0;
#endif
    timer.m_Type = Timer::ALLOC;

    // Send timer
    GTimerManager->Add( timer );

    // Pop timer
    TlsData->m_Timers.pop_back();

    SendContext( prologContext, epilogContext );
    PopContext();

    void* ReturnAddress = TlsData->m_ReturnAdresses.back().m_OriginalReturnAddress;
    TlsData->m_ReturnAdresses.pop_back();
    return ReturnAddress;
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::PushReturnAddress( void** a_AddressOfReturnAddress )
{   
    ReturnAddress returnAddress;
    returnAddress.m_AddressOfReturnAddress = a_AddressOfReturnAddress;
    returnAddress.m_OriginalReturnAddress = *a_AddressOfReturnAddress;
    TlsData->m_ReturnAdresses.push_back( returnAddress );
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::PopReturnAddress()
{
    TlsData->m_ReturnAdresses.pop_back();
}

//-----------------------------------------------------------------------------
__forceinline void* Hijacking::GetReturnAddress()
{
    return TlsData->m_ReturnAdresses.back().m_OriginalReturnAddress;
}

//-----------------------------------------------------------------------------
__forceinline FunctionArgInfo* Hijacking::GetArgInfo( void* a_Address )
{
    auto it = m_FunctionArgsMap.find( (ULONG64)a_Address );
    return it != m_FunctionArgsMap.end() ? &it->second : nullptr;
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::PushContext( const Context* a_Context, void* a_OriginalFunction )
{
    TlsData->m_Contexts.push_back( a_Context );

    // Note: disabled argument tracking in favor of better perf.  It required work anyway, will re-enable.
    // FunctionArgInfo* argInfo = GetArgInfo( a_OriginalFunction );
    // context->m_StackSize = argInfo ? argInfo->m_NumStackBytes : 0;
    // memcpy( context, a_Context, Context::GetFixedDataSize() + context->m_StackSize );
    // context->m_RET = (AddressType)a_OriginalFunction;
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::PushZoneContext( const Context* a_Context, void* a_OriginalFunction )
{
    TlsData->m_Contexts.push_back( a_Context );
}

//-----------------------------------------------------------------------------
__forceinline void Hijacking::PopContext()
{
    TlsData->m_Contexts.pop_back();
}

//-----------------------------------------------------------------------------
inline void Hijacking::SendContext( const Context * a_Context, EpilogContext* a_EpilogContext )
{
#if 0
    void* address = a_Context->GetRet();
    FunctionArgInfo* argInfo = GetArgInfo( address );
    size_t argDataSize = argInfo ? argInfo->m_ArgDataSize : 0;
    std::vector<unsigned char> messageData( sizeof(SavedContext) + argDataSize );
    
    Message msg(Msg_SavedContext);
    msg.m_Size = (int)messageData.size();
    msg.m_Header.m_GenericHeader.m_Address = (ULONG64)address;
    
    // Contexts
    size_t offset = 0;
    memcpy( messageData.data(), a_Context, sizeof(Context) );
    offset += sizeof(Context);
    memcpy( messageData.data() + offset, a_EpilogContext, sizeof(EpilogContext) );
    offset += sizeof(EpilogContext);

    // Argument data
    if( argInfo )
    {
        for( Argument & arg : argInfo->m_Args )
        {
            // TODO: Argument tracking was hijacked by data tracking
            //       We should separate both concepts and revive argument
            //       tracking.
            void* This = a_Context->GetThis();
            void* data = (void*)( (char*)This + arg.m_Offset );
            memcpy( messageData.data() + offset, data, arg.m_NumBytes );
            offset += arg.m_NumBytes;
        }
    }

    if ( GTimerManager )
    {
        GTcpClient->Send(msg, (void*)messageData.data());
    }
#endif
}

//-----------------------------------------------------------------------------
bool Hijacking::Initialize()
{
    static bool s_Initialized = false;
    if( !s_Initialized )
    {
        s_Initialized = ( MH_Initialize() == MH_OK );
    }
    return s_Initialized;
}

//-----------------------------------------------------------------------------
bool Hijacking::Deinitialize()
{
    return MH_Uninitialize() == MH_OK;
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &Prolog, &Epilog );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateZoneStartHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologZoneStart, &EpilogEmpty );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateZoneStopHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologZoneStop, &EpilogZoneStop );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateOutputDebugStringHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologOutputDbg, &EpilogEmpty );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateSendDataHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologSendData, &EpilogEmpty );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateUnrealActorHook( void * a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologUnrealActor, &Epilog );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateAllocHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &Prolog, &EpilogAlloc );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateFreeHook( void* a_FunctionAddress )
{
    return CreateHook( a_FunctionAddress, &PrologFree, &Epilog );
}

//-----------------------------------------------------------------------------
bool Hijacking::CreateHook(void* a_FunctionAddress, void* a_PrologCallback, void* a_EpilogCallback)
{
    Initialize();
    MH_STATUS MinHookError = MH_Orbit_CreateHookPrologEpilog( a_FunctionAddress, a_PrologCallback, a_EpilogCallback, &GetReturnAddress );
    return MinHookError == MH_OK;
}

//-----------------------------------------------------------------------------
bool Hijacking::EnableHook(void* a_FunctionAddress)
{
    return MH_EnableHook( a_FunctionAddress ) == MH_OK;
}

//-----------------------------------------------------------------------------
void Hijacking::EnableHooks( DWORD64* a_Addresses, uint32_t a_NumAddresses )
{
    MH_EnableHooks( a_Addresses, a_NumAddresses, true );
}

//-----------------------------------------------------------------------------
bool Hijacking::DisableHook( void* a_FunctionAddress )
{
    return MH_DisableHook( a_FunctionAddress ) == MH_OK;
}

//-----------------------------------------------------------------------------
bool Hijacking::DisableAllHooks()
{
    return MH_DisableAllHooks() == MH_OK;
}

//-----------------------------------------------------------------------------
bool Hijacking::SuspendBusyLoopThread( OrbitWaitLoop * a_WaitLoop )
{
    HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, a_WaitLoop->m_ThreadId );
    DWORD result = SuspendThread( hThread );
    void* address = (void*)a_WaitLoop->m_Address;
    
    if( result != (DWORD)-1 )
    {
        DWORD oldProtect;
        if( !VirtualProtect( address, 2, PAGE_EXECUTE_READWRITE, &oldProtect ) )
            return false;

        // Write original 2 bytes
        memcpy( address, a_WaitLoop->m_OriginalBytes, 2 );

        VirtualProtect( address, 2, oldProtect, &oldProtect );
        FlushInstructionCache( GetCurrentProcess(), address, 2 );

        CONTEXT c;
#ifdef _M_X64
        DWORD64 *pIP = &c.Rip;
#else
        DWORD   *pIP = &c.Eip;
#endif

        c.ContextFlags = CONTEXT_CONTROL;
        if( !GetThreadContext( hThread, &c ) )
        {
            CloseHandle(hThread);
            return false;
        }

        // Set instruction pointer to start of function
#ifdef _M_X64
        *pIP = a_WaitLoop->m_Address;
#else
        *pIP = (DWORD)a_WaitLoop->m_Address;
#endif
        
        if( !SetThreadContext( hThread, &c ) )
        {
            CloseHandle(hThread);
            return false;
        }

        CloseHandle(hThread);
        return true;
    }

    CloseHandle(hThread);
    return false;
}

//-----------------------------------------------------------------------------
bool Hijacking::ThawMainThread( OrbitWaitLoop * a_WaitLoop )
{
    HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, a_WaitLoop->m_ThreadId );
    DWORD result = ResumeThread( hThread );
    CloseHandle(hThread);
    return result != (DWORD)-1;
}

//-----------------------------------------------------------------------------
void Hijacking::ClearFunctionArguments()
{
    m_FunctionArgsMap.clear();
    m_SendCallstacks.clear();
}

//-----------------------------------------------------------------------------
void Hijacking::SetFunctionArguments( ULONG64 a_FunctionAddress, const FunctionArgInfo & a_Args )
{
    m_FunctionArgsMap[a_FunctionAddress] = a_Args;
}

//-----------------------------------------------------------------------------
void Hijacking::TrackCallstack( ULONG64 a_FunctionAddress )
{
    m_SendCallstacks.insert( a_FunctionAddress );
}

//-----------------------------------------------------------------------------
void Hijacking::SetUnrealInfo( OrbitUnrealInfo & a_UnrealInfo )
{
    m_UnrealInfo = a_UnrealInfo;
    GetDisplayNameEntry = (GetDisplayNameEntryFunction)(void*)a_UnrealInfo.m_GetDisplayNameEntryAddress;
}
