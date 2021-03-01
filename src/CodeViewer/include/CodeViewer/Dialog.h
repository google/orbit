// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_VIEWER_DIALOG_H_
#define CODE_VIEWER_DIALOG_H_

#include <QDialog>
#include <QSyntaxHighlighter>
#include <memory>
#include <optional>

#include "CodeReport.h"
#include "CodeViewer/FontSizeInEm.h"
#include "CodeViewer/Viewer.h"

namespace Ui {
class CodeViewerDialog;  // IWYU pragma: keep
}

namespace orbit_code_viewer {

/*
  This is a dialog for displaying source code (and assembly to be strict).

  The typical use-case is to instantiate locally and call the exec function
  which blocks until the dialog is closed:

  orbit_code_viewer::Dialog dialog{};
  dialog.SetSourceCode(source_code);
  dialog.exec();

  Optionally a syntax highlighter can be provided with the source code.
  Check out the Syntax highlighting module for more details on this.
  Example:

  orbit_code_viewer::Dialog dialog{};
  dialog.SetSourceCode(source_code, std::make_unique<orbit_syntax_highlighter::X86Assembly>());
  dialog.exec();

  Check out orbit_code_viewer::Viewer if you don't need a dialog but rather a widget
  which can be embedded into other windows.
*/
class Dialog : public QDialog {
  Q_OBJECT

 public:
  explicit Dialog(QWidget* parent = nullptr);
  ~Dialog() noexcept override;

  void SetSourceCode(const QString& code);
  void SetSourceCode(const QString& code, std::unique_ptr<QSyntaxHighlighter> syntax_highlighter);

  void SetHeatmap(FontSizeInEm heatmap_bar_width, const CodeReport* code_report);
  void ClearHeatmap();

  void SetEnableLineNumbers(bool enabled);
  void SetLineNumberMargins(FontSizeInEm left, FontSizeInEm right);

  void GoToLineNumber(size_t line_number);

  void SetHighlightCurrentLine(bool enabled);
  [[nodiscard]] bool IsCurrentLineHighlighted() const;

 private:
  std::unique_ptr<Ui::CodeViewerDialog> ui_;
  std::unique_ptr<QSyntaxHighlighter> syntax_highlighter_;
};
}  // namespace orbit_code_viewer

#endif  // CODE_VIEWER_DIALOG_H_