// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_DATA_H_
#define MIZAR_DATA_MIZAR_DATA_H_

#include <absl/container/flat_hash_set.h>

#include <memory>
#include <optional>
#include <vector>

#include "CaptureClient/AbstractCaptureListener.h"
#include "CaptureClient/CaptureListener.h"
#include "CaptureFile/CaptureFile.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "GrpcProtos/module.pb.h"
#include "MizarDataProvider.h"
#include "OrbitBase/Logging.h"
#include "OrbitPaths/Paths.h"
#include "Symbols/SymbolHelper.h"

namespace orbit_mizar_data {

// This class is used by Mizar to read a capture file and load the symbols.
// Also owns a map from the function absolute addresses to their names.
class MizarData : public orbit_capture_client::AbstractCaptureListener<MizarData>,
                  public MizarDataProvider {
 public:
  MizarData() = default;
  MizarData(MizarData&) = delete;
  MizarData& operator=(const MizarData& other) = delete;

  MizarData(MizarData&& other) = default;
  MizarData& operator=(MizarData&& other) = delete;

  virtual ~MizarData() = default;

  [[nodiscard]] absl::flat_hash_map<uint64_t, std::string> AllAddressToName() const override;

  // virtual for testing purposes
  [[nodiscard]] virtual std::optional<std::string> GetFunctionNameFromAddress(
      uint64_t address) const override;

  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                        std::optional<std::filesystem::path> file_path,
                        absl::flat_hash_set<uint64_t> frame_track_function_ids) override;

  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& /*capture_finished*/) override {
    GetMutableCaptureData().OnCaptureComplete();
    LoadSymbolsForAllModules();
  }

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  // Ignored, as we only load D, MS and sampling data.
  void OnKeyAndString(uint64_t /*key*/, std::string /*str*/) override {}

  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo module_info) override {
    UpdateModules({module_info});
  }
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> module_infos) override {
    UpdateModules(module_infos);
  }
  void OnThreadStateSlice(orbit_client_data::ThreadStateSliceInfo /*thread_state_slice*/) override {
  }
  void OnApiStringEvent(const orbit_client_data::ApiStringEvent& /*unused*/) override {}
  void OnApiTrackValue(const orbit_client_data::ApiTrackValue&) override {}
  void OnWarningEvent(orbit_grpc_protos::WarningEvent /*warning_event*/) override {}
  void OnClockResolutionEvent(
      orbit_grpc_protos::ClockResolutionEvent /*clock_resolution_event*/) override {}
  void OnErrorsWithPerfEventOpenEvent(
      orbit_grpc_protos::ErrorsWithPerfEventOpenEvent /*errors_with_perf_event_open_event*/)
      override {}
  void OnErrorEnablingOrbitApiEvent(
      orbit_grpc_protos::ErrorEnablingOrbitApiEvent /*error_enabling_orbit_api_event*/) override {}
  void OnErrorEnablingUserSpaceInstrumentationEvent(
      orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent /*error_event*/) override {}
  void OnWarningInstrumentingWithUserSpaceInstrumentationEvent(
      orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent
      /*warning_event*/) override {}
  void OnLostPerfRecordsEvent(
      orbit_grpc_protos::LostPerfRecordsEvent /*lost_perf_records_event*/) override {}
  void OnOutOfOrderEventsDiscardedEvent(orbit_grpc_protos::OutOfOrderEventsDiscardedEvent
                                        /*out_of_order_events_discarded_event*/) override {}

 private:
  void UpdateModules(const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos);

  void LoadSymbolsForAllModules();

  void LoadSymbols(orbit_client_data::ModuleData& module_data);

  std::unique_ptr<orbit_client_data::ModuleManager> module_manager_;
  orbit_symbols::SymbolHelper symbol_helper_{orbit_paths::CreateOrGetCacheDir()};
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_DATA_H_