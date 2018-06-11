//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Capture.h"
#include "TimerManager.h"
#include "TcpServer.h"
#include "TcpForward.h"
#include "Path.h"
#include "Injection.h"
#include "SamplingProfiler.h"
#include "OrbitSession.h"
#include "Serialization.h"
#include "Pdb.h"
#include "Log.h"
#include "Params.h"
#include "EventTracer.h"
#include "OrbitUnreal.h"
#include "CoreApp.h"
#include "Params.h"
#include "OrbitRule.h"
#include "CoreApp.h"
#include <fstream>
#include <ostream>

bool        Capture::GInjected = false;
bool        Capture::GIsConnected = false;
std::string Capture::GInjectedProcess;
std::wstring Capture::GInjectedProcessW;
double      Capture::GOpenCaptureTime;
bool        Capture::GIsSampling = false;
bool        Capture::GIsTesting = false;
int         Capture::GNumSamples = 0;
int         Capture::GNumSamplingTicks = 0;
int         Capture::GFunctionIndex = -1;
int         Capture::GNumInstalledHooks;
bool        Capture::GHasSamples;
bool        Capture::GHasContextSwitches;
Timer       Capture::GTestTimer;
ULONG64     Capture::GMainFrameFunction;
ULONG64     Capture::GNumContextSwitches;
ULONG64     Capture::GNumProfileEvents;
int         Capture::GCapturePort = 0;
std::wstring Capture::GCaptureHost = L"localhost";
std::wstring Capture::GPresetToLoad = L"";
std::wstring Capture::GProcessToInject = L"";

std::map< ULONG64, Function* >          Capture::GSelectedFunctionsMap;
std::map< ULONG64, Function* >          Capture::GVisibleFunctionsMap;
std::unordered_map< ULONG64, ULONG64 >  Capture::GFunctionCountMap;
std::shared_ptr<CallStack>              Capture::GSelectedCallstack;
std::vector<ULONG64>                    Capture::GSelectedAddressesByType[Function::NUM_TYPES];
std::unordered_map< DWORD64, std::shared_ptr<CallStack> > Capture::GCallstacks;
Mutex                                                     Capture::GCallstackMutex;
std::unordered_map< DWORD64, std::string >                Capture::GZoneNames;
TextBox*    Capture::GSelectedTextBox;
ThreadID    Capture::GSelectedThreadId;
Timer       Capture::GCaptureTimer;
std::chrono::system_clock::time_point Capture::GCaptureTimePoint;
Capture::LoadPdbAsyncFunc Capture::GLoadPdbAsync;

std::shared_ptr<SamplingProfiler> Capture::GSamplingProfiler = nullptr;
std::shared_ptr<Process>          Capture::GTargetProcess    = nullptr;
std::shared_ptr<Session>          Capture::GSessionPresets   = nullptr;

void(*Capture::GClearCaptureDataFunc)();
void(*Capture::GSamplingDoneCallback)( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler );
std::vector< std::shared_ptr<SamplingProfiler> > GOldSamplingProfilers;
bool Capture::GUnrealSupported = false;

//-----------------------------------------------------------------------------
void Capture::Init()
{
    GTargetProcess = std::make_shared<Process>();
    Capture::GCapturePort = GParams.m_Port;
}

//-----------------------------------------------------------------------------
bool Capture::Inject( bool a_WaitForConnection )
{
    Injection inject;
    wstring dllName = Path::GetDllPath( GTargetProcess->GetIs64Bit() );

    GTcpServer->Disconnect();

    GInjected = inject.Inject( dllName.c_str(), *GTargetProcess, "OrbitInit" );
    if( GInjected )
    {
        ORBIT_LOG( Format( "Injected in %s", GTargetProcess->GetName().c_str() ) );
        GInjectedProcessW = GTargetProcess->GetName();
        GInjectedProcess = ws2s(GInjectedProcessW);
    }

    if( a_WaitForConnection )
    {
        int numTries = 50;
        while( !GTcpServer->HasConnection() && numTries-- > 0 )
        {
            ORBIT_LOG( Format( "Waiting for connection on port %i", Capture::GCapturePort ) );
            Sleep(100);
        }

        GInjected = GInjected && GTcpServer->HasConnection();
    }

    return GInjected;
}

