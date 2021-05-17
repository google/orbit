// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file will NOT be compiled as part of Orbit 's build system. It' s meant to generate the
// testdata. Check out `testdata/Makefile` on how to compile it.

#include <cstdio>

extern "C" void PrintHelloWorld() { std::puts("Hello World!\n"); }
