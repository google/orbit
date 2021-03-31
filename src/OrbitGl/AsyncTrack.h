// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ASYNC_TRACK_H_
#define ORBIT_GL_ASYNC_TRACK_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CallstackThreadBar.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "TextBox.h"
#include "TimerChain.h"
#include "TimerTrack.h"
#include "Track.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class OrbitApp;

class AsyncTrack final : public TimerTrack {
 public:
  explicit AsyncTrack(TimeGraph* time_graph, TimeGraphLayout* layout, const std::string& name,
                      OrbitApp* app, const CaptureData* capture_data);

  [[nodiscard]] Type GetType() const override { return kAsyncTrack; };
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() const override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void UpdateBoxHeight() override;

 protected:
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, float min_x, float z_offset,
                        TextBox* text_box) override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer_info,
                                    bool is_selected, bool is_highlighted) const override;

  // Used for determining what row can receive a new timer with no overlap.
  absl::flat_hash_map<uint32_t, uint64_t> max_span_time_by_depth_;
};

#endif  // ORBIT_GL_ASYNC_TRACK_H_
