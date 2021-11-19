// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/InstrumentProcess.h"

#include <absl/base/casts.h>
#include <dlfcn.h>
#include <unistd.h>

#include <filesystem>
#include <mutex>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ObjectUtils/Address.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/UniqueResource.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/AddressRange.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ModuleInfo;

/* copybara:insert(In internal tests the library path depends on the current path)
// Since some part could potentially change the current working directory we store
// the initial value here.
const std::filesystem::path initial_current_path = std::filesystem::current_path();
*/

ErrorMessageOr<std::filesystem::path> GetLibraryPath() {
  // When packaged, liborbituserspaceinstrumentation.so is found alongside OrbitService. In
  // development, it is found in "../lib", relative to OrbitService.
  constexpr const char* kLibName = "liborbituserspaceinstrumentation.so";
  const std::filesystem::path exe_dir = orbit_base::GetExecutableDir();
  std::vector<std::filesystem::path> potential_paths = {exe_dir / kLibName,
                                                        exe_dir / ".." / "lib" / kLibName};
  /* copybara:insert(In internal tests the library is in a different place)
  potential_paths.emplace_back(initial_current_path /
  "@@LIB_ORBIT_USER_SPACE_INSTRUMENTATION_PATH@@");
  */
  for (const auto& path : potential_paths) {
    if (std::filesystem::exists(path)) {
      return path;
    }
  }
  return ErrorMessage(absl::StrFormat("%s not found on system.", kLibName));
}

bool ProcessWithPidExists(pid_t pid) {
  const std::string pid_dirname = absl::StrFormat("/proc/%d", pid);
  auto result = orbit_base::FileExists(pid_dirname);
  FAIL_IF(result.has_error(), "Accessing \"%s\" failed: %s", pid_dirname, result.error().message());
  return result.value();
}

}  // namespace

// Holds all the data necessary to keep track of a process we instrument.
// Needs to be created via the static factory function `Create`. This will inject the shared library
// with our instrumentaion code into the target process and create the return trampoline. Once
// created we can instrument functions in the target process and deactivate the instrumentation
// again (see `InstrumentFunctions`, `UninstrumentFunctions` below).
class InstrumentedProcess {
 public:
  InstrumentedProcess(const InstrumentedProcess&) = delete;
  InstrumentedProcess(InstrumentedProcess&&) = delete;
  InstrumentedProcess& operator=(const InstrumentedProcess&) = delete;
  InstrumentedProcess& operator=(InstrumentedProcess&&) = delete;
  ~InstrumentedProcess() = default;

  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<InstrumentedProcess>> Create(
      const CaptureOptions& capture_options);

  // Instruments the functions capture_options.instrumented_functions. Returns a set of
  // function_id's of successfully instrumented functions, a map of function_id's to errors for
  // functions that couldn't be instrumented, the address ranges dedicated to trampolines, and the
  // map name of the injected library.
  [[nodiscard]] ErrorMessageOr<InstrumentationManager::InstrumentationResult> InstrumentFunctions(
      const CaptureOptions& capture_options);

  // Removes the instrumentation for all functions in capture_options.instrumented_functions that
  // have been instrumented previously.
  [[nodiscard]] ErrorMessageOr<void> UninstrumentFunctions();

  // Returns the pid of the process.
  [[nodiscard]] pid_t GetPid() const { return pid_; }

 private:
  InstrumentedProcess() = default;

  // Returns an address where we can construct a new trampoline for some function in the module
  // identified by `address_range`. Handles the allocation in the tracee and the tracks the
  // allocated memory in `trampolines_for_modules_` below.
  [[nodiscard]] ErrorMessageOr<uint64_t> GetTrampolineMemory(AddressRange address_range);
  // Releases the address previously obtained by `GetTrampolineMemory` such that it can be reused.
  // Note that this must only be called once for each call to `GetTrampolineMemory`.
  [[nodiscard]] ErrorMessageOr<void> ReleaseMostRecentlyAllocatedTrampolineMemory(
      AddressRange address_range);

  [[nodiscard]] ErrorMessageOr<void> EnsureTrampolinesWritable();
  [[nodiscard]] ErrorMessageOr<void> EnsureTrampolinesExecutable();

