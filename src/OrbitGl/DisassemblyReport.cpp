// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DisassemblyReport.h"

std::optional<uint32_t> DisassemblyReport::GetNumSamplesAtLine(size_t line) const {
  uint64_t address = disasm_.GetAddressAtLine(line);
  if (address == 0) {
    // We return an empty optional when there is no data available for the current line.
    // That allows the user to differentiate between a line without samples and a line without data.
    return std::nullopt;
  }

  if (function_count_ == 0) {
    return 0;
  }

  // On calls the address sampled might not be the address of the
  // beginning of the instruction, but instead at the end. Thus, we
  // iterate over all addresses that fall into this instruction.
  uint64_t next_address = disasm_.GetAddressAtLine(line + 1);

  // If the current instruction is the last one (next address is 0), it
  // can not be a call, thus we can only consider this address.
  if (next_address == 0) {
    next_address = address + 1;
  }
  if (!post_processed_sampling_data_.has_value()) {
    return 0.0;
  }
  const ThreadSampleData* data = post_processed_sampling_data_->GetSummary();
  if (data == nullptr) {
    return 0.0;
  }
  uint32_t count = 0;
  while (address < next_address) {
    count += data->GetCountForAddress(address);
    address++;
  }
  return count;
}
