// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/strings/match.h>
#include <absl/time/clock.h>
#include <grpcpp/grpcpp.h>
#include <sys/inotify.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <memory>
#include <thread>

#include "ApiUtils/GetFunctionTableAddressPrefix.h"
#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/ModuleManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "FakeCaptureEventProcessor.h"
#include "Flags.h"
#include "GraphicsCaptureEventProcessor.h"
#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/ThreadPool.h"

namespace {

using orbit_grpc_protos::CaptureOptions;
using DynamicInstrumentationMethod =
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod;
using UnwindingMethod = orbit_grpc_protos::CaptureOptions::UnwindingMethod;

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
// offset in the module (address minus load bias); in the case of user space instrumentation, the
// function size is also needed. But CaptureClient requires much more than that, through the
// ModuleManager and the FunctionInfos. For now just keep it simple and leave the fields that are
// not needed empty.
void ManipulateModuleManagerAndSelectedFunctionsToAddInstrumentedFunctionFromOffset(
    orbit_client_data::ModuleManager* module_manager,
    absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>* selected_functions,
    const std::string& file_path, const std::string& function_name, uint64_t file_offset,
    uint64_t function_size, uint64_t function_id) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
      orbit_object_utils::CreateElfFile(std::filesystem::path{file_path});
  ORBIT_FAIL_IF(error_or_elf_file.has_error(), "%s", error_or_elf_file.error().message());
  std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();
  std::string build_id = elf_file->GetBuildId();
  uint64_t load_bias = elf_file->GetLoadBias();

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_file_path(file_path);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);
  module_info.set_executable_segment_offset(elf_file->GetExecutableSegmentOffset());
  ORBIT_CHECK(module_manager->AddOrUpdateModules({module_info}).empty());

  orbit_client_protos::FunctionInfo function_info;
  function_info.set_pretty_name(function_name);
  function_info.set_module_path(file_path);
  function_info.set_module_build_id(build_id);
  function_info.set_address(load_bias + file_offset);
  function_info.set_size(function_size);
  selected_functions->emplace(function_id, function_info);
}

void ManipulateModuleManagerAndSelectedFunctionsToAddInstrumentedFunctionFromFunctionNameInDebugSymbols(
    orbit_client_data::ModuleManager* module_manager,
    absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>* selected_functions,
    const std::string& file_path, const std::string& demangled_function_name,
    uint64_t function_id) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
      orbit_object_utils::CreateElfFile(std::filesystem::path{file_path});
  ORBIT_FAIL_IF(error_or_elf_file.has_error(), "%s", error_or_elf_file.error().message());
  std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();
  std::string build_id = elf_file->GetBuildId();
  uint64_t executable_segment_offset = elf_file->GetExecutableSegmentOffset();

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_name(elf_file->GetName());
  module_info.set_file_path(file_path);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(elf_file->GetLoadBias());
  module_info.set_executable_segment_offset(executable_segment_offset);
  ORBIT_CHECK(module_manager->AddOrUpdateModules({module_info}).empty());

  ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> symbols_or_error = elf_file->LoadDebugSymbols();
  ORBIT_FAIL_IF(symbols_or_error.has_error(), "%s", symbols_or_error.error().message());
  orbit_grpc_protos::ModuleSymbols& symbols = symbols_or_error.value();

  std::optional<orbit_grpc_protos::SymbolInfo> symbol = std::nullopt;
  for (const orbit_grpc_protos::SymbolInfo& symbol_candidate : symbols.symbol_infos()) {
    if (symbol_candidate.demangled_name() == demangled_function_name) {
      symbol = symbol_candidate;
      break;
    }
  }
  ORBIT_FAIL_IF(!symbol.has_value(), "Could not find function \"%s\" in module \"%s\"",
                demangled_function_name, file_path);

  orbit_client_protos::FunctionInfo function_info;
  function_info.set_name(symbol->name());
  function_info.set_pretty_name(symbol->demangled_name());
  function_info.set_module_path(file_path);
  function_info.set_module_build_id(build_id);
  function_info.set_address(symbol->address());
  function_info.set_size(symbol->size());
  selected_functions->emplace(function_id, function_info);
}

