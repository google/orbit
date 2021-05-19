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

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

// TODO(kuebler): remove this once we have the validator complete
ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");

// TODO: Remove this flag once we have a way to toggle the display return values
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");

ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");

// TODO(b/181736566): Remove this flag entirely
ABSL_FLAG(bool, enable_source_code_view, true, "Enable the experimental source code view");

// TODO(b/185099421): Remove this flag once we have a clear explanation of the memory warning
// threshold (i.e., production limit).
ABSL_FLAG(bool, enable_warning_threshold, false,
          "Enable setting and showing the memory warning threshold");

// TODO(b/187388305): Set default to true in 1.65, remove the flag in 1.66
ABSL_FLAG(bool, enable_capture_autosave, true, "Enable automatic saving of capture");
