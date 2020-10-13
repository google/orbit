// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DISASSEMBLER_H_
#define ORBIT_GL_DISASSEMBLER_H_

#include <optional>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"

namespace orbit_gl {

enum class Architecture { kX86, kX86_64 };
struct CodeSegmentView {
  const void* machine_code;
  size_t size;
  uint64_t starting_address;
  Architecture architecture;
};

struct DisassembledCode {
  Architecture architecture;
  std::string code_segment_title;
  std::string disassembled_code;
  std::vector<uint64_t> line_to_address;
};

ErrorMessageOr<DisassembledCode> Disassemble(CodeSegmentView code, std::string code_segment_title);
}  // namespace orbit_gl

#endif  // ORBIT_GL_DISASSEMBLER_H_