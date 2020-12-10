// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>

#include <cstdint>
#include <string>

// Flag declarations
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");
ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");
ABSL_FLAG(bool, thread_state, false, "Collect thread states");

// TODO(170468590): Remove this flag when the new UI is finished
ABSL_FLAG(bool, enable_ui_beta, false, "Enable the new user interface");