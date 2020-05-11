// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FramePointerValidatorService.h"

#include <vector>

#include "FramePointerValidator.h"
#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Pdb.h"

FramePointerValidatorService::FramePointerValidatorService(
    const ProcessList* process_list, TransactionService* transaction_service)
    : process_list_{process_list}, transaction_service_{transaction_service} {
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  transaction_service_->RegisterTransactionRequestHandler(
      {on_request, Msg_ValidateFramePointers, "Validate Frame Pointers"});
}

void FramePointerValidatorService::HandleRequest(const Message& message) {
  // TODO(kuebler): The code below is about 90% copy&past from
  //  SymbolsService.cpp. We should refactor the shared code with the other
  //  service.
  std::vector<ModuleDebugInfo> module_infos;
  transaction_service_->ReceiveRequest(message, &module_infos);

  for (const auto& module_info : module_infos) {
    // Find process.
    uint32_t pid = module_info.m_PID;
    std::shared_ptr<Process> process = nullptr;
    process = process_list_->GetProcess(pid);
    if (process == nullptr) {
      ERROR("Unable to find process with pid %u", pid);
      continue;
    }

    const bool is_64_bit = process->GetIs64Bit();

    // Find module.
    const std::string& module_name = module_info.m_Name;
    std::shared_ptr<Module> module = process->GetModuleFromName(module_name);
    if (!module) {
      ERROR("Unable to find module \"%s\"", module_name);
      continue;
    }

    std::shared_ptr<Pdb> pdb = module->m_Pdb;
    if (!pdb) {
      ERROR("Unable to retrieve debug information \"%s\"", module_name);
      continue;
    }

    std::vector<std::shared_ptr<Function>> functions =
        FramePointerValidator::GetFpoFunctions(pdb->GetFunctions(),
                                               module->m_FullName, is_64_bit);

    transaction_service_->SendResponse(message.GetType(), functions);
  }
}

//
