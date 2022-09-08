// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Compile on Windows with
//   cl no_export_table.cpp
// Make sure you are using the 64-bit flavor of the compiler,
// for example, by using the 64-bit Visual Studio command prompt.

#include <iostream>

int main() { std::cout << "Hello World!" << std::endl; }
