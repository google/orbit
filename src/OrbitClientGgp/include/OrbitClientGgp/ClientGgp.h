// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <grpcpp/grpcpp.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "ClientData/TracepointCustom.h"
#include "ClientData/UserDefinedCaptureData.h"
#include "ClientModel/CaptureData.h"
#include "ClientServices/ProcessClient.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientGgp/ClientGgpOptions.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "tracepoint.pb.h"

class ClientGgp {
 public:
  ClientGgp() = default;

  explicit ClientGgp(ClientGgpOptions&& options) : options_(std::move(options)) {}

  bool InitClient();
  ErrorMessageOr<void> RequestStartCapture(ThreadPool* thread_pool);
  bool StopCapture();
  void UpdateCaptureFunctions(std::vector<std::string> capture_functions);

  // CaptureListener implementation

 private:
  std::filesystem::path GenerateFilePath();

  ClientGgpOptions options_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<orbit_client_data::ProcessData> target_process_;
  orbit_client_data::ModuleManager module_manager_;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions_;
  orbit_client_data::ModuleData* main_module_ = nullptr;
  std::unique_ptr<orbit_capture_client::CaptureClient> capture_client_;
  std::unique_ptr<orbit_client_services::ProcessClient> process_client_;

  ErrorMessageOr<std::unique_ptr<orbit_client_data::ProcessData>> GetOrbitProcessByPid(int32_t pid);
  bool InitCapture();
  ErrorMessageOr<void> LoadModuleAndSymbols();
  void LoadSelectedFunctions();
  std::string SelectedFunctionMatch(const orbit_client_protos::FunctionInfo& func);
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> GetSelectedFunctions();
  void InformUsedSelectedCaptureFunctions(
      const absl::flat_hash_set<std::string>& capture_functions_used);
};

#endif  // ORBIT_CLIENT_GGP_CLIENT_GGP_H_
