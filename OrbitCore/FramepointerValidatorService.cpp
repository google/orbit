// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FramepointerValidatorService.h"

#include <capstone/capstone.h>
#include <vector>

#include "FunctionFramepointerValidator.h"
#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Pdb.h"

FramepointerValidatorService::FramepointerValidatorService(
    const ProcessList* process_list, TransactionService* transaction_service)
    : process_list_{process_list}, transaction_service_{transaction_service} {
  auto on_request = [this](const Message& msg) { HandleRequest(msg); };
  transaction_service_->RegisterTransactionRequestHandler(
      {on_request, Msg_ValidateFramepointer, "Validate Framepointers"});
}

void FramepointerValidatorService::HandleRequest(const Message& message) {
  // TODO: The code below is about 90% copy&past from SymbolsService.cpp.
  //  We should discuss.
  std::vector<ModuleDebugInfo> module_infos;
  transaction_service_->ReceiveRequest(message, &module_infos);

  for (const auto& module_info : module_infos) {
    // Find process.
    uint32_t pid = module_info.m_PID;
    std::shared_ptr<Process> process = nullptr;
    process = process_list_->GetProcess(pid);
    if (process == nullptr) {
      ERROR("Unable to find process %u", pid);
      continue;
    }

    const bool is64Bit = process->GetIs64Bit();

    // Find module.
    const std::string& module_name = module_info.m_Name;
    std::shared_ptr<Module> module = process->GetModuleFromName(module_name);
    if (!module) {
      ERROR("Unable to find module %s", module_name.c_str());
      continue;
    }

    std::shared_ptr<Pdb> pdb = module->m_Pdb;
    if (!pdb) {
      ERROR("Unable to retrieve Pdb %s", module_name.c_str());
      continue;
    }

    std::vector<std::shared_ptr<Function>> functions =
        CheckFramepointers(pdb.get(), is64Bit);

    transaction_service_->SendResponse(message.GetType(), functions);
  }
}

std::vector<std::shared_ptr<Function>>
FramepointerValidatorService::CheckFramepointers(Pdb* pdb, bool is64Bit) {
  std::vector<std::shared_ptr<Function>> result;

  cs_mode mode = is64Bit ? CS_MODE_64 : CS_MODE_32;
  csh handle;
  if (cs_open(CS_ARCH_X86, mode, &handle) != CS_ERR_OK) {
    ERROR("Unable to open capstone.");
    return result;
  }

  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  std::ifstream instream(pdb->GetFileName(), std::ios::in | std::ios::binary);
  std::vector<uint8_t> binary((std::istreambuf_iterator<char>(instream)),
                              std::istreambuf_iterator<char>());

  for (std::shared_ptr<Function> function : pdb->GetFunctions()) {
    uint64_t function_size = function->Size();
    if (function_size == 0) {
      continue;
    }

    FunctionFramepointerValidator validator{
        handle, binary.data() + function->Offset(), function_size};

    if (!validator.Validate()) result.push_back(function);
  }
  return result;
}

//
