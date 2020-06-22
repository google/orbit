// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FramePointerValidatorServiceImpl.h"

#include <vector>

#include "ElfFile.h"
#include "FramePointerValidator.h"
#include "OrbitFunction.h"

grpc::Status FramePointerValidatorServiceImpl::ValidateFramePointers(
    grpc::ServerContext*, const ValidateFramePointersRequest* request,
    ValidateFramePointersResponse* response) {
  for (const auto& module_info : request->modules()) {
    // Even though this information should be available on the client,
    // we want not rely on this here, and for this particular use case we are
    // fine with doing some extra work, and compute it here.
    bool is_64_bit = ElfFile::Create(module_info.module_path())->Is64Bit();

    std::vector<FunctionInfo> function_infos;

    std::transform(
        module_info.functions().begin(), module_info.functions().end(),
        std::back_inserter(function_infos),
        [](const FunctionInformation& f) -> FunctionInfo {
          return FunctionInfo{.offset = f.offset(), .size = f.size()};
        });

    std::optional<std::vector<FunctionInfo>> functions =
        FramePointerValidator::GetFpoFunctions(
            function_infos, module_info.module_path(), is_64_bit);

    if (!functions.has_value()) {
      return grpc::Status(
          grpc::StatusCode::INTERNAL,
          absl::StrFormat("Unable to verify functions of module %s",
                          module_info.module_path()));
    }

    for (const auto& function : functions.value()) {
      FunctionInformation* added_function =
          response->add_functions_without_frame_pointer();
      added_function->set_offset(function.offset);
      added_function->set_size(function.size);
    }
  }

  return grpc::Status::OK;
}

//
