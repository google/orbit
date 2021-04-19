// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "OrbitClientGgp/ClientGgpOptions.h"
#include "OrbitClientModel/CaptureData.h"
#include "OrbitClientServices/ProcessClient.h"
#include "StringManager.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "grpcpp/grpcpp.h"
#include "tracepoint.pb.h"

class ClientGgp final : public CaptureListener {
 public:
  ClientGgp() : main_module_{nullptr} {}

  explicit ClientGgp(ClientGgpOptions&& options)
      : options_(std::move(options)), main_module_{nullptr} {}

  bool InitClient();
  bool RequestStartCapture(ThreadPool* thread_pool);
  bool StopCapture();
  bool SaveCapture();
  void UpdateCaptureFunctions(std::vector<std::string> capture_functions);

  // CaptureListener implementation
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& capture_started) override;
  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                        absl::flat_hash_set<uint64_t> frame_track_function_ids) override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void OnSystemMemoryUsage(
      const orbit_grpc_protos::SystemMemoryUsage& /*system_memory_usage*/) override {}
  void OnKeyAndString(uint64_t key, std::string str) override;
  void OnUniqueCallStack(CallStack callstack) override;
  void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) override;
  void OnThreadName(int32_t thread_id, std::string thread_name) override;
  void OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo thread_state_slice) override;
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) override;
  void OnUniqueTracepointInfo(uint64_t key,
                              orbit_grpc_protos::TracepointInfo tracepoint_info) override;
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) override;
  void OnModuleUpdate(uint64_t timestamp_ns, orbit_grpc_protos::ModuleInfo module_info) override;
  void OnModulesSnapshot(uint64_t timestamp_ns,
                         std::vector<orbit_grpc_protos::ModuleInfo> module_infos) override;

 private:
  [[nodiscard]] CaptureData& GetMutableCaptureData() {
    CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }
  [[nodiscard]] const CaptureData& GetCaptureData() const {
    CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }
  ClientGgpOptions options_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  std::unique_ptr<ProcessData> target_process_;
  orbit_client_data::ModuleManager module_manager_;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions_;
  ModuleData* main_module_;
  std::shared_ptr<StringManager> string_manager_;
  std::unique_ptr<CaptureClient> capture_client_;
  std::unique_ptr<ProcessClient> process_client_;
  std::unique_ptr<CaptureData> capture_data_;
  std::vector<orbit_client_protos::TimerInfo> timer_infos_;

  ErrorMessageOr<std::unique_ptr<ProcessData>> GetOrbitProcessByPid(int32_t pid);
  bool InitCapture();
  void ClearCapture();
  ErrorMessageOr<void> LoadModuleAndSymbols();
  void LoadSelectedFunctions();
  std::string SelectedFunctionMatch(const orbit_client_protos::FunctionInfo& func);
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> GetSelectedFunctions();
  void InformUsedSelectedCaptureFunctions(
      const absl::flat_hash_set<std::string>& capture_functions_used);
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info);
  void PostprocessCaptureData();
};

#endif  // ORBIT_CLIENT_GGP_CLIENT_GGP_H_
