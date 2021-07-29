// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Compile on Windows with
//   cl /LD /Zi dllmain.cpp
// to generate both the .dll and the .pdb file.

#include <windows.h>

#include <iostream>

void PrintHelloWorldInternal() { std::cout << "Hello World!" << std::endl; }

extern "C" __declspec(dllexport) void PrintHelloWorld() { PrintHelloWorldInternal(); }

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) { return TRUE; }
