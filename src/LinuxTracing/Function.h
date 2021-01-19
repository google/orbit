// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_FUNCTION_H_
#define LINUX_TRACING_FUNCTION_H_

#include <absl/hash/hash.h>

#include <cstdint>
#include <string>

namespace orbit_linux_tracing {
class Function {
 public:
  Function(uint64_t function_id, std::string file_path, uint64_t file_offset)
      : function_id_{function_id}, file_path_{std::move(file_path)}, file_offset_{file_offset} {}

  [[nodiscard]] uint64_t function_id() const { return function_id_; }
  [[nodiscard]] const std::string& file_path() const { return file_path_; }
  [[nodiscard]] uint64_t file_offset() const { return file_offset_; }

  bool operator==(const Function& other) {
    return (this->file_offset_ == other.file_offset_) && (this->file_path_ == other.file_path_);
  }

 private:
  uint64_t function_id_;
  std::string file_path_;
  uint64_t file_offset_;
};

template <typename H>
H AbslHashValue(H state, const Function& function) {
  return H::combine(std::move(state), function.file_path(), function.file_offset());
}
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_FUNCTION_H_
