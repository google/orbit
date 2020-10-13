// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DisassemblyReport.h"

uint32_t DisassemblyReport::GetNumSamplesAtLine(size_t line) const {
  if (function_count_ == 0) {
    return 0;
  }

  if (line >= line_to_address_.size()) {
    return 0;
  }

  const ThreadSampleData* sample_data = profiler_.GetSummary();
  if (sample_data == nullptr) {
    return 0;
  }

  const uint64_t address = line_to_address_[line];

  // On calls the address sampled might not be the address of the
  // beginning of the instruction, but instead at the end. Thus, we
  // iterate over all addresses that fall into this instruction.
  const auto next_address = [&]() {
    const auto next_line = line + 1;
    if (next_line < line_to_address_.size()) {
      return line_to_address_[next_line];
    } else {
      return address + 1;
    }
  }();

  uint32_t count = 0;
  for (auto i = address; i < next_address; ++i) {
    count += sample_data->GetCountForAddress(i);
  }

  return count;
}
