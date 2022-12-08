// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_MOCK_SCOPE_STATS_COLLECTION_H_
#define ORBIT_CLIENT_DATA_MOCK_SCOPE_STATS_COLLECTION_H_

#include <gmock/gmock.h>
#include <stdint.h>

#include "ClientData/ScopeStatsCollection.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

class MockScopeStatsCollection : public ScopeStatsCollectionInterface {
 public:
  MOCK_METHOD(std::vector<ScopeId>, GetAllProvidedScopeIds, (), (const, override));
  MOCK_METHOD(const ScopeStats&, GetScopeStatsOrDefault, (ScopeId), (const, override));
  MOCK_METHOD(const std::vector<uint64_t>*, GetSortedTimerDurationsForScopeId, (ScopeId),
              (const, override));

  MOCK_METHOD(void, UpdateScopeStats, (ScopeId, const TimerInfo& timer), (override));
  MOCK_METHOD(void, SetScopeStats, (ScopeId, ScopeStats), (override));
  MOCK_METHOD(void, OnCaptureComplete, (), (override));
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_MOCK_SCOPE_STATS_COLLECTION_H_