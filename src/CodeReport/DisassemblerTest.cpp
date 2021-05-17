
// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string_view>

#include "CodeReport/Disassembler.h"

// Compiled and verified using CompilerExplorer with the following code:
// int fib(int a) {
//    if (a == 0) return 0;
//    if (a == 1) return 1;
//    return fib(a-1) + fib(a-2);
// }
//
// Link:
// https://gcc.godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(fontScale:14,fontUsePx:'0',j:2,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:8,positionColumn:1,positionLineNumber:8,selectionStartColumn:1,selectionStartLineNumber:8,startColumn:1,startLineNumber:8),source:'%0A%0Aint+fib(int+a)+%7B%0A++++if+(a+%3D%3D+0)+return+0%3B%0A++++if+(a+%3D%3D+1)+return+1%3B%0A++++return+fib(a-1)+%2B+fib(a-2)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%232',t:'0')),header:(),k:53.77650338398414,l:'4',m:100,n:'0',o:'',s:0,t:'0'),(g:!((h:compiler,i:(compiler:g103,filters:(b:'0',binary:'0',commentOnly:'0',demangle:'1',directives:'0',execute:'1',intel:'0',libraryCode:'0',trim:'1'),fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!((name:abseil,ver:trunk)),options:'-Os+-pthread+-std%3Dc%2B%2B17',selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:2),l:'5',n:'0',o:'x86-64+gcc+10.3+(Editor+%232,+Compiler+%231)+C%2B%2B',t:'0')),header:(),k:46.22349661601587,l:'4',m:100,n:'0',o:'',s:0,t:'0')),l:'2',n:'0',o:'',t:'0')),version:4

using namespace std::string_view_literals;

constexpr std::string_view kFibonacci =
    "\x31\xc0\xc3\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x00\x55\x31\xed\x53\x89\xfb\x51"
    "\x85\xdb\x74\x14\x83\xfb\x01\x74\x0f\x8d\x7b\xff\x83\xeb\x02\xe8\xe5\xff\xff\xff\x01\xc5\xeb"
    "\xe8\x8d\x44\x1d\x00\x5a\x5b\x5d\xc3\x01\x1f\x80\x00\x00\x00\x00"sv;

constexpr std::string_view kFibonacciDisassembled =
    "Platform: X86 64 (Intel syntax)\n"  // Line 0
    "0x401020:\txor          eax, eax\n"
    "0x401022:\tret          \n"
    "0x401023:\tnop          word ptr cs:[rax + rax]\n"
    "0x40102d:\tnop          dword ptr [rax]\n"  // Line 4
    "0x401030:\tpush         rbp\n"
    "0x401031:\txor          ebp, ebp\n"
    "0x401033:\tpush         rbx\n"
    "0x401034:\tmov          ebx, edi\n"
    "0x401036:\tpush         rcx\n"
    "0x401037:\ttest         ebx, ebx\n"
    "0x401039:\tje           0x40104f\n"
    "0x40103b:\tcmp          ebx, 1\n"  // Line 12
    "0x40103e:\tje           0x40104f\n"
    "0x401040:\tlea          edi, [rbx - 1]\n"
    "0x401043:\tsub          ebx, 2\n"
    "0x401046:\tcall         0x401030\n"
    "0x40104b:\tadd          ebp, eax\n"
    "0x40104d:\tjmp          0x401037\n"
    "0x40104f:\tlea          eax, [rbp + rbx]\n"
    "0x401053:\tpop          rdx\n"
    "0x401054:\tpop          rbx\n"
    "0x401055:\tpop          rbp\n"
    "0x401056:\tret          \n"
    "0x401057:\tadd          dword ptr [rdi], ebx\n"  // Line 24
    "0x401059:\tadd          byte ptr [rax], 0\n"
    "0x40105c:\tadd          byte ptr [rax], al\n"
    "0x40105e:\n\n"sv;  // Line 27 & 28

TEST(Disassembler, Disassemble) {
  orbit_code_report::Disassembler disassembler{};
  disassembler.Disassemble(static_cast<const void*>(kFibonacci.data()), kFibonacci.size(), 0x401020,
                           true);
  EXPECT_EQ(disassembler.GetResult(), kFibonacciDisassembled);
  EXPECT_EQ(disassembler.GetAddressAtLine(0), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(4), 0x40102d);
  EXPECT_EQ(disassembler.GetAddressAtLine(12), 0x40103b);
  EXPECT_EQ(disassembler.GetAddressAtLine(24), 0x401057);
  EXPECT_EQ(disassembler.GetAddressAtLine(27), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(28), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(29), 0);  // 29 is the first invalid line index.
  EXPECT_EQ(disassembler.GetAddressAtLine(1024), 0);

  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(0x40102c).has_value());
  EXPECT_EQ(disassembler.GetLineAtAddress(0x40102d).value_or(0), 4);
  EXPECT_EQ(disassembler.GetLineAtAddress(0x40103b).value_or(0), 12);
  EXPECT_EQ(disassembler.GetLineAtAddress(0x401057).value_or(0), 24);
  EXPECT_FALSE(disassembler.GetLineAtAddress(0x4010ff).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
}