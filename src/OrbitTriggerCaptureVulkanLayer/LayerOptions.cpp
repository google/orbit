// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LayerOptions.h"

#include <absl/strings/str_format.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitTriggerCaptureVulkanLayer/layer_config.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

using orbit_vulkan_capture_protos::LayerConfig;

namespace {
constexpr char const* kConfigFileName =
    "/mnt/developer/orbit_trigger_capture_vulkan_layer_config.pb.txt";
constexpr char const* kOrbitCaptureService = "/mnt/developer/OrbitCaptureGgpService";
constexpr char const* kLogDirectory = "/var/game/";
constexpr double kFrameTimeThresholdMillisecondsDefault = 1000.0 / 60.0;
constexpr uint32_t kCaptureLengthSecondsDefault = 10;
}  // namespace

void LayerOptions::Init() {
  // Load data from config file
  ORBIT_LOG("Loading  vulkan layer config file from %s", std::string(kConfigFileName));

  // Config is a proto text file
  int config_file_descriptor = open(kConfigFileName, O_RDONLY);
  if (config_file_descriptor < 0) {
    ORBIT_ERROR("Not being able to open file: %s. Default values will be used",
                SafeStrerror(errno));
    return;
  }

  google::protobuf::io::FileInputStream config_file_stream(config_file_descriptor);
  if (!google::protobuf::TextFormat::Parse(&config_file_stream, &layer_config_)) {
    ORBIT_ERROR("Parsing vulkan layer config file. Default values will be used");
    layer_config_.Clear();
  } else {
    ORBIT_LOG("Config data loaded successfully");
  }

  if (close(config_file_descriptor) < 0) {
    ORBIT_ERROR("Closing config file: %s", SafeStrerror(errno));
  }
}

double LayerOptions::GetFrameTimeThresholdMilliseconds() {
  if (layer_config_.has_layer_options() &&
      layer_config_.layer_options().frame_time_threshold_ms() > 0) {
    return layer_config_.layer_options().frame_time_threshold_ms();
  }
  return kFrameTimeThresholdMillisecondsDefault;
}

uint32_t LayerOptions::GetCaptureLengthSeconds() {
  if (layer_config_.has_layer_options() && layer_config_.layer_options().capture_length_s() > 0) {
    return layer_config_.layer_options().capture_length_s();
  }
  return kCaptureLengthSecondsDefault;
}

std::vector<std::string> LayerOptions::BuildOrbitCaptureServiceArgv(std::string_view game_pid) {
  std::vector<std::string> argv;

  // Set mandatory arguments: service, pid
  argv.push_back(kOrbitCaptureService);
  argv.push_back("-pid");
  argv.push_back(std::string{game_pid});

  // Set arguments that are always provided but can be set by the user
  // Create a log file for OrbitCaptureService; by default kLogDirectory
  argv.push_back("-log_directory");
  if (layer_config_.has_capture_service_arguments() &&
      !layer_config_.capture_service_arguments().log_directory().empty()) {
    argv.push_back(layer_config_.capture_service_arguments().log_directory());
  } else {
    argv.push_back(kLogDirectory);
  }

  // Set optional arguments if set by the user; otherwise not included in the call
  // Available optional arguments are: functions, file_directory and sample_rate
  // file_directory and sample_rate are given default values in OrbitCaptureGgpService
  if (layer_config_.has_capture_service_arguments() &&
      layer_config_.capture_service_arguments().functions_size() > 0) {
    argv.push_back("-functions");
    std::string functions_str;
    for (const auto& function : layer_config_.capture_service_arguments().functions()) {
      if (functions_str.empty()) {
        absl::StrAppendFormat(&functions_str, "%s", function);
      } else {
        absl::StrAppendFormat(&functions_str, ",%s", function);
      }
    }
    argv.push_back(functions_str);
  }

  if (layer_config_.has_capture_service_arguments() &&
      !layer_config_.capture_service_arguments().file_directory().empty()) {
    argv.push_back("-file_directory");
    argv.push_back(layer_config_.capture_service_arguments().file_directory());
  }

  if (layer_config_.has_capture_service_arguments() &&
      layer_config_.capture_service_arguments().sampling_rate() > 0) {
    argv.push_back("-sampling_rate");
    std::string sampling_rate_str =
        absl::StrFormat("%d", layer_config_.capture_service_arguments().sampling_rate());
    argv.push_back(sampling_rate_str);
  }

  return argv;
}
