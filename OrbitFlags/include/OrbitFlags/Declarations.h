// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/declare.h>

#include <cstdint>
#include <string>

ABSL_DECLARE_FLAG(bool, enable_stale_features);

ABSL_DECLARE_FLAG(bool, devmode);

ABSL_DECLARE_FLAG(bool, nodeploy);

ABSL_DECLARE_FLAG(std::string, collector);

ABSL_DECLARE_FLAG(std::string, collector_root_password);

ABSL_DECLARE_FLAG(uint16_t, grpc_port);

ABSL_DECLARE_FLAG(bool, local);

ABSL_DECLARE_FLAG(bool, enable_tutorials_feature);

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_DECLARE_FLAG(uint16_t, sampling_rate);

// TODO(b/160549506): Remove this flag once it can be specified in the ui.
ABSL_DECLARE_FLAG(bool, frame_pointer_unwinding);

// TODO(kuebler): remove this once we have the validator complete
ABSL_DECLARE_FLAG(bool, enable_frame_pointer_validator);

// TODO: Remove this flag once we have a way to toggle the display return values
ABSL_DECLARE_FLAG(bool, show_return_values);

ABSL_DECLARE_FLAG(bool, enable_tracepoint_feature);

ABSL_DECLARE_FLAG(bool, thread_state);

// TODO(170468590): [ui beta] Remove this flag when the new UI is finished
ABSL_DECLARE_FLAG(bool, enable_ui_beta);

ABSL_DECLARE_FLAG(std::string, log_dir);