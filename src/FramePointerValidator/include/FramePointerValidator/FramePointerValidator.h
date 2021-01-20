// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FRAME_POINTER_VALIDATOR_FRAME_POINTER_VALIDATOR_H_
#define FRAME_POINTER_VALIDATOR_FRAME_POINTER_VALIDATOR_H_

#include <filesystem>
#include <optional>
#include <vector>

#include "code_block.pb.h"

class FramePointerValidator {
 public:
  // Checks all given functions if they were compiled with frame pointers and
  // returns the functions, where validation failed. If there was an error
  // during validation, nullopt will be return.
  static std::optional<std::vector<orbit_grpc_protos::CodeBlock>> GetFpoFunctions(
      const std::vector<orbit_grpc_protos::CodeBlock>& functions,
      const std::filesystem::path& file_name, bool is_64_bit);
};

#endif  // FRAME_POINTER_VALIDATOR_FRAME_POINTER_VALIDATOR_H_
