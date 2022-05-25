// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData.h"

#include "ClientData/ScopeIdConstants.h"
#include "ClientData/ScopeInfo.h"

namespace orbit_mizar {

void MizarData::OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                                 std::optional<std::filesystem::path> file_path,
                                 absl::flat_hash_set<uint64_t> frame_track_function_ids) {
  capture_data_ = std::make_unique<orbit_client_data::CaptureData>(
      capture_started, file_path, std::move(frame_track_function_ids),
      orbit_client_data::CaptureData::DataSource::kLoadedCapture);
}

void MizarData::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  const uint64_t scope_id = capture_data_->ProvideScopeId(timer_info);
  if (scope_id == orbit_client_data::kInvalidScopeId) return;

  const orbit_client_data::ScopeType scope_type = capture_data_->GetScopeInfo(scope_id).GetType();
  if (scope_type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      scope_type == orbit_client_data::ScopeType::kApiScope) {
    capture_data_->GetThreadTrackDataProvider()->AddTimer(timer_info);
  }
}

}  // namespace orbit_mizar