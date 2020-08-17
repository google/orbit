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

CaptureData Capture::capture_data_;
absl::flat_hash_map<uint64_t, FunctionInfo> Capture::GVisibleFunctionsMap;
TextBox* Capture::GSelectedTextBox = nullptr;
ThreadID Capture::GSelectedThreadId;

std::shared_ptr<SamplingProfiler> Capture::GSamplingProfiler = nullptr;
std::shared_ptr<Process> Capture::GTargetProcess = nullptr;
std::shared_ptr<PresetFile> Capture::GSessionPresets = nullptr;

void Capture::Init() { GTargetProcess = std::make_shared<Process>(); }

void Capture::SetTargetProcess(std::shared_ptr<Process> process) {
  if (process != GTargetProcess) {
    GSamplingProfiler = std::make_shared<SamplingProfiler>(process);
    GTargetProcess = std::move(process);
  }
}

ErrorMessageOr<void> Capture::StartCapture(
    uint64_t process_id, std::string process_name,
    absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions) {
  if (GTargetProcess->GetID() == 0) {
    return ErrorMessage("No process selected. Please choose a target process for the capture.");
  }

  capture_data_ = CaptureData(process_id, std::move(process_name), std::move(selected_functions));
  return outcome::success();
}

void Capture::FinalizeCapture() {
  if (Capture::GSamplingProfiler != nullptr) {
    Capture::GSamplingProfiler->ProcessSamples();
  }
}

void Capture::ClearCaptureData() {
  GSelectedTextBox = nullptr;
  GSelectedThreadId = 0;
}

void Capture::PreSave() {
  // Add selected functions' exact address to sampling profiler
  for (auto& pair : capture_data_.selected_functions()) {
    GSamplingProfiler->UpdateAddressInfo(pair.first);
  }
}
