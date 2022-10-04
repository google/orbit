// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"

using ::testing::ElementsAre;
using ::testing::Mock;
using ::testing::SaveArg;

namespace orbit_linux_tracing {

namespace {

constexpr uint64_t kTimestamp = 100;
constexpr pid_t kPidTargetNamespace = 42;
constexpr pid_t kTidTargetNamespace = 43;
constexpr pid_t kPidRootNamespace = 1042;
constexpr pid_t kTidRootNamespace = 1043;
constexpr uint64_t kEncodedName = 0x4142414241424142;
constexpr int kEncodedNameAdditionalLength = 3;
constexpr uint32_t kColorRgb = 44;
constexpr uint64_t kGroupId = 45;
constexpr uint64_t kId = 46;
constexpr uint64_t kAddressInFunction = 45;
constexpr int kData = 46;

class UprobesUnwindingVisitorApiTest : public ::testing::Test {
 protected:
  void SetUp() override {
    visitor_.SetInitialTidToRootNamespaceTidMapping(
        {{kPidTargetNamespace, kPidRootNamespace}, {kTidTargetNamespace, kTidRootNamespace}});
    memset(encoded_name_additional_.get(), 0x42, kEncodedNameAdditionalLength * sizeof(uint64_t));
  }

  template <typename APerfEventData>
  void SetUpCommonFieldsInPerfEvent(TypedPerfEvent<APerfEventData>& perf_event) {
    perf_event.timestamp = kTimestamp;
    perf_event.ordered_stream =
        PerfEventOrderedStream::ManualInstrumentationThreadId(kTidTargetNamespace);
    perf_event.data.pid = kPidTargetNamespace;
    perf_event.data.tid = kTidTargetNamespace;
    perf_event.data.encoded_name_1 = kEncodedName;
    perf_event.data.encoded_name_2 = kEncodedName;
    perf_event.data.encoded_name_3 = kEncodedName;
    perf_event.data.encoded_name_4 = kEncodedName;
    perf_event.data.encoded_name_5 = kEncodedName;
    perf_event.data.encoded_name_6 = kEncodedName;
    perf_event.data.encoded_name_7 = kEncodedName;
    perf_event.data.encoded_name_8 = kEncodedName;
    perf_event.data.encoded_name_additional =
        std::make_unique<uint64_t[]>(kEncodedNameAdditionalLength);
    perf_event.data.encoded_name_additional_length = kEncodedNameAdditionalLength;
    perf_event.data.color_rgba = kColorRgb;
    memcpy(perf_event.data.encoded_name_additional.get(), encoded_name_additional_.get(),
           kEncodedNameAdditionalLength * sizeof(uint64_t));
  }

  template <typename ApiProto>
  void VerifyCommonFieldsInPerfEvent(const ApiProto& api_proto) {
    EXPECT_EQ(kPidRootNamespace, api_proto.pid());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_1());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_2());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_3());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_4());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_5());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_6());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_7());
    EXPECT_EQ(kEncodedName, api_proto.encoded_name_8());
    EXPECT_EQ(kEncodedNameAdditionalLength, api_proto.encoded_name_additional_size());
    EXPECT_EQ(0, memcmp(encoded_name_additional_.get(), api_proto.encoded_name_additional().data(),
                        kEncodedNameAdditionalLength * sizeof(uint64_t)));
    EXPECT_EQ(kColorRgb, api_proto.color_rgba());
  }

  MockTracerListener listener_;
  MockUprobesReturnAddressManager return_address_manager_{nullptr};

  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   /*user_space_instrumentation_addresses=*/nullptr,
                                   /*absolute_address_to_size_of_functions_to_stop_at_=*/nullptr};
  std::unique_ptr<uint64_t[]> encoded_name_additional_{
      std::make_unique<uint64_t[]>(kEncodedNameAdditionalLength)};

 private:
  UprobesFunctionCallManager function_call_manager_;
  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{128};
};

}  // namespace