//-----------------------------------------------------------------------------
bool Capture::InjectRemote()
{
    Injection inject;
    wstring dllName = Path::GetDllPath( GTargetProcess->GetIs64Bit() );
    GTcpServer->Disconnect();

    GInjected = inject.Inject( dllName.c_str(), *GTargetProcess, "OrbitInitRemote" );
    
    if( GInjected )
    {
        ORBIT_LOG( Format( "Injected in %s", GTargetProcess->GetName().c_str() ) );
        GInjectedProcessW = GTargetProcess->GetName();
        GInjectedProcess = ws2s( GInjectedProcessW );
    }

    return GInjected;
}

//-----------------------------------------------------------------------------
void Capture::SetTargetProcess( const std::shared_ptr< Process > & a_Process )
{
    if( a_Process != GTargetProcess )
    {
        GInjected = false;
        GInjectedProcess = "";

        if( a_Process && !a_Process->GetIsRemote() )
        {
            // In the case of a remote process, 
            // connection is already active
            GTcpServer->Disconnect();
        }

        GTargetProcess = a_Process;
        GSamplingProfiler = std::make_shared<SamplingProfiler>( a_Process );
        GSelectedFunctionsMap.clear();
        GSessionPresets = nullptr;
        GOrbitUnreal.Clear();
		GTargetProcess->LoadDebugInfo();
		GTargetProcess->ClearWatchedVariables();
    }
}

//-----------------------------------------------------------------------------
bool Capture::Connect()
{
    if( !GInjected )
    {
        Inject();
    }

    return GInjected;
}

//-----------------------------------------------------------------------------
bool Capture::StartCapture()
{
    SCOPE_TIMER_LOG( L"Capture::StartCapture" );

    if( GTargetProcess->GetName().size() == 0 )
        return false;

    GCaptureTimer.Start();
    GCaptureTimePoint = std::chrono::system_clock::now();

    if( !IsRemote() )
    {
        if( !Connect() )
        {
            return false;
        }
    }

    GInjected = true;
    ++Message::GSessionID;
    GTcpServer->Send( Msg_NewSession );
    GTimerManager->StartRecording();
    
    ClearCaptureData();
    SendFunctionHooks();

    if( Capture::IsTrackingEvents() )
    {
        GEventTracer.Start();
    }

    if( GSelectedFunctionsMap.size() > 0 )
    {
        GCoreApp->SendToUiNow( L"startcapture" );
    }
    
    return true;
}

//-----------------------------------------------------------------------------
void Capture::StopCapture()
{
    if( IsTrackingEvents() )
    {
        GEventTracer.Stop();
    }

    if (!GInjected)
    {
        return;
    }

    GTcpServer->Send( Msg_StopCapture );
    GTimerManager->StopRecording();
}

//-----------------------------------------------------------------------------
void Capture::ToggleRecording()
{
    if( GTimerManager )
    {
        if( GTimerManager->m_IsRecording )
            StopCapture();
        else
            StartCapture();
    }
}

//-----------------------------------------------------------------------------
void Capture::ClearCaptureData()
{
    GSelectedFunctionsMap.clear();
    GFunctionCountMap.clear();
    GZoneNames.clear();
    GSelectedTextBox = nullptr;
    GSelectedThreadId = 0;
    GNumProfileEvents = 0;
    GTcpServer->ResetStats();
    GOrbitUnreal.NewSession();
    GHasSamples = false;
    GHasContextSwitches = false;
}

