// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTE_SYMBOL_PROVIDER_MICROSOFT_SYMBOL_SERVER_SYMBOL_PROVIDER_H_
#define REMOTE_SYMBOL_PROVIDER_MICROSOFT_SYMBOL_SERVER_SYMBOL_PROVIDER_H_

#include <memory>
#include <string>

#include "Http/DownloadManager.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "SymbolProvider/SymbolProvider.h"
#include "Symbols/SymbolCacheInterface.h"

namespace orbit_remote_symbol_provider {

class MicrosoftSymbolServerSymbolProvider : public orbit_symbol_provider::SymbolProvider {
 public:
  explicit MicrosoftSymbolServerSymbolProvider(
      const orbit_symbols::SymbolCacheInterface* symbol_cache,
      orbit_http::DownloadManager* download_manager);

  [[nodiscard]] orbit_base::Future<orbit_symbol_provider::SymbolLoadingOutcome> RetrieveSymbols(
      const orbit_symbol_provider::ModuleIdentifier& module_id,
      orbit_base::StopToken stop_token) const override;

 private:
  [[nodiscard]] static std::string GetDownloadUrl(
      const orbit_symbol_provider::ModuleIdentifier& module_id);

  const orbit_symbols::SymbolCacheInterface* symbol_cache_;
  orbit_http::DownloadManager* download_manager_;
  std::shared_ptr<orbit_base::MainThreadExecutor> main_thread_executor_;
};

}  // namespace orbit_remote_symbol_provider

#endif  // REMOTE_SYMBOL_PROVIDER_MICROSOFT_SYMBOL_SERVER_SYMBOL_PROVIDER_H_