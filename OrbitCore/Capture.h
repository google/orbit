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
  static void SetTargetProcess(std::shared_ptr<Process> process);
  static void FinalizeCapture();
  static void PreSave();

  static CaptureData capture_data_;

  static std::shared_ptr<SamplingProfiler> GSamplingProfiler;
  static std::shared_ptr<Process> GTargetProcess;
};

#endif  // ORBIT_CORE_CAPTURE_H_
