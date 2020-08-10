// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>

#include "Utils.h"
#include "absl/strings/str_format.h"

class Disassembler {
 public:
  void Disassemble(const void* machine_code, size_t size, uint64_t address,
                   bool is_64bit);
  [[nodiscard]] const std::string& GetResult() const { return result_; }
  [[nodiscard]] uint64_t GetAddressAtLine(size_t line) const;

  void LOGF(const std::string& format) {
    result_ += format;
    line_to_address_.push_back(0);
  }

  void LogHex(const uint8_t* str, size_t len);

 private:
  std::string result_;
  std::vector<uint64_t> line_to_address_;
};
