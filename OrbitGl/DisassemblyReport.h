// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DISASSEMBLY_REPORT_H_
#define ORBIT_GL_DISASSEMBLY_REPORT_H_

#include <utility>

#include "CodeReport.h"
#include "Disassembler.h"
#include "SamplingProfiler.h"

class DisassemblyReport : public CodeReport {
 public:
  DisassemblyReport(Disassembler disasm, uint64_t function_address,
                    std::shared_ptr<SamplingProfiler> profiler, uint32_t samples_count)
      : disasm_{std::move(disasm)},
        profiler_{std::move(profiler)},
        function_count_{(profiler_ == nullptr) ? 0
                                               : profiler_->GetCountOfFunction(function_address)},
        samples_count_(samples_count) {}

  explicit DisassemblyReport(Disassembler disasm)
      : disasm_{std::move(disasm)}, profiler_{nullptr}, function_count_{0}, samples_count_{0} {};

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return function_count_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return samples_count_; }
  [[nodiscard]] uint32_t GetNumSamplesAtLine(size_t line) const override;

 private:
  const Disassembler disasm_;
  const std::shared_ptr<SamplingProfiler> profiler_;
  const uint32_t function_count_;
  const uint32_t samples_count_;
};

#endif  // ORBIT_GL_DISASSEMBLY_REPORT_H_
