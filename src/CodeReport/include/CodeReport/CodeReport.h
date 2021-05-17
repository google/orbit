// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_CODE_REPORT_H_
#define CODE_REPORT_CODE_REPORT_H_

#include <stddef.h>
#include <stdint.h>

#include <optional>

namespace orbit_code_report {
class CodeReport {
 public:
  virtual ~CodeReport() = default;
  [[nodiscard]] virtual uint32_t GetNumSamplesInFunction() const = 0;
  [[nodiscard]] virtual uint32_t GetNumSamples() const = 0;

  // A returned empty optional means there is no data available for this line.
  [[nodiscard]] virtual std::optional<uint32_t> GetNumSamplesAtLine(size_t line) const = 0;
};
}  // namespace orbit_code_report

#endif  // CODE_REPORT_CODE_REPORT_H_
