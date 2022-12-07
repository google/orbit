// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/types.h>

#include <filesystem>
#include <string>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "IntegrationTestPuppet.h"
#include "IntegrationTestUtils.h"
#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing_integration_tests {

using PuppetConstants = IntegrationTestPuppetConstants;

void AddPuppetOuterAndInnerFunctionToCaptureOptions(
    orbit_grpc_protos::CaptureOptions* capture_options, pid_t pid, uint64_t outer_function_id,
    uint64_t inner_function_id) {
  // Find the offset in the ELF file of the "outer" function and the "inner" function and add those
  // functions to the CaptureOptions to be instrumented.
  const orbit_grpc_protos::ModuleInfo& module_info = GetExecutableBinaryModuleInfo(pid);
  const orbit_grpc_protos::ModuleSymbols& module_symbols = GetExecutableBinaryModuleSymbols(pid);
  const std::filesystem::path& executable_path = GetExecutableBinaryPath(pid);

  bool outer_function_symbol_found = false;
  bool inner_function_symbol_found = false;
  for (const orbit_grpc_protos::SymbolInfo& symbol : module_symbols.symbol_infos()) {
    if (absl::StrContains(symbol.demangled_name(), PuppetConstants::kOuterFunctionName)) {
      ORBIT_CHECK(!outer_function_symbol_found);
      outer_function_symbol_found = true;
      orbit_grpc_protos::InstrumentedFunction instrumented_function;
      instrumented_function.set_file_path(executable_path);
      instrumented_function.set_file_offset(symbol.address() - module_info.load_bias());
      instrumented_function.set_function_id(outer_function_id);
      instrumented_function.set_function_virtual_address(symbol.address());
      instrumented_function.set_function_size(symbol.size());
      instrumented_function.set_function_name(symbol.demangled_name());
      instrumented_function.set_record_return_value(true);
      capture_options->mutable_instrumented_functions()->Add(std::move(instrumented_function));
    }

    if (absl::StrContains(symbol.demangled_name(), PuppetConstants::kInnerFunctionName)) {
      ORBIT_CHECK(!inner_function_symbol_found);
      inner_function_symbol_found = true;
      orbit_grpc_protos::InstrumentedFunction instrumented_function;
      instrumented_function.set_file_path(executable_path);
      instrumented_function.set_file_offset(symbol.address() - module_info.load_bias());
      instrumented_function.set_function_id(inner_function_id);
      instrumented_function.set_function_virtual_address(symbol.address());
      instrumented_function.set_function_size(symbol.size());
      instrumented_function.set_function_name(symbol.demangled_name());
      instrumented_function.set_record_arguments(true);
      capture_options->mutable_instrumented_functions()->Add(std::move(instrumented_function));
    }
  }
  ORBIT_CHECK(outer_function_symbol_found);
  ORBIT_CHECK(inner_function_symbol_found);
}

void VerifyFunctionCallsOfPuppetOuterAndInnerFunction(
    absl::Span<const orbit_grpc_protos::FunctionCall> function_calls, uint32_t pid,
    uint64_t outer_function_id, uint64_t inner_function_id,
    bool expect_return_value_and_registers) {
  for (const orbit_grpc_protos::FunctionCall& function_call : function_calls) {
    ASSERT_EQ(function_call.pid(), pid);
    ASSERT_EQ(function_call.tid(), pid);
  }

  // We expect an ordered sequence of kInnerFunctionCallCount calls to the "inner" function followed
  // by one call to the "outer" function, repeated kOuterFunctionCallCount times.
  ASSERT_EQ(function_calls.size(), PuppetConstants::kOuterFunctionCallCount +
                                       PuppetConstants::kOuterFunctionCallCount *
                                           PuppetConstants::kInnerFunctionCallCount);
  size_t function_call_index = 0;
  for (size_t outer_index = 0; outer_index < PuppetConstants::kOuterFunctionCallCount;
       ++outer_index) {
    uint64_t inner_calls_duration_ns_sum = 0;
    for (size_t inner_index = 0; inner_index < PuppetConstants::kInnerFunctionCallCount;
         ++inner_index) {
      const orbit_grpc_protos::FunctionCall& function_call = function_calls[function_call_index];
      EXPECT_EQ(function_call.function_id(), inner_function_id);
      EXPECT_GT(function_call.duration_ns(), 0);
      inner_calls_duration_ns_sum += function_call.duration_ns();
      if (function_call_index > 0) {
        EXPECT_GT(function_call.end_timestamp_ns(),
                  function_calls[function_call_index - 1].end_timestamp_ns());
      }
      EXPECT_EQ(function_call.depth(), 1);
      if (expect_return_value_and_registers) {
        EXPECT_EQ(function_call.return_value(), 0);
        EXPECT_THAT(function_call.registers(), testing::ElementsAre(1, 2, 3, 4, 5, 6));
      } else {
        EXPECT_EQ(function_call.return_value(), 0);
        EXPECT_THAT(function_call.registers(), testing::ElementsAre());
      }
      ++function_call_index;
    }

    {
      const orbit_grpc_protos::FunctionCall& function_call = function_calls[function_call_index];
      EXPECT_EQ(function_call.function_id(), outer_function_id);
      EXPECT_GT(function_call.duration_ns(), inner_calls_duration_ns_sum);
      if (function_call_index > 0) {
        EXPECT_GT(function_call.end_timestamp_ns(),
                  function_calls[function_call_index - 1].end_timestamp_ns());
      }
      EXPECT_EQ(function_call.depth(), 0);
      if (expect_return_value_and_registers) {
        EXPECT_EQ(function_call.return_value(), PuppetConstants::kOuterFunctionReturnValue);
        EXPECT_THAT(function_call.registers(), testing::ElementsAre());
      } else {
        EXPECT_EQ(function_call.return_value(), 0);
        EXPECT_THAT(function_call.registers(), testing::ElementsAre());
      }
      ++function_call_index;
    }
  }
}

}  // namespace orbit_linux_tracing_integration_tests
