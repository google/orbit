// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FramePointerValidator.h"

#include <capstone/capstone.h>

#include "FunctionFramePointerValidator.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/UniqueResource.h"

std::optional<std::vector<std::shared_ptr<Function>>>
FramePointerValidator::GetFpoFunctions(
    const std::vector<std::shared_ptr<Function>>& functions,
    const std::string& file_name, bool is_64_bit) {
  std::vector<std::shared_ptr<Function>> result;

  cs_mode mode = is_64_bit ? CS_MODE_64 : CS_MODE_32;
  csh temp_handle;
  if (cs_open(CS_ARCH_X86, mode, &temp_handle) != CS_ERR_OK) {
    ERROR("Unable to open capstone.");
    return {};
  }
  OrbitBase::unique_resource handle{std::move(temp_handle),
                                    [](csh handle) { cs_close(&handle); }};

  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  std::ifstream instream(file_name, std::ios::in | std::ios::binary);
  std::vector<uint8_t> binary((std::istreambuf_iterator<char>(instream)),
                              std::istreambuf_iterator<char>());

  for (const auto& function : functions) {
    uint64_t function_size = function->Size();
    if (function_size == 0) {
      continue;
    }

    FunctionFramePointerValidator validator{
        handle, binary.data() + function->Offset(), function_size};

    if (!validator.Validate()) {
      result.push_back(function);
    }
  }
  return result;
}