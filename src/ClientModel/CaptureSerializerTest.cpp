// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash.h>
#include <absl/strings/str_cat.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "ClientModel/CaptureSerializer.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"

using orbit_client_data::CaptureData;
using orbit_client_data::ModuleManager;

using orbit_grpc_protos::CaptureStarted;

namespace orbit_client_model {

TEST(CaptureSerializer, GenerateCaptureFileName) {
  constexpr int32_t kProcessId = 42;

  CaptureStarted capture_started;
  capture_started.set_process_id(kProcessId);
  capture_started.set_capture_start_timestamp_ns(1'392'033'600'000'000);
  capture_started.set_executable_path("/path/to/p");

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_load_bias(0);
  module_info.set_file_path("path/to/module");
  module_info.set_address_start(15);
  module_info.set_address_end(1000);

  ModuleManager module_manager;
  CaptureData capture_data{
      capture_started, std::filesystem::path{}, {}, CaptureData::DataSource::kLiveCapture};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());
  capture_data.mutable_process()->UpdateModuleInfos({module_info});

  std::string expected_file_name = absl::StrCat(
      "p_",
      orbit_client_model_internal::FormatTimeWithUnderscores(capture_data.capture_start_time()),
      "_suffix.orbit");
  EXPECT_EQ(expected_file_name,
            capture_serializer::GenerateCaptureFileName(
                capture_data.process_name(), capture_data.capture_start_time(), "_suffix"));
}

TEST(CaptureSerializer, IncludeOrbitExtensionInFile) {
  std::string file_name_with_extension = "process_000.orbit";
  std::string expected_file_name = file_name_with_extension;
  capture_serializer::IncludeOrbitExtensionInFile(file_name_with_extension);
  EXPECT_EQ(expected_file_name, file_name_with_extension);

  std::string file_name_without_extension = "process_000";
  capture_serializer::IncludeOrbitExtensionInFile(file_name_without_extension);
  EXPECT_EQ(expected_file_name, file_name_without_extension);
}

}  // namespace orbit_client_model
