// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RemoteSymbolProvider/StadiaSymbolStoreSymbolProvider.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include "OrbitBase/File.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitGgp/SymbolDownloadInfo.h"

using orbit_base::CanceledOr;
using orbit_base::Future;

namespace orbit_remote_symbol_provider {
ErrorMessageOr<std::unique_ptr<StadiaSymbolStoreSymbolProvider>>
StadiaSymbolStoreSymbolProvider::Create(std::filesystem::path cache_directory,
                                        orbit_http::DownloadManagerInterface* download_manager,
                                        orbit_ggp::Client* ggp_client) {
  std::string error_message = "Create StadiaSymbolStoreSymbolProvider failed with error:\n";
  if (auto exists_or_error = orbit_base::FileExists(cache_directory); exists_or_error.has_error()) {
    error_message.append(
        absl::StrFormat("Cache directory not exist: %s", exists_or_error.error().message()));
    return ErrorMessage{error_message};
  }

  if (download_manager == nullptr) {
    error_message.append("Invalid DownloadManger.");
    return ErrorMessage{error_message};
  }

  if (ggp_client == nullptr) {
    error_message.append("Invalid GGP Client.");
    return ErrorMessage{error_message};
  }

  return std::unique_ptr<StadiaSymbolStoreSymbolProvider>(
      new StadiaSymbolStoreSymbolProvider(cache_directory, download_manager, ggp_client));
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>>
StadiaSymbolStoreSymbolProvider::RetrieveSymbols(
    const orbit_symbol_provider::ModuleIdentifier& module_id, orbit_base::StopToken stop_token) {
  std::filesystem::path module_path(module_id.file_path);
  std::vector<orbit_ggp::Client::SymbolDownloadQuery> queries = {
      {module_path.filename().string(), module_id.build_id}};

  return orbit_base::UnwrapFuture(ggp_client_->GetSymbolDownloadInfoAsync(queries).ThenIfSuccess(
      main_thread_executor_.get(),
      [this, module_file_path = module_id.file_path, stop_token = std::move(stop_token)](
          const std::vector<orbit_ggp::SymbolDownloadInfo>& download_info)
          -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
        if (download_info.empty()) return ErrorMessage{"No symbol download info returned."};

        std::string url = download_info.front().url.toStdString();
        std::string filename = absl::StrReplaceAll(module_file_path, {{"/", "_"}});
        std::filesystem::path save_file_path = cache_directory_ / filename;
        return download_manager_->Download(std::move(url), save_file_path, std::move(stop_token))
            .ThenIfSuccess(
                main_thread_executor_.get(),
                [save_file_path = std::move(save_file_path)](
                    CanceledOr<void> download_result) -> CanceledOr<std::filesystem::path> {
                  if (orbit_base::IsCanceled(download_result)) return {orbit_base::Canceled{}};
                  return CanceledOr<std::filesystem::path>{save_file_path};
                });
      }));
}

}  // namespace orbit_remote_symbol_provider