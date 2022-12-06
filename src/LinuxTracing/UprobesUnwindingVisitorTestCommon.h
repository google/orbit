// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_UPROBES_UNWINDING_VISITOR_TEST_COMMON_H_
#define LINUX_TRACING_UPROBES_UNWINDING_VISITOR_TEST_COMMON_H_

#include <absl/types/span.h>
#include <asm/perf_regs.h>
#include <gmock/gmock.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include <array>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "LeafFunctionCallManager.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "UprobesReturnAddressManager.h"

namespace orbit_linux_tracing {

class MockLibunwindstackMaps : public LibunwindstackMaps {
 public:
  MOCK_METHOD(std::shared_ptr<unwindstack::MapInfo>, Find, (uint64_t), (override));
  MOCK_METHOD(unwindstack::Maps*, Get, (), (override));
  MOCK_METHOD(void, AddAndSort, (uint64_t, uint64_t, uint64_t, uint64_t, std::string_view),
              (override));
};

class MockLibunwindstackUnwinder : public LibunwindstackUnwinder {
 public:
  MOCK_METHOD(LibunwindstackResult, Unwind,
              (pid_t, unwindstack::Maps*, (const std::array<uint64_t, PERF_REG_X86_64_MAX>&),
               absl::Span<const StackSliceView>, bool, size_t),
              (override));
  MOCK_METHOD(std::optional<bool>, HasFramePointerSet, (uint64_t, pid_t, unwindstack::Maps*),
              (override));
};

class MockUprobesReturnAddressManager : public UprobesReturnAddressManager {
 public:
  explicit MockUprobesReturnAddressManager(
      UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses)
      : UprobesReturnAddressManager{user_space_instrumentation_addresses} {}

  MOCK_METHOD(void, ProcessFunctionEntry, (pid_t, uint64_t, uint64_t), (override));
  MOCK_METHOD(void, ProcessFunctionExit, (pid_t), (override));
  MOCK_METHOD(void, PatchSample, (pid_t, uint64_t, void*, uint64_t), (override));
  MOCK_METHOD(bool, PatchCallchain, (pid_t, uint64_t*, uint64_t, LibunwindstackMaps*), (override));
};

class MockLeafFunctionCallManager : public LeafFunctionCallManager {
 public:
  explicit MockLeafFunctionCallManager(uint16_t stack_dump_size)
      : LeafFunctionCallManager(stack_dump_size) {}
  MOCK_METHOD(orbit_grpc_protos::Callstack::CallstackType, PatchCallerOfLeafFunction,
              (const CallchainSamplePerfEventData*, LibunwindstackMaps*, LibunwindstackUnwinder*),
              (override));
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_UPROBES_UNWINDING_VISITOR_TEST_COMMON_H_
