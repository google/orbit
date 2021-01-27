// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_CONSTANTS_H_
#define CAPTURE_FILE_CONSTANTS_H_

#include <stddef.h>
#include <stdint.h>

#include <array>

// We want the signature length to be divisible by sizeof(uint32_t) to have
// following fields aligned to 4 as well.
constexpr size_t kFileSignatureSize = 4;
constexpr const char* kFileSignature = "ORBT";

constexpr uint32_t kFileVersion = 1;

#endif  // CAPTURE_FILE_CONSTANTS_H_
