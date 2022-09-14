// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RemoteSymbolProvider/StadiaSymbolStoreSymbolProvider.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>

#include "OrbitBase/NotFoundOr.h"
#include "OrbitGgp/SymbolDownloadInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"

using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_ggp::SymbolDownloadInfo;
using orbit_symbol_provider::SymbolLoadingOutcome;
using orbit_symbol_provider::SymbolLoadingSuccessResult;

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

Future<SymbolLoadingOutcome> StadiaSymbolStoreSymbolProvider::RetrieveSymbols(
    const orbit_symbol_provider::ModuleIdentifier& module_id,
    orbit_base::StopToken stop_token) const {
  std::filesystem::path module_path(module_id.file_path);
  auto call_ggp_future = ggp_client_->GetSymbolDownloadInfoAsync(
      {module_path.filename().string(), module_id.build_id});

  return orbit_base::UnwrapFuture(call_ggp_future.ThenIfSuccess(
      main_thread_executor_.get(),
      [this, module_file_path = module_id.file_path, stop_token = std::move(stop_token)](
          const NotFoundOr<SymbolDownloadInfo>& call_ggp_result) mutable
      -> Future<SymbolLoadingOutcome> {
        if (orbit_base::IsNotFound(call_ggp_result)) {
          return {orbit_base::NotFound{"Symbols not found in Stadia symbol store"}};
        }

        SymbolDownloadInfo download_info = orbit_base::GetFound(call_ggp_result);
        std::string url = download_info.url.toStdString();
        std::filesystem::path save_file_path =
            symbol_cache_->GenerateCachedFilePath(module_file_path);
        return download_manager_->Download(std::move(url), save_file_path, std::move(stop_token))
            .ThenIfSuccess(
                main_thread_executor_.get(),
                [save_file_path = std::move(save_file_path)](
                    CanceledOr<NotFoundOr<void>> download_result) -> SymbolLoadingOutcome {
                  if (orbit_base::IsCanceled(download_result)) return {orbit_base::Canceled{}};

                  // If symbol not found in Stadia symbol store, no download url will be generated.
                  ORBIT_CHECK(!orbit_base::IsNotFound(orbit_base::GetNotCanceled(download_result)));

                  return {SymbolLoadingSuccessResult{
                      save_file_path, SymbolLoadingSuccessResult::SymbolSource::kStadiaSymbolStore,
                      SymbolLoadingSuccessResult::SymbolFileSeparation::kDifferentFile}};
                });
      }));
}

}  // namespace orbit_remote_symbol_provider