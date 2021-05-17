// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_ANNOTATING_LINE_H_
#define CODE_REPORT_ANNOTATING_LINE_H_

#include <stdint.h>

#include <string>

namespace orbit_code_report {

// Combines metadata and contents for a line that annotates another line. The current implementation
// of Viewer inserts the annotating line above the annotated(reference) line. The association is
// done via line numbers. Both line number fields are one-indexed.
struct AnnotatingLine {
  uint64_t reference_line;
  uint64_t line_number;
  std::string line_contents;
};
}  // namespace orbit_code_report

#endif  // CODE_REPORT_ANNOTATING_LINE_H_
