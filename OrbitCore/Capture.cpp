// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Capture.h"

#include <ostream>

#include "SamplingProfiler.h"
#include "absl/strings/str_format.h"

CaptureData Capture::capture_data_;

std::shared_ptr<SamplingProfiler> Capture::GSamplingProfiler = nullptr;

void Capture::SetTargetProcess(std::shared_ptr<Process> process) {
  GSamplingProfiler = std::make_shared<SamplingProfiler>(std::move(process));
}

void Capture::FinalizeCapture() {
  if (Capture::GSamplingProfiler != nullptr) {
    Capture::GSamplingProfiler->ProcessSamples(*Capture::capture_data_.GetCallstackData());
  }
}

void Capture::PreSave() {
  // Add selected functions' exact address to sampling profiler
  for (auto& pair : capture_data_.selected_functions()) {
    GSamplingProfiler->UpdateAddressInfo(pair.first);
  }
}
