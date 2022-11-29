// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CLIENT_FLAGS_H_
#define FAKE_CLIENT_FLAGS_H_

#include <absl/flags/flag.h>

constexpr const char* kEventProcessorVulkanLayerString = "vulkan_layer";
constexpr const char* kEventProcessorFakeString = "fake";

enum class EventProcessorType { kFake, kVulkanLayer };

inline bool AbslParseFlag(absl::string_view text, EventProcessorType* out, std::string* error) {
  if (text == kEventProcessorFakeString) {
    *out = EventProcessorType::kFake;
    return true;
  }
  if (text == kEventProcessorVulkanLayerString) {
    *out = EventProcessorType::kVulkanLayer;
    return true;
  }
  *error = "Unrecognized event processor.";
  return false;
}

inline std::string AbslUnparseFlag(EventProcessorType in) {
  if (in == EventProcessorType::kFake) {
    return std::string(kEventProcessorFakeString);
  }
  if (in == EventProcessorType::kVulkanLayer) {
    return std::string(kEventProcessorVulkanLayerString);
  }
  return "unknown";
}

ABSL_FLAG(uint64_t, port, 44765, "Port OrbitService's gRPC service is listening on");
ABSL_FLAG(int32_t, pid, 0, "PID of the process to capture");
ABSL_FLAG(uint32_t, duration, std::numeric_limits<uint32_t>::max(),
          "Duration of the capture in seconds (stop earlier with Ctrl+C)");
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Callstack sampling rate in samples per second (0: no sampling)");
ABSL_FLAG(bool, frame_pointers, false, "Use frame pointers for unwinding");
ABSL_FLAG(std::string, instrument_path, "", "Path of the binary of the function to instrument");
ABSL_FLAG(std::string, instrument_name, "", "Name of the function to instrument");
ABSL_FLAG(uint64_t, instrument_offset, 0, "Offset in the binary of the function to instrument");
ABSL_FLAG(int64_t, instrument_size, -1, "Size in bytes of the function to instrument");
ABSL_FLAG(bool, is_hotpatchable, false, "Whether the function to instrument is hotpatchable");
ABSL_FLAG(bool, user_space_instrumentation, false,
          "Use user space instrumentation instead of uprobes");
ABSL_FLAG(bool, scheduling, true, "Collect scheduling information");
ABSL_FLAG(bool, thread_state, false, "Collect thread state information");
ABSL_FLAG(bool, gpu_jobs, true, "Collect GPU jobs");
ABSL_FLAG(bool, orbit_api, false, "Enable Orbit API");
ABSL_FLAG(uint16_t, memory_sampling_rate, 0,
          "Memory usage sampling rate in samples per second (0: no sampling)");
ABSL_FLAG(bool, frame_time, true, "Instrument vkQueuePresentKHR to compute avg. frame time");
ABSL_FLAG(EventProcessorType, event_processor, EventProcessorType::kFake, "");
ABSL_FLAG(std::string, pid_file_path, "",
          "Path of the file to watch that will contain the target PID (the file must exists)");
ABSL_FLAG(std::string, output_path, "", "Path of the output files");

#endif  // FAKE_CLIENT_FLAGS_H_