//-----------------------------------------------------------------------------
MessageType GetMessageType( Function::OrbitType a_type )
{
    static std::map<Function::OrbitType, MessageType> typeMap;
    if( typeMap.size() == 0 )
    {
        typeMap[Function::NONE]                      = Msg_FunctionHook;
        typeMap[Function::ORBIT_TIMER_START]         = Msg_FunctionHookZoneStart;
        typeMap[Function::ORBIT_TIMER_STOP]          = Msg_FunctionHookZoneStop;
        typeMap[Function::ORBIT_LOG]                 = Msg_FunctionHook;
        typeMap[Function::ORBIT_OUTPUT_DEBUG_STRING] = Msg_FunctionHookOutputDebugString;
        typeMap[Function::UNREAL_ACTOR]              = Msg_FunctionHookUnrealActor;
        typeMap[Function::ALLOC]                     = Msg_FunctionHookAlloc;
        typeMap[Function::FREE]                      = Msg_FunctionHookFree;
        typeMap[Function::REALLOC]                   = Msg_FunctionHookRealloc;
        typeMap[Function::ORBIT_DATA]                = Msg_FunctionHookOrbitData;
    }

    assert(typeMap.size() == Function::OrbitType::NUM_TYPES );
    
    return typeMap[a_type];
}

//-----------------------------------------------------------------------------
void Capture::PreFunctionHooks()
{
    // Clear selected functions
    for( int i = 0; i < Function::NUM_TYPES; ++i )
    {
        GSelectedAddressesByType[i].clear();
    }

    // Clear current argument tracking data
    GTcpServer->Send( Msg_ClearArgTracking );

    // Find OutputDebugStringA
    if( GParams.m_HookOutputDebugString )
    {
        if( DWORD64 outputAddr = GTargetProcess->GetOutputDebugStringAddress() )
        {
            GSelectedAddressesByType[Function::ORBIT_OUTPUT_DEBUG_STRING].push_back( outputAddr );
        }
    }

    // Find alloc/free functions
    GTargetProcess->FindCoreFunctions();

    // Unreal
    CheckForUnrealSupport();
}

//-----------------------------------------------------------------------------
void Capture::SendFunctionHooks()
{
    PreFunctionHooks();

    for( Function * func : GTargetProcess->GetFunctions() )
    {
        if( func->IsSelected() || func->IsOrbitFunc() )
        {
            func->PreHook();
            DWORD64 address = func->GetVirtualAddress();
            GSelectedAddressesByType[func->m_OrbitType].push_back(address);
            GSelectedFunctionsMap[(ULONG64)address] = func;
            func->ResetStats();
            GFunctionCountMap[address] = 0;
        }
    }

    GVisibleFunctionsMap = GSelectedFunctionsMap;

    if( GClearCaptureDataFunc )
    {
        GClearCaptureDataFunc();
    }

    GTcpServer->Send( Msg_StartCapture );

    // Unreal
    if( Capture::GUnrealSupported )
    {
        const OrbitUnrealInfo & info = GOrbitUnreal.GetUnrealInfo();
        GTcpServer->Send( Msg_OrbitUnrealInfo, info );
    }

    // Send argument tracking info
    SendDataTrackingInfo();

    // Send all hooks by type
    for( int i = 0; i < Function::NUM_TYPES; ++i )
    {
        std::vector<DWORD64> & addresses = GSelectedAddressesByType[i];
        if( addresses.size() )
        {
            MessageType msgType = GetMessageType( (Function::OrbitType)i );
            GTcpServer->Send( msgType, addresses );
        }
    }
}

