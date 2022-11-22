// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_TEST_DATA_H_
#define ORBIT_GL_TRACK_TEST_DATA_H_

#include <stddef.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_gl {

struct TrackTestData {
  static constexpr uint64_t kCallstackId = 1;
  static constexpr uint64_t kFunctionId = 1;
  static constexpr uint64_t kFunctionAbsoluteAddress = 0x30;
  static constexpr uint64_t kInstructionAbsoluteAddress = 0x31;
  static constexpr int32_t kThreadId = 42;
  static constexpr const char* kFunctionName = "example function";
  static constexpr const char* kModuleName = "example module";
  static constexpr const char* kThreadName = "example thread";

  static constexpr size_t kTimerOnlyThreadId = 128;
  static constexpr const char* kTimerOnlyThreadName = "timer only thread";

  static std::unique_ptr<orbit_client_data::CaptureData> GenerateTestCaptureData();
  static std::vector<orbit_client_protos::TimerInfo> GenerateTimers();
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TRACK_TEST_DATA_H_