// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_DATA_PROVIDER_CAPTURE_DATA_PROVIDER_H_
#define CAPTURE_DATA_PROVIDER_CAPTURE_DATA_PROVIDER_H_

#include <string>

#include "CaptureDataProvider/CaptureDataModel.h"
#include "ClientData/TrackDataManager.h"
#include "StringManager/StringManager.h"

namespace orbit_capture_data_provider {

class CaptureDataProvider {
 public:
  CaptureDataProvider() = default;
  virtual ~CaptureDataProvider() = default;

  [[nodiscard]] virtual std::vector<Track> GetTracks() = 0;

  // These functions are use on the hot path.
  [[nodiscard]] virtual std::vector<TimerBlock> GetTimers(uint64_t pane_id,
                                                          uint64_t start_timestamp_ns,
                                                          uint64_t end_timestamp_ns) = 0;

  [[nodiscard]] virtual std::vector<SampleBlock> GetSamples(uint64_t pane_id,
                                                            uint64_t start_timestamp_ns,
                                                            uint64_t end_timestamp_ns) = 0;

  [[nodiscard]] virtual std::string GetLabel(uint64_t label_id) = 0;

  // TODO: The functions are used on the slow path
};

std::unique_ptr<CaptureDataProvider> CreateLegacyCaptureDataProvider(
    const orbit_string_manager::StringManager* string_manager,
    const orbit_client_data::TrackDataManager* track_data_manager);

}  // namespace orbit_capture_data_provider

#endif  // CAPTURE_DATA_PROVIDER_CAPTURE_DATA_PROVIDER_H_
