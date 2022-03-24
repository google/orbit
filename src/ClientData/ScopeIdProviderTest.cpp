// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include "ClientData/ScopeIdProvider.h"
#include "ClientFlags/ClientFlags.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/Constants.h"

namespace orbit_client_data {

const std::vector<std::string> kNames{"A", "B", "C", "D", "A", "B", "B"};

[[nodiscard]] static orbit_client_protos::TimerInfo MakeTimerInfo(
    const std::string& name, orbit_client_protos::TimerInfo_Type type) {
  orbit_client_protos::TimerInfo timer_info;
  timer_info.set_api_scope_name(name);
  timer_info.set_type(type);
  timer_info.set_function_id(orbit_grpc_protos::kInvalidFunctionId);
  return timer_info;
}

[[nodiscard]] static std::vector<orbit_client_protos::TimerInfo> MakeTimerInfos(
    const std::vector<std::string>& names, orbit_client_protos::TimerInfo_Type type) {
  std::vector<orbit_client_protos::TimerInfo> timer_infos;
  std::transform(std::begin(names), std::end(names), std::back_inserter(timer_infos),
                 [type](const auto& name) { return MakeTimerInfo(name, type); });
  return timer_infos;
}

static void AssertNameToIdIsBijective(const std::vector<orbit_client_protos::TimerInfo>& timers,
                                      const std::vector<uint64_t>& ids) {
  absl::flat_hash_map<std::string, uint64_t> name_to_id;
  for (size_t i = 0; i < timers.size(); ++i) {
    name_to_id[timers[i].api_scope_name()] = ids[i];
  }

  absl::flat_hash_set<uint64_t> ids_set(std::begin(ids), std::end(ids));
  ASSERT_EQ(ids_set.size(), name_to_id.size());

  for (size_t i = 0; i < timers.size(); ++i) {
    ASSERT_EQ(ids[i], name_to_id[timers[i].api_scope_name()]);
  }
}

static std::vector<uint64_t> GetIds(const std::vector<orbit_client_protos::TimerInfo>& timers) {
  orbit_grpc_protos::CaptureOptions capture_options;
  auto id_provider = NameEqualityScopeIdProvider::Create(capture_options);

  std::vector<uint64_t> ids;
  std::transform(std::begin(timers), std::end(timers), std::back_inserter(ids),
                 [&id_provider](const TimerInfo& timer) { return id_provider->ProvideId(timer); });
  return ids;
}

static void TestProvideId(std::vector<orbit_client_protos::TimerInfo>& timer_infos) {
  AssertNameToIdIsBijective(timer_infos, GetIds(timer_infos));
}

TEST(NameEqualityScopeIdProviderTest, ProvideIdIsCorrectForApiScope) {
  absl::SetFlag(&FLAGS_devmode, true);
  auto timer_infos = MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScope);
  TestProvideId(timer_infos);
}

TEST(NameEqualityScopeIdProviderTest, ProvideIdIsCorrectForApiScopeAsync) {
  absl::SetFlag(&FLAGS_devmode, true);

  auto async_timer_infos =
      MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScopeAsync);
  TestProvideId(async_timer_infos);
}

TEST(NameEqualityScopeIdProviderTest, SyncAndAsyncScopesOfTheSameNameGetDifferentIds) {
  absl::SetFlag(&FLAGS_devmode, true);
  TimerInfo sync = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScope);
  TimerInfo async = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScopeAsync);

  orbit_grpc_protos::CaptureOptions capture_options;
  auto id_provider = NameEqualityScopeIdProvider::Create(capture_options);

  ASSERT_NE(id_provider->ProvideId(sync), id_provider->ProvideId(async));
}

TEST(NameEqualityScopeIdProviderTest, CreateIsCorrect) {
  absl::SetFlag(&FLAGS_devmode, true);
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.add_instrumented_functions()->set_function_id(10);
  capture_options.add_instrumented_functions()->set_function_id(13);
  capture_options.add_instrumented_functions()->set_function_id(15);

  auto setter = NameEqualityScopeIdProvider::Create(capture_options);
  TimerInfo timer_info = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScope);

  ASSERT_EQ(setter->ProvideId(timer_info), 16);
}

}  // namespace orbit_client_data