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
  DisassemblyReport(Disassembler disasm, uint64_t function_address,
                    orbit_client_data::PostProcessedSamplingData post_processed_sampling_data,
                    uint32_t samples_count)
      : disasm_{std::move(disasm)},
        post_processed_sampling_data_{std::move(post_processed_sampling_data)},
        function_count_{post_processed_sampling_data_->GetCountOfFunction(function_address)},
        samples_count_(samples_count) {}

  explicit DisassemblyReport(Disassembler disasm)
      : disasm_{std::move(disasm)}, function_count_{0}, samples_count_{0} {};

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return function_count_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return samples_count_; }
  [[nodiscard]] std::optional<uint32_t> GetNumSamplesAtLine(size_t line) const override;

 private:
  Disassembler disasm_;
  std::optional<orbit_client_data::PostProcessedSamplingData> post_processed_sampling_data_;
  uint32_t function_count_;
  uint32_t samples_count_;
};

}  // namespace orbit_code_report

#endif  // CODE_REPORT_DISASSEMBLY_REPORT_H_