  [[nodiscard]] ErrorMessageOr<std::vector<ModuleInfo>> ModulesFromModulePath(std::string path);

  // Returns a vector of the address ranges dedicated to all entry trampolines for this process. The
  // number of address ranges is usually very small as kTrampolinesPerChunk is high.
  [[nodiscard]] std::vector<AddressRange> GetEntryTrampolineAddressRanges();

  pid_t pid_ = -1;

  uint64_t start_new_capture_function_address_ = 0;
  uint64_t entry_payload_function_address_ = 0;
  uint64_t exit_payload_function_address_ = 0;

  uint64_t return_trampoline_address_ = 0;

  // Keep track of each relocated instruction that has been moved into a trampoline. Used to move
  // the instruction pointers out of overwritten memory areas after the instrumentation has been
  // done.
  absl::flat_hash_map<uint64_t, uint64_t> relocation_map_;

  // Keep track of all trampolines we created for this process.
  struct TrampolineData {
    uint64_t trampoline_address;
    uint64_t address_after_prologue;
    // The first few bytes of the function. Guaranteed to contain everything that was overwritten.
    std::vector<uint8_t> function_data;
  };
  // Maps function addresses to TrampolineData.
  absl::flat_hash_map<uint64_t, TrampolineData> trampoline_map_;

  // Trampolines are allocated in chunks of kTrampolinesPerChunk. Trampolines are fixed size
  // (compare `GetMaxTrampolineSize`) and are never freed; we just allocate new chunks when that
  // last one is filled up. Each module (identified by its address range) gets it own sequence of
  // chunks (`trampolines_for_modules_`).
  static constexpr int kTrampolinesPerChunk = 4096;
  struct TrampolineMemoryChunk {
    TrampolineMemoryChunk() = default;
    TrampolineMemoryChunk(std::unique_ptr<MemoryInTracee> m, int first_available)
        : memory(std::move(m)), first_available(first_available) {}
    std::unique_ptr<MemoryInTracee> memory;
    int first_available = 0;
  };
  using TrampolineMemoryChunks = std::vector<TrampolineMemoryChunk>;
  absl::flat_hash_map<AddressRange, TrampolineMemoryChunks> trampolines_for_modules_;

  // Map path of a module in the process to all loaded instances of that module (usually there will
  // only be one, but a module can be loaded more than once).
  absl::flat_hash_map<std::string, std::vector<ModuleInfo>> modules_from_path_;

  // When instrumenting a function we record the address here. This is used when we uninstrument: we
  // look up the original bytes in `trampoline_map_` above.
  absl::flat_hash_set<uint64_t> addresses_of_instrumented_functions_;

  // The absolute canonical path to the library injected into the target process. This path should
  // appear in the maps of the target process.
  std::filesystem::path injected_library_path_;
};

ErrorMessageOr<std::unique_ptr<InstrumentedProcess>> InstrumentedProcess::Create(
    const CaptureOptions& capture_options) {
  std::unique_ptr<InstrumentedProcess> process(new InstrumentedProcess());
  const pid_t pid = orbit_base::ToNativeProcessId(capture_options.pid());
  process->pid_ = pid;
  OUTCOME_TRY(AttachAndStopProcess(pid));
  orbit_base::unique_resource detach_on_exit{pid, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};

  // Inject library into target process.
  auto library_path_or_error = GetLibraryPath();
  if (library_path_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to get path to library: %s",
                                        library_path_or_error.error().message()));
  }
  const std::filesystem::path library_path = library_path_or_error.value();
  CHECK(library_path.is_absolute());
  process->injected_library_path_ = std::filesystem::canonical(library_path);

  auto library_handle_or_error = DlopenInTracee(pid, library_path, RTLD_NOW);
  if (library_handle_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to open library in tracee: %s",
                                        library_handle_or_error.error().message()));
  }
  void* library_handle = library_handle_or_error.value();

  // Get function pointers into the injected library.
  constexpr const char* kStartNewCaptureFunctionName = "StartNewCapture";
  constexpr const char* kEntryPayloadFunctionName = "EntryPayload";
  constexpr const char* kExitPayloadFunctionName = "ExitPayload";
  OUTCOME_TRY(auto&& start_new_capture_function_address,
              DlsymInTracee(pid, library_handle, kStartNewCaptureFunctionName));
  process->start_new_capture_function_address_ =
      absl::bit_cast<uint64_t>(start_new_capture_function_address);
  OUTCOME_TRY(auto&& entry_payload_function_address,
              DlsymInTracee(pid, library_handle, kEntryPayloadFunctionName));
  process->entry_payload_function_address_ =
      absl::bit_cast<uint64_t>(entry_payload_function_address);
  OUTCOME_TRY(auto&& exit_payload_function_address,
              DlsymInTracee(pid, library_handle, kExitPayloadFunctionName));
  process->exit_payload_function_address_ = absl::bit_cast<uint64_t>(exit_payload_function_address);

  // Get memory, create the return trampoline and make it executable.
  OUTCOME_TRY(auto&& return_trampoline_memory,
              MemoryInTracee::Create(pid, 0, GetReturnTrampolineSize()));
  process->return_trampoline_address_ = return_trampoline_memory->GetAddress();
  auto result = CreateReturnTrampoline(pid, process->exit_payload_function_address_,
                                       process->return_trampoline_address_);
  OUTCOME_TRY(return_trampoline_memory->EnsureMemoryExecutable());

  return process;
}

ErrorMessageOr<InstrumentationManager::InstrumentationResult>
InstrumentedProcess::InstrumentFunctions(const CaptureOptions& capture_options) {
  OUTCOME_TRY(auto&& already_attached_tids, AttachAndStopProcess(pid_));
  orbit_base::unique_resource detach_on_exit{pid_, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};
  // Init Capstone disassembler.
  csh capstone_handle = 0;
  cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
  if (error_code != CS_ERR_OK) {
    return ErrorMessage("Failed to open Capstone disassembler.");
  }
  error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
  if (error_code != CS_ERR_OK) {
    return ErrorMessage("Failed to configure Capstone disassembler.");
  }
  orbit_base::unique_resource close_on_exit{
      &capstone_handle, [](csh* capstone_handle) { cs_close(capstone_handle); }};

  OUTCOME_TRY(ExecuteInProcess(pid_, absl::bit_cast<void*>(start_new_capture_function_address_)));
  // StartNewFunction could (and will) spawn new threads. Stop those too, as the assumption here is
  // that the target process is completely stopped.
  OUTCOME_TRY(AttachAndStopNewThreadsOfProcess(pid_, std::move(already_attached_tids)));

  OUTCOME_TRY(EnsureTrampolinesWritable());

  InstrumentationManager::InstrumentationResult result;
  for (const auto& function : capture_options.instrumented_functions()) {
    const uint64_t function_id = function.function_id();
    constexpr uint64_t kMaxFunctionPrologueBackupSize = 20;
    const uint64_t backup_size = std::min(kMaxFunctionPrologueBackupSize, function.function_size());
    if (backup_size == 0) {
      const std::string message = absl::StrFormat(
          "Can't instrument function \"%s\" since it has size zero.", function.function_name());
      ERROR("%s", message);
      result.function_ids_to_error_messages[function_id] = message;
      continue;
    }
    // Get all modules with the right path (usually one, but might be more) and get a function
    // address to instrument for each of them.
    OUTCOME_TRY(auto&& modules, ModulesFromModulePath(function.file_path()));
    for (const auto& module : modules) {
      const uint64_t function_address = orbit_object_utils::SymbolOffsetToAbsoluteAddress(
          function.file_offset(), module.address_start(), module.executable_segment_offset());
      if (!trampoline_map_.contains(function_address)) {
        const AddressRange module_address_range(module.address_start(), module.address_end());
        auto trampoline_address_or_error = GetTrampolineMemory(module_address_range);
        if (trampoline_address_or_error.has_error()) {
          ERROR("Failed to allocate memory for trampoline: %s",
                trampoline_address_or_error.error().message());
          continue;
        }
        const uint64_t trampoline_address = trampoline_address_or_error.value();
        TrampolineData trampoline_data;
        trampoline_data.trampoline_address = trampoline_address;
        OUTCOME_TRY(auto&& function_data, ReadTraceesMemory(pid_, function_address, backup_size));
        trampoline_data.function_data = function_data;
        auto address_after_prologue_or_error =
            CreateTrampoline(pid_, function_address, function_data, trampoline_address,
                             entry_payload_function_address_, return_trampoline_address_,
                             capstone_handle, relocation_map_);
        if (address_after_prologue_or_error.has_error()) {
          const std::string message = absl::StrFormat(
              "Can't instrument function \"%s\". Failed to create trampoline: %s",
              function.function_name(), address_after_prologue_or_error.error().message());
          ERROR("%s", message);
          result.function_ids_to_error_messages[function_id] = message;
          OUTCOME_TRY(ReleaseMostRecentlyAllocatedTrampolineMemory(module_address_range));
          continue;
        }
        trampoline_data.address_after_prologue = address_after_prologue_or_error.value();
        trampoline_map_.emplace(function_address, trampoline_data);
      }
      auto it = trampoline_map_.find(function_address);
      if (it == trampoline_map_.end()) {
        continue;
      }
      const TrampolineData& trampoline_data = it->second;

      auto result_or_error = InstrumentFunction(pid_, function_address, function_id,
                                                trampoline_data.address_after_prologue,
                                                trampoline_data.trampoline_address);
      if (result_or_error.has_error()) {
        const std::string message =
            absl::StrFormat("Can't instrument function \"%s\": %s", function.function_name(),
                            result_or_error.error().message());
        ERROR("%s", message);
        result.function_ids_to_error_messages[function_id] = message;
      } else {
        addresses_of_instrumented_functions_.insert(function_address);
        result.instrumented_function_ids.insert(function_id);
      }
    }
  }

  result.entry_trampoline_address_ranges = GetEntryTrampolineAddressRanges();
  result.return_trampoline_address_range = AddressRange{
      return_trampoline_address_, return_trampoline_address_ + GetReturnTrampolineSize()};
  result.injected_library_path = injected_library_path_;

  MoveInstructionPointersOutOfOverwrittenCode(pid_, relocation_map_);

  OUTCOME_TRY(EnsureTrampolinesExecutable());

  return result;
}

ErrorMessageOr<void> InstrumentedProcess::UninstrumentFunctions() {
  OUTCOME_TRY(AttachAndStopProcess(pid_));
  orbit_base::unique_resource detach_on_exit{pid_, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};
  for (uint64_t function_address : addresses_of_instrumented_functions_) {
    auto it = trampoline_map_.find(function_address);
    // Skip if this function was not instrumented.
    if (it == trampoline_map_.end()) continue;
    const TrampolineData& trampoline_data = it->second;
    std::vector<uint8_t> code(trampoline_data.function_data.begin(),
                              trampoline_data.function_data.begin() +
                                  (trampoline_data.address_after_prologue - function_address));
    auto write_result_or_error = WriteTraceesMemory(pid_, function_address, code);
    FAIL_IF(write_result_or_error.has_error(), "%s", write_result_or_error.error().message());
  }
  return outcome::success();
}

ErrorMessageOr<uint64_t> InstrumentedProcess::GetTrampolineMemory(AddressRange address_range) {
  if (!trampolines_for_modules_.contains(address_range)) {
    trampolines_for_modules_.emplace(address_range, TrampolineMemoryChunks());
  }
  auto it = trampolines_for_modules_.find(address_range);
  TrampolineMemoryChunks& trampoline_memory_chunks = it->second;
  if (trampoline_memory_chunks.empty() ||
      trampoline_memory_chunks.back().first_available == kTrampolinesPerChunk) {
    OUTCOME_TRY(auto&& trampoline_memory,
                AllocateMemoryForTrampolines(pid_, address_range,
                                             kTrampolinesPerChunk * GetMaxTrampolineSize()));

    trampoline_memory_chunks.emplace_back(std::move(trampoline_memory), 0);
  }
  const uint64_t result = trampoline_memory_chunks.back().memory->GetAddress() +
                          trampoline_memory_chunks.back().first_available * GetMaxTrampolineSize();
  trampoline_memory_chunks.back().first_available++;
  return result;
}

ErrorMessageOr<void> InstrumentedProcess::ReleaseMostRecentlyAllocatedTrampolineMemory(
    AddressRange address_range) {
  if (!trampolines_for_modules_.contains(address_range)) {
    return ErrorMessage("Tried to release trampoline memory for a non existent address range");
  }
  trampolines_for_modules_.find(address_range)->second.back().first_available--;
  return outcome::success();
}

ErrorMessageOr<std::vector<ModuleInfo>> InstrumentedProcess::ModulesFromModulePath(
    std::string path) {
  if (!modules_from_path_.contains(path)) {
    std::vector<ModuleInfo> result;
    OUTCOME_TRY(auto&& modules, orbit_object_utils::ReadModules(pid_));
    for (const ModuleInfo& module : modules) {
      if (module.file_path() == path) {
        result.push_back(module);
      }
    }
    if (result.empty()) {
      return ErrorMessage(absl::StrFormat("Unable to find module for path \"%s\"", path));
    }
    modules_from_path_.emplace(path, result);
  }
  return modules_from_path_.find(path)->second;
}

ErrorMessageOr<void> InstrumentedProcess::EnsureTrampolinesWritable() {
  for (auto& trampoline_for_module : trampolines_for_modules_) {
    for (auto& memory_chunk : trampoline_for_module.second) {
      OUTCOME_TRY(memory_chunk.memory->EnsureMemoryWritable());
    }
  }
  return outcome::success();
}

ErrorMessageOr<void> InstrumentedProcess::EnsureTrampolinesExecutable() {
  for (auto& trampoline_for_module : trampolines_for_modules_) {
    for (auto& memory_chunk : trampoline_for_module.second) {
      OUTCOME_TRY(memory_chunk.memory->EnsureMemoryExecutable());
    }
  }
  return outcome::success();
}

std::vector<AddressRange> InstrumentedProcess::GetEntryTrampolineAddressRanges() {
  std::vector<AddressRange> address_ranges;
  for (const auto& [unused_module_address_range, trampoline_memory_chunks] :
       trampolines_for_modules_) {
    for (const TrampolineMemoryChunk& trampoline_memory_chunk : trampoline_memory_chunks) {
      address_ranges.emplace_back(
          trampoline_memory_chunk.memory->GetAddress(),
          trampoline_memory_chunk.memory->GetAddress() + trampoline_memory_chunk.memory->GetSize());
    }
  }
  return address_ranges;
}

std::unique_ptr<InstrumentationManager> InstrumentationManager::Create() {
  static std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  static bool first_call = true;
  FAIL_IF(!first_call, "InstrumentationManager should be globally unique.");
  first_call = false;
  return std::unique_ptr<InstrumentationManager>(new InstrumentationManager());
}

InstrumentationManager::~InstrumentationManager() = default;

ErrorMessageOr<InstrumentationManager::InstrumentationResult>
InstrumentationManager::InstrumentProcess(const CaptureOptions& capture_options) {
  const pid_t pid = orbit_base::ToNativeProcessId(capture_options.pid());

  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We would need to attach to / stop our own process.
  if (pid == getpid()) {
    return InstrumentationResult();
  }

  if (!process_map_.contains(pid)) {
    // Delete entries belonging to processes that are not running anymore.
    auto it = process_map_.begin();
    while (it != process_map_.end()) {
      if (!ProcessWithPidExists(it->second->GetPid())) {
        process_map_.erase(it++);
      } else {
        it++;
      }
    }

    auto process_or_error = InstrumentedProcess::Create(capture_options);
    if (process_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat("Unable to initialize process %d: %s", pid,
                                          process_or_error.error().message()));
    }
    process_map_.emplace(pid, std::move(process_or_error.value()));
  }
  OUTCOME_TRY(auto&& instrumentation_result,
              process_map_[pid]->InstrumentFunctions(capture_options));

  return std::move(instrumentation_result);
}

ErrorMessageOr<void> InstrumentationManager::UninstrumentProcess(pid_t pid) {
  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We would need to attach to / stop our own process. Therefore nothing was
  // instrumented in the first place and we can just return here.
  if (pid == getpid()) {
    return outcome::success();
  }

  if (process_map_.contains(pid)) {
    OUTCOME_TRY(process_map_[pid]->UninstrumentFunctions());
  }

  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation
