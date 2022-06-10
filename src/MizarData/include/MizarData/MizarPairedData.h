// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_MIZAR_PAIRED_DATA_H_
#define MIZAR_DATA_MIZAR_PAIRED_DATA_H_

#include <absl/container/flat_hash_map.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientProtos/capture_data.pb.h"
#include "MizarData/NonWrappingAddition.h"
#include "MizarData/SampledFunctionId.h"
#include "OrbitBase/ThreadConstants.h"

namespace orbit_mizar_data {

// This class represents the data loaded from a capture that has been made aware of its counterpart
// it will be compared against. In particular, it is aware of the functions that has been sampled in
// the other capture. Also, it is aware of the sampled function ids assigned to the functions.
template <typename Data>
class MizarPairedData {
 public:
  MizarPairedData(std::unique_ptr<Data> data, absl::flat_hash_map<uint64_t, SFID> address_to_sfid)
      : data_(std::move(data)), address_to_sfid_(std::move(address_to_sfid)) {}

  [[nodiscard]] std::vector<uint64_t> ActiveInvocationTimes(
      uint64_t scope_id, uint64_t min_relative_timestamp_ns,
      uint64_t max_relative_timestamp_ns) const {
    const auto [min_timestamp_ns, max_timestamp_ns] =
        RelativeToAbsoluteTimestampRange(min_relative_timestamp_ns, max_relative_timestamp_ns);
    const std::vector<const orbit_client_protos::TimerInfo*> timers =
        data_->GetCaptureData().GetTimersForScope(scope_id, min_timestamp_ns, max_timestamp_ns);
    if (timers.size() < 2) return {};

    // TODO(b/235572160) Estimate the actual sampling period and use it instead
    const uint64_t sampling_period = data_->GetNominalSamplingPeriodNs();

    const uint32_t tid = timers.front()->thread_id();

    std::vector<uint64_t> result;
    for (size_t i = 0; i + 1 < timers.size(); ++i) {
      const uint64_t callstack_count =
          CallstackSamplesCount(tid, timers[i]->start(), timers[i + 1]->start());
      result.push_back(sampling_period * callstack_count);
    }
    return result;
  }

  // Action is a void callable that takes a single argument of type
  // `const std::vector<uint64_t>` representing a callstack sample, each element of the vector is a
  // sampled function id.
  // `min_relative_timestamp_ns` and `max_relative_timestamp_ns` are the nanoseconds elapsed since
  // capture start.
  template <typename Action>
  void ForEachCallstackEvent(uint32_t tid, uint64_t min_relative_timestamp_ns,
                             uint64_t max_relative_timestamp_ns, Action&& action) const {
    const orbit_client_data::CallstackData& callstack_data =
        data_->GetCaptureData().GetCallstackData();

    auto action_on_callstack_events =
        [this, &callstack_data, &action](const orbit_client_data::CallstackEvent& event) -> void {
      const orbit_client_data::CallstackInfo* callstack =
          callstack_data.GetCallstack(event.callstack_id());
      const std::vector<SFID> sfids = CallstackWithSFIDs(callstack);
      std::invoke(std::forward<Action>(action), sfids);
    };

    const auto [min_timestamp_ns, max_timestamp_ns] =
        RelativeToAbsoluteTimestampRange(min_relative_timestamp_ns, max_relative_timestamp_ns);
    if (tid == orbit_base::kAllProcessThreadsTid) {
      callstack_data.ForEachCallstackEventInTimeRange(min_timestamp_ns, max_timestamp_ns,
                                                      action_on_callstack_events);
    } else {
      callstack_data.ForEachCallstackEventOfTidInTimeRange(tid, min_timestamp_ns, max_timestamp_ns,
                                                           action_on_callstack_events);
    }
  }

 private:
  [[nodiscard]] uint64_t CallstackSamplesCount(uint32_t tid, uint64_t min_timestamp_ns,
                                               uint64_t max_timestamp_ns) const {
    uint64_t count = 0;
    data_->GetCaptureData().GetCallstackData().ForEachCallstackEventOfTidInTimeRange(
        tid, min_timestamp_ns, max_timestamp_ns, [&count](const auto& /*_*/) { count++; });
    return count;
  }

  [[nodiscard]] std::pair<uint64_t, uint64_t> RelativeToAbsoluteTimestampRange(
      uint64_t min_relative_timestamp_ns, uint64_t max_relative_timestamp_ns) const {
    return std::make_pair(
        NonWrappingAddition(data_->GetCaptureStartTimestampNs(), min_relative_timestamp_ns),
        NonWrappingAddition(data_->GetCaptureStartTimestampNs(), max_relative_timestamp_ns));
  }

  [[nodiscard]] std::vector<SFID> CallstackWithSFIDs(
      const orbit_client_data::CallstackInfo* callstack) const {
    if (callstack->frames().empty()) return {};
    if (callstack->type() != orbit_client_data::CallstackType::kComplete) {
      return CallstackWithSFIDs({callstack->frames()[0]});
    }
    return CallstackWithSFIDs(callstack->frames());
  }

  [[nodiscard]] std::vector<SFID> CallstackWithSFIDs(const std::vector<uint64_t>& frames) const {
    std::vector<SFID> result;
    for (const uint64_t address : frames) {
      if (auto it = address_to_sfid_.find(address); it != address_to_sfid_.end()) {
        result.push_back(it->second);
      }
    }
    return result;
  }

  std::unique_ptr<Data> data_;
  absl::flat_hash_map<uint64_t, SFID> address_to_sfid_;
};

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_MIZAR_PAIRED_DATA_H_
