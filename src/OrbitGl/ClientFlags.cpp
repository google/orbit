// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>

#include <cstdint>
#include <string>

ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

ABSL_FLAG(bool, nodeploy, false, "Disable automatic deployment of OrbitService");

ABSL_FLAG(std::string, collector, "", "Full path of collector to be deployed");

ABSL_FLAG(std::string, collector_root_password, "", "Collector's machine root password");

ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

ABSL_FLAG(std::string, process_name, "",
          "Automatically select and connect to the specified process");

ABSL_FLAG(bool, enable_tutorials_feature, false, "Enable tutorials");

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");

// Max to pass to perf_event_open without getting an error is (1u << 16u) - 8,
// because the kernel stores this in a short and because of alignment reasons.
// But the size the kernel actually returns is smaller and we leave some extra room (see
// `PerfEventOpen.cpp`).
ABSL_FLAG(uint16_t, stack_dump_size, 65000,
          "Number of bytes to copy from the stack per sample. Max: 65000");

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

// TODO(kuebler): remove this once we have the validator complete
ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");

// TODO: Remove this flag once we have a way to toggle the display return values
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");

ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");

// TODO(b/185099421): Remove this flag once we have a clear explanation of the memory warning
// threshold (i.e., production limit).
ABSL_FLAG(bool, enable_warning_threshold, false,
          "Enable setting and showing the memory warning threshold");

ABSL_FLAG(bool, enable_cgroup_memory, false,
          "Enable collecting cgroup and process memory usage information");
