// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LINUX_TRACING_STACK_SLICE_H_
#define LINUX_TRACING_STACK_SLICE_H_

#include <memory>

struct StackSlice {
  uint64_t start_address;
  uint64_t size;
  std::unique_ptr<char[]> data;
};

#endif  // LINUX_TRACING_STACK_SLICE_H_
