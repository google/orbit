// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LINUX_TRACING_STACK_SLICE_H_
#define LINUX_TRACING_STACK_SLICE_H_

#include <absl/types/span.h>

#include <memory>

namespace orbit_linux_tracing {

// This class holds a copy of some stack data from collected from the target process.
// The class must have a single owner, to avoid unnecessary copies. Instead of passing pointers or
// references of this class around, `StackSliceView` should be used instead.
class StackSlice {
 public:
  StackSlice(uint64_t start_address, uint64_t size, std::unique_ptr<char[]>&& data)
      : start_address_{start_address}, size_{size}, data_{std::move(data)} {}

  [[nodiscard]] uint64_t GetStartAddress() const { return start_address_; }
  [[nodiscard]] uint64_t GetSize() const { return size_; }
  [[nodiscard]] const char* GetData() const { return data_.get(); }

 private:
  uint64_t start_address_;
  uint64_t size_;
  std::unique_ptr<char[]> data_;
};

// This class is a "view" of a `StackSlice`, which contains a bare pointer (as "absl::Span") to the
// data field of the `StackSlice` this corresponds to. The lifetime of this class is bound to the
// lifetime of the `StackSlice` it is pointing to.
class StackSliceView {
 public:
  // NOLINTNEXTLINE
  StackSliceView(const StackSlice& stack_slice)
      : start_address_{stack_slice.GetStartAddress()},
        data_{absl::MakeConstSpan(stack_slice.GetData(), stack_slice.GetSize())} {}

  StackSliceView(uint64_t start_address, uint64_t size, char* data)
      : start_address_{start_address}, data_{absl::MakeConstSpan(data, size)} {}

  [[nodiscard]] uint64_t GetStartAddress() const { return start_address_; }
  [[nodiscard]] uint64_t GetEndAddress() const { return start_address_ + data_.size(); }
  [[nodiscard]] uint64_t GetSize() const { return data_.size(); }
  [[nodiscard]] const char* GetData() const { return data_.data(); }

 private:
  uint64_t start_address_;
  absl::Span<const char> data_;
};
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_STACK_SLICE_H_