// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LayerData.h"

#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "layer_config.pb.h"

using orbit_vulkan_capture_protos::LayerConfig;

namespace {
constexpr char const* kConfigFileName =
    "/mnt/developer/orbit_trigger_capture_vulkan_layer_config.pb.txt";
constexpr char const* kOrbitCaptureService = "/mnt/developer/OrbitCaptureGgpService";
constexpr char const* kLogDirectory = "/var/game/";
constexpr double kFrameTimeThresholdMillisecondsDefault = 1000.0 / 60.0;
constexpr uint32_t kCaptureLengthSecondsDefault = 10;
}  // namespace

void LayerData::Init() {
  // Load data from config file
  LOG("Loading  vulkan layer config file from %s", std::string(kConfigFileName));

  // Config is a proto text file
  int config_file_descriptor = open(kConfigFileName, O_RDONLY);
  if (config_file_descriptor < 0) {
    ERROR("Not being able to open file: %s. Default values will be used", SafeStrerror(errno));
    return;
  }

  google::protobuf::io::FileInputStream config_file_stream(config_file_descriptor);
  if (!google::protobuf::TextFormat::Parse(&config_file_stream, &layer_config_)) {
    ERROR("Parsing vulkan layer config file. Default values will be used");
    layer_config_.Clear();
  } else {
    LOG("Config data loaded successfully");
  }

  if (close(config_file_descriptor) < 0) {
    ERROR("Closing config file: %s", SafeStrerror(errno));
  }
}

double LayerData::GetFrameTimeThresholdMilliseconds() {
  if (layer_config_.has_layer_options() &&
      layer_config_.layer_options().frame_time_threshold_ms() > 0) {
    return layer_config_.layer_options().frame_time_threshold_ms();
  }
  return kFrameTimeThresholdMillisecondsDefault;
}

uint32_t LayerData::GetCaptureLengthSeconds() {
  if (layer_config_.has_layer_options() && layer_config_.layer_options().capture_length_s() > 0) {
    return layer_config_.layer_options().capture_length_s();
  }
  return kCaptureLengthSecondsDefault;
}

std::vector<char*> LayerData::BuildOrbitCaptureServiceArgv(const std::string& game_pid) {
  std::vector<char*> argv;

  // Set mandatory arguments: service, pid
  argv.push_back(const_cast<char*>(kOrbitCaptureService));
  argv.push_back(const_cast<char*>("-pid"));
  argv.push_back(const_cast<char*>(game_pid.c_str()));

  // Set arguments that are always provided but can be set by the user
  // Create a log file for OrbitCaptureService; by default kLogDirectory
  argv.push_back(const_cast<char*>("-log_directory"));
  if (layer_config_.has_capture_service_arguments() &&
      !layer_config_.capture_service_arguments().log_directory().empty()) {
    auto& log_directory = layer_config_.capture_service_arguments().log_directory();
    argv.push_back(const_cast<char*>(log_directory.c_str()));
  } else {
    argv.push_back(const_cast<char*>(kLogDirectory));
  }

  // Set optional arguments if set by the user; otherwise not included in the call
  // Available optional arguments are: functions, file_directory and sample_rate
  // file_directory and sample_rate are given default values in OrbitCaptureGgpService
  if (layer_config_.has_capture_service_arguments() &&
      layer_config_.capture_service_arguments().functions_size() > 0) {
    argv.push_back(const_cast<char*>("-functions"));
    builder_functions_str_ = "";
    for (auto& function : layer_config_.capture_service_arguments().functions()) {
      if (builder_functions_str_.empty()) {
        absl::StrAppendFormat(&builder_functions_str_, "%s", function);
      } else {
        absl::StrAppendFormat(&builder_functions_str_, ",%s", function);
      }
    }
    argv.push_back(const_cast<char*>(builder_functions_str_.c_str()));
  }

  if (layer_config_.has_capture_service_arguments() &&
      !layer_config_.capture_service_arguments().file_directory().empty()) {
    argv.push_back(const_cast<char*>("-file_directory"));
    auto& file_directory = layer_config_.capture_service_arguments().file_directory();
    argv.push_back(const_cast<char*>(file_directory.c_str()));
  }

  if (layer_config_.has_capture_service_arguments() &&
      layer_config_.capture_service_arguments().sampling_rate() > 0) {
    argv.push_back(const_cast<char*>("-sampling_rate"));
    builder_sampling_rate_str_ =
        absl::StrFormat("%d", layer_config_.capture_service_arguments().sampling_rate());
    argv.push_back(const_cast<char*>(builder_sampling_rate_str_.c_str()));
  }

  // Execv requires NULL as the last argument
  argv.push_back(nullptr);

  return argv;
}
