// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "FramePointerValidatorClient.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <string>

#include "App.h"
#include "FunctionUtils.h"
#include "Pdb.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"

FramePointerValidatorClient::FramePointerValidatorClient(
    OrbitApp* app, std::shared_ptr<grpc::Channel> channel)
    : app_{app},
      frame_pointer_validator_service_{
          FramePointerValidatorService::NewStub(channel)} {}

void FramePointerValidatorClient::AnalyzeModules(
    const std::vector<std::shared_ptr<Module>>& modules) {
  if (modules.empty()) {
    ERROR("No module to validate, cancelling");
    return;
  }

  std::vector<std::string> dialogue_messages;
  dialogue_messages.push_back("Validation complete.");

  for (const std::shared_ptr<Module>& module : modules) {
    ValidateFramePointersRequest request;
    ValidateFramePointersResponse response;
    if (module == nullptr) continue;
    if (module->m_Pdb == nullptr) continue;

    request.set_module_path(module->m_FullName);
    for (const auto& function : module->m_Pdb->GetFunctions()) {
      CodeBlock* function_info = request.add_functions();
      function_info->set_offset(FunctionUtils::Offset(*function));
      function_info->set_size(function->size());
    }
    grpc::ClientContext context;
    std::chrono::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::minutes(1);
    context.set_deadline(deadline);

    // careful this is the synchronous call (maybe async is better)
    grpc::Status status =
        frame_pointer_validator_service_->ValidateFramePointers(
            &context, request, &response);

    if (!status.ok()) {
      return app_->SendErrorToUi(
          "Frame Pointer Validation",
          absl::StrFormat(
              "Grpc call for frame-pointer validation failed for module %s: %s",
              module->m_Name, status.error_message()));
    }
    int fpo_functions = response.functions_without_frame_pointer_size();
    int no_fpo_functions = module->m_Pdb->GetFunctions().size() - fpo_functions;
    dialogue_messages.push_back(absl::StrFormat(
        "Module %s: %d functions support frame pointers, %d functions don't.",
        module->m_Name, no_fpo_functions, fpo_functions));
  }

  std::string text = absl::StrJoin(dialogue_messages, "\n");
  app_->SendInfoToUi("Frame Pointer Validation", text);
}