// This is a very simple version of the logic for finding a debug symbols file that we have in
// SymbolHelper. For a file binary.ext we look for symbols in binary.ext, binary.debug, and
// binary.ext.debug.
std::optional<orbit_grpc_protos::ModuleSymbols> FindAndLoadDebugSymbols(
    const std::string& file_path) {
  std::vector<std::filesystem::path> candidate_paths = {file_path};
  std::filesystem::path file_name = std::filesystem::path{file_path}.filename();
  std::filesystem::path file_dir = std::filesystem::path{file_path}.parent_path();
  static constexpr const char* kDebugFileExt = ".debug";
  {
    std::filesystem::path file_name_dot_debug{file_name};
    file_name_dot_debug.replace_extension(kDebugFileExt);
    candidate_paths.emplace_back(file_dir / file_name_dot_debug);
  }
  {
    std::filesystem::path file_name_plus_debug{file_name.string() + kDebugFileExt};
    candidate_paths.emplace_back(file_dir / file_name_plus_debug);
  }

  for (const std::filesystem::path& candidate_path : candidate_paths) {
    ErrorMessageOr<bool> error_or_exists = orbit_base::FileExists(candidate_path);
    ORBIT_FAIL_IF(error_or_exists.has_error(), "%s", error_or_exists.error().message());
    if (!error_or_exists.value()) {
      continue;
    }

    ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
        orbit_object_utils::CreateElfFile(candidate_path);
    if (error_or_elf_file.has_error()) {
      ORBIT_ERROR("%s", error_or_elf_file.error().message());
      continue;
    }
    std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();

    ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> symbols_or_error =
        elf_file->LoadDebugSymbols();
    // Load debug symbols from the first of the candidate files that contains any.
    if (symbols_or_error.has_error()) {
      continue;
    }

    ORBIT_LOG("Loaded debug symbols of module \"%s\" from \"%s\"", file_name.string(),
              elf_file->GetName());
    return symbols_or_error.value();
  }

  ORBIT_ERROR("Could not find debug symbols of module \"%s\"", file_name.string());
  return std::nullopt;
}

void ManipulateModuleManagerToAddFunctionFromFunctionPrefixInSymtabIfExists(
    orbit_client_data::ModuleManager* module_manager, const std::string& file_path,
    const std::string& demangled_function_prefix) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> error_or_elf_file =
      orbit_object_utils::CreateElfFile(std::filesystem::path{file_path});
  ORBIT_FAIL_IF(error_or_elf_file.has_error(), "%s", error_or_elf_file.error().message());
  std::unique_ptr<orbit_object_utils::ElfFile>& elf_file = error_or_elf_file.value();
  std::string build_id = elf_file->GetBuildId();
  uint64_t load_bias = elf_file->GetLoadBias();

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_name(elf_file->GetName());
  module_info.set_file_path(file_path);
  module_info.set_build_id(build_id);
  module_info.set_load_bias(load_bias);
  module_info.set_executable_segment_offset(elf_file->GetExecutableSegmentOffset());
  ORBIT_CHECK(module_manager->AddOrUpdateModules({module_info}).empty());

  std::optional<orbit_grpc_protos::ModuleSymbols> symbols_opt = FindAndLoadDebugSymbols(file_path);
  if (!symbols_opt.has_value()) {
    return;
  }
  orbit_grpc_protos::ModuleSymbols& symbols = symbols_opt.value();

  std::optional<orbit_grpc_protos::SymbolInfo> symbol = std::nullopt;
  for (const orbit_grpc_protos::SymbolInfo& symbol_candidate : symbols.symbol_infos()) {
    if (absl::StartsWith(symbol_candidate.demangled_name(), demangled_function_prefix)) {
      symbol = symbol_candidate;
      break;
    }
  }
  if (!symbol.has_value()) {
    ORBIT_ERROR("Could not find function with prefix \"%s\" in module \"%s\"",
                demangled_function_prefix, elf_file->GetName());
    return;
  }
  ORBIT_LOG("Found function \"%s\" in module \"%s\"", symbol->name(), elf_file->GetName());

  orbit_grpc_protos::SymbolInfo symbol_info;
  symbol_info.set_name(symbol->name());
  symbol_info.set_demangled_name(symbol->demangled_name());
  symbol_info.set_address(symbol->address());
  symbol_info.set_size(symbol->size());

  orbit_grpc_protos::ModuleSymbols module_symbols;
  module_symbols.set_load_bias(load_bias);
  module_symbols.set_symbols_file_path(file_path);
  module_symbols.mutable_symbol_infos()->Add(std::move(symbol_info));

  module_manager->GetMutableModuleByPathAndBuildId(file_path, build_id)->AddSymbols(module_symbols);
}

