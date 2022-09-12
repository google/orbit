// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RemoteSymbolProvider/StadiaSymbolStoreSymbolProvider.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include "OrbitBase/File.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitGgp/SymbolDownloadInfo.h"

using orbit_base::CanceledOr;
using orbit_base::Future;

namespace orbit_remote_symbol_provider {

StadiaSymbolStoreSymbolProvider::StadiaSymbolStoreSymbolProvider(
    orbit_symbols::SymbolCacheInterface* symbol_cache,
    orbit_http::DownloadManager* download_manager, orbit_ggp::Client* ggp_client)
    : symbol_cache_(symbol_cache),
      download_manager_(download_manager),
      ggp_client_(ggp_client),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
  ORBIT_CHECK(symbol_cache != nullptr);
  ORBIT_CHECK(download_manager != nullptr);
  ORBIT_CHECK(ggp_client != nullptr);
}

// TODO(b/245522908): Treat NotFound as error message now. We will better handle the NotFound case
// when changing symbol provider to return SymbolLoadingOutcome.
Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>>
StadiaSymbolStoreSymbolProvider::RetrieveSymbols(
    const orbit_symbol_provider::ModuleIdentifier& module_id, orbit_base::StopToken stop_token) {
  std::filesystem::path module_path(module_id.file_path);
  std::vector<orbit_ggp::Client::SymbolDownloadQuery> queries = {
      {module_path.filename().string(), module_id.build_id}};

  // TODO(b/245920841): Client::GetSymbolDownloadInfoAsync should support returning ErrorMessage and
  // NotFound.
  return orbit_base::UnwrapFuture(ggp_client_->GetSymbolDownloadInfoAsync(queries).ThenIfSuccess(
      main_thread_executor_.get(),
      [this, module_file_path = module_id.file_path, stop_token = std::move(stop_token)](
          const std::vector<orbit_ggp::SymbolDownloadInfo>& download_info) mutable
      -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
        if (download_info.empty()) return ErrorMessage{"No symbol download info returned."};

        std::string url = download_info.front().url.toStdString();
        std::filesystem::path save_file_path =
            symbol_cache_->GenerateCachedFilePath(module_file_path);
        return download_manager_->Download(std::move(url), save_file_path, std::move(stop_token))
            .ThenIfSuccess(
                main_thread_executor_.get(),
                [save_file_path = std::move(save_file_path)](
                    CanceledOr<orbit_base::NotFoundOr<void>> download_result)
                    -> CanceledOr<std::filesystem::path> {
                  if (orbit_base::IsCanceled(download_result)) return {orbit_base::Canceled{}};

                  // If symbol not found in Stadia symbol store, no download url will be generated.
                  ORBIT_CHECK(!orbit_base::IsNotFound(orbit_base::GetNotCanceled(download_result)));

                  return {save_file_path};
                });
      }));
}

}  // namespace orbit_remote_symbol_provider