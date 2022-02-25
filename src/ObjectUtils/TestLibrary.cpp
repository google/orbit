// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file will NOT be compiled as part of Orbit 's build system. It' s meant to generate the
// testdata. Check out `testdata/Makefile` on how to compile it.

#include <cstdio>

extern "C" struct Foo {
  int a;
  int b;

  const char* Function(int input) {
    std::printf("%d\n", input);
    return "Done";
  }
};

extern "C" void PrintHelloWorld() { std::puts("Hello World!\n"); }

extern "C" void PrintString(const char* input) { std::puts(input); }
extern "C" void TakesVolatileInt(volatile int input) { printf("%d\n", input); }
extern "C" void TakesFooReference(Foo& input) { printf("%d\n", input.a); }
extern "C" void TakesFooRValueReference(Foo&& input) { printf("%d\n", input.a); }
extern "C" void TakesConstPtrToInt(int* const input) { printf("%d\n", *input); }
extern "C" void TakesReferenceToIntPtr(int*& input) { printf("%d\n", *input); }
extern "C" void TakesVoidFunctionPointer(void (*f)(int)) { f(0); }
extern "C" void TakesCharFunctionPointer(char (*f)(int)) { printf("%d\n", f(0)); }
extern "C" void TakesMemberFunctionPointer(const char* (Foo::*f)(int), Foo foo) {
  std::puts((foo.*f)(0));
}
extern "C" void TakesVolatilePointerToConstUnsignedChar(const unsigned char* volatile input) {
  std::puts(reinterpret_cast<const char* volatile>(input));
}
extern "C" void TakesVolatileConstPtrToVolatileConstChar(
    volatile const char* volatile const input) {
  std::puts((const char*)input);
}