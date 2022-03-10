// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Compile on Windows with
//   cl /LD /Zi dllmain.cpp
// to generate both the .dll and the .pdb file. Make sure you are using the
// 64-bit flavor of the compiler, for example, by using the 64-bit Visual Studio
// command prompt.

#include <windows.h>

#include <iostream>

void PrintHelloWorldInternal() { std::cout << "Hello World!" << std::endl; }

extern "C" __declspec(dllexport) void PrintHelloWorld() { PrintHelloWorldInternal(); }

class Foo {
 public:
  int a;
  int b;

  const char* Function(int input) {
    std::printf("%d\n", input);
    return "Done";
  }
};

void PrintString(const char* input) { std::puts(input); }
void TakesVolatileInt(volatile int input) { printf("%d\n", input); }
void TakesFooReference(Foo& input) { printf("%d\n", input.a); }
void TakesFooRValueReference(Foo&& input) { printf("%d\n", input.a); }
void TakesConstPtrToInt(int* const input) { printf("%d\n", *input); }
void TakesReferenceToIntPtr(int*& input) { printf("%d\n", *input); }
void TakesVoidFunctionPointer(void (*f)(int)) { f(0); }
void TakesCharFunctionPointer(char (*f)(int)) { printf("%d\n", f(0)); }
void TakesMemberFunctionPointer(const char* (Foo::*f)(int), Foo foo) { std::puts((foo.*f)(0)); }
void TakesVolatilePointerToConstUnsignedChar(const unsigned char* volatile input) {
  std::puts(reinterpret_cast<const char* volatile>(input));
}
void TakesVolatileConstPtrToVolatileConstChar(volatile const char* volatile const input) {
  std::puts((const char*)input);
}

void TakesConstPointerToConstFunctionPointer(char (*const* const f)(int)) {
  printf("%d\n", (*f)(0));
}

void TakesVariableArguments(int num, ...) {
  va_list valist;
  va_start(valist, num);
  for (size_t i = 0; i < num; i++) {
    std::printf("%d\n", va_arg(valist, int));
  }
  va_end(valist);
}

namespace A {
struct FooA {
  int a;
};
namespace B {
struct FooAB {
  double a;
};
}  // namespace B
}  // namespace A

void TakesUserTypeInNamespace(A::FooA foo_a, A::B::FooAB foo_ab) {
  std::printf("%d,%f\n", foo_a.a, foo_ab.a);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) { return TRUE; }
