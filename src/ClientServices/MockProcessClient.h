// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>

#include "ClientServices/ProcessClient.h"

namespace orbit_client_services {

class MockProcessClient : public ProcessClient {
 public:
  MOCK_METHOD(ErrorMessageOr<std::vector<orbit_grpc_protos::ProcessInfo>>, GetProcessList, (),
              (override));
  MOCK_METHOD(ErrorMessageOr<orbit_grpc_protos::ProcessInfo>, LaunchProcess,
              (const orbit_grpc_protos::ProcessToLaunch&), (override));
  MOCK_METHOD(ErrorMessageOr<void>, SuspendProcessSpinningAtEntryPoint, (uint32_t), (override));
  MOCK_METHOD(ErrorMessageOr<void>, ResumeProcessSuspendedAtEntryPoint, (uint32_t), (override));
  MOCK_METHOD(ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>>, LoadModuleList,
              (uint32_t), (override));
  MOCK_METHOD(ErrorMessageOr<std::string>, FindDebugInfoFile,
              (const std::string& module_path, absl::Span<const std::string>), (override));
  MOCK_METHOD(ErrorMessageOr<std::string>, LoadProcessMemory, (uint32_t, uint64_t, uint64_t),
              (override));
};

}  // namespace orbit_client_services
