// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_H_
#define ORBIT_CORE_CAPTURE_H_

#include <chrono>
#include <outcome.hpp>
#include <string>

#include "CallstackTypes.h"
#include "OrbitBase/Result.h"
#include "OrbitProcess.h"
#include "OrbitSession.h"
#include "Threading.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class Process;
class SamplingProfiler;
struct CallStack;

class Capture {
 public:
  enum class State { kEmpty = 0, kStarted, kStopping, kDone };

  static void Init();
  static void SetTargetProcess(const std::shared_ptr<Process>& a_Process);
  static ErrorMessageOr<void> StartCapture();
  static void StopCapture();
  static void FinalizeCapture();
  static void ClearCaptureData();
  static std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
  GetSelectedFunctions();
  static void PreFunctionHooks();
  static bool IsCapturing();
  static void DisplayStats();
  static void TestHooks();
  static ErrorMessageOr<void> SavePreset(const std::string& filename);
  static void NewSamplingProfiler();
  static void RegisterZoneName(uint64_t a_ID, const char* a_Name);
  static void AddCallstack(CallStack& a_CallStack);
  static std::shared_ptr<CallStack> GetCallstack(CallstackID a_ID);
  static orbit_client_protos::LinuxAddressInfo* GetAddressInfo(
      uint64_t address);
  static void PreSave();

  static State GState;
  static bool GInjected;
  static std::string GInjectedProcess;
  static std::string GPresetToLoad;  // TODO: allow multiple presets
  static std::string GProcessToInject;
  static std::string GFunctionFilter;
  static bool GIsSampling;
  static uint32_t GFunctionIndex;
  static uint32_t GNumInstalledHooks;
  static bool GHasContextSwitches;

  static Timer GTestTimer;
  static uint64_t GNumContextSwitches;
  static ULONG64 GNumLinuxEvents;
  static ULONG64 GNumProfileEvents;
  static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
  static std::shared_ptr<Process> GTargetProcess;
  static std::shared_ptr<Preset> GSessionPresets;
  static std::shared_ptr<CallStack> GSelectedCallstack;
  static void (*GClearCaptureDataFunc)();
  static std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      GSelectedFunctions;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GSelectedFunctionsMap;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GVisibleFunctionsMap;
  static std::unordered_map<uint64_t, uint64_t> GFunctionCountMap;
  static std::unordered_map<uint64_t, std::shared_ptr<CallStack>> GCallstacks;
  static int32_t GProcessId;
  static std::string GProcessName;
  static std::unordered_map<int32_t, std::string> GThreadNames;
  static std::unordered_map<uint64_t, orbit_client_protos::LinuxAddressInfo>
      GAddressInfos;
  static std::unordered_map<uint64_t, std::string> GAddressToFunctionName;
  static std::unordered_map<uint64_t, std::string> GAddressToModuleName;
  static std::unordered_map<uint64_t, std::string> GZoneNames;
  static class TextBox* GSelectedTextBox;
  static ThreadID GSelectedThreadId;
  static std::chrono::system_clock::time_point GCaptureTimePoint;
  static Mutex GCallstackMutex;
};

#endif  // ORBIT_CORE_CAPTURE_H_
