// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "FramePointerValidatorClient.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <string>

#include "App.h"
#include "Message.h"
#include "Pdb.h"

FramePointerValidatorClient::FramePointerValidatorClient(
    OrbitApp* app, TransactionClient* transaction_client)
    : app_{app}, transaction_client_{transaction_client} {
  auto on_response = [this](const Message& msg, uint64_t id) {
    HandleResponse(msg, id);
  };

  transaction_client_->RegisterTransactionResponseHandler(
      {on_response, Msg_ValidateFramePointers, "Validate Frame Pointer"});
}

void FramePointerValidatorClient::AnalyzeModule(
    uint32_t process_id, const std::vector<std::shared_ptr<Module>>& modules) {
  if (modules.empty()) {
    ERROR("No module to validate, cancelling");
    return;
  }

  std::vector<ModuleDebugInfo> remote_module_infos;

  for (const std::shared_ptr<Module>& module : modules) {
    if (module == nullptr) continue;
    ModuleDebugInfo module_info;
    const char* name = module->m_Name.c_str();
    module_info.m_Name = name;
    module_info.m_PID = process_id;
    remote_module_infos.push_back(std::move(module_info));
  }

  if (remote_module_infos.empty()) {
    return;
  }

  uint64_t id = transaction_client_->EnqueueRequest(Msg_ValidateFramePointers,
                                                    remote_module_infos);

  absl::MutexLock lock(&id_mutex_);
  modules_map_[id] = modules;
}

void FramePointerValidatorClient::HandleResponse(const Message& message,
                                                 uint64_t id) {
  std::tuple<bool, std::vector<std::shared_ptr<Function>>> response;
  transaction_client_->ReceiveResponse(message, &response);

  id_mutex_.Lock();
  std::vector<std::shared_ptr<Module>> modules = modules_map_[id];
  modules_map_.erase(id);
  id_mutex_.Unlock();

  if (!std::get<0>(response)) {
    app_->SendErrorToUi("Frame Pointer Validation",
                        "Failed to validate frame pointers");
    return;
  }

  uint64_t num_functions = 0;
  for (const auto& module : modules) {
    num_functions += module->m_Pdb->GetFunctions().size();
  }

  std::string text =
      absl::StrFormat("Failed to validate %d out of %d functions",
                      std::get<1>(response).size(), num_functions);
  app_->SendInfoToUi("Frame Pointer Validation", text);
}
