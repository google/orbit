// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h"

#include <absl/strings/substitute.h>

using orbit_base::CanceledOr;
using orbit_base::Future;

namespace orbit_remote_symbol_provider {

MicrosoftSymbolServerSymbolProvider::MicrosoftSymbolServerSymbolProvider(
    orbit_symbols::SymbolCacheInterface* symbol_cache,
    orbit_http::DownloadManager* download_manager)
    : symbol_cache_(symbol_cache),
      download_manager_(download_manager),
      main_thread_executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
  ORBIT_CHECK(symbol_cache != nullptr);
  ORBIT_CHECK(download_manager != nullptr);
}

std::string MicrosoftSymbolServerSymbolProvider::GetDownloadUrl(
    const orbit_symbol_provider::ModuleIdentifier& module_id) const {
  constexpr std::string_view kUrlToSymbolServer = "https://msdl.microsoft.com/download/symbols";
  std::filesystem::path module_path(module_id.file_path);
  return absl::Substitute("$0/$1/$2/$1", kUrlToSymbolServer, module_path.filename().string(),
                          module_id.build_id);
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>>
MicrosoftSymbolServerSymbolProvider::RetrieveSymbols(
    const orbit_symbol_provider::ModuleIdentifier& module_id, orbit_base::StopToken stop_token) {
  std::filesystem::path save_file_path = symbol_cache_->GenerateCachedFilePath(module_id.file_path);
  std::string url = GetDownloadUrl(module_id);

  return download_manager_->Download(std::move(url), save_file_path, std::move(stop_token))
      .ThenIfSuccess(main_thread_executor_.get(),
                     [save_file_path = std::move(save_file_path)](
                         CanceledOr<void> download_result) -> CanceledOr<std::filesystem::path> {
                       if (orbit_base::IsCanceled(download_result)) return {orbit_base::Canceled{}};
                       return CanceledOr<std::filesystem::path>{save_file_path};
                     });
}

}  // namespace orbit_remote_symbol_provider