uint32_t ReadPidFromFile(const std::string& file_path) {
  ErrorMessageOr<std::string> pid_string = orbit_base::ReadFileToString(file_path);
  ORBIT_FAIL_IF(pid_string.has_error(), "Reading from \"%s\": %s", file_path,
                pid_string.error().message());
  uint32_t process_id = 0;
  ORBIT_FAIL_IF(!absl::SimpleAtoi<uint32_t>(pid_string.value(), &process_id),
                "Failed to read the PID from \"%s\"", file_path);
  return process_id;
}

void WaitForFileModification(const std::string& file_path) {
  int inotify_fd = -1;
  int wd = -1;

  inotify_fd = inotify_init();
  ORBIT_FAIL_IF(inotify_fd == -1, "Failed to initialize inotify");

  wd = inotify_add_watch(inotify_fd, file_path.c_str(), IN_MODIFY);
  ORBIT_FAIL_IF(wd == -1, "Failed to watch \"%s\"", file_path);
  ORBIT_LOG("Started to watch \"%s\"", file_path);

  // Wait for the first modify event received to read its PID
  constexpr ssize_t kMaxBufferSize = 1024;
  constexpr ssize_t kMinReadSize = sizeof(inotify_event);

  ssize_t offset = 0;
  char buffer[kMaxBufferSize];
  while (offset < kMinReadSize) {
    ssize_t bytes_read = read(inotify_fd, buffer + offset, kMaxBufferSize - offset);
    ORBIT_FAIL_IF(bytes_read == -1, "Failed to read watch event");
    offset += bytes_read;
  }

  inotify_event* event = reinterpret_cast<inotify_event*>(buffer);
  ORBIT_CHECK(event->wd == wd);

  close(inotify_fd);
  ORBIT_LOG("Stopped watching \"%s\"", file_path);
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

  ORBIT_LOG("Starting Client");
  uint32_t duration_s = absl::GetFlag(FLAGS_duration);
  ORBIT_FAIL_IF(duration_s == 0, "Specified a zero-length duration");
  ORBIT_FAIL_IF(
      (absl::GetFlag(FLAGS_instrument_path).empty()) !=
          (absl::GetFlag(FLAGS_instrument_offset) == 0),
      "Binary path and offset of the function to instrument need to be specified together");

  uint32_t process_id = absl::GetFlag(FLAGS_pid);
  if (process_id == 0) {
    const std::string pid_file_path = absl::GetFlag(FLAGS_pid_file_path);
    ORBIT_FAIL_IF(pid_file_path.empty(), "A PID or a path to a file is needed.");
    WaitForFileModification(pid_file_path);
    process_id = ReadPidFromFile(pid_file_path);
  }
  ORBIT_LOG("process_id=%d", process_id);
  ORBIT_FAIL_IF(process_id == 0, "PID to capture not specified");

  uint16_t samples_per_second = absl::GetFlag(FLAGS_sampling_rate);
  ORBIT_LOG("samples_per_second=%u", samples_per_second);
  constexpr uint16_t kStackDumpSize = 65000;
  const UnwindingMethod unwinding_method =
      absl::GetFlag(FLAGS_frame_pointers) ? CaptureOptions::kFramePointers : CaptureOptions::kDwarf;
  ORBIT_LOG("unwinding_method=%s",
            unwinding_method == CaptureOptions::kFramePointers ? "Frame pointers" : "DWARF");

  std::string file_path = absl::GetFlag(FLAGS_instrument_path);
  uint64_t file_offset = absl::GetFlag(FLAGS_instrument_offset);
  bool instrument_function = !file_path.empty() && file_offset != 0;
  const int64_t function_size = absl::GetFlag(FLAGS_instrument_size);
  const std::string function_name = absl::GetFlag(FLAGS_instrument_name);
  DynamicInstrumentationMethod instrumentation_method =
      absl::GetFlag(FLAGS_user_space_instrumentation) ? CaptureOptions::kUserSpaceInstrumentation
                                                      : CaptureOptions::kKernelUprobes;
  ORBIT_LOG("user_space_instrumentation=%d",
            instrumentation_method == CaptureOptions::kUserSpaceInstrumentation);
  if (instrument_function) {
    ORBIT_LOG("file_path=%s", file_path);
    ORBIT_LOG("file_offset=%#x", file_offset);
    if (instrumentation_method == CaptureOptions::kUserSpaceInstrumentation) {
      ORBIT_FAIL_IF(function_size == -1, "User space instrumentation requires the function size");
      ORBIT_LOG("function_size=%d", function_size);
      ORBIT_FAIL_IF(function_name.empty(), "User space instrumentation requires the function name");
      ORBIT_LOG("function_name=%s", function_name);
    }
  }
  constexpr bool kAlwaysRecordArguments = false;
  constexpr bool kRecordReturnValues = false;

  bool collect_scheduling_info = absl::GetFlag(FLAGS_scheduling);
  ORBIT_LOG("collect_scheduling_info=%d", collect_scheduling_info);
  bool collect_thread_state = absl::GetFlag(FLAGS_thread_state);
  ORBIT_LOG("collect_thread_state=%d", collect_thread_state);
  bool collect_gpu_jobs = absl::GetFlag(FLAGS_gpu_jobs);
  ORBIT_LOG("collect_gpu_jobs=%d", collect_gpu_jobs);
  bool enable_api = absl::GetFlag(FLAGS_orbit_api);
  ORBIT_LOG("enable_api=%d", enable_api);
  constexpr bool kEnableIntrospection = false;
  constexpr uint64_t kMaxLocalMarkerDepthPerCommandBuffer = std::numeric_limits<uint64_t>::max();
  bool collect_memory_info = absl::GetFlag(FLAGS_memory_sampling_rate) > 0;
  ORBIT_LOG("collect_memory_info=%d", collect_memory_info);
  uint64_t memory_sampling_period_ms = 0;
  if (collect_memory_info) {
    memory_sampling_period_ms = 1'000 / absl::GetFlag(FLAGS_memory_sampling_rate);
    ORBIT_LOG("memory_sampling_period_ms=%u", memory_sampling_period_ms);
  }

  uint32_t grpc_port = absl::GetFlag(FLAGS_port);
  std::string service_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  std::shared_ptr<grpc::Channel> grpc_channel =
      grpc::CreateChannel(service_address, grpc::InsecureChannelCredentials());
  ORBIT_LOG("service_address=%s", service_address);
  ORBIT_CHECK(grpc_channel != nullptr);

  InstallSigintHandler();

  orbit_capture_client::CaptureClient capture_client{grpc_channel};
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(1, 1, absl::Seconds(1));

  orbit_client_data::ModuleManager module_manager;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions;
  if (instrument_function) {
    constexpr uint64_t kInstrumentedFunctionId = 1;
    ManipulateModuleManagerAndSelectedFunctionsToAddInstrumentedFunctionFromOffset(
        &module_manager, &selected_functions, file_path, function_name, file_offset, function_size,
        kInstrumentedFunctionId);
  }

  if (enable_api) {
    ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> modules_or_error =
        orbit_object_utils::ReadModules(process_id);
    ORBIT_FAIL_IF(modules_or_error.has_error(), "%s", modules_or_error.error().message());
    for (const orbit_grpc_protos::ModuleInfo& module : modules_or_error.value()) {
      ManipulateModuleManagerToAddFunctionFromFunctionPrefixInSymtabIfExists(
          &module_manager, module.file_path(),
          orbit_api_utils::kOrbitApiGetFunctionTableAddressPrefix);
    }
  }

  const bool calculate_frame_time = absl::GetFlag(FLAGS_frame_time);
  ORBIT_LOG("frame_time=%d", calculate_frame_time);
  if (calculate_frame_time) {
    // Instrument vkQueuePresentKHR, if possible.
    // Some application don't call libVulkan library directly; instead, they just query the
    // function addresses and use those. So let's just instrument the `ggpvlk QueuePresentKHR`
    static const std::string kGgpvlkModuleName = "ggpvlk.so";
    static const std::string kQueuePresentFunctionName{
        "yeti::internal::vulkan::(anonymous namespace)::QueuePresentKHR(VkQueue_T*, "
        "VkPresentInfoKHR const*)"};

    ErrorMessageOr<std::vector<orbit_grpc_protos::ModuleInfo>> modules_or_error =
        orbit_object_utils::ReadModules(process_id);
    ORBIT_FAIL_IF(modules_or_error.has_error(), "%s", modules_or_error.error().message());
    std::string libvulkan_file_path;
    for (const orbit_grpc_protos::ModuleInfo& module : modules_or_error.value()) {
      if (module.soname() == kGgpvlkModuleName) {
        libvulkan_file_path = module.file_path();
        break;
      }
    }
    if (!libvulkan_file_path.empty()) {
      ORBIT_LOG("%s found: instrumenting %s", kGgpvlkModuleName, kQueuePresentFunctionName);
      ManipulateModuleManagerAndSelectedFunctionsToAddInstrumentedFunctionFromFunctionNameInDebugSymbols(
          &module_manager, &selected_functions, libvulkan_file_path, kQueuePresentFunctionName,
          orbit_fake_client::FakeCaptureEventProcessor::kFrameBoundaryFunctionId);
      ORBIT_LOG("%s instrumented", kQueuePresentFunctionName);
    } else {
      ORBIT_LOG("%s not found", kGgpvlkModuleName);
    }
  }

  std::unique_ptr<orbit_capture_client::CaptureEventProcessor> capture_event_processor;
  switch (absl::GetFlag(FLAGS_event_processor)) {
    case EventProcessorType::kFake:
      capture_event_processor = std::make_unique<orbit_fake_client::FakeCaptureEventProcessor>();
      break;
    case EventProcessorType::kVulkanLayer:
      capture_event_processor =
          std::make_unique<orbit_fake_client::GraphicsCaptureEventProcessor>();
      break;
    default:
      ORBIT_UNREACHABLE();
  }

  auto capture_outcome_future = capture_client.Capture(
      thread_pool.get(), process_id, module_manager, selected_functions, kAlwaysRecordArguments,
      kRecordReturnValues, orbit_client_data::TracepointInfoSet{}, samples_per_second,
      kStackDumpSize, unwinding_method, collect_scheduling_info, collect_thread_state,
      collect_gpu_jobs, enable_api, kEnableIntrospection, instrumentation_method,
      kMaxLocalMarkerDepthPerCommandBuffer, collect_memory_info, memory_sampling_period_ms,
      std::move(capture_event_processor));
  ORBIT_LOG("Asked to start capture");

  absl::Time start_time = absl::Now();
  while (!exit_requested && absl::Now() < start_time + absl::Seconds(duration_s)) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    ORBIT_CHECK(!capture_outcome_future.IsFinished());
  }
  ORBIT_CHECK(capture_client.StopCapture());
  ORBIT_LOG("Asked to stop capture");

  auto capture_outcome_or_error = capture_outcome_future.Get();
  if (capture_outcome_or_error.has_error()) {
    ORBIT_FATAL("Capture failed: %s", capture_outcome_or_error.error().message());
  }
  thread_pool->ShutdownAndWait();

  ORBIT_CHECK(capture_outcome_or_error.value() ==
              orbit_capture_client::CaptureListener::CaptureOutcome::kComplete);
  ORBIT_LOG("Capture completed");
  return 0;
}