// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientSymbols/SymbolFinder.h"

#include "OrbitBase/Logging.h"

using orbit_client_data::ModuleIdentifier;

namespace orbit_client_symbols {

std::optional<SymbolFinder::SymbolFindingResult> SymbolFinder::GetDownloadingResultByModulePath(
    const std::string& module_file_path) const {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  const auto it = symbol_files_currently_downloading_.find(module_file_path);
  if (it == symbol_files_currently_downloading_.end()) return std::nullopt;

  return it->second.future;
}

bool SymbolFinder::IsModuleDownloading(const std::string& module_file_path) const {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  return symbol_files_currently_downloading_.contains(module_file_path);
}

void SymbolFinder::StopModuleDownloading(const std::string& module_file_path) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());

  if (!symbol_files_currently_downloading_.contains(module_file_path)) return;
  symbol_files_currently_downloading_.at(module_file_path).stop_source.RequestStop();
}

void SymbolFinder::AddToCurrentlyDownloading(std::string module_file_path,
                                             ModuleDownloadOperation download_operation) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  symbol_files_currently_downloading_.emplace(std::move(module_file_path),
                                              std::move(download_operation));
}

void SymbolFinder::RemoveFromCurrentlyDownloading(const std::string& module_file_path) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  symbol_files_currently_downloading_.erase(module_file_path);
}

std::optional<SymbolFinder::SymbolFindingResult> SymbolFinder::GetRetrievingResultForModule(
    const ModuleIdentifier& module_id) const {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  const auto it = symbol_files_currently_retrieving_.find(module_id);
  if (it == symbol_files_currently_retrieving_.end()) return std::nullopt;

  return it->second;
}

void SymbolFinder::AddToCurrentlyRetrieving(ModuleIdentifier module_id,
                                            SymbolFindingResult finding_result) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  symbol_files_currently_retrieving_.emplace(std::move(module_id), std::move(finding_result));
}

void SymbolFinder::RemoveFromCurrentlyRetrieving(
    const orbit_client_data::ModuleIdentifier& module_id) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  symbol_files_currently_retrieving_.erase(module_id);
}

}  // namespace orbit_client_symbols