//-----------------------------------------------------------------------------
void Capture::SendDataTrackingInfo()
{
    // Send information about arguments we want to track
    for( auto & pair : *GCoreApp->GetRules() )
    {
        const std::shared_ptr<Rule> rule = pair.second;
        Function* func = rule->m_Function;
        Message msg( Msg_ArgTracking );
        ArgTrackingHeader & header = msg.m_Header.m_ArgTrackingHeader;
        ULONG64 address = (ULONG64)func->m_Pdb->GetHModule() + (ULONG64)func->m_Address;
        header.m_Function = address;
        header.m_NumArgs = (int)rule->m_TrackedVariables.size();

        // TODO: Argument tracking was hijacked by data tracking
        //       We should separate both concepts and revive argument
        //       tracking.
        std::vector<Argument> args;
        for( const std::shared_ptr<Variable > var : rule->m_TrackedVariables )
        {
            Argument arg;
            arg.m_Offset = (DWORD)var->m_Address;
            arg.m_NumBytes = var->m_Size;
            args.push_back(arg);
        }

        msg.m_Size = (int)args.size() * sizeof(Argument);
        
        GTcpServer->Send( msg, (void*)args.data() );
    }
}

//-----------------------------------------------------------------------------
void Capture::TestHooks()
{
    if( !GIsTesting )
    {
        GIsTesting = true;
        GFunctionIndex = 0;
        GTestTimer.Start();
    }
    else
    {
        GIsTesting = false;
    }
}

//-----------------------------------------------------------------------------
void Capture::StartSampling()
{
    if( !GIsSampling && Capture::IsTrackingEvents() && GTargetProcess->GetName().size() )
    {
        SCOPE_TIMER_LOG( L"Capture::StartSampling" );

        GCaptureTimer.Start();
        GCaptureTimePoint = std::chrono::system_clock::now();

        ClearCaptureData();
        GTimerManager->StartRecording();
        GEventTracer.Start();

        GIsSampling = true;
    }
}

//-----------------------------------------------------------------------------
void Capture::StopSampling()
{
    if( GIsSampling )
    {
        if( IsTrackingEvents() )
        {
            GEventTracer.Stop();
        }

        GTimerManager->StopRecording();
    }
}

//-----------------------------------------------------------------------------
bool Capture::IsCapturing()
{
    return GTimerManager && GTimerManager->m_IsRecording;
}

//-----------------------------------------------------------------------------
void Capture::Update()
{
    if( GIsSampling )
    {
        if( GSamplingProfiler->ShouldStop() )
        {
            GSamplingProfiler->StopCapture();
        }

        if( GSamplingProfiler->GetState() == SamplingProfiler::DoneProcessing )
        {
            if( GSamplingDoneCallback )
            {
                GSamplingDoneCallback( GSamplingProfiler );
            }
            GIsSampling = false;
        }
    }

    if( GPdbDbg )
    {
        GPdbDbg->Update();
    }

    if( GInjected && !GTcpServer->HasConnection() )
    {
        StopCapture();
        GInjected = false;
    }

    Capture::GHasSamples = GEventTracer.GetEventBuffer().HasEvent();
}

//-----------------------------------------------------------------------------
void Capture::DisplayStats()
{
    if (GSamplingProfiler)
    {
        TRACE_VAR(GSamplingProfiler->GetNumSamples());
    }
}

//-----------------------------------------------------------------------------
void Capture::OpenCapture( const std::wstring & a_CaptureName )
{
    LocalScopeTimer Timer( &GOpenCaptureTime );
    SCOPE_TIMER_LOG( L"OpenCapture" );

    // TODO!
}

//-----------------------------------------------------------------------------
bool Capture::IsOtherInstanceRunning()
{
    DWORD procID = 0;
    HANDLE procHandle = Injection::GetTargetProcessHandle( ORBIT_EXE_NAME, procID );
    PRINT_FUNC;
    bool otherInstanceFound = procHandle != NULL;
    PRINT_VAR( otherInstanceFound );
    return otherInstanceFound;
}

//-----------------------------------------------------------------------------
void Capture::LoadSession( const shared_ptr<Session> & a_Session )
{
    GSessionPresets = a_Session;

    std::vector<std::wstring> modulesToLoad;
    for( auto & it : a_Session->m_Modules )
    {
        SessionModule & module = it.second;
        ORBIT_LOG_DEBUG( module.m_Name );
        modulesToLoad.push_back( it.first );
    }

    if( GLoadPdbAsync )
    {
        GLoadPdbAsync( modulesToLoad );
    }

    GParams.m_ProcessPath = ws2s( a_Session->m_ProcessFullPath );
    GParams.m_Arguments = ws2s( a_Session->m_Arguments );
    GParams.m_WorkingDirectory = ws2s( a_Session->m_WorkingDirectory );

    GCoreApp->SendToUiNow(L"SetProcessParams");
}

