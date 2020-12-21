// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>

#include <cstdint>
#include <string>

ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");

ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

ABSL_FLAG(bool, nodeploy, false, "Disable automatic deployment of OrbitService");

ABSL_FLAG(std::string, collector, "", "Full path of collector to be deployed");

ABSL_FLAG(std::string, collector_root_password, "", "Collector's machine root password");

ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");

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

ABSL_FLAG(bool, thread_state, false, "Collect thread states");

// TODO(170468590): [ui beta] Remove this flag when the new UI is finished
ABSL_FLAG(bool, enable_ui_beta, false, "Enable the new user interface");