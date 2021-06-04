// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_CAPTURE_FILE_SECTION_H_
#define CAPTURE_FILE_CAPTURE_FILE_SECTION_H_

#include <cstdint>

namespace orbit_capture_file {

constexpr uint64_t kSectionTypeUserData = 1;

struct CaptureFileSection {
  uint64_t type;
  uint64_t offset;
  uint64_t size;
};

}  // namespace orbit_capture_file
#endif  // CAPTURE_FILE_CAPTURE_FILE_SECTION_H_
