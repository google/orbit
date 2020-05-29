/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ORBIT_LINUX_TRACING_FUNCTION_H_
#define ORBIT_LINUX_TRACING_FUNCTION_H_

#include <cstdint>
#include <string>

namespace LinuxTracing {
class Function {
 public:
  Function(std::string binary_path, uint64_t file_offset,
           uint64_t virtual_address)
      : binary_path_{std::move(binary_path)},
        file_offset_{file_offset},
        virtual_address_{virtual_address} {}

  const std::string& BinaryPath() const { return binary_path_; }

  uint64_t FileOffset() const { return file_offset_; }

  uint64_t VirtualAddress() const { return virtual_address_; }

 private:
  std::string binary_path_;
  uint64_t file_offset_;
  uint64_t virtual_address_;
};
}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_FUNCTION_H_
