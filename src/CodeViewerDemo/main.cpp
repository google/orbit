// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>

#include "CodeExamples.h"
#include "CodeViewer/Dialog.h"
#include "SyntaxHighlighter/Cpp.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};

  orbit_code_viewer::Dialog dialog{};

  // Example file
  const QString content = testing_example;

  dialog.SetSourceCode(content, std::make_unique<orbit_syntax_highlighter::Cpp>());

  const auto line_numbers = content.count('\n');
  dialog.SetHeatmap(orbit_code_viewer::FontSizeInEm{1.2f}, [line_numbers](int line_number) {
    return static_cast<float>(line_number) / static_cast<float>(line_numbers);
  });

  dialog.SetEnableLineNumbers(true);
  dialog.GoToLineNumber(10);
  dialog.SetHighlightCurrentLine(true);

  return dialog.exec();
}