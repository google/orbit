// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/InstrumentProcess.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <dlfcn.h>
#include <unistd.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/ExecuteInProcess.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::ModuleInfo;

// Holds all the data necessary to keep track of a process we instrument.
// Needs to be created via the static factory function `Create`. This will inject the shared library
// with out instrumentaion code into the target process and create the return trampoline. Once
// created we can instrument functions in the target process and deactiveate the instrumentation
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

  // Instruments the functions capture_options.instrumented_functions and returns a set of
  // function_id's of succefully instrumented functions.
  [[nodiscard]] ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentFunctions(
      const CaptureOptions& capture_options);

  // Removes the instrumentation for all functions in capture_options.instrumented_functions that
  // have been instrumented previously.
  [[nodiscard]] ErrorMessageOr<void> UninstrumentFunctions(const CaptureOptions& capture_options);

  // Returns the pid of the process.
  [[nodiscard]] pid_t GetPid() const { return pid_; }

 private:
  InstrumentedProcess() = default;

  // Returns an address where we can construct a new trampoline for some function in the module
  // identified by `address_range`. Handles the allocation in the tracee and the tracks the
  // allocated memory in `trampolines_for_modules_` below.
  ErrorMessageOr<uint64_t> GetTrampolineMemory(AddressRange address_range);
  // Releases the address priviously obtained by `GetTrampolineMemory` such that it can be reused.
  // Note that this must only be called once for each call to `GetTrampolineMemory`.
  ErrorMessageOr<void> ReleaseMostRecentlyAllocatedTrampolineMemory(AddressRange address_range);

  [[nodiscard]] ErrorMessageOr<void> EnsureTrampolinesWritable();
  [[nodiscard]] ErrorMessageOr<void> EnsureTrampolinesExecutable();

  ErrorMessageOr<std::vector<ModuleInfo>> ModulesFromModulePath(std::string path);

  pid_t pid_ = -1;

  uint64_t start_new_capture_function_address_ = 0;
  uint64_t entry_payload_function_address_ = 0;
  uint64_t exit_payload_function_address_ = 0;

  uint64_t return_trampoline_address_ = 0;

  // Keep track of each relocted instruction that has been moved into a trampoline. Used to move the
  // instruction pointers out of overwritten memory areas after the instrumentaion has been done.
  absl::flat_hash_map<uint64_t, uint64_t> relocation_map_;

  // Keep track of all trampolines we created for this process.
  struct TrampolineData {
    uint64_t trampoline_address;
    uint64_t address_after_prologue;
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
    TrampolineMemoryChunk(std::unique_ptr<MemoryInTracee>&& m, int first_available)
        : first_available(first_available) {
      memory = std::move(m);
    }
    std::unique_ptr<MemoryInTracee> memory;
    int first_available = 0;
  };
  typedef std::vector<TrampolineMemoryChunk> TrampolineMemoryChunks;
  absl::flat_hash_map<AddressRange, TrampolineMemoryChunks> trampolines_for_modules_;

  // Map path of a module in the process to all loaded instances of that module (usually there will
  // only be one, but a module can be loaded more than once).
  absl::flat_hash_map<std::string, std::vector<ModuleInfo>> modules_from_path_;
};

typedef absl::flat_hash_map<pid_t, std::unique_ptr<InstrumentedProcess>> ProcessMap;

ErrorMessageOr<std::filesystem::path> GetLibraryPath() {
  // When packaged, libOrbitUserSpaceInstrumentation.so is found alongside OrbitService. In
  // development, it is found in "../lib", relative to OrbitService.
  constexpr const char* kLibName = "libOrbitUserSpaceInstrumentation.so";
  const std::filesystem::path exe_dir = orbit_base::GetExecutableDir();
  std::vector<std::filesystem::path> potential_paths = {exe_dir / kLibName,
                                                        exe_dir / ".." / "lib" / kLibName};
  for (const auto& path : potential_paths) {
    if (std::filesystem::exists(path)) {
      return path;
    }
  }
  return ErrorMessage(absl::StrFormat("%s not found on system.", kLibName));
}

