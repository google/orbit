// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CODE_REPORT_H_
#define ORBIT_GL_CODE_REPORT_H_

#include <cstddef>
#include <cstdint>

class CodeReport {
 public:
  virtual ~CodeReport() = default;
  [[nodiscard]] virtual uint32_t GetNumSamplesInFunction() const = 0;
  [[nodiscard]] virtual uint32_t GetNumSamples() const = 0;
  [[nodiscard]] virtual uint32_t GetNumSamplesAtLine(size_t line) const = 0;
};

#endif  // ORBIT_GL_CODE_REPORT_H_
