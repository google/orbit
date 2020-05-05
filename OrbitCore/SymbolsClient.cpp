// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolsClient.h"

#include "CoreApp.h"
#include "SymbolHelper.h"

SymbolsClient::SymbolsClient(CoreApp* core_app,
                             TransactionClient* transaction_client)
    : core_app_{core_app}, transaction_client_{transaction_client} {
  auto on_response = [this](const Message& msg, uint64_t id) {
    HandleResponse(msg, id);
  };
  transaction_client_->RegisterTransactionResponseHandler(
      {on_response, Msg_DebugSymbols, "Debug Symbols"});
}

void SymbolsClient::LoadSymbolsFromModules(
    Process* process, const std::vector<std::shared_ptr<Module>>& modules,
    const std::shared_ptr<Session>& session) {
  if (modules.empty()) {
    ERROR("No module to load, cancelling");
    return;
  }

  std::vector<ModuleDebugInfo> remote_module_infos;

  for (const std::shared_ptr<Module>& module : modules) {
    if (module == nullptr) continue;
    ModuleDebugInfo module_info;
    const char* name = module->m_Name.c_str();
    module_info.m_Name = name;
    module_info.m_PID = process->GetID();

    // Try to load modules from local machine.
    const SymbolHelper symbol_helper;
    if (symbol_helper.LoadSymbolsUsingSymbolsFile(module)) {
      symbol_helper.FillDebugInfoFromModule(module, module_info);
      size_t num_functions = module_info.m_Functions.size();
      LOG("Loaded %lu function symbols locally for %s", num_functions, name);
    } else {
      LOG("Did not find local symbols for module %s", name);
      remote_module_infos.push_back(std::move(module_info));
    }
  }

  // Don't request anything from service if we found all modules locally.
  if (remote_module_infos.empty()) {
    if (session != nullptr) {
      core_app_->ApplySession(*session);
    }
    return;
  }

  // Send request to service for modules that were not found locally.
  uint64_t id = transaction_client_->EnqueueRequest(Msg_DebugSymbols,
                                                    remote_module_infos);

  absl::MutexLock lock(&id_sessions_mutex_);
  id_sessions_[id] = session;
}

void SymbolsClient::LoadSymbolsFromSession(
    Process* process, const std::shared_ptr<Session>& session) {
  std::vector<std::shared_ptr<Module>> modules;
  for (auto& pair : session->m_Modules) {
    const std::string& file_name = Path::GetFileName(pair.first);
    std::shared_ptr<Module> module = process->GetModuleFromName(file_name);
    if (module != nullptr) {
      modules.emplace_back(module);
    }
  }
  LoadSymbolsFromModules(process, modules, session);
}

void SymbolsClient::HandleResponse(const Message& message, uint64_t id) {
  // Deserialize response message.
  std::vector<ModuleDebugInfo> infos;
  transaction_client_->ReceiveResponse(message, &infos);

  // Notify app of new debug symbols.
  core_app_->OnRemoteModuleDebugInfo(infos);

  // Finalize transaction.
  id_sessions_mutex_.Lock();
  std::shared_ptr<Session> session = id_sessions_[id];
  id_sessions_.erase(id);
  id_sessions_mutex_.Unlock();
  if (session != nullptr) {
    core_app_->ApplySession(*session);
  }
}
