// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h"

#include <absl/strings/str_replace.h>
#include <absl/strings/substitute.h>

#include <filesystem>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"
#include "QtUtils/MainThreadExecutor.h"

using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_symbol_provider::SymbolLoadingOutcome;
using orbit_symbol_provider::SymbolLoadingSuccessResult;

namespace orbit_remote_symbol_provider {

MicrosoftSymbolServerSymbolProvider::MicrosoftSymbolServerSymbolProvider(
    const orbit_symbols::SymbolCacheInterface* symbol_cache,
    orbit_http::DownloadManager* download_manager)
    : symbol_cache_(symbol_cache), download_manager_(download_manager) {
  ORBIT_CHECK(symbol_cache != nullptr);
  ORBIT_CHECK(download_manager != nullptr);
}

std::string MicrosoftSymbolServerSymbolProvider::GetDownloadUrl(
    const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id) {
  constexpr std::string_view kUrlToSymbolServer = "https://msdl.microsoft.com/download/symbols";
  std::filesystem::path module_path(module_path_and_build_id.module_path);
  std::filesystem::path symbol_filename = module_path.filename();
  symbol_filename.replace_extension(".pdb");
  std::string build_id_modified =
      absl::StrReplaceAll(module_path_and_build_id.build_id, {{"-", ""}});
  return absl::Substitute("$0/$1/$2/$1", kUrlToSymbolServer, symbol_filename.string(),
                          build_id_modified);
}

Future<SymbolLoadingOutcome> MicrosoftSymbolServerSymbolProvider::RetrieveSymbols(
    const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id,
    orbit_base::StopToken stop_token) {
  std::filesystem::path save_file_path =
      symbol_cache_->GenerateCachedFilePath(module_path_and_build_id.module_path);
  std::string url = GetDownloadUrl(module_path_and_build_id);

  return download_manager_->Download(std::move(url), save_file_path, std::move(stop_token))
      .ThenIfSuccess(
          &main_thread_executor_,
          [save_file_path = std::move(save_file_path)](
              const CanceledOr<NotFoundOr<void>>& download_result) -> SymbolLoadingOutcome {
            if (orbit_base::IsCanceled(download_result)) return {orbit_base::Canceled{}};
            if (orbit_base::IsNotFound(orbit_base::GetNotCanceled(download_result))) {
              return {orbit_base::NotFound{"Symbols not found in Microsoft symbol server"}};
            }
            return {SymbolLoadingSuccessResult{
                save_file_path, SymbolLoadingSuccessResult::SymbolSource::kMicrosoftSymbolServer,
                SymbolLoadingSuccessResult::SymbolFileSeparation::kDifferentFile}};
          });
}

}  // namespace orbit_remote_symbol_provider