// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include "ClientData/ScopeIdProvider.h"
#include "ClientFlags/ClientFlags.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"

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

static std::vector<uint64_t> GetIds(ScopeIdProvider* id_provider,
                                    const std::vector<orbit_client_protos::TimerInfo>& timers) {
  std::vector<uint64_t> ids;
  std::transform(std::begin(timers), std::end(timers), std::back_inserter(ids),
                 [id_provider](const TimerInfo& timer) { return id_provider->ProvideId(timer); });
  return ids;
}

static void TestProvideId(std::vector<orbit_client_protos::TimerInfo>& timer_infos) {
  orbit_grpc_protos::CaptureOptions capture_options;
  auto id_provider = NameEqualityScopeIdProvider::Create(capture_options);

  const std::vector<uint64_t> ids = GetIds(id_provider.get(), timer_infos);
  AssertNameToIdIsBijective(timer_infos, ids);
  for (size_t i = 0; i < timer_infos.size(); ++i) {
    EXPECT_EQ(id_provider->GetScopeName(ids[i]), timer_infos[i].api_scope_name());
  }
}

TEST(NameEqualityScopeIdProviderTest, ProvideIdIsCorrectForApiScope) {
  // TODO (b/226565085) remove the flag when the manual instrumentation grouping feature is
  // released.
  absl::SetFlag(&FLAGS_devmode, true);
  auto timer_infos = MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScope);
  TestProvideId(timer_infos);
}

TEST(NameEqualityScopeIdProviderTest, ProvideIdIsCorrectForApiScopeAsync) {
  // TODO (b/226565085) remove the flag when the manual instrumentation grouping feature is
  // released.
  absl::SetFlag(&FLAGS_devmode, true);

  auto async_timer_infos =
      MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScopeAsync);
  TestProvideId(async_timer_infos);
}

TEST(NameEqualityScopeIdProviderTest, SyncAndAsyncScopesOfTheSameNameGetDifferentIds) {
  // TODO (b/226565085) remove the flag when the manual instrumentation grouping feature is
  // released.
  absl::SetFlag(&FLAGS_devmode, true);
  TimerInfo sync = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScope);
  TimerInfo async = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScopeAsync);

  orbit_grpc_protos::CaptureOptions capture_options;
  auto id_provider = NameEqualityScopeIdProvider::Create(capture_options);

  ASSERT_NE(id_provider->ProvideId(sync), id_provider->ProvideId(async));
}

constexpr size_t kFunctionCount = 3;
constexpr std::array<uint64_t, kFunctionCount> kFunctionIds = {10, 13, 15};
const std::array<std::string, kFunctionCount> kFunctionNames = {"foo()", "bar()", "baz()"};

static void AddInstrumentedFunction(orbit_grpc_protos::CaptureOptions& capture_options,
                                    uint64_t function_id, const std::string& name) {
  orbit_grpc_protos::InstrumentedFunction* function = capture_options.add_instrumented_functions();
  function->set_function_id(function_id);
  function->set_function_name(name);
}

TEST(NameEqualityScopeIdProviderTest, CreateIsCorrect) {
  // TODO (b/226565085) remove the flag when the manual instrumentation grouping feature is
  // released.
  absl::SetFlag(&FLAGS_devmode, true);
  orbit_grpc_protos::CaptureOptions capture_options;
  for (size_t i = 0; i < kFunctionCount; ++i) {
    AddInstrumentedFunction(capture_options, kFunctionIds[i], kFunctionNames[i]);
  }

  auto setter = NameEqualityScopeIdProvider::Create(capture_options);
  TimerInfo timer_info = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScope);

  ASSERT_EQ(setter->ProvideId(timer_info),
            *std::max_element(std::begin(kFunctionIds), std::end(kFunctionIds)) + 1);

  for (size_t i = 0; i < kFunctionCount; ++i) {
    EXPECT_EQ(setter->GetScopeName(kFunctionIds[i]), kFunctionNames[i]);
  }
}

}  // namespace orbit_client_data