// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <cstdlib>

#include "FuzzingUtils/Fuzzer.h"
#include "ObjectUtils/ElfFile.h"

ORBIT_DEFINE_FUZZER(uint8_t* buf, size_t len) {
  (void)orbit_object_utils::CreateElfFileFromBuffer("INMEMORY", buf, len);
  return 0;
}