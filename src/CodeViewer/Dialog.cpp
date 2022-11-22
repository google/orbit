// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeViewer/Dialog.h"

#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <utility>

#include "ui_Dialog.h"

namespace orbit_code_viewer {
Dialog::Dialog(QWidget* parent) : QDialog{parent}, ui_{std::make_unique<Ui::CodeViewerDialog>()} {
  ui_->setupUi(this);

  ui_->notification_box->hide();
  ui_->buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  QObject::connect(ui_->buttonBox->button(QDialogButtonBox::StandardButton::Close),
                   &QPushButton::clicked, this, &QDialog::close);
  QObject::connect(ui_->notification_action_button, &QPushButton::clicked, this,
                   [this]() { emit StatusMessageButtonClicked(); });
}

Dialog::~Dialog() = default;

void Dialog::SetMainContent(const QString& code) {
  ui_->viewer->setPlainText(code);
  syntax_highlighter_.reset();
  ui_->viewer->document()->setDefaultFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void Dialog::SetMainContent(const QString& code,
                            std::unique_ptr<QSyntaxHighlighter> syntax_highlighter) {
  SetMainContent(code);
  syntax_highlighter_ = std::move(syntax_highlighter);
  syntax_highlighter_->setDocument(ui_->viewer->document());
}

void Dialog::SetHeatmap(FontSizeInEm heatmap_bar_width,
                        const orbit_code_report::CodeReport* code_report) {
  ui_->viewer->SetHeatmapBarWidth(heatmap_bar_width);
  ui_->viewer->SetHeatmapSource(code_report);
}

void Dialog::ClearHeatmap() {
  ui_->viewer->SetHeatmapBarWidth(FontSizeInEm{0.0});
  ui_->viewer->ClearHeatmapSource();
}

void Dialog::SetLineNumberMargins(FontSizeInEm left, FontSizeInEm right) {
  ui_->viewer->SetLineNumberMargins(left, right);
}

void Dialog::SetLineNumberTypes(LineNumberTypes line_number_types) {
  ui_->viewer->SetLineNumberTypes(line_number_types);
}

void Dialog::SetEnableSampleCounters(bool enabled) {
  ui_->viewer->SetEnableSampleCounters(enabled);
}

void Dialog::GoToLineNumber(size_t line_number) {
  const QTextBlock block =
      ui_->viewer->document()->findBlockByLineNumber(static_cast<int>(line_number) - 1);
  if (!block.isValid()) return;

  ui_->viewer->setTextCursor(QTextCursor{block});
}

void Dialog::SetHighlightCurrentLine(bool enabled) {
  ui_->viewer->SetHighlightCurrentLine(enabled);
}

bool Dialog::IsCurrentLineHighlighted() const { return ui_->viewer->IsCurrentLineHighlighted(); }

void Dialog::SetAnnotatingContent(
    absl::Span<const orbit_code_report::AnnotatingLine> annotating_lines) {
  ui_->viewer->SetAnnotatingContent(annotating_lines);
}

void Dialog::SetTopBarTitle(const QString& title) { ui_->viewer->SetTopBarTitle(title); }
const QString& Dialog::GetTopBarTitle() const { return ui_->viewer->GetTopBarTitle(); }

void Dialog::SetStatusMessage(const QString& message, const std::optional<QString>& button_text) {
  ui_->notification_text->setText(message);
  ui_->notification_action_button->setHidden(!button_text.has_value());
  ui_->notification_action_button->setText(button_text.value_or(QString{}));

  ui_->notification_box->show();
  ui_->buttonBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void Dialog::ClearStatusMessage() {
  ui_->notification_box->hide();
  ui_->buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

}  // namespace orbit_code_viewer
