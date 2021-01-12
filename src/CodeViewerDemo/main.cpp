// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>

#include "CodeViewer/Dialog.h"
#include "SyntaxHighlighter/X86Assembly.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};

  orbit_code_viewer::Dialog dialog{};

  // Example assembly as it's produced by capstone.
  const QString content = R"(Platform: X86 64 (Intel syntax)
0x557e1f8091d0:	push         rbp
0x557e1f8091d1:	mov          rbp, rsp
0x557e1f8091d4:	sub          rsp, 0x10
0x557e1f8091d8:	mov          rax, qword ptr fs:[0x28]
0x557e1f8091e1:	mov          qword ptr [rbp - 8], rax
0x557e1f8091e5:	xorps        xmm0, xmm0
0x557e1f8091e8:	movups       xmmword ptr [rdi], xmm0
0x557e1f8091eb:	mov          qword ptr [rdi + 0x10], 0
0x557e1f8091f3:	mov          byte ptr [rdi], 0x2a
0x557e1f8091f6:	movabs       rax, 0x65646f6d20726570
0x557e1f809200:	mov          qword ptr [rdi + 0xe], rax
0x557e1f809204:	movups       xmm0, xmmword ptr [rip - 0x36e2d5]
0x557e1f80920b:	movups       xmmword ptr [rdi + 1], xmm0
0x557e1f80920f:	mov          byte ptr [rdi + 0x16], 0
0x557e1f809213:	mov          rax, qword ptr fs:[0x28]
0x557e1f80921c:	cmp          rax, qword ptr [rbp - 8]
0x557e1f809220:	jne          0x557e1f80922b
0x557e1f809222:	mov          rax, rdi
0x557e1f809225:	add          rsp, 0x10
0x557e1f809229:	pop          rbp
0x557e1f80922a:	ret          
0x557e1f80922b:	call         0x557e2000aa70
0x557e1f809230:)";

  dialog.SetSourceCode(content, std::make_unique<orbit_syntax_highlighter::X86Assembly>());

  const auto line_numbers = content.count('\n');
  dialog.SetHeatmap(orbit_code_viewer::FontSizeInEm{1.2f}, [line_numbers](int line_number) {
    return static_cast<float>(line_number) / static_cast<float>(line_numbers);
  });

  dialog.SetEnableLineNumbers(true);

  return dialog.exec();
}