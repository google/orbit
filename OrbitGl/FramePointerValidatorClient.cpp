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

  uint64_t num_fpo_functions = 0;

  for (const std::shared_ptr<Module>& module : modules) {
    ValidateFramePointersRequest request;
    ValidateFramePointersResponse response;
    if (module == nullptr) continue;
    if (module->m_Pdb == nullptr) continue;

    request.set_module_path(module->m_FullName);
    for (const auto& function : module->m_Pdb->GetFunctions()) {
      CodeBlock* function_info = request.add_functions();
      function_info->set_offset(function->Offset());
      function_info->set_size(function->Size());
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
    num_fpo_functions += response.functions_without_frame_pointer_size();
  }

  uint64_t num_functions = 0;
  for (const auto& module : modules) {
    num_functions += module->m_Pdb->GetFunctions().size();
  }

  std::string text = absl::StrFormat(
      "Validation complete.\n%d functions support frame pointers, %d functions "
      "don't.",
      num_functions - num_fpo_functions, num_fpo_functions);
  app_->SendInfoToUi("Frame Pointer Validation", text);
}