// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Capture.h"

#include <ostream>

#include "Core.h"
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
bool Capture::GInjected = false;
std::string Capture::GInjectedProcess;
uint32_t Capture::GNumInstalledHooks;
uint64_t Capture::GNumContextSwitches;
uint64_t Capture::GNumLinuxEvents;
uint64_t Capture::GNumProfileEvents;
std::string Capture::GProcessToInject;
std::string Capture::GFunctionFilter;

std::vector<std::shared_ptr<FunctionInfo>> Capture::GSelectedInCaptureFunctions;
std::map<uint64_t, FunctionInfo*> Capture::GSelectedFunctionsMap;
std::map<uint64_t, FunctionInfo*> Capture::GVisibleFunctionsMap;
std::unordered_map<uint64_t, uint64_t> Capture::GFunctionCountMap;
int32_t Capture::GProcessId = -1;
std::string Capture::GProcessName;
std::unordered_map<int32_t, std::string> Capture::GThreadNames;
std::unordered_map<uint64_t, LinuxAddressInfo> Capture::GAddressInfos;
std::unordered_map<uint64_t, std::string> Capture::GAddressToFunctionName;
std::unordered_map<uint64_t, std::string> Capture::GAddressToModuleName;
TextBox* Capture::GSelectedTextBox = nullptr;
ThreadID Capture::GSelectedThreadId;
std::chrono::system_clock::time_point Capture::GCaptureTimePoint;

std::shared_ptr<SamplingProfiler> Capture::GSamplingProfiler = nullptr;
std::shared_ptr<Process> Capture::GTargetProcess = nullptr;
std::shared_ptr<PresetFile> Capture::GSessionPresets = nullptr;

void (*Capture::GClearCaptureDataFunc)();
std::vector<std::shared_ptr<SamplingProfiler>> GOldSamplingProfilers;

//-----------------------------------------------------------------------------
void Capture::Init() { GTargetProcess = std::make_shared<Process>(); }

//-----------------------------------------------------------------------------
void Capture::SetTargetProcess(const std::shared_ptr<Process>& a_Process) {
  if (a_Process != GTargetProcess) {
    GInjected = false;
    GInjectedProcess = "";

    GTargetProcess = a_Process;
    GSamplingProfiler = std::make_shared<SamplingProfiler>(a_Process);
    GSelectedFunctionsMap.clear();
    GFunctionCountMap.clear();
  }
}

//-----------------------------------------------------------------------------
ErrorMessageOr<void> Capture::StartCapture() {
  if (GTargetProcess->GetID() == 0) {
    return ErrorMessage(
        "No process selected. Please choose a target process for the capture.");
  }

  ClearCaptureData();

  GCaptureTimePoint = std::chrono::system_clock::now();
  GProcessId = GTargetProcess->GetID();
  GProcessName = GTargetProcess->GetName();

  GInjected = true;

  PreFunctionHooks();

  Capture::NewSamplingProfiler();

  GState = State::kStarted;

  return outcome::success();
}

//-----------------------------------------------------------------------------
void Capture::StopCapture() {
  if (!GInjected) {
    return;
  }

  GState = State::kStopping;
}

//-----------------------------------------------------------------------------
void Capture::FinalizeCapture() {
  if (Capture::GSamplingProfiler != nullptr) {
    Capture::GSamplingProfiler->ProcessSamples();
  }

  GState = State::kDone;
}

//-----------------------------------------------------------------------------
void Capture::ClearCaptureData() {
  GFunctionCountMap.clear();
  GProcessId = -1;
  GProcessName = "";
  GThreadNames.clear();
  GAddressInfos.clear();
  GAddressToFunctionName.clear();
  GAddressToModuleName.clear();
  GSelectedTextBox = nullptr;
  GSelectedThreadId = 0;
  GNumProfileEvents = 0;
  GNumLinuxEvents = 0;
  GNumContextSwitches = 0;
  GState = State::kEmpty;
}

//-----------------------------------------------------------------------------
void Capture::PreFunctionHooks() {
  GSelectedInCaptureFunctions = GetSelectedFunctions();

  for (auto& func : GSelectedInCaptureFunctions) {
    uint64_t address = FunctionUtils::GetAbsoluteAddress(*func);
    GSelectedFunctionsMap[address] = func.get();
    func->clear_stats();
    GFunctionCountMap[address] = 0;
  }

  GVisibleFunctionsMap = GSelectedFunctionsMap;

  if (GClearCaptureDataFunc) {
    GClearCaptureDataFunc();
  }
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

//-----------------------------------------------------------------------------
bool Capture::IsCapturing() {
  return GState == State::kStarted || GState == State::kStopping;
}

//-----------------------------------------------------------------------------
void Capture::DisplayStats() {
  if (GSamplingProfiler) {
    TRACE_VAR(GSamplingProfiler->GetNumSamples());
  }
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void Capture::NewSamplingProfiler() {
  if (GSamplingProfiler) {
    // To prevent destruction while processing data...
    GOldSamplingProfilers.push_back(GSamplingProfiler);
  }

  Capture::GSamplingProfiler =
      std::make_shared<SamplingProfiler>(Capture::GTargetProcess);
}

//-----------------------------------------------------------------------------
LinuxAddressInfo* Capture::GetAddressInfo(uint64_t address) {
  auto address_info_it = GAddressInfos.find(address);
  if (address_info_it == GAddressInfos.end()) {
    return nullptr;
  }
  return &address_info_it->second;
}

//-----------------------------------------------------------------------------
void Capture::PreSave() {
  // Add selected functions' exact address to sampling profiler
  for (auto& pair : GSelectedFunctionsMap) {
    GSamplingProfiler->UpdateAddressInfo(pair.first);
  }
}
