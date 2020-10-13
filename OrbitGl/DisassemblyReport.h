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
  DisassemblyReport(std::vector<uint64_t> line_to_address, uint64_t function_address,
                    SamplingProfiler profiler, uint32_t samples_count)
      : line_to_address_(std::move(line_to_address)),
        profiler_{std::move(profiler)},
        function_count_{profiler_.GetCountOfFunction(function_address)},
        samples_count_(samples_count) {}

  explicit DisassemblyReport(std::vector<uint64_t> line_to_address)
      : line_to_address_{std::move(line_to_address)}, function_count_{0}, samples_count_{0} {};

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return function_count_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return samples_count_; }
  [[nodiscard]] uint32_t GetNumSamplesAtLine(size_t line) const override;

  [[nodiscard]] bool empty() const { return samples_count_ == 0; }

 private:
  std::vector<uint64_t> line_to_address_;
  SamplingProfiler profiler_;
  uint32_t function_count_;
  uint32_t samples_count_;
};

#endif  // ORBIT_GL_DISASSEMBLY_REPORT_H_
