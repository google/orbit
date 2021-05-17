// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_DISASSEMBLER_H_
#define CODE_REPORT_DISASSEMBLER_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "absl/strings/str_format.h"

namespace orbit_code_report {
class Disassembler {
 public:
  void Disassemble(const void* machine_code, size_t size, uint64_t address, bool is_64bit);
  void AddLine(std::string, uint64_t address = 0);
  [[nodiscard]] const std::string& GetResult() const { return result_; }
  [[nodiscard]] uint64_t GetAddressAtLine(size_t line) const;

 private:
  std::string result_;
  std::vector<uint64_t> line_to_address_;
};
}  // namespace orbit_code_report

#endif  // CODE_REPORT_DISASSEMBLER_H_
