// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DynamicInstrumentationManager.h"

#include <absl/strings/ascii.h>
#include <absl/strings/match.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::SchedulingSlice;

namespace orbit_windows_tracing {

namespace {

struct CaptureListener : public orbit_lib::CaptureListener {
  CaptureListener() = delete;
  explicit CaptureListener(TracerListener* listener) : listener_(listener){};
  void OnError(const char* message) override { ERROR("%s", message); }
  void OnTimer(uint64_t absolute_address, uint64_t raw_start, uint64_t raw_end, uint32_t tid,
               uint32_t pid) override {
    uint64_t start = orbit_base::RawTimestampToNs(raw_start);
    uint64_t end = orbit_base::RawTimestampToNs(raw_end);
    orbit_grpc_protos::FunctionCall function_call;
    function_call.set_function_id(absolute_address);
    function_call.set_end_timestamp_ns(end);
    function_call.set_duration_ns(end - start);
    function_call.set_tid(tid);
    function_call.set_pid(pid);

    if (listener_) {
      listener_->OnFunctionCall(std::move(function_call));
    }
  }
  TracerListener* listener_ = nullptr;
};

bool IsWindowsApiDll(const std::string& file_path) {
  static std::vector<std::string> windows_api_dlls = {"ntdll.dll", "kernel32.dll",
                                                      "kernelbase.dll"};

  std::string lower_file_path = absl::AsciiStrToLower(file_path);
  for (const std::string& windows_api_dll : windows_api_dlls) {
    if (absl::StrContains(lower_file_path, windows_api_dll)) return true;
  }
  return false;
}

bool IsFileIoFunction(const std::string& file_path, const std::string& function_name) {
  if (!IsWindowsApiDll(file_path)) return false;
  static std::vector<std::string> file_io_function_names = {"WriteFile", "ReadFile"};
  for (const std::string& name : file_io_function_names) {
    if (absl::StrContains(function_name, name)) return true;
  }
  return false;
}

orbit_lib::FunctionHook FunctionHookFromInstrumentedFunction(
    const InstrumentedFunction& instrumented_function) {
  orbit_lib::FunctionHook function_hook;
  function_hook.address = instrumented_function.function_id();
  function_hook.type =
      IsFileIoFunction(instrumented_function.file_path(), instrumented_function.function_name())
          ? orbit_lib::FunctionHook::Type::kFileIO
          : orbit_lib::FunctionHook::Type::kRegular;
  return function_hook;
}

}  // namespace

void DynamicInstrumentationManager::Start(const orbit_grpc_protos::CaptureOptions& capture_options,
                                          TracerListener* listener) {
  std::vector<orbit_lib::FunctionHook> function_hooks;
  for (const InstrumentedFunction& instrumented_function :
       capture_options.instrumented_functions()) {
    LOG("Hooking function %lu", instrumented_function.function_id());
    function_hooks.push_back(FunctionHookFromInstrumentedFunction(instrumented_function));
    // TODO-PG: Adopt function id instead of using it as address
    // instrumented_functions_.emplace_back(function_id, instrumented_function.file_path(),
    //                                      instrumented_function.file_offset());
  }

  capture_listener_ = std::make_unique<CaptureListener>(listener);
  orbit_lib::StartCapture(capture_options.pid(), function_hooks.data(), function_hooks.size(),
                          capture_listener_.get());
}

void DynamicInstrumentationManager::Stop() { orbit_lib::StopCapture(); }

}  // namespace orbit_windows_tracing