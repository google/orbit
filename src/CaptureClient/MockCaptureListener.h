// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_MOCK_CAPTURE_LISTENER_H_
#define CAPTURE_CLIENT_MOCK_CAPTURE_LISTENER_H_

#include <gmock/gmock.h>

#include "CaptureClient/CaptureListener.h"

namespace orbit_capture_client {

class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(void, OnCaptureStarted,
              (const orbit_grpc_protos::CaptureStarted&, std::optional<std::filesystem::path>,
               absl::flat_hash_set<uint64_t>),
              (override));
  MOCK_METHOD(void, OnCaptureFinished, (const orbit_grpc_protos::CaptureFinished&), (override));
  MOCK_METHOD(void, OnTimer, (const orbit_client_protos::TimerInfo&), (override));
  MOCK_METHOD(void, OnCgroupAndProcessMemoryInfo,
              (const orbit_client_data::CgroupAndProcessMemoryInfo&), (override));
  MOCK_METHOD(void, OnPageFaultsInfo, (const orbit_client_data::PageFaultsInfo&), (override));
  MOCK_METHOD(void, OnSystemMemoryInfo, (const orbit_client_data::SystemMemoryInfo&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallstack, (uint64_t, orbit_client_data::CallstackInfo), (override));
  MOCK_METHOD(void, OnCallstackEvent, (orbit_client_data::CallstackEvent), (override));
  MOCK_METHOD(void, OnThreadName, (uint32_t, std::string), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_client_data::ThreadStateSliceInfo), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_client_data::LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo, (uint64_t, orbit_client_data::TracepointInfo),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_client_data::TracepointEventInfo), (override));
  MOCK_METHOD(void, OnModuleUpdate, (uint64_t, orbit_grpc_protos::ModuleInfo), (override));
  MOCK_METHOD(void, OnModulesSnapshot, (uint64_t, std::vector<orbit_grpc_protos::ModuleInfo>),
              (override));
  MOCK_METHOD(void, OnPresentEvent, (const orbit_grpc_protos::PresentEvent&), (override));
  MOCK_METHOD(void, OnApiStringEvent, (const orbit_client_data::ApiStringEvent&), (override));
  MOCK_METHOD(void, OnApiTrackValue, (const orbit_client_data::ApiTrackValue&), (override));
  MOCK_METHOD(void, OnWarningEvent, (orbit_grpc_protos::WarningEvent), (override));
  MOCK_METHOD(void, OnClockResolutionEvent, (orbit_grpc_protos::ClockResolutionEvent), (override));
  MOCK_METHOD(void, OnErrorsWithPerfEventOpenEvent,
              (orbit_grpc_protos::ErrorsWithPerfEventOpenEvent), (override));
  MOCK_METHOD(void, OnWarningInstrumentingWithUprobesEvent,
              (orbit_grpc_protos::WarningInstrumentingWithUprobesEvent), (override));
  MOCK_METHOD(void, OnErrorEnablingOrbitApiEvent, (orbit_grpc_protos::ErrorEnablingOrbitApiEvent),
              (override));
  MOCK_METHOD(void, OnErrorEnablingUserSpaceInstrumentationEvent,
              (orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent), (override));
  MOCK_METHOD(void, OnWarningInstrumentingWithUserSpaceInstrumentationEvent,
              (orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent),
              (override));
  MOCK_METHOD(void, OnLostPerfRecordsEvent, (orbit_grpc_protos::LostPerfRecordsEvent), (override));
  MOCK_METHOD(void, OnOutOfOrderEventsDiscardedEvent,
              (orbit_grpc_protos::OutOfOrderEventsDiscardedEvent), (override));
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_MOCK_CAPTURE_LISTENER_H_