// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_MACHINE_CODE_H_
#define USER_SPACE_INSTRUMENTATION_MACHINE_CODE_H_

#include <absl/types/span.h>

#include <cstdint>
#include <vector>

namespace orbit_user_space_instrumentation {

// Tooling to write machine code and build the corresponding vector of bytes in a structured way.
//
// Usage example:
// code.AppendBytes({0x48, 0xb8})
//     .AppendImmediate64(address)
//     .AppendBytes({0xff, 0xd0})
//     .AppendBytes({0xcc});
//
// WriteTraceesMemory(pid, code_address, code.GetResultAsVector());
//
class MachineCode {
 public:
  MachineCode& AppendBytes(absl::Span<const uint8_t> data);
  MachineCode& AppendImmediate64(uint64_t data);
  MachineCode& AppendImmediate32(uint32_t data);
  MachineCode& AppendImmediate32(int32_t data);
  MachineCode& AppendImmediate8(int8_t data);
  const std::vector<uint8_t>& GetResultAsVector() const { return data_; }

 private:
  std::vector<uint8_t> data_;
};

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_MACHINE_CODE_H_