// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <chrono>
#include <outcome.hpp>
#include <string>

#include "CallstackTypes.h"
#include "LinuxAddressInfo.h"
#include "OrbitType.h"
#include "Threading.h"
#include "absl/container/flat_hash_map.h"

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
  //  client-side. remote_address is only used by the client-side.
  static outcome::result<void, std::string> StartCapture(
      std::string_view remote_address);
  static void StopCapture();
  static void ClearCaptureData();
  static std::vector<std::shared_ptr<Function>> GetSelectedFunctions();
  static void PreFunctionHooks();
  static void SendFunctionHooks();
  static void StartSampling();
  static void StopSampling();
  static bool IsCapturing();
  static void Update();
  static void DisplayStats();
  static void TestHooks();
  static outcome::result<void, std::string> SaveSession(
      const std::string& filename);
  static void NewSamplingProfiler();
  static bool IsTrackingEvents();
  // True when Orbit is receiving data from remote source
  static bool IsRemote();
  // True if receiving data from Linux remote source
  static bool IsLinuxData();
  static void RegisterZoneName(uint64_t a_ID, const char* a_Name);
  static void AddCallstack(CallStack& a_CallStack);
  static std::shared_ptr<CallStack> GetCallstack(CallstackID a_ID);
  static LinuxAddressInfo* GetAddressInfo(uint64_t address);
  static void CheckForUnrealSupport();
  static void PreSave();

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
  static std::unordered_map<uint64_t, uint64_t> GFunctionCountMap;
  static std::vector<uint64_t> GSelectedAddressesByType[Function::NUM_TYPES];
  static std::unordered_map<uint64_t, std::shared_ptr<CallStack>> GCallstacks;
  static std::unordered_map<uint64_t, LinuxAddressInfo> GAddressInfos;
  static std::unordered_map<uint64_t, std::string> GAddressToFunctionName;
  static std::unordered_map<uint64_t, std::string> GZoneNames;
  static class TextBox* GSelectedTextBox;
  static ThreadID GSelectedThreadId;
  static Timer GCaptureTimer;
  static std::chrono::system_clock::time_point GCaptureTimePoint;
  static Mutex GCallstackMutex;

 private:
  static bool GUnrealSupported;
  static SamplingDoneCallback sampling_done_callback_;
  static void* sampling_done_callback_user_data_;
};
