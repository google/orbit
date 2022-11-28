// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_DISASSEMBLER_H_
#define CODE_REPORT_DISASSEMBLER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <stddef.h>
#include <stdint.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"

namespace orbit_code_report {
class Disassembler {
 public:
  void Disassemble(orbit_client_data::ProcessData& process,
                   orbit_client_data::ModuleManager& module_manager, const void* machine_code,
                   size_t size, uint64_t address, bool is_64bit);
  void AddLine(std::string, std::optional<uint64_t> address = std::nullopt);

  [[nodiscard]] const std::string& GetResult() const { return result_; }
  [[nodiscard]] uint64_t GetAddressAtLine(size_t line) const;
  [[nodiscard]] std::optional<size_t> GetLineAtAddress(uint64_t address) const;

 private:
  std::string result_;
  std::vector<uint64_t> line_to_address_;
  absl::flat_hash_map<uint64_t, size_t> address_to_line_;
};
}  // namespace orbit_code_report

#endif  // CODE_REPORT_DISASSEMBLER_H_
