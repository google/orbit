// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>

#include "CodeExamples.h"
#include "CodeViewer/Dialog.h"
#include "SyntaxHighlighter/Cpp.h"

class DummyCodeReport : public CodeReport {
 public:
  explicit DummyCodeReport(uint32_t num_samples) : num_samples_{num_samples} {}

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return num_samples_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return num_samples_; }
  [[nodiscard]] std::optional<uint32_t> GetNumSamplesAtLine(size_t line) const override {
    return line;
  }

 private:
  uint32_t num_samples_ = 0;
};

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};

  orbit_code_viewer::Dialog dialog{};

  // Example file
  const QString content = testing_example;

  dialog.SetSourceCode(content, std::make_unique<orbit_syntax_highlighter::Cpp>());

  const auto line_numbers = content.count('\n');
  const DummyCodeReport code_report{static_cast<uint32_t>(line_numbers)};
  dialog.SetHeatmap(orbit_code_viewer::FontSizeInEm{1.2f}, &code_report);

  dialog.SetEnableLineNumbers(true);
  dialog.GoToLineNumber(10);
  dialog.SetHighlightCurrentLine(true);

  return dialog.exec();
}