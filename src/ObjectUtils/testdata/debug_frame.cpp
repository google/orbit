// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Compile on Linux with
//   clang++ debug_frame.cpp -o debug_frame -fforce-dwarf-frame
// This will cause the .debug_frame section to be generated in addition to .eh_frame.

#include <cstdio>

int main() { printf("Hello World!\n"); }
