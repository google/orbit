// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_COMMONS_H_
#define LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_COMMONS_H_

#include <absl/types/span.h>
#include <sys/types.h>

#include "GrpcProtos/capture.pb.h"

namespace orbit_linux_tracing_integration_tests {

// Adds `IntegrationTestPuppet`'s functions `OuterFunctionToInstrument` and
// `InnerFunctionToInstrument` to the `CaptureOptions` as functions to dynamically instrument.
// The details of the functions are retrieved by searching the debug symbols of the binary.
void AddPuppetOuterAndInnerFunctionToCaptureOptions(
    orbit_grpc_protos::CaptureOptions* capture_options, pid_t pid, uint64_t outer_function_id,
    uint64_t inner_function_id);

// Verifies the expectations on the number and content of the `FunctionCall` events produced when
// dynamically instrumenting `IntegrationTestPuppet`'s functions `OuterFunctionToInstrument` and
// `InnerFunctionToInstrument`.
void VerifyFunctionCallsOfPuppetOuterAndInnerFunction(
    absl::Span<const orbit_grpc_protos::FunctionCall> function_calls, uint32_t pid,
    uint64_t outer_function_id, uint64_t inner_function_id, bool expect_return_value_and_registers);

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_COMMONS_H_
