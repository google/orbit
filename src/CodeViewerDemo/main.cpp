// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>

#include <QApplication>
#include <QString>
#include <QSyntaxHighlighter>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CodeExamples.h"
#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/CodeReport.h"
#include "CodeViewer/Dialog.h"
#include "CodeViewer/FontSizeInEm.h"
#include "Style/Style.h"
#include "SyntaxHighlighter/X86Assembly.h"

class DummyCodeReport : public orbit_code_report::CodeReport {
 public:
  explicit DummyCodeReport(uint32_t num_samples) : num_samples_{num_samples} {}

  [[nodiscard]] uint32_t GetNumSamplesInFunction() const override { return num_samples_; }
  [[nodiscard]] uint32_t GetNumSamples() const override { return num_samples_; }
  [[nodiscard]] std::optional<uint32_t> GetNumSamplesAtLine(size_t line) const override {
    return static_cast<uint32_t>(line);
  }

 private:
  uint32_t num_samples_ = 0;
};

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  orbit_style::ApplyStyle(&app);

  orbit_code_viewer::Dialog dialog{};

  // Example file
  const QString content = x86Assembly_example;

  dialog.SetMainContent(content, std::make_unique<orbit_syntax_highlighter::X86Assembly>());

  std::vector<orbit_code_report::AnnotatingLine> lines{};
  lines.emplace_back();
  lines.back().reference_line = 9;
  lines.back().line_number = 42;
  lines.back().line_contents = "void main() {";

  lines.emplace_back();
  lines.back().reference_line = 14;
  lines.back().line_number = 43;
  lines.back().line_contents = "echo \"Hello World!\";";

  dialog.SetAnnotatingContent(lines);

  const auto line_numbers = content.count('\n');
  const DummyCodeReport code_report{static_cast<uint32_t>(line_numbers)};
  dialog.SetHeatmap(orbit_code_viewer::FontSizeInEm{1.2f}, &code_report);

  dialog.SetLineNumberTypes(orbit_code_viewer::Dialog::LineNumberTypes::kOnlyAnnotatingLines);
  dialog.SetEnableSampleCounters(true);
  dialog.GoToLineNumber(10);
  dialog.SetHighlightCurrentLine(true);

  dialog.SetTopBarTitle("Demo title");

  dialog.SetStatusMessage("<b>Important message</b><br>A new notification is available.",
                          "Show me!");

  return dialog.exec();
}