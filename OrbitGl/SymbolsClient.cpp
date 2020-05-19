// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolsClient.h"

#include "App.h"
#include "SymbolHelper.h"
#include "absl/strings/str_join.h"

SymbolsClient::SymbolsClient(OrbitApp* app,
                             TransactionClient* transaction_client)
    : app_{app}, transaction_client_{transaction_client} {
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
    if (module == nullptr || module->GetLoaded()) {
      continue;
    }

    ModuleDebugInfo module_info;
    module_info.m_Name = module->m_Name;
    module_info.m_PID = process->GetID();

    // Try to load modules from local machine.
    const SymbolHelper symbol_helper;
    if (symbol_helper.LoadSymbolsUsingSymbolsFile(module)) {
      symbol_helper.FillDebugInfoFromModule(module, &module_info);
      size_t num_functions = module_info.m_Functions.size();
      LOG("Loaded %lu function symbols locally for module: %s", num_functions,
          module->m_Name);
    } else {
      LOG("Did not find local symbols for module: %s", module->m_Name);
      remote_module_infos.push_back(std::move(module_info));
    }
  }

  // Don't request anything from the service if we found all modules locally.
  if (remote_module_infos.empty()) {
    if (session != nullptr) {
      app_->ApplySession(*session);
    }
    return;
  }

  // Send request to service for modules that were not found locally.
  uint64_t id = transaction_client_->EnqueueRequest(Msg_DebugSymbols,
                                                    remote_module_infos);

  absl::MutexLock lock(&mutex_);
  id_sessions_[id] = session;
  for (const ModuleDebugInfo& not_found_module : remote_module_infos) {
    id_not_found_module_names_[id].emplace_back(not_found_module.m_Name);
  }
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
  app_->OnRemoteModuleDebugInfo(infos);

  // Finalize transaction.
  mutex_.Lock();
  std::shared_ptr<Session> session = id_sessions_[id];
  id_sessions_.erase(id);

  std::vector<std::string> not_found_module_names =
      std::move(id_not_found_module_names_[id]);
  id_not_found_module_names_.erase(id);
  mutex_.Unlock();

  if (session != nullptr) {
    app_->ApplySession(*session);
  }

  // Show an error MessageBox for modules not found locally nor remotely.
  for (const ModuleDebugInfo& module_info : infos) {
    if (!module_info.m_Functions.empty()) {
      not_found_module_names.erase(std::find(not_found_module_names.begin(),
                                             not_found_module_names.end(),
                                             module_info.m_Name));
    }
  }
  if (!not_found_module_names.empty()) {
    std::string text = "Could not load symbols for modules:\n  " +
                       absl::StrJoin(not_found_module_names, "\n  ");
    app_->SendErrorToUi("Error loading symbols", text);
  }
}
