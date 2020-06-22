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

  ValidateFramePointersRequest request;
  ValidateFramePointersResponse response;

  // TODO:
  request.set_is_64_bit(true);

  for (const std::shared_ptr<Module>& module : modules) {
    if (module == nullptr) continue;
    if (module->m_Pdb == nullptr) continue;

    ModuleInformation* module_info = request.add_modules();
    module_info->set_module_path(module->m_FullName);
    for (const auto& function : module->m_Pdb->GetFunctions()) {
      FunctionInformation* function_info = module_info->add_functions();
      function_info->set_offset(function->Offset());
      function_info->set_size(function->Size());
    }
  }

  if (request.modules_size() == 0) {
    return;
  }

  grpc::ClientContext context;
  // TODO(kuebler) reasonable deadline time and use constant
  std::chrono::time_point deadline =
      std::chrono::system_clock::now() + std::chrono::seconds(60);
  context.set_deadline(deadline);

  // careful this is the synchronous call (maybe async is better)
  grpc::Status status = frame_pointer_validator_service_->ValidateFramePointers(
      &context, request, &response);

  if (!status.ok()) {
    return app_->SendErrorToUi(
        "Frame Pointer Validation",
        absl::StrFormat("Grpc call for frame-pointer validation failed: %s",
                        status.error_message()));
  }

  uint64_t num_functions = 0;
  for (const auto& module : modules) {
    num_functions += module->m_Pdb->GetFunctions().size();
  }

  std::string text = absl::StrFormat(
      "Failed to validate %d out of %d functions",
      response.functions_without_frame_pointer_size(), num_functions);
  app_->SendInfoToUi("Frame Pointer Validation", text);
}