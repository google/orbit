// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <cstdlib>

#include "ElfUtils/ElfFile.h"

extern "C" int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  (void)orbit_elf_utils::ElfFile::CreateFromBuffer("INMEMORY", buf, len);
  return 0;
}