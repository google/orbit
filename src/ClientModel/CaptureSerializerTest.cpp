// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "CaptureSerializationTestMatchers.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TracepointCustom.h"
#include "ClientModel/CaptureData.h"
#include "ClientModel/CaptureSerializer.h"
#include "CoreUtils.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "tracepoint.pb.h"

using orbit_client_data::ModuleManager;

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::CaptureInfo;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::TracepointEventInfo;

using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::TracepointInfo;

using ::testing::ElementsAreArray;

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

  CaptureData capture_data{&module_manager, capture_started, std::filesystem::path{}, {}};
  EXPECT_TRUE(module_manager.AddOrUpdateModules({module_info}).empty());
  capture_data.mutable_process()->UpdateModuleInfos({module_info});

  std::string expected_file_name = absl::StrCat(
      "p_", orbit_core::FormatTime(capture_data.capture_start_time()), "_suffix.orbit");
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