//-----------------------------------------------------------------------------
void Capture::SaveSession( const std::wstring & a_FileName )
{
    Session session;
    session.m_ProcessFullPath = GTargetProcess->GetFullName();

    GCoreApp->SendToUiNow(L"UpdateProcessParams");

    session.m_ProcessFullPath = s2ws(GParams.m_ProcessPath);
    session.m_Arguments = s2ws(GParams.m_Arguments);
    session.m_WorkingDirectory = s2ws(GParams.m_WorkingDirectory);
    
    for( Function* func : GTargetProcess->GetFunctions() )
    {
        if( func->IsSelected() )
        {
            session.m_Modules[func->m_Pdb->GetName()].m_FunctionHashes.push_back( func->Hash() );
        }
    }

    std::wstring saveFileName = a_FileName;
    if( !EndsWith( a_FileName, L".opr" ) )
    {
        saveFileName += L".opr";
    }

    SCOPE_TIMER_LOG( Format( L"Saving Orbit session in %s", saveFileName.c_str() ) );
    std::ofstream file( saveFileName, std::ios::binary );
    cereal::BinaryOutputArchive archive(file);
    archive( cereal::make_nvp("Session", session) );
}

//-----------------------------------------------------------------------------
void Capture::NewSamplingProfiler()
{
    if( GSamplingProfiler )
    {
        // To prevent destruction while processing data...
        GOldSamplingProfilers.push_back( GSamplingProfiler );
    }

    Capture::GSamplingProfiler = std::make_shared< SamplingProfiler >( Capture::GTargetProcess, true );
}

//-----------------------------------------------------------------------------
bool Capture::IsTrackingEvents()
{
    static bool yieldEvents = false;
    if( yieldEvents && IsOtherInstanceRunning() && GTargetProcess )
    {
        if( Contains( GTargetProcess->GetName(), L"Orbit.exe" ) )
        {
            return false;
        }
    }

    if( GTargetProcess->GetIsRemote() && !GTcpServer->IsLocalConnection() )
    {
        return false;
    }
    
    return GParams.m_TrackContextSwitches || GParams.m_TrackSamplingEvents;
}

//-----------------------------------------------------------------------------
bool Capture::IsRemote()
{
    return GTargetProcess && GTargetProcess->GetIsRemote();
}

//-----------------------------------------------------------------------------
void Capture::RegisterZoneName( DWORD64 a_ID, char* a_Name )
{
    GZoneNames[a_ID] = a_Name;
}

//-----------------------------------------------------------------------------
void Capture::AddCallstack( CallStack & a_CallStack )
{
    ScopeLock lock( GCallstackMutex );
    Capture::GCallstacks[a_CallStack.m_Hash] = std::make_shared<CallStack>(a_CallStack);
}

//-----------------------------------------------------------------------------
std::shared_ptr<CallStack> Capture::GetCallstack( CallstackID a_ID )
{
    ScopeLock lock( GCallstackMutex );
    
    auto it = Capture::GCallstacks.find( a_ID );
    if( it != Capture::GCallstacks.end() )
    {
        return it->second;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
void Capture::CheckForUnrealSupport()
{
    GUnrealSupported = GCoreApp->GetUnrealSupportEnabled() && GOrbitUnreal.HasFnameInfo();
}

//-----------------------------------------------------------------------------
void Capture::PreSave()
{
    // Add selected functions' exact address to sampling profiler
    for( auto & pair : GSelectedFunctionsMap )
    {
        GSamplingProfiler->AddAddress( pair.first );
    }
}
