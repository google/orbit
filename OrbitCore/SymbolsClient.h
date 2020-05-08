// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SYMBOLS_CLIENT_H_
#define ORBIT_CORE_SYMBOLS_CLIENT_H_

#include "OrbitProcess.h"
#include "TransactionClient.h"

class CoreApp;

class SymbolsClient {
 public:
  SymbolsClient(CoreApp* core_app, TransactionClient* transaction_client);

  SymbolsClient() = delete;
  SymbolsClient(const SymbolsClient&) = delete;
  SymbolsClient& operator=(const SymbolsClient&) = delete;
  SymbolsClient(SymbolsClient&&) = delete;
  SymbolsClient& operator=(SymbolsClient&&) = delete;

  void LoadSymbolsFromModules(
      Process* process, const std::vector<std::shared_ptr<Module>>& modules,
      const std::shared_ptr<Session>& session);
  void LoadSymbolsFromSession(Process* process,
                              const std::shared_ptr<Session>& session);

 private:
  void HandleResponse(const Message& message, uint64_t id);

  CoreApp* core_app_ = nullptr;
  TransactionClient* transaction_client_;
  absl::flat_hash_map<uint64_t, std::shared_ptr<Session>> id_sessions_;
  absl::flat_hash_map<uint64_t, std::vector<std::string>>
      id_not_found_module_names_;
  absl::Mutex mutex_;
};

#endif  // ORBIT_CORE_SYMBOLS_CLIENT_H_
