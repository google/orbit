// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentationTestLib.h"

#include <stdio.h>

int TrivialFunction() { return 42; }

uint64_t TrivialSum(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5) {
  return p0 + p1 + p2 + p3 + p4 + p5;
}

void TrivialLog(uint64_t function_address) { printf("Called function at %#lx", function_address); }
