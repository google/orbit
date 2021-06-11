// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_DISASSEMBLY_REPORT_H_
#define CODE_REPORT_DISASSEMBLY_REPORT_H_

#include <stddef.h>
#include <stdint.h>

#include <optional>
#include <utility>

#include "ClientData/PostProcessedSamplingData.h"
#include "CodeReport/CodeReport.h"
#include "CodeReport/Disassembler.h"

namespace orbit_code_report {
class DisassemblyReport : public orbit_code_report::CodeReport {
 public:
  DisassemblyReport(Disassembler disasm, uint64_t absolute_function_address,
                    orbit_client_data::ThreadSampleData thread_sample_data, uint32_t function_count,
                    uint32_t samples_count)
      : disasm_{std::move(disasm)},
        thread_sample_data_{std::move(thread_sample_data)},
        function_count_{function_count},
        samples_count_(samples_count),
        absolute_function_address_{absolute_function_address} {}

  explicit DisassemblyReport(Disassembler disasm, uint64_t absolute_function_address)
      : disasm_{std::move(disasm)},
        function_count_{0},
        samples_count_{0},
        absolute_function_address_{absolute_function_address} {};

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return function_count_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return samples_count_; }
  [[nodiscard]] std::optional<uint32_t> GetNumSamplesAtLine(size_t line) const override;

  [[nodiscard]] std::optional<size_t> GetLineAtAddress(uint64_t address) const;

  [[nodiscard]] uint64_t GetAbsoluteFunctionAddress() const { return absolute_function_address_; }

 private:
  Disassembler disasm_;
  std::optional<orbit_client_data::ThreadSampleData> thread_sample_data_;
  uint32_t function_count_;
  uint32_t samples_count_;
  uint64_t absolute_function_address_;
};

}  // namespace orbit_code_report

#endif  // CODE_REPORT_DISASSEMBLY_REPORT_H_
