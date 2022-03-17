#include <absl/container/flat_hash_set.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <string>

#include "CaptureClient/ApiEventIdSetter.h"
#include "ClientProtos/capture_data.pb.h"

using ::testing::Each;
using ::testing::Eq;
using ::testing::Property;

[[nodiscard]] static orbit_client_protos::TimerInfo MakeTimerInfo(
    const std::string& name, orbit_client_protos::TimerInfo_Type type) {
  orbit_client_protos::TimerInfo timer_info;
  timer_info.set_api_scope_name(name);
  timer_info.set_type(type);
  return timer_info;
}

[[nodiscard]] static std::vector<orbit_client_protos::TimerInfo> MakeTimerInfos(
    const std::vector<std::string>& names, orbit_client_protos::TimerInfo_Type type) {
  std::vector<orbit_client_protos::TimerInfo> timer_infos;
  std::transform(std::begin(names), std::end(names), std::back_inserter(timer_infos),
                 [type](const auto& name) { return MakeTimerInfo(name, type); });
  return timer_infos;
}

void AssertApiScopeGroupIdUniqueness(const std::vector<TimerInfo>& timers) {
  absl::flat_hash_map<std::string, uint64_t> name_to_id;
  for (const auto& timer : timers) {
    name_to_id[timer.api_scope_name()] = timer.api_scope_group_id();
  }

  absl::flat_hash_set<uint64_t> ids;
  std::transform(std::begin(name_to_id), std::end(name_to_id), std::inserter(ids, ids.begin()),
                 [](const auto& key_value) { return key_value.second; });

  ASSERT_EQ(ids.size(), name_to_id.size());

  for (const auto& timer : timers) {
    ASSERT_EQ(timer.api_scope_group_id(), name_to_id[timer.api_scope_name()]);
  }
}

const std::vector<std::string> kNames{"A", "B", "C", "D", "A", "B", "B"};

namespace orbit_capture_client {

void SetIds(std::vector<TimerInfo>& timer_infos) {
  NameEqualityApiEventIdSetter setter;
  for (auto& timer_info : timer_infos) {
    setter.SetId(timer_info);
  }
}

void TestSetId(std::vector<TimerInfo>& timer_infos) {
  SetIds(timer_infos);
  AssertApiScopeGroupIdUniqueness(timer_infos);
}

TEST(NameEqualityApiEventIdSetterTest, SetIdIsCorrectForApiScope) {
  auto timer_infos = MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScope);
  TestSetId(timer_infos);
}

TEST(NameEqualityApiEventIdSetterTest, SetIdIsCorrectForApiScopeAsync) {
  auto async_timer_infos =
      MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kApiScopeAsync);
  TestSetId(async_timer_infos);
}

TEST(NameEqualityApiEventIdSetter, SetIdDoesNotSetForNonApiScope) {
  std::vector<TimerInfo> function_timer_infos =
      MakeTimerInfos(kNames, orbit_client_protos::TimerInfo_Type_kNone);
  SetIds(function_timer_infos);
  ASSERT_THAT(function_timer_infos,
              Each(Property("api_scope_group_id", &TimerInfo::api_scope_group_id, Eq(0))));
}

TEST(NameEqualityApiEventIdSetterTest, CreateIsCorrect) {
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.add_instrumented_functions()->set_function_id(10);
  capture_options.add_instrumented_functions()->set_function_id(13);
  capture_options.add_instrumented_functions()->set_function_id(15);

  NameEqualityApiEventIdSetter setter = NameEqualityApiEventIdSetter::Create(capture_options);
  TimerInfo timer_info = MakeTimerInfo("A", orbit_client_protos::TimerInfo_Type_kApiScope);

  setter.SetId(timer_info);

  ASSERT_EQ(timer_info.api_scope_group_id(), 16);
}

}  // namespace orbit_capture_client