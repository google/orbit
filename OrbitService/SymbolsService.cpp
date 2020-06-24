// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolsService.h"

#include "OrbitProcess.h"
#include "SymbolHelper.h"

SymbolsService::SymbolsService(const ProcessList* process_list,
                               TransactionService* transaction_service)
    : process_list_{process_list}, transaction_service_{transaction_service} {
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  transaction_service_->RegisterTransactionRequestHandler(
      {on_request, Msg_DebugSymbols, "Debug Symbols"});
}

void SymbolsService::HandleRequest(const Message& message) {
  // Deserialize request message.
  std::vector<ModuleDebugInfo> module_infos;
  transaction_service_->ReceiveRequest(message, &module_infos);

  for (auto& module_info : module_infos) {
    // Find process.
    int32_t pid = module_info.m_PID;
    std::shared_ptr<Process> process = nullptr;
    process = process_list_->GetProcess(pid);
    if (process == nullptr) {
      ERROR("Unable to find process %d", pid);
      continue;
    }

    // Make sure modules are loaded.
    process->ListModules();

    // Find module.
    const std::string& module_name = module_info.m_Name;
    std::shared_ptr<Module> module = process->GetModuleFromName(module_name);
    if (!module) {
      ERROR("Unable to find module %s", module_name.c_str());
      continue;
    }

    // Load debug information.
    const SymbolHelper symbol_helper;
    if (symbol_helper.LoadSymbolsCollector(module)) {
      symbol_helper.FillDebugInfoFromModule(module, &module_info);
      LOG("Loaded %lu function symbols for module %s",
          module_info.m_Functions.size(), module_name.c_str());
    } else {
      ERROR("Unable to load symbols of module %s", module->m_Name.c_str());
    }
  }

  // Send response to the client.
  transaction_service_->SendResponse(message.GetType(), module_infos);
}
