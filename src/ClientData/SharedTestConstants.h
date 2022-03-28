// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef CLIENT_DATA_SHARED_TEST_CONSTANTS_H_
#define CLIENT_DATA_SHARED_TEST_CONSTANTS_H_

#include <array>
#include <cstddef>
#include <cstdint>

#include "ClientData/TimerTrackDataIdManager.h"

namespace orbit_client_data {

constexpr size_t kTimersForFirstId = 3;
constexpr size_t kTimersForSecondId = 2;
constexpr size_t kTimerCount = kTimersForFirstId + kTimersForSecondId;
constexpr uint64_t kFirstId = 1;
constexpr uint64_t kSecondId = 2;
constexpr std::array<uint64_t, kTimerCount> kTimerIds = {kFirstId, kFirstId, kFirstId, kSecondId,
                                                         kSecondId};
constexpr std::array<uint64_t, kTimerCount> kStarts = {10, 20, 30, 40, 50};
constexpr std::array<uint64_t, kTimersForFirstId> kDurationsForFirstId = {300, 100, 200};
constexpr std::array<uint64_t, kTimersForFirstId> kSortedDurationsForFirstId = {100, 200, 300};
constexpr std::array<uint64_t, kTimersForSecondId> kDurationsForSecondId = {500, 400};
constexpr std::array<uint64_t, kTimersForSecondId> kSortedDurationsForSecondId = {400, 500};

const std::array<uint64_t, kTimerCount> kDurations = []() {
  std::array<uint64_t, kTimerCount> result;
  std::copy(std::begin(kDurationsForFirstId), std::end(kDurationsForFirstId), std::begin(result));
  std::copy(std::begin(kDurationsForSecondId), std::end(kDurationsForSecondId),
            std::begin(result) + kTimersForFirstId);
  return result;
}();
const std::array<TimerInfo, kTimerCount> kTimerInfos = []() {
  std::array<TimerInfo, kTimerCount> result;
  for (size_t i = 0; i < kTimerCount; ++i) {
    result[i].set_function_id(kTimerIds[i]);
    result[i].set_start(kStarts[i]);
    result[i].set_end(kStarts[i] + kDurations[i]);
  }
  return result;
}();
}  // namespace orbit_client_data
#endif  // CLIENT_DATA_SHARED_TEST_CONSTANTS_H_
