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
#include "Threading.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"
#include "preset.pb.h"

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
  static ErrorMessageOr<void> SavePreset(const std::string& filename);
  static void NewSamplingProfiler();
  static orbit_client_protos::LinuxAddressInfo* GetAddressInfo(
      uint64_t address);
  static void PreSave();

  static State GState;
  static bool GInjected;
  static std::string GInjectedProcess;
  static uint32_t GNumInstalledHooks;

  static uint64_t GNumContextSwitches;
  static uint64_t GNumLinuxEvents;
  static uint64_t GNumProfileEvents;
  static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
  static std::shared_ptr<Process> GTargetProcess;
  static std::shared_ptr<orbit_client_protos::PresetFile> GSessionPresets;
  static void (*GClearCaptureDataFunc)();
  static std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
      GSelectedInCaptureFunctions;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GSelectedFunctionsMap;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GVisibleFunctionsMap;
  static std::unordered_map<uint64_t, uint64_t> GFunctionCountMap;
  static int32_t GProcessId;
  static std::string GProcessName;
  static std::unordered_map<int32_t, std::string> GThreadNames;
  static std::unordered_map<uint64_t, orbit_client_protos::LinuxAddressInfo>
      GAddressInfos;
  static std::unordered_map<uint64_t, std::string> GAddressToFunctionName;
  static std::unordered_map<uint64_t, std::string> GAddressToModuleName;
  static class TextBox* GSelectedTextBox;
  static ThreadID GSelectedThreadId;
  static std::chrono::system_clock::time_point GCaptureTimePoint;
};

#endif  // ORBIT_CORE_CAPTURE_H_
