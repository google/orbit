#include "DisassemblyReport.h"

uint32_t DisassemblyReport::GetNumSamplesAtLine(size_t line) const {
  if (function_count_ == 0) {
    return 0;
  }
  uint64_t address = disasm_.GetAddressAtLine(line);
  if (address == 0) {
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
  const ThreadSampleData* data = profiler_.GetSummary();
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
