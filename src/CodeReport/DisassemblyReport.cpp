// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeReport/DisassemblyReport.h"

namespace orbit_code_report {

std::optional<uint32_t> DisassemblyReport::GetNumSamplesAtLine(size_t line) const {
  // The given line number will be 1-indexed, but `Disassembler` works with 0-indexed line numbers.
  line -= 1;

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
  if (!thread_sample_data_.has_value()) {
    return 0;
  }
  uint32_t count = 0;
  while (address < next_address) {
    count += thread_sample_data_->GetCountForAddress(address);
    address++;
  }
  return count;
}

std::optional<size_t> DisassemblyReport::GetLineAtAddress(uint64_t address) const {
  return disasm_.GetLineAtAddress(address);
}
}  // namespace orbit_code_report