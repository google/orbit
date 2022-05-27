// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData/MizarData.h"

#include "ClientData/ScopeIdConstants.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "OrbitBase/Logging.h"

namespace orbit_mizar_data {

void MizarData::OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                                 std::optional<std::filesystem::path> file_path,
                                 absl::flat_hash_set<uint64_t> frame_track_function_ids) {
  ConstructCaptureData(capture_started, std::move(file_path), std::move(frame_track_function_ids),
                       orbit_client_data::CaptureData::DataSource::kLoadedCapture);
}

void MizarData::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  const uint64_t scope_id = GetCaptureData().ProvideScopeId(timer_info);
  if (scope_id == orbit_client_data::kInvalidScopeId) return;

  const orbit_client_data::ScopeType scope_type = GetCaptureData().GetScopeInfo(scope_id).GetType();
  if (scope_type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      scope_type == orbit_client_data::ScopeType::kApiScope) {
    GetMutableCaptureData().GetThreadTrackDataProvider()->AddTimer(timer_info);
  }
}

std::optional<std::string> MizarData::GetFunctionNameFromAddress(uint64_t address) const {
  const orbit_client_data::LinuxAddressInfo* address_info =
      GetCaptureData().GetAddressInfo(address);
  if (address_info == nullptr) return std::nullopt;
  return address_info->function_name();
}

}  // namespace orbit_mizar_data