TEST_F(UprobesUnwindingVisitorApiTest, ApiScopeStart) {
  ApiScopeStartPerfEvent api_scope_start_perf_event;
  SetUpCommonFieldsInPerfEvent(api_scope_start_perf_event);
  api_scope_start_perf_event.data.group_id = kGroupId;
  api_scope_start_perf_event.data.address_in_function = kAddressInFunction;

  orbit_grpc_protos::ApiScopeStart actual_api_scope_start;
  EXPECT_CALL(listener_, OnApiScopeStart).Times(1).WillOnce(SaveArg<0>(&actual_api_scope_start));

  PerfEvent{std::move(api_scope_start_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_scope_start);
  EXPECT_EQ(kGroupId, actual_api_scope_start.group_id());
  EXPECT_EQ(kAddressInFunction, actual_api_scope_start.address_in_function());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiScopeStartAsync) {
  ApiScopeStartAsyncPerfEvent api_scope_start_async_perf_event;
  SetUpCommonFieldsInPerfEvent(api_scope_start_async_perf_event);
  api_scope_start_async_perf_event.data.id = kId;
  api_scope_start_async_perf_event.data.address_in_function = kAddressInFunction;

  orbit_grpc_protos::ApiScopeStartAsync actual_api_scope_start_async;
  EXPECT_CALL(listener_, OnApiScopeStartAsync)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_api_scope_start_async));

  PerfEvent{std::move(api_scope_start_async_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_scope_start_async);
  EXPECT_EQ(kId, actual_api_scope_start_async.id());
  EXPECT_EQ(kAddressInFunction, actual_api_scope_start_async.address_in_function());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiScopeStop) {
  ApiScopeStopPerfEvent api_scope_stop_perf_event;
  api_scope_stop_perf_event.timestamp = kTimestamp;
  api_scope_stop_perf_event.ordered_stream =
      PerfEventOrderedStream::ManualInstrumentationThreadId(kTidTargetNamespace);

  api_scope_stop_perf_event.data.pid = kPidTargetNamespace;
  api_scope_stop_perf_event.data.tid = kTidTargetNamespace;

  orbit_grpc_protos::ApiScopeStop actual_api_scope_stop;
  EXPECT_CALL(listener_, OnApiScopeStop).Times(1).WillOnce(SaveArg<0>(&actual_api_scope_stop));

  PerfEvent{std::move(api_scope_stop_perf_event)}.Accept(&visitor_);

  EXPECT_EQ(kTimestamp, actual_api_scope_stop.timestamp_ns());
  EXPECT_EQ(kPidRootNamespace, actual_api_scope_stop.pid());
  EXPECT_EQ(kTidRootNamespace, actual_api_scope_stop.tid());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiScopeStopAsync) {
  ApiScopeStopAsyncPerfEvent api_scope_stop_async_perf_event;
  api_scope_stop_async_perf_event.timestamp = kTimestamp;
  api_scope_stop_async_perf_event.ordered_stream =
      PerfEventOrderedStream::ManualInstrumentationThreadId(kTidTargetNamespace);

  api_scope_stop_async_perf_event.data.pid = kPidTargetNamespace;
  api_scope_stop_async_perf_event.data.tid = kTidTargetNamespace;
  api_scope_stop_async_perf_event.data.id = kId;

  orbit_grpc_protos::ApiScopeStopAsync actual_api_scope_stop_async;
  EXPECT_CALL(listener_, OnApiScopeStopAsync)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_api_scope_stop_async));

  PerfEvent{std::move(api_scope_stop_async_perf_event)}.Accept(&visitor_);

  EXPECT_EQ(kTimestamp, actual_api_scope_stop_async.timestamp_ns());
  EXPECT_EQ(kPidRootNamespace, actual_api_scope_stop_async.pid());
  EXPECT_EQ(kTidRootNamespace, actual_api_scope_stop_async.tid());
  EXPECT_EQ(kId, actual_api_scope_stop_async.id());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiStringEvent) {
  ApiStringEventPerfEvent api_string_event_perf_event;
  SetUpCommonFieldsInPerfEvent(api_string_event_perf_event);
  api_string_event_perf_event.data.id = kId;

  orbit_grpc_protos::ApiStringEvent actual_api_string_event;
  EXPECT_CALL(listener_, OnApiStringEvent).Times(1).WillOnce(SaveArg<0>(&actual_api_string_event));

  PerfEvent{std::move(api_string_event_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_string_event);
  EXPECT_EQ(kId, actual_api_string_event.id());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackDouble) {
  ApiTrackDoublePerfEvent api_track_double_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_double_perf_event);
  api_track_double_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackDouble actual_api_track_double;
  EXPECT_CALL(listener_, OnApiTrackDouble).Times(1).WillOnce(SaveArg<0>(&actual_api_track_double));

  PerfEvent{std::move(api_track_double_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_double);
  EXPECT_EQ(kData, actual_api_track_double.data());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackFloat) {
  ApiTrackFloatPerfEvent api_track_float_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_float_perf_event);
  api_track_float_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackFloat actual_api_track_float;
  EXPECT_CALL(listener_, OnApiTrackFloat).Times(1).WillOnce(SaveArg<0>(&actual_api_track_float));

  PerfEvent{std::move(api_track_float_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_float);
  EXPECT_EQ(kData, actual_api_track_float.data());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackInt) {
  ApiTrackIntPerfEvent api_track_int_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_int_perf_event);
  api_track_int_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackInt actual_api_track_int;
  EXPECT_CALL(listener_, OnApiTrackInt).Times(1).WillOnce(SaveArg<0>(&actual_api_track_int));

  PerfEvent{std::move(api_track_int_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_int);
  EXPECT_EQ(kData, actual_api_track_int.data());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackInt64) {
  ApiTrackInt64PerfEvent api_track_int64_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_int64_perf_event);
  api_track_int64_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackInt64 actual_api_track_int64;
  EXPECT_CALL(listener_, OnApiTrackInt64).Times(1).WillOnce(SaveArg<0>(&actual_api_track_int64));

  PerfEvent{std::move(api_track_int64_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_int64);
  EXPECT_EQ(kData, actual_api_track_int64.data());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackUint) {
  ApiTrackUintPerfEvent api_track_uint_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_uint_perf_event);
  api_track_uint_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackUint actual_api_track_uint;
  EXPECT_CALL(listener_, OnApiTrackUint).Times(1).WillOnce(SaveArg<0>(&actual_api_track_uint));

  PerfEvent{std::move(api_track_uint_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_uint);
  EXPECT_EQ(kData, actual_api_track_uint.data());
}

TEST_F(UprobesUnwindingVisitorApiTest, ApiTrackUint64) {
  ApiTrackUint64PerfEvent api_track_uint64_perf_event;
  SetUpCommonFieldsInPerfEvent(api_track_uint64_perf_event);
  api_track_uint64_perf_event.data.data = kData;

  orbit_grpc_protos::ApiTrackUint64 actual_api_track_uint64;
  EXPECT_CALL(listener_, OnApiTrackUint64).Times(1).WillOnce(SaveArg<0>(&actual_api_track_uint64));

  PerfEvent{std::move(api_track_uint64_perf_event)}.Accept(&visitor_);

  VerifyCommonFieldsInPerfEvent(actual_api_track_uint64);
  EXPECT_EQ(kData, actual_api_track_uint64.data());
}

}  // namespace orbit_linux_tracing
