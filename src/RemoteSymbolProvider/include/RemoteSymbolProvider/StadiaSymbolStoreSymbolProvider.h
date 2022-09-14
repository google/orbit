// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTE_SYMBOL_PROVIDER_STADIA_SYMBOL_STORE_SYMBOL_PROVIDER_H_
#define REMOTE_SYMBOL_PROVIDER_STADIA_SYMBOL_STORE_SYMBOL_PROVIDER_H_

#include "Http/DownloadManager.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitGgp/Client.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "SymbolProvider/SymbolProvider.h"
#include "Symbols/SymbolCacheInterface.h"

namespace orbit_remote_symbol_provider {

class StadiaSymbolStoreSymbolProvider : public orbit_symbol_provider::SymbolProvider {
 public:
  explicit StadiaSymbolStoreSymbolProvider(orbit_symbols::SymbolCacheInterface* symbol_cache,
                                           orbit_http::DownloadManager* download_manager,
                                           orbit_ggp::Client* ggp_client);

  [[nodiscard]] orbit_base::Future<orbit_symbol_provider::SymbolLoadingOutcome> RetrieveSymbols(
      const orbit_symbol_provider::ModuleIdentifier& module_id,
      orbit_base::StopToken stop_token) const override;

 private:
  orbit_symbols::SymbolCacheInterface* symbol_cache_;
  orbit_http::DownloadManager* download_manager_;
  orbit_ggp::Client* ggp_client_;
  std::shared_ptr<orbit_base::MainThreadExecutor> main_thread_executor_;
};

}  // namespace orbit_remote_symbol_provider

#endif  // REMOTE_SYMBOL_PROVIDER_STADIA_SYMBOL_STORE_SYMBOL_PROVIDER_H_
