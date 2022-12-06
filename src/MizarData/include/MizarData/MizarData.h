// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_DATA_H_
#define MIZAR_DATA_MIZAR_DATA_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CaptureClient/AbstractCaptureListener.h"
#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientData/CaptureData.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/ProcessData.h"
#include "ClientData/ScopeId.h"
#include "ClientData/SystemMemoryInfo.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/FunctionSymbols.h"
#include "MizarBase/Time.h"
#include "MizarDataProvider.h"
#include "OrbitPaths/Paths.h"
#include "Symbols/SymbolHelper.h"

namespace orbit_mizar_data {

// This class is used by Mizar to read a capture file and load the symbols.
// Also owns a map from the function absolute addresses to their names.
class MizarData : public orbit_capture_client::AbstractCaptureListener<MizarData>,
                  public MizarDataProvider {
  using ScopeId = ::orbit_client_data::ScopeId;
  using PresentEvent = ::orbit_grpc_protos::PresentEvent;
  using RelativeTimeNs = ::orbit_mizar_base::RelativeTimeNs;
  using AbsoluteAddress = ::orbit_mizar_base::AbsoluteAddress;
  using FunctionSymbol = ::orbit_mizar_base::FunctionSymbol;

 public:
  MizarData() = default;
  MizarData(MizarData&) = delete;
  MizarData& operator=(const MizarData& other) = delete;

  MizarData(MizarData&& other) = default;
  MizarData& operator=(MizarData&& other) = delete;

  virtual ~MizarData() = default;

  [[nodiscard]] const absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>>&
  source_to_present_events() const override {
    return source_to_present_events_;
  }

  [[nodiscard]] absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> AllAddressToFunctionSymbol()
      const override;

  [[nodiscard]] std::optional<std::string> GetFunctionNameFromAddress(
      AbsoluteAddress address) const override;

  [[nodiscard]] orbit_mizar_base::TimestampNs GetCaptureStartTimestampNs() const override {
    return orbit_mizar_base::TimestampNs(
        GetCaptureData().GetCaptureStarted().capture_start_timestamp_ns());
  }

  [[nodiscard]] RelativeTimeNs GetNominalSamplingPeriodNs() const override {
    const double samples_per_second =
        GetCaptureData().GetCaptureStarted().capture_options().samples_per_second();
    return orbit_mizar_base::RelativeTimeNs(
        static_cast<uint64_t>(1'000'000'000 / samples_per_second));
  }

  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                        std::optional<std::filesystem::path> file_path,
                        absl::flat_hash_set<uint64_t> frame_track_function_ids) override;

  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& /*capture_finished*/) override {
    GetMutableCaptureData().OnCaptureComplete();
    LoadSymbolsForAllModules();
  }

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  void OnModuleUpdate(uint64_t /*timestamp_ns*/,
                      orbit_grpc_protos::ModuleInfo module_info) override {
    GetMutableCaptureData().mutable_process()->AddOrUpdateModuleInfo(module_info);
    UpdateModules({module_info});
  }
  void OnModulesSnapshot(uint64_t /*timestamp_ns*/,
                         std::vector<orbit_grpc_protos::ModuleInfo> module_infos) override {
    UpdateModules(module_infos);
  }
  void OnPresentEvent(const PresentEvent& event) override {
    source_to_present_events_[event.source()].push_back(event);
  }

  // The following events are ignored, as we only load D, MS and sampling data.
  void OnCgroupAndProcessMemoryInfo(const orbit_client_data::CgroupAndProcessMemoryInfo&
                                    /*cgroup_and_process_memory_info*/) override {}
  void OnPageFaultsInfo(const orbit_client_data::PageFaultsInfo&
                        /*page_faults_info*/) override {}
  void OnSystemMemoryInfo(const orbit_client_data::SystemMemoryInfo&
                          /*system_memory_info*/) override {}
  void OnKeyAndString(uint64_t /*key*/, std::string /*str*/) override {}
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
  void OnWarningInstrumentingWithUprobesEvent(
      orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
      /*warning_instrumenting_with_uprobes_event*/) override {}
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
  void UpdateModules(absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);

  void LoadSymbolsForAllModules();

  void LoadSymbols(orbit_client_data::ModuleData& module_data);

  [[nodiscard]] std::string GetModuleFilenameWithoutExtension(AbsoluteAddress address) const;

  std::unique_ptr<orbit_client_data::ModuleManager> module_manager_;
  orbit_symbols::SymbolHelper symbol_helper_{orbit_paths::CreateOrGetCacheDirUnsafe()};
  absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>> source_to_present_events_;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_DATA_H_