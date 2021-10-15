// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsProcessService/ProcessServiceImpl.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <stdint.h>

#include <filesystem>
#include <string>
#include <vector>

#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListProcesses.h"
#include "WindowsUtils/ReadProcessMemory.h"
#include "module.pb.h"
#include "process.pb.h"

namespace orbit_windows_process_service {

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using orbit_grpc_protos::GetDebugInfoFileRequest;
using orbit_grpc_protos::GetDebugInfoFileResponse;
using orbit_grpc_protos::GetModuleListRequest;
using orbit_grpc_protos::GetModuleListResponse;
using orbit_grpc_protos::GetProcessListRequest;
using orbit_grpc_protos::GetProcessListResponse;
using orbit_grpc_protos::GetProcessMemoryRequest;
using orbit_grpc_protos::GetProcessMemoryResponse;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

using orbit_windows_utils::Module;
using orbit_windows_utils::Process;

using orbit_object_utils::CreateObjectFile;
using orbit_object_utils::CreateSymbolsFile;
using orbit_object_utils::ObjectFile;
using orbit_object_utils::SymbolsFile;

namespace fs = std::filesystem;

Status ProcessServiceImpl::GetProcessList(ServerContext*, const GetProcessListRequest*,
                                          GetProcessListResponse* response) {
  const std::vector<Process> processes = orbit_windows_utils::ListProcesses();
  if (processes.empty()) {
    return Status(StatusCode::NOT_FOUND, "Error listing processes");
  }

  for (const Process& process : processes) {
    ProcessInfo* process_info = response->add_processes();
    process_info->set_pid(process.pid);
    process_info->set_name(process.name);
    process_info->set_full_path(process.full_path);
    process_info->set_build_id(process.build_id);
    process_info->set_is_64_bit(process.is_64_bit);
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetModuleList(ServerContext* /*context*/,
                                         const GetModuleListRequest* request,
                                         GetModuleListResponse* response) {
  std::vector<Module> modules = orbit_windows_utils::ListModules(request->process_id());
  if (modules.empty()) {
    return Status(StatusCode::NOT_FOUND, "Error listing modules");
  }

  for (const Module& module : modules) {
    ModuleInfo* module_info = response->add_modules();
    module_info->set_name(module.name);
    module_info->set_file_path(module.full_path);
    module_info->set_address_start(module.address_start);
    module_info->set_address_end(module.address_end);
    module_info->set_build_id(module.build_id);
  }

  return Status::OK;
}

Status ProcessServiceImpl::GetProcessMemory(ServerContext*, const GetProcessMemoryRequest* request,
                                            GetProcessMemoryResponse* response) {
  const uint64_t size = std::min(request->size(), kMaxGetProcessMemoryResponseSize);
  ErrorMessageOr<std::string> result =
      orbit_windows_utils::ReadProcessMemory(request->pid(), request->address(), size);

  if (result.has_error()) {
    return Status(StatusCode::PERMISSION_DENIED, result.error().message());
  }

  *response->mutable_memory() = std::move(result.value());
  return Status::OK;
}

static [[nodiscard]] ErrorMessageOr<fs::path> FindSymbolsFilePath(
    const orbit_grpc_protos::GetDebugInfoFileRequest& request) {
  const fs::path module_path{request.module_path()};

  // Create object file for the module.
  OUTCOME_TRY(std::unique_ptr<ObjectFile> object_file, CreateObjectFile(module_path));

  // If module does not contain a build id, no searching for separate symbol files can be done
  if (object_file->GetBuildId().empty()) {
    return ErrorMessage{
        absl::StrFormat("Module \"%s\" does not contain symbols and does not contain a build id, "
                        "therefore Orbit cannot search for a separate symbols file",
                        module_path.string())};
  }

  // Coff is currently the only supported format for the Windows service.
  CHECK(object_file->IsCoff());

  const std::string& build_id = object_file->GetBuildId();

  // Search in module's directory and in the user provided additional directories.
  std::vector<fs::path> search_directories{object_file->GetFilePath().parent_path()};
  const auto& additional_search_directories = request.additional_search_directories();
  search_directories.insert(search_directories.end(), additional_search_directories.begin(),
                            additional_search_directories.end());

  // Files with the following formats are considered: "module.sym_ext" and "module.mod_ext.sym_ext".
  // "mod_ext" is the module file extension, usually ".exe" or ".dll"; "sym_ext" is ".pdb".
  const std::string sym_ext = ".pdb";
  const fs::path& filename = module_path.filename();
  fs::path filename_dot_sym_ext = filename;
  filename_dot_sym_ext.replace_extension(sym_ext);
  fs::path filename_plus_sym_ext = filename;
  filename_plus_sym_ext.replace_extension(filename.extension().string() + sym_ext);

  std::set<fs::path> search_paths;
  for (const auto& directory : search_directories) {
    search_paths.insert(directory / filename_dot_sym_ext);
    search_paths.insert(directory / filename_plus_sym_ext);
  }

  std::vector<std::string> error_messages;

  for (const auto& search_path : search_paths) {
    ErrorMessageOr<bool> file_exists = orbit_base::FileExists(search_path);

    if (file_exists.has_error()) {
      ERROR("%s", file_exists.error().message());
      error_messages.emplace_back(file_exists.error().message());
      continue;
    }

    if (!file_exists.value()) {
      // no error message when file simply does not exist
      continue;
    }

    orbit_object_utils::ObjectFileInfo object_file_info{object_file->GetLoadBias(),
                                                        object_file->GetExecutableSegmentOffset()};

    ErrorMessageOr<std::unique_ptr<SymbolsFile>> symbols_file_or_error =
        CreateSymbolsFile(search_path, object_file_info);

    if (symbols_file_or_error.has_error()) {
      error_messages.push_back(symbols_file_or_error.error().message());
      continue;
    }
    const std::unique_ptr<SymbolsFile>& symbols_file{symbols_file_or_error.value()};

    if (symbols_file->GetBuildId().empty()) {
      error_messages.push_back(absl::StrFormat(
          "Potential symbols file \"%s\" does not have a build id.", search_path.string()));
      continue;
    }

    if (symbols_file->GetBuildId() != build_id) {
      error_messages.push_back(absl::StrFormat(
          "Potential symbols file \"%s\" has a different build id than the module requested by "
          "the client. \"%s\" != \"%s\"",
          search_path.string(), symbols_file->GetBuildId(), build_id));
      continue;
    }

    return search_path;
  }

  std::string error_message_for_client{absl::StrFormat(
      "Unable to find debug symbols on the instance for module \"%s\".", module_path.string())};
  if (!error_messages.empty()) {
    absl::StrAppend(&error_message_for_client, "\nDetails:\n* ",
                    absl::StrJoin(error_messages, "\n* "));
  }

  return ErrorMessage(error_message_for_client);
}

Status ProcessServiceImpl::GetDebugInfoFile(ServerContext*, const GetDebugInfoFileRequest* request,
                                            GetDebugInfoFileResponse* response) {
  CHECK(request != nullptr);
  const ErrorMessageOr<fs::path> symbols_path = FindSymbolsFilePath(*request);
  if (symbols_path.has_error()) {
    return Status(StatusCode::NOT_FOUND, symbols_path.error().message());
  }

  response->set_debug_info_file_path(symbols_path.value().string());
  return Status::OK;
}

}  // namespace orbit_windows_process_service
