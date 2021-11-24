// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FramePointerValidatorService/FramePointerValidatorServiceImpl.h"

#include <absl/strings/str_format.h>

#include <memory>
#include <optional>
#include <vector>

#include "FramePointerValidator/FramePointerValidator.h"
#include "GrpcProtos/code_block.pb.h"
#include "ObjectUtils/ElfFile.h"

namespace orbit_frame_pointer_validator_service {

using orbit_grpc_protos::CodeBlock;
using orbit_grpc_protos::ValidateFramePointersRequest;
using orbit_grpc_protos::ValidateFramePointersResponse;

grpc::Status FramePointerValidatorServiceImpl::ValidateFramePointers(
    grpc::ServerContext* /*context*/, const ValidateFramePointersRequest* request,
    ValidateFramePointersResponse* response) {
  // Even though this information should be available on the client,
  // we want not rely on this here, and for this particular use case we are
  // fine with doing some extra work, and read it from the elf file.
  auto elf_file_result = orbit_object_utils::CreateElfFile(request->module_path());

  if (elf_file_result.has_error()) {
    return grpc::Status(grpc::StatusCode::INTERNAL,
                        absl::StrFormat("Unable to load module \"%s\": %s", request->module_path(),
                                        elf_file_result.error().message()));
  }

  bool is_64_bit = elf_file_result.value()->Is64Bit();

  std::vector<CodeBlock> function_infos(request->functions().begin(), request->functions().end());

  std::optional<std::vector<CodeBlock>> functions =
      FramePointerValidator::GetFpoFunctions(function_infos, request->module_path(), is_64_bit);

  if (!functions.has_value()) {
    return grpc::Status(
        grpc::StatusCode::INTERNAL,
        absl::StrFormat("Unable to verify functions of module %s", request->module_path()));
  }

  for (const auto& function : functions.value()) {
    CodeBlock* added_function = response->add_functions_without_frame_pointer();
    added_function->set_offset(function.offset());
    added_function->set_size(function.size());
  }

  return grpc::Status::OK;
}

}  // namespace orbit_frame_pointer_validator_service
