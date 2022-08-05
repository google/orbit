// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UtilWidgets/NoticeWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QStyleOption>
#include <QWidget>

#include "ui_NoticeWidget.h"

namespace orbit_util_widgets {

NoticeWidget::~NoticeWidget() = default;

NoticeWidget::NoticeWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::NoticeWidget>()) {
  ui_->setupUi(this);
  connect(ui_->noticeButton, &QPushButton::clicked, this, &NoticeWidget::ButtonClicked);
}

void NoticeWidget::Initialize(const std::string& label_text, const std::string& button_text) {
  ui_->noticeLabel->setText(QString::fromStdString(label_text));
  ui_->noticeButton->setText(QString::fromStdString(button_text));
}

void NoticeWidget::InitializeAsInspection() {
  constexpr const char* kLabelText =
      "You are currently in an inspection, limiting the tree to specific callstacks.";
  constexpr const char* kButtonText = "Leave Inspection";
  Initialize(kLabelText, kButtonText);
}

void NoticeWidget::paintEvent(QPaintEvent* event) {
  // This is necessary in order to apply a unique background style to a custom widget.
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
  QWidget::paintEvent(event);
}

}  // namespace orbit_util_widgets