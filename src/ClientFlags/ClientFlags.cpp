// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/strings/string_view.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");

ABSL_FLAG(bool, nodeploy, false, "Disable automatic deployment of OrbitService");

ABSL_FLAG(std::string, collector, "", "Full path of collector to be deployed");

ABSL_FLAG(std::string, collector_root_password, "", "Collector's machine root password");

ABSL_FLAG(uint16_t, grpc_port, 44765,
          "The service's GRPC server port (use default value if unsure)");

ABSL_FLAG(std::string, process_name, "",
          "Automatically select and connect to the specified process");

// TODO: Remove this flag once we have a way to toggle the display return values
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");

ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");

// TODO(b/185099421): Remove this flag once we have a clear explanation of the memory warning
// threshold (i.e., production limit).
ABSL_FLAG(bool, enable_warning_threshold, false,
          "Enable setting and showing the memory warning threshold");

// Additional folder in which OrbitService will look for symbols
ABSL_FLAG(std::string, instance_symbols_folder, "",
          "Additional folder in which OrbitService will look for symbols");

ABSL_FLAG(bool, enforce_full_redraw, false,
          "Enforce full redraw every frame (used for performance measurements)");

ABSL_FLAG(std::vector<std::string>, additional_symbol_paths, {},
          "Additional local symbol locations (comma-separated)");

// Clears QSettings. This is intended for e2e tests.
ABSL_FLAG(bool, clear_settings, false,
          "Clears user defined settings. This includes symbol locations and source path mappings.");

// TODO(http://b/170712621): Remove this flag when we decide which timestamp format we will use.
ABSL_FLAG(bool, iso_timestamps, true, "Show timestamps using ISO-8601 format.");

ABSL_FLAG(bool, enable_unsafe_symbols, false,
          "Enable the possibility to use symbol files that do not have a matching build ID.");

ABSL_FLAG(
    bool, auto_symbol_loading, true,
    "Enable automatic symbol loading. This is turned on by default. If Orbit becomes unresponsive, "
    "try turning automatic symbol loading off (--auto_symbol_loading=false)");

ABSL_FLAG(bool, auto_frame_track, true, "Automatically add the default Frame Track.");

ABSL_FLAG(bool, time_range_selection, true, "Enable time range selection feature.");

ABSL_FLAG(bool, symbol_store_support, false, "Enable experimental symbol store support.");

// Disables retrieving symbols from the instance. This is intended for symbol store e2e tests.
ABSL_FLAG(bool, disable_instance_symbols, false, "Disable retrieving symbols from the instance.");

// SSH Flags
ABSL_FLAG(std::string, ssh_hostname, "", "Hostname (IP address) of machine for an SSH connection.");
ABSL_FLAG(uint16_t, ssh_port, 22, "Port for SSH connection. Default is 22");
ABSL_FLAG(std::string, ssh_user, "", "User for SSH connection.");
ABSL_FLAG(std::string, ssh_known_host_path, "", "Path to known_host file for SSH connection.");
ABSL_FLAG(std::string, ssh_key_path, "", "Path to key file for SSH connection.");
ABSL_FLAG(std::string, ssh_target_process, "",
          "Process name or path for SSH connection. If specified, Orbit will directly set up a ssh "
          "a SSH connection. This means --ssh_hostname, --ssh_user, --ssh_known_host_path and "
          "--ssh_key_path also need to be specified (--ssh_port will default to 22). If multiple "
          "instances of the same process exist, the one with the highest PID will be chosen.");

// Introspection from entry point.
ABSL_FLAG(bool, introspect, false, "Introspect from entry point");