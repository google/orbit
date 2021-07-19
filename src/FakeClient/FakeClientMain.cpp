// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/time/clock.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <memory>
#include <thread>

#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/ModuleManager.h"
#include "FakeCaptureEventProcessor.h"
#include "GrpcProtos/Constants.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadPool.h"
#include "capture_data.pb.h"

ABSL_FLAG(uint64_t, port, 44765, "Port OrbitService's gRPC service is listening on");
ABSL_FLAG(int32_t, pid, 0, "PID of the process to capture");
ABSL_FLAG(uint32_t, duration, std::numeric_limits<uint32_t>::max(),
          "Duration of the capture in seconds (stop earlier with Ctrl+C)");
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Callstack sampling rate in samples per second (0: no sampling)");
ABSL_FLAG(bool, frame_pointers, false, "Use frame pointers for unwinding");
ABSL_FLAG(std::string, instrument_path, "", "Path of the binary of the function to instrument");
ABSL_FLAG(uint64_t, instrument_offset, 0, "Offset in the binary of the function to instrument");
ABSL_FLAG(bool, scheduling, true, "Collect scheduling information");
ABSL_FLAG(bool, thread_state, false, "Collect thread state information");
ABSL_FLAG(bool, gpu_jobs, true, "Collect GPU jobs");
ABSL_FLAG(uint16_t, memory_sampling_rate, 0,
          "Memory usage sampling rate in samples per second (0: no sampling)");

namespace {
std::atomic<bool> exit_requested = false;

void SigintHandler(int signum) {
  if (signum == SIGINT) {
    exit_requested = true;
  }
}

// Use SIGINT to stop capturing before the specified duration has elapsed.
void InstallSigintHandler() {
  struct sigaction act {};
  act.sa_handler = SigintHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_restorer = nullptr;
  sigaction(SIGINT, &act, nullptr);
}

// On OrbitService's side, and in particular in LinuxTracing, the only information needed to
// instrument a function is what uprobes need, i.e., the path of the module and the function's
// offset in the module (address minus load bias). But CaptureClient requires much more than that,
// through the ModuleManager and the FunctionInfos. For now just keep it simple and leave the fields
// that are not needed empty.
void ManipulateModuleManagerAndSelectedFunctionsToAddUprobeFromOffset(
    orbit_client_data::ModuleManager* module_manager,
    absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>* selected_functions,
    const std::string& file_path, uint64_t file_offset, uint64_t function_id) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
      orbit_object_utils::CreateElfFile(std::filesystem::path{file_path});
  FAIL_IF(error_or_elf_file.has_error(), "%s", error_or_elf_file.error().message());
  std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();
  std::string build_id = elf_file->GetBuildId();
  uint64_t load_bias = elf_file->GetLoadBias();
  uint64_t executable_segment_offset = elf_file->GetExecutableSegmentOffset();

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_file_path(file_path);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);
  module_info.set_executable_segment_offset(executable_segment_offset);
  CHECK(module_manager->AddOrUpdateModules({module_info}).empty());

  orbit_client_protos::FunctionInfo function_info;
  function_info.set_module_path(file_path);
  function_info.set_module_build_id(build_id);
  function_info.set_address(load_bias + file_offset);
  function_info.set_orbit_type(orbit_client_protos::FunctionInfo::kNone);
  selected_functions->emplace(function_id, function_info);
}

void ManipulateModuleManagerAndSelectedFunctionsToAddUprobeFromFunctionName(
    orbit_client_data::ModuleManager* module_manager,
    absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>* selected_functions,
    const std::string& file_path, const std::string& mangled_function_name, uint64_t function_id) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
      orbit_object_utils::CreateElfFile(std::filesystem::path{file_path});
  FAIL_IF(error_or_elf_file.has_error(), "%s", error_or_elf_file.error().message());
  std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();
  std::string build_id = elf_file->GetBuildId();
  uint64_t load_bias = elf_file->GetLoadBias();
  uint64_t executable_segment_offset = elf_file->GetExecutableSegmentOffset();

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_file_path(file_path);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);
  module_info.set_executable_segment_offset(executable_segment_offset);
  CHECK(module_manager->AddOrUpdateModules({module_info}).empty());

  ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> symbols_or_error =
      elf_file->LoadSymbolsFromDynsym();
  FAIL_IF(symbols_or_error.has_error(), "%s", symbols_or_error.error().message());
  orbit_grpc_protos::ModuleSymbols& symbols = symbols_or_error.value();

  std::optional<uint64_t> address = std::nullopt;
  for (const orbit_grpc_protos::SymbolInfo& symbol : symbols.symbol_infos()) {
    if (symbol.name() == mangled_function_name) {
      address = symbol.address();
    }
  }
  FAIL_IF(!address.has_value(), "Could not find function \"%s\" in module \"%s\"",
          mangled_function_name, file_path);

  orbit_client_protos::FunctionInfo function_info;
  function_info.set_module_path(file_path);
  function_info.set_module_build_id(build_id);
  function_info.set_address(address.value());
  function_info.set_orbit_type(orbit_client_protos::FunctionInfo::kNone);
  selected_functions->emplace(function_id, function_info);
}

}  // namespace

// OrbitFakeClient is a simple command line client that connects to a local instance of
// OrbitService and asks it to take a capture, with capture options specified through command line
// arguments.
// It provides a simple way to make OrbitService take a capture in order to tests its performance
// with various capture options.
// In general, received events are mostly discarded. Only minimal processing is applied to report
// some basic metrics, such as event count and their total size, and average frame time of the
// target process. See FakeCaptureEventProcessor.
int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("Orbit fake client for testing");
  absl::ParseCommandLine(argc, argv);

  FAIL_IF(absl::GetFlag(FLAGS_pid) == 0, "PID to capture not specified");
  FAIL_IF(absl::GetFlag(FLAGS_duration) == 0, "Specified a zero-length duration");
  FAIL_IF((absl::GetFlag(FLAGS_instrument_path).empty()) !=
              (absl::GetFlag(FLAGS_instrument_offset) == 0),
          "Binary path and offset of the function to instrument need to be specified together");

  int32_t process_id = absl::GetFlag(FLAGS_pid);
  LOG("process_id=%d", process_id);
  uint16_t samples_per_second = absl::GetFlag(FLAGS_sampling_rate);
  LOG("samples_per_second=%u", samples_per_second);
  constexpr uint16_t kStackDumpSize = 65000;
  orbit_grpc_protos::UnwindingMethod unwinding_method =
      absl::GetFlag(FLAGS_frame_pointers)
          ? orbit_grpc_protos::UnwindingMethod::kFramePointerUnwinding
          : orbit_grpc_protos::UnwindingMethod::kDwarfUnwinding;
  LOG("unwinding_method=%s",
      unwinding_method == orbit_grpc_protos::UnwindingMethod::kFramePointerUnwinding
          ? "Frame pointers"
          : "DWARF");
  std::string file_path = absl::GetFlag(FLAGS_instrument_path);
  uint64_t file_offset = absl::GetFlag(FLAGS_instrument_offset);
  bool instrument_function = !file_path.empty() && file_offset != 0;
  if (instrument_function) {
    LOG("file_path=%s", file_path);
    LOG("file_offset=%#x", file_offset);
  }
  constexpr bool kAlwaysRecordArguments = false;
  constexpr bool kRecordReturnValues = false;
  bool collect_scheduling_info = absl::GetFlag(FLAGS_scheduling);
  LOG("collect_scheduling_info=%d", collect_scheduling_info);
  bool collect_thread_state = absl::GetFlag(FLAGS_thread_state);
  LOG("collect_thread_state=%d", collect_thread_state);
  bool collect_gpu_jobs = absl::GetFlag(FLAGS_gpu_jobs);
  LOG("collect_gpu_jobs=%d", collect_gpu_jobs);
  constexpr bool kEnableApi = false;
  constexpr bool kEnableIntrospection = false;
  constexpr bool kEnableUserSpaceInstrumentation = false;
  constexpr uint64_t kMaxLocalMarkerDepthPerCommandBuffer = std::numeric_limits<uint64_t>::max();
  bool collect_memory_info = absl::GetFlag(FLAGS_memory_sampling_rate) > 0;
  LOG("collect_memory_info=%d", collect_memory_info);
  uint64_t memory_sampling_period_ms = 0;
  if (collect_memory_info) {
    memory_sampling_period_ms = 1'000 / absl::GetFlag(FLAGS_memory_sampling_rate);
    LOG("memory_sampling_period_ms=%u", memory_sampling_period_ms);
  }

  uint32_t grpc_port = absl::GetFlag(FLAGS_port);
  std::string service_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  std::shared_ptr<grpc::Channel> grpc_channel =
      grpc::CreateChannel(service_address, grpc::InsecureChannelCredentials());
  LOG("service_address=%s", service_address);
  CHECK(grpc_channel != nullptr);

  InstallSigintHandler();

  orbit_capture_client::CaptureClient capture_client{grpc_channel};
  std::shared_ptr<ThreadPool> thread_pool = ThreadPool::Create(1, 1, absl::Seconds(1));

  orbit_client_data::ModuleManager module_manager;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions;
  if (instrument_function) {
    constexpr uint64_t kInstrumentedFunctionId = 1;
    ManipulateModuleManagerAndSelectedFunctionsToAddUprobeFromOffset(
        &module_manager, &selected_functions, file_path, file_offset, kInstrumentedFunctionId);
  }

  // Instrument vkQueuePresentKHR, if possible.
  {
    ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> modules_or_error =
        orbit_object_utils::ReadModules(process_id);
    FAIL_IF(modules_or_error.has_error(), "%s", modules_or_error.error().message());
    static const std::string kLibvulkanModuleName{"libvulkan.so.1"};
    std::string libvulkan_file_path;
    for (const orbit_grpc_protos::ModuleInfo& module : modules_or_error.value()) {
      if (module.soname() == kLibvulkanModuleName) {
        libvulkan_file_path = module.file_path();
        break;
      }
    }
    if (!libvulkan_file_path.empty()) {
      static const std::string kQueuePresentFunctionName{"vkQueuePresentKHR"};
      LOG("%s found: instrumenting %s", kLibvulkanModuleName, kQueuePresentFunctionName);
      ManipulateModuleManagerAndSelectedFunctionsToAddUprobeFromFunctionName(
          &module_manager, &selected_functions, libvulkan_file_path, kQueuePresentFunctionName,
          orbit_fake_client::FakeCaptureEventProcessor::kFrameBoundaryFunctionId);
    }
  }

  auto capture_event_processor = std::make_unique<orbit_fake_client::FakeCaptureEventProcessor>();

  auto capture_outcome_future = capture_client.Capture(
      thread_pool.get(), process_id, module_manager, selected_functions, kAlwaysRecordArguments,
      kRecordReturnValues, orbit_client_data::TracepointInfoSet{}, samples_per_second,
      kStackDumpSize, unwinding_method, collect_scheduling_info, collect_thread_state,
      collect_gpu_jobs, kEnableApi, kEnableIntrospection, kEnableUserSpaceInstrumentation,
      kMaxLocalMarkerDepthPerCommandBuffer, collect_memory_info, memory_sampling_period_ms,
      std::move(capture_event_processor));
  LOG("Asked to start capture");

  uint32_t duration_s = absl::GetFlag(FLAGS_duration);
  absl::Time start_time = absl::Now();
  while (!exit_requested && absl::Now() < start_time + absl::Seconds(duration_s)) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    CHECK(!capture_outcome_future.IsFinished());
  }
  CHECK(capture_client.StopCapture());
  LOG("Asked to stop capture");

  auto capture_outcome_or_error = capture_outcome_future.Get();
  if (capture_outcome_or_error.has_error()) {
    FATAL("Capture failed: %s", capture_outcome_or_error.error().message());
  }
  CHECK(capture_outcome_or_error.value() ==
        orbit_capture_client::CaptureListener::CaptureOutcome::kComplete);
  LOG("Capture completed");

  thread_pool->ShutdownAndWait();

  return 0;
}
