// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UtilWidgets/NoticeWidget.h"

#include <absl/strings/str_format.h>

#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QWidget>

#include "CoreMath.h"
#include "ui_NoticeWidget.h"

namespace {
const Color kGreen{0, 255, 0, 26};
}  // namespace
namespace orbit_util_widgets {

NoticeWidget::~NoticeWidget() = default;

NoticeWidget::NoticeWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::NoticeWidget>()) {
  ui_->setupUi(this);
  connect(ui_->noticeButton, &QPushButton::clicked, this, &NoticeWidget::ButtonClicked);
}

void NoticeWidget::Initialize(const std::string& label_text, const std::string& button_text,
                              const Color& color) {
  ui_->noticeLabel->setText(QString::fromStdString(label_text));
  ui_->noticeButton->setText(QString::fromStdString(button_text));
  setStyleSheet(QString::fromStdString(
      absl::StrFormat("QWidget#%s{ border-radius: 5px; border: 1px solid palette(text); "
                      "background: rgba(%d, %d, %d, %d);}",
                      objectName().toStdString(), color[0], color[1], color[2], color[3])));
}

void NoticeWidget::InitializeAsInspection() {
  constexpr const char* kLabelText =
      "You are currently in an inspection, limiting the tree to specific callstacks.";
  constexpr const char* kButtonText = "Leave Inspection";
  Initialize(kLabelText, kButtonText, kGreen);
}

void NoticeWidget::paintEvent(QPaintEvent* event) {
  // This is necessary in order to apply a unique background style to a custom widget.
  QStyleOption opt;
  opt.init(this);
  QPainter painter(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
  QWidget::paintEvent(event);
}

}  // namespace orbit_util_widgets