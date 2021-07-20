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
  Function(uint64_t function_id, std::string file_path, uint64_t file_offset, bool record_arguments,
           bool record_return_value)
      : function_id_{function_id},
        file_path_{std::move(file_path)},
        file_offset_{file_offset},
        record_arguments_{record_arguments},
        record_return_value_{record_return_value} {}

  [[nodiscard]] uint64_t function_id() const { return function_id_; }
  [[nodiscard]] const std::string& file_path() const { return file_path_; }
  [[nodiscard]] uint64_t file_offset() const { return file_offset_; }
  [[nodiscard]] bool record_arguments() const { return record_arguments_; }
  [[nodiscard]] bool record_return_value() const { return record_return_value_; }

 private:
  uint64_t function_id_;
  std::string file_path_;
  uint64_t file_offset_;
  bool record_arguments_;
  bool record_return_value_;
};

inline bool operator==(const Function& lhs, const Function& rhs) {
  return (lhs.file_path() == rhs.file_path()) && (lhs.file_offset() == rhs.file_offset());
}

template <typename H>
H AbslHashValue(H state, const Function& function) {
  return H::combine(std::move(state), function.file_path(), function.file_offset());
}
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_FUNCTION_H_
