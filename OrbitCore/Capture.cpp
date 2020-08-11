// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Capture.h"

#include <ostream>

#include "EventBuffer.h"
#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "Pdb.h"
#include "SamplingProfiler.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;

Capture::State Capture::GState = Capture::State::kEmpty;

CaptureData Capture::capture_data_;
std::map<uint64_t, FunctionInfo*> Capture::GSelectedFunctionsMap;
std::map<uint64_t, FunctionInfo*> Capture::GVisibleFunctionsMap;
std::unordered_map<int32_t, std::string> Capture::GThreadNames;
TextBox* Capture::GSelectedTextBox = nullptr;
ThreadID Capture::GSelectedThreadId;

std::shared_ptr<SamplingProfiler> Capture::GSamplingProfiler = nullptr;
std::shared_ptr<Process> Capture::GTargetProcess = nullptr;
std::shared_ptr<PresetFile> Capture::GSessionPresets = nullptr;

void Capture::Init() { GTargetProcess = std::make_shared<Process>(); }

void Capture::SetTargetProcess(const std::shared_ptr<Process>& a_Process) {
  if (a_Process != GTargetProcess) {
    GTargetProcess = a_Process;
    GSamplingProfiler = std::make_shared<SamplingProfiler>(a_Process);
    GSelectedFunctionsMap.clear();
  }
}

ErrorMessageOr<void> Capture::StartCapture() {
  if (GTargetProcess->GetID() == 0) {
    return ErrorMessage(
        "No process selected. Please choose a target process for the capture.");
  }

  capture_data_ =
      CaptureData(GTargetProcess->GetID(), GTargetProcess->GetName(),
                  GetSelectedFunctions());

  PreFunctionHooks();

  Capture::GSamplingProfiler =
      std::make_shared<SamplingProfiler>(Capture::GTargetProcess);

  GState = State::kStarted;

  return outcome::success();
}

void Capture::StopCapture() { GState = State::kStopping; }

void Capture::FinalizeCapture() {
  if (Capture::GSamplingProfiler != nullptr) {
    Capture::GSamplingProfiler->ProcessSamples();
  }

  GState = State::kDone;
}

void Capture::ClearCaptureData() {
  GThreadNames.clear();
  GSelectedTextBox = nullptr;
  GSelectedThreadId = 0;
}

void Capture::PreFunctionHooks() {
  GSelectedFunctionsMap.clear();
  for (auto& func : capture_data_.selected_functions()) {
    uint64_t address = FunctionUtils::GetAbsoluteAddress(*func);
    GSelectedFunctionsMap[address] = func.get();
    func->clear_stats();
  }

  GVisibleFunctionsMap = GSelectedFunctionsMap;
}

std::vector<std::shared_ptr<FunctionInfo>> Capture::GetSelectedFunctions() {
  std::vector<std::shared_ptr<FunctionInfo>> selected_functions;
  for (auto& func : GTargetProcess->GetFunctions()) {
    if (FunctionUtils::IsSelected(*func) || FunctionUtils::IsOrbitFunc(*func)) {
      selected_functions.push_back(func);
    }
  }
  return selected_functions;
}

bool Capture::IsCapturing() {
  return GState == State::kStarted || GState == State::kStopping;
}

ErrorMessageOr<void> Capture::SavePreset(const std::string& filename) {
  PresetInfo preset;
  preset.set_process_full_path(GTargetProcess->GetFullPath());

  for (auto& func : GTargetProcess->GetFunctions()) {
    if (FunctionUtils::IsSelected(*func)) {
      uint64_t hash = FunctionUtils::GetHash(*func);
      (*preset.mutable_path_to_module())[func->loaded_module_path()]
          .add_function_hashes(hash);
    }
  }

  std::string filename_with_ext = filename;
  if (!absl::EndsWith(filename, ".opr")) {
    filename_with_ext += ".opr";
  }

  std::ofstream file(filename_with_ext, std::ios::binary);
  if (file.fail()) {
    ERROR("Saving preset in \"%s\": %s", filename_with_ext, "file.fail()");
    return ErrorMessage("Error opening the file for writing");
  }

  {
    SCOPE_TIMER_LOG(
        absl::StrFormat("Saving preset in \"%s\"", filename_with_ext));
    preset.SerializeToOstream(&file);
  }

  return outcome::success();
}

void Capture::PreSave() {
  // Add selected functions' exact address to sampling profiler
  for (auto& pair : GSelectedFunctionsMap) {
    GSamplingProfiler->UpdateAddressInfo(pair.first);
  }
}
