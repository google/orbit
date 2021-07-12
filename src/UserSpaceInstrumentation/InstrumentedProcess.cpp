// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "InstrumentedProcess.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <capstone/capstone.h>
#include <dlfcn.h>

#include <filesystem>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::ModuleInfo;

ErrorMessageOr<std::filesystem::path> GetLibraryPath() {
  // When packaged, libUserSpaceInstrumentation.so is found alongside OrbitService.  In development,
  // it is found in "../lib", relative to OrbitService.
  constexpr const char* kLibInjectUserSpaceInstrumentation = "libInjectUserSpaceInstrumentation.so";
  const std::filesystem::path exe_dir = orbit_base::GetExecutableDir();
  std::vector<std::filesystem::path> potential_paths = {
      exe_dir / kLibInjectUserSpaceInstrumentation,
      exe_dir / ".." / "lib" / kLibInjectUserSpaceInstrumentation};
  for (const auto& path : potential_paths) {
    if (std::filesystem::exists(path)) {
      return path;
    }
  }
  return ErrorMessage("libInjectUserSpaceInstrumentation.so not found on system.");
}

void DumpDissasembly(csh handle, const std::vector<u_int8_t>& code, uint64_t start_address) {
  cs_insn* insn = nullptr;
  size_t count = cs_disasm(handle, static_cast<const uint8_t*>(code.data()), code.size(),
                           start_address, 0, &insn);
  size_t j;
  for (j = 0; j < count; j++) {
    std::string machine_code;
    for (int k = 0; k < insn[j].size; k++) {
      machine_code =
          absl::StrCat(machine_code, k == 0 ? absl::StrFormat("%#0.2x", insn[j].bytes[k])
                                            : absl::StrFormat(" %0.2x", insn[j].bytes[k]));
    }
    LOG("0x%llx:\t%-12s %s , %s", insn[j].address, insn[j].mnemonic, insn[j].op_str, machine_code);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", insn[j - 1].address + insn[j - 1].size);
  cs_free(insn, count);
}

}  // namespace

ErrorMessageOr<void> InstrumentedProcess::Init(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  // Attach to process, inject library and get the addresses of the payload functions.
  pid_ = capture_options.pid();
  OUTCOME_TRY(AttachAndStopProcess(pid_));
  orbit_base::unique_resource detach_on_exit{pid_, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};

  OUTCOME_TRY(library_path, GetLibraryPath());
  OUTCOME_TRY(library_handle, DlopenInTracee(pid_, library_path, RTLD_NOW));

  constexpr const char* kEntryPayloadFunctionName = "EntryPayload";
  constexpr const char* kExitPayloadFunctionName = "ExitPayload";
  OUTCOME_TRY(entry_payload_function_address,
              DlsymInTracee(pid_, library_handle, kEntryPayloadFunctionName));
  entry_payload_function_address_ = absl::bit_cast<uint64_t>(entry_payload_function_address);
  OUTCOME_TRY(exit_payload_function_address,
              DlsymInTracee(pid_, library_handle, kExitPayloadFunctionName));
  exit_payload_function_address_ = absl::bit_cast<uint64_t>(exit_payload_function_address);

  // Get memory and create the return trampoline.
  OUTCOME_TRY(return_trampoline_address, AllocateInTracee(pid_, 0, GetReturnTrampolineSize()));
  return_trampoline_address_ = return_trampoline_address;
  auto result =
      CreateReturnTrampoline(pid_, exit_payload_function_address_, return_trampoline_address_);

  return outcome::success();
}