ErrorMessageOr<std::unique_ptr<InstrumentedProcess>> InstrumentedProcess::Create(
    const CaptureOptions& capture_options) {
  std::unique_ptr<InstrumentedProcess> process(new InstrumentedProcess());
  const pid_t pid = capture_options.pid();
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

ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentedProcess::InstrumentFunctions(
    const CaptureOptions& capture_options) {
  OUTCOME_TRY(AttachAndStopProcess(pid_));
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

  OUTCOME_TRY(EnsureTrampolinesWritable());

  absl::flat_hash_set<uint64_t> instrumented_function_ids;
  for (const auto& function : capture_options.instrumented_functions()) {
    const uint64_t function_id = function.function_id();
    constexpr uint64_t kMaxFunctionPrologueBackupSize = 20;
    const uint64_t backup_size = std::min(kMaxFunctionPrologueBackupSize, function.function_size());
    if (backup_size == 0) {
      // Can't instrument function of size zero
      continue;
    }
    // Get all modules with the right path (usually one, but might be more) and get a function
    // address to instrument for each of them.
    OUTCOME_TRY(auto&& modules, ModulesFromModulePath(function.file_path()));
    for (const auto& module : modules) {
      // TODO: there is a function for that now, right? The page alignment is not handled here...
      const uint64_t function_address =
          module.address_start() + function.file_offset() - module.executable_segment_offset();

      if (!trampoline_map_.contains(function_address)) {
        const AddressRange module_address_range(module.address_start(), module.address_end());
        auto trampoline_address_or_error = GetTrampolineMemory(module_address_range);
        if (trampoline_address_or_error.has_error()) {
          LOG("Failed to allocate memory for trampoline: %s",
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
          LOG("Failed to create trampoline: %s", address_after_prologue_or_error.error().message());
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

      auto result = InstrumentFunction(pid_, function_address, function_id,
                                       trampoline_data.address_after_prologue,
                                       trampoline_data.trampoline_address);
      if (result.has_error()) {
        LOG("Unable to instrument %s: %s", function.function_name(), result.error().message());
      } else {
        instrumented_function_ids.insert(function_id);
      }
    }
  }

  MoveInstructionPointersOutOfOverwrittenCode(pid_, relocation_map_);

  OUTCOME_TRY(EnsureTrampolinesExecutable());

  return instrumented_function_ids;
}

ErrorMessageOr<void> InstrumentedProcess::UninstrumentFunctions(
    const CaptureOptions& capture_options) {
  OUTCOME_TRY(AttachAndStopProcess(pid_));
  orbit_base::unique_resource detach_on_exit{pid_, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};

  for (const auto& function : capture_options.instrumented_functions()) {
    OUTCOME_TRY(auto&& modules, ModulesFromModulePath(function.file_path()));
    for (const auto& module : modules) {
      const uint64_t function_address =
          module.address_start() + function.file_offset() - module.executable_segment_offset();
      auto it = trampoline_map_.find(function_address);
      // Skip if this function was not instrumented.
      if (it == trampoline_map_.end()) continue;
      const TrampolineData& trampoline_data = it->second;
      std::vector<uint8_t> code(trampoline_data.function_data.begin(),
                                trampoline_data.function_data.begin() +
                                    (trampoline_data.address_after_prologue - function_address));
      auto write_result_or_error = WriteTraceesMemory(pid_, function_address, code);
      CHECK(!write_result_or_error.has_error());
    }
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
    return ErrorMessage("Tried to release trampoline memory for an non existent address range");
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
      return ErrorMessage(absl::StrFormat("Unable to find module for path %s", path));
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

// Return the singleton map containing the instrumented processes.
ProcessMap& GetInstrumentedProcesses() {
  static ProcessMap instrumented_processes;
  return instrumented_processes;
}

bool ProcessWithPidExists(pid_t pid) {
  const std::string pid_dirname = absl::StrFormat("/proc/%d", pid);
  auto result = orbit_base::FileExists(pid_dirname);
  FAIL_IF(result.has_error(), "Accessing \"%s\" failed: %s", pid_dirname, result.error().message());
  return result.value();
}

}  // namespace

ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentProcess(
    const CaptureOptions& capture_options) {
  const pid_t pid = capture_options.pid();

  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We would need to attach to / stop our own process.
  if (pid == getpid()) {
    return absl::flat_hash_set<uint64_t>();
  }

  ProcessMap& instrumented_processes = GetInstrumentedProcesses();
  if (!instrumented_processes.contains(pid)) {
    // Delete entries belonging to processes that are not running anymore.
    auto it = instrumented_processes.begin();
    while (it != instrumented_processes.end()) {
      if (!ProcessWithPidExists(it->second->GetPid())) {
        instrumented_processes.erase(it++);
      } else {
        it++;
      }
    }

    // InstrumentedProcess process;
    // auto init_result = process.Init(capture_options);
    auto process_or_error = InstrumentedProcess::Create(capture_options);
    if (process_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat("Unable to initialize process %d: %s", pid,
                                          process_or_error.error().message()));
    }
    instrumented_processes.emplace(pid, std::move(process_or_error.value()));
  }
  OUTCOME_TRY(auto&& instrumented_function_ids,
              instrumented_processes[pid]->InstrumentFunctions(capture_options));

  return std::move(instrumented_function_ids);
}

ErrorMessageOr<void> UninstrumentProcess(const CaptureOptions& capture_options) {
  const pid_t pid = capture_options.pid();

  // If the user tries to instrument this instance of OrbitService we can't use user space
  // instrumentation: We would need to attach to / stop our own process. Therefore nothing was
  // instrumented in the first place and we can just return here.
  if (pid == getpid()) {
    return outcome::success();
  }

  ProcessMap& instrumented_processes = GetInstrumentedProcesses();
  if (instrumented_processes.contains(pid)) {
    OUTCOME_TRY(instrumented_processes[pid]->UninstrumentFunctions(capture_options));
  }

  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation
