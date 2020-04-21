//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <chrono>
#include <string>

#include "CallstackTypes.h"
#include "LinuxTracingSession.h"
#include "OrbitType.h"
#include "Threading.h"

class Process;
class Session;
class SamplingProfiler;
class Function;
struct CallStack;

class Capture {
 public:
  static void Init();
  static bool Inject(std::string_view remote_address);
  static bool Connect(std::string_view remote_address);
  static bool InjectRemote(std::string_view remote_address);
  static void SetTargetProcess(const std::shared_ptr<Process>& a_Process);
  // TODO: This method needs to be split into 2, the server side and the
  // client-side. session here is only used by the server side and
  // remote_address is only used by the client-side.
  static bool StartCapture(LinuxTracingSession* session,
                           std::string_view remote_address);
  static void StopCapture();
  static void ClearCaptureData();
  static std::vector<std::shared_ptr<Function>> GetSelectedFunctions();
  static void PreFunctionHooks();
  static void SendFunctionHooks();
  static void SendDataTrackingInfo();
  static void StartSampling();
  static void StopSampling();
  static bool IsCapturing();
  static void Update();
  static void DisplayStats();
  static void TestHooks();
  static void SaveSession(const std::string& a_FileName);
  static void NewSamplingProfiler();
  static bool IsTrackingEvents();
  // True when Orbit is receiving data from remote source
  static bool IsRemote();
  // True if receiving data from Linux remote source
  static bool IsLinuxData();
  static void RegisterZoneName(DWORD64 a_ID, char* a_Name);
  static void AddCallstack(CallStack& a_CallStack);
  static std::shared_ptr<CallStack> GetCallstack(CallstackID a_ID);
  static void CheckForUnrealSupport();
  static void PreSave();

  typedef void (*LoadPdbAsyncFunc)(const std::vector<std::string>& a_Modules);
  static void SetLoadPdbAsyncFunc(LoadPdbAsyncFunc a_Func) {
    GLoadPdbAsync = a_Func;
  }

  typedef void (*SamplingDoneCallback)(
      std::shared_ptr<SamplingProfiler>& sampling_profiler, void* user_data);
  static void SetSamplingDoneCallback(SamplingDoneCallback callback,
                                      void* user_data) {
    sampling_done_callback_ = callback;
    sampling_done_callback_user_data_ = user_data;
  }

  static void TestRemoteMessages();
  static class TcpEntity* GetMainTcpEntity();

  static bool GInjected;
  static std::string GInjectedProcess;
  static std::string GPresetToLoad;  // TODO: allow multiple presets
  static std::string GProcessToInject;
  static bool GIsSampling;
  static bool GIsTesting;
  static uint32_t GFunctionIndex;
  static uint32_t GNumInstalledHooks;
  static bool GHasContextSwitches;

  static Timer GTestTimer;
  static ULONG64 GMainFrameFunction;
  static uint64_t GNumContextSwitches;
  static ULONG64 GNumLinuxEvents;
  static ULONG64 GNumProfileEvents;
  static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
  static std::shared_ptr<Process> GTargetProcess;
  static std::shared_ptr<Session> GSessionPresets;
  static std::shared_ptr<CallStack> GSelectedCallstack;
  static void (*GClearCaptureDataFunc)();
  static std::vector<std::shared_ptr<Function>> GSelectedFunctions;
  static std::map<uint64_t, Function*> GSelectedFunctionsMap;
  static std::map<uint64_t, Function*> GVisibleFunctionsMap;
  static std::unordered_map<ULONG64, ULONG64> GFunctionCountMap;
  static std::vector<ULONG64> GSelectedAddressesByType[Function::NUM_TYPES];
  static std::unordered_map<DWORD64, std::shared_ptr<CallStack>> GCallstacks;
  static std::unordered_map<DWORD64, std::string> GZoneNames;
  static class TextBox* GSelectedTextBox;
  static ThreadID GSelectedThreadId;
  static Timer GCaptureTimer;
  static std::chrono::system_clock::time_point GCaptureTimePoint;
  static Mutex GCallstackMutex;
  static LoadPdbAsyncFunc GLoadPdbAsync;

 private:
  static bool GUnrealSupported;
  static SamplingDoneCallback sampling_done_callback_;
  static void* sampling_done_callback_user_data_;
};
