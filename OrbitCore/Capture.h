//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CallstackTypes.h"
#include "OrbitType.h"
#include "Threading.h"
#include <string>
#include <chrono>

class Process;
class Session;
class SamplingProfiler;
class Function;
struct CallStack;

struct Capture
{
    static void Init();
    static bool Inject( bool a_WaitForConnection = true );
    static bool Connect();
    static bool InjectRemote();
    static void SetTargetProcess( const std::shared_ptr< Process > & a_Process );
    static bool StartCapture();
    static void StopCapture();
    static void ToggleRecording();
    static void ClearCaptureData();
    static void PreFunctionHooks();
    static void SendFunctionHooks();
    static void SendDataTrackingInfo();
    static void StartSampling();
    static void StopSampling();
    static bool IsCapturing();
    static void Update();
    static void DisplayStats();
    static void TestHooks();
    static void OpenCapture( const std::wstring & a_CaptureName );
    static bool IsOtherInstanceRunning();
    static void LoadSession( const std::shared_ptr<Session> & a_Session );
    static void SaveSession( const std::wstring & a_FileName );
    static void NewSamplingProfiler();
    static bool IsTrackingEvents();
    static bool IsRemote();
    static void RegisterZoneName( DWORD64 a_ID, char* a_Name );
    static void AddCallstack( CallStack & a_CallStack );
    static std::shared_ptr<CallStack> GetCallstack( CallstackID a_ID );
    static void CheckForUnrealSupport();
    static void PreSave();
    
    typedef void (*LoadPdbAsyncFunc)( const std::vector<std::wstring> & a_Modules );
    static void SetLoadPdbAsyncFunc( LoadPdbAsyncFunc a_Func ){ GLoadPdbAsync = a_Func; }

    static bool         GInjected;
    static bool         GIsConnected;
    static std::string  GInjectedProcess;
    static std::wstring GInjectedProcessW;
    static double       GOpenCaptureTime;
    static int          GCapturePort;
    static std::wstring GCaptureHost;
    static std::wstring GPresetToLoad;  // TODO: allow multiple presets
    static std::wstring GProcessToInject;
    static bool         GIsSampling;
    static bool         GIsTesting;
    static int          GNumSamples;
    static int          GNumSamplingTicks;
    static int          GFunctionIndex;
    static int          GNumInstalledHooks;
    static bool         GHasSamples;
    static bool         GHasContextSwitches;

    static Timer        GTestTimer;
    static ULONG64      GMainFrameFunction;
    static ULONG64      GNumContextSwitches;
    static ULONG64      GNumProfileEvents;
    static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
    static std::shared_ptr<Process>          GTargetProcess;
    static std::shared_ptr<Session>          GSessionPresets;
    static std::shared_ptr<CallStack>        GSelectedCallstack;
    static void( *GClearCaptureDataFunc )( );
    static void( *GSamplingDoneCallback )( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler );
    static std::map< ULONG64, Function* > GSelectedFunctionsMap;
    static std::map< ULONG64, Function* > GVisibleFunctionsMap;
    static std::unordered_map< ULONG64, ULONG64 > GFunctionCountMap;
    static std::vector<ULONG64> GSelectedAddressesByType[Function::NUM_TYPES];
    static std::unordered_map< DWORD64, std::shared_ptr<CallStack> > GCallstacks;
    static std::unordered_map< DWORD64, std::string > GZoneNames;
    static class TextBox* GSelectedTextBox;
    static ThreadID GSelectedThreadId;
    static Timer GCaptureTimer;
    static std::chrono::system_clock::time_point GCaptureTimePoint;
    static Mutex GCallstackMutex;
    static LoadPdbAsyncFunc GLoadPdbAsync;
    static bool GUnrealSupported;
};