ErrorMessageOr<absl::flat_hash_set<uint64_t>> InstrumentedProcess::InstrumentFunctions(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
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

  absl::flat_hash_set<uint64_t> instrumented_function_ids;

  for (const auto& function : capture_options.instrumented_functions()) {
    if (function.function_type() != InstrumentedFunction::kRegular) continue;
    const uint64_t function_id = function.function_id();
    constexpr uint64_t kMaxFunctionPrologueBackupSize = 20;
    const uint64_t backup_size = std::min(kMaxFunctionPrologueBackupSize, function.function_size());
    if (backup_size == 0) {
      LOG("Can't instrument function \"%s\" of size zero.", function.function_name());
      continue;
    }
    // Get all modules with the right path (usually one, but might be more) and get a function
    // address to instrument for each of them.
    OUTCOME_TRY(modules, ModulesFromModulePath(function.file_path()));
    for (const auto& module : modules) {
      const uint64_t function_address =
          module.address_start() + function.file_offset() - module.executable_segment_offset();
      if (!trampoline_map_.contains(function_address)) {
        const AddressRange module_address_range(module.address_start(), module.address_end());
        OUTCOME_TRY(trampoline_address, GetTrampolineMemory(module_address_range));
        TrampolineData trampoline_data;
        trampoline_data.trampoline_address = trampoline_address;
        OUTCOME_TRY(function_data, ReadTraceesMemory(pid_, function_address, backup_size));
        trampoline_data.function_data = function_data;
        auto address_after_prologue_or_error =
            CreateTrampoline(pid_, function_address, function_data, trampoline_address,
                             entry_payload_function_address_, return_trampoline_address_,
                             capstone_handle, relocation_map_);
        if (address_after_prologue_or_error.has_error()) {
          // TODO: Should this be in Orbit log? I'd like to know how often this happens and it would
          // be great to see the reasons.
          LOG("Failed to create trampoline: %s", address_after_prologue_or_error.error().message());
          OUTCOME_TRY(ReleaseMostRecentTrampolineMemory(module_address_range));
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

      LOG("original function adress: %#x", function_address);
      auto orig_function = ReadTraceesMemory(pid_, function_address, 100);
      DumpDissasembly(capstone_handle, orig_function.value(), function_address);

      auto result = InstrumentFunction(pid_, function_address, function_id,
                                       trampoline_data.address_after_prologue,
                                       trampoline_data.trampoline_address);

      LOG("overwritten_function ");
      auto overwritten_function = ReadTraceesMemory(pid_, function_address, 100);
      DumpDissasembly(capstone_handle, overwritten_function.value(), function_address);
      LOG("trampoline ");
      auto trampoline_function = ReadTraceesMemory(pid_, trampoline_data.trampoline_address, 200);
      DumpDissasembly(capstone_handle, trampoline_function.value(),
                      trampoline_data.trampoline_address);

      // TODO: I'm unsure about the reporting. It might be useful for the user to know that some of
      // the functions will be instrumented with uprobes (which is fine but slower).
      if (result.has_error()) {
        LOG("Unable to instrument %s: %s", function.function_name(), result.error().message());
      } else {
        instrumented_function_ids.insert(function_id);
      }
    }
  }
  MoveInstructionPointersOutOfOverwrittenCode(pid_, relocation_map_);
  return instrumented_function_ids;
}

ErrorMessageOr<void> InstrumentedProcess::UninstrumentFunctions(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  OUTCOME_TRY(AttachAndStopProcess(pid_));
  orbit_base::unique_resource detach_on_exit{pid_, [](int32_t pid) {
                                               if (DetachAndContinueProcess(pid).has_error()) {
                                                 ERROR("Detaching from %i", pid);
                                               }
                                             }};

  for (const auto& function : capture_options.instrumented_functions()) {
    if (function.function_type() != InstrumentedFunction::kRegular) continue;

    OUTCOME_TRY(modules, ModulesFromModulePath(function.file_path()));
    for (const auto& module : modules) {
      const uint64_t function_address =
          module.address_start() + function.file_offset() - module.executable_segment_offset();
      auto it = trampoline_map_.find(function_address);
      // Skip if this function was not instrumented.
      if (it == trampoline_map_.end()) continue;
      const TrampolineData& trampoline_data = it->second;
      // TODO: Only store the bytes we need in function_data?
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
    OUTCOME_TRY(trampoline_memory,
                AllocateMemoryForTrampolines(pid_, address_range,
                                             kTrampolinesPerChunk * GetMaxTrampolineSize()));

    trampoline_memory_chunks.push_back({trampoline_memory, 0});
  }
  const uint64_t result = trampoline_memory_chunks.back().first_available * GetMaxTrampolineSize() +
                          trampoline_memory_chunks.back().address;
  trampoline_memory_chunks.back().first_available++;
  return result;
}

ErrorMessageOr<void> InstrumentedProcess::ReleaseMostRecentTrampolineMemory(
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
    OUTCOME_TRY(modules, orbit_object_utils::ReadModules(pid_));
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

}  // namespace orbit_user_space_instrumentation