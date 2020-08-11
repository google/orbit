// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_H_
#define ORBIT_CORE_CAPTURE_H_

#include <chrono>
#include <outcome.hpp>
#include <string>

#include "CallstackTypes.h"
#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "OrbitProcess.h"
#include "Threading.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"
#include "preset.pb.h"

class Process;
class SamplingProfiler;

class Capture {
 public:

  static void Init();
  static void SetTargetProcess(const std::shared_ptr<Process>& a_Process);
  static ErrorMessageOr<void> StartCapture();
  static void FinalizeCapture();
  static void ClearCaptureData();
  static std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>
  GetSelectedFunctions();
  static void PreFunctionHooks();
  static ErrorMessageOr<void> SavePreset(const std::string& filename);
  static void PreSave();

  static CaptureData capture_data_;

  static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
  static std::shared_ptr<Process> GTargetProcess;
  static std::shared_ptr<orbit_client_protos::PresetFile> GSessionPresets;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GSelectedFunctionsMap;
  static std::map<uint64_t, orbit_client_protos::FunctionInfo*>
      GVisibleFunctionsMap;
  static class TextBox* GSelectedTextBox;
  static ThreadID GSelectedThreadId;
};

#endif  // ORBIT_CORE_CAPTURE_H_
