// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TutorialOverlay.h"

#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRegion>
#include <QTabBar>

#include "OrbitBase/Logging.h"
#include "Utils.h"
#include "absl/strings/str_format.h"
#include "ui_TutorialOverlay.h"

static void RecursiveUpdate(QWidget* widget) {
  for (auto child : widget->children()) {
    auto child_widget = dynamic_cast<QWidget*>(child);
    if (child_widget != nullptr) {
      RecursiveUpdate(child_widget);
    }
  }

  widget->update();
}

TutorialOverlay::TutorialOverlay(QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::TutorialOverlay>()) {
  ui_->setupUi(this);

  if (parent != nullptr) {
    setGeometry(QRect(0, 0, parent->rect().width(), parent->rect().height()));
  }
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

  background_label_ = std::make_unique<QLabel>(this);
  background_label_->setStyleSheet("background-color: rgba(0, 0, 0, 150);");

  ui_->tabWidget->raise();
  ui_->titleFrame->raise();
  ui_->btnClose->raise();
  QTabBar* tabBar = ui_->tabWidget->findChild<QTabBar*>();
  tabBar->hide();

  InitAllStepsFromUi();
}

TutorialOverlay::Step TutorialOverlay::InitializeStep(int tab_index) {
  QWidget* tab = ui_->tabWidget->widget(tab_index);

  Step step;
  step.tab_index = tab_index;
  step.name = tab->objectName().toStdString();
  step.cutout_widget = tab->findChild<CutoutWidget*>();

  if (step.cutout_widget) {
    QRect cutout_rect = step.cutout_widget->geometry();
    for (auto child : tab->findChildren<QWidget*>()) {
      if (child != step.cutout_widget && child->parent() == tab) {
        step.hints.push_back(DeriveHintDescription(cutout_rect, child));
      }
    }
  }

  return step;
}

void TutorialOverlay::InitAllStepsFromUi() {
  steps_.clear();
  for (int i = 0; i < ui_->tabWidget->count(); ++i) {
    const std::string tab_name = ui_->tabWidget->widget(i)->objectName().toStdString();
    CHECK(steps_.find(tab_name) == steps_.end());
    steps_[tab_name] = InitializeStep(i);
  }
}

QRect TutorialOverlay::AbsoluteGeometry(QWidget* widget) const {
  if (widget->parentWidget() == nullptr || widget->parentWidget() == parentWidget()) {
    return widget->geometry();
  }

  QWidget* parent_widget = widget->parentWidget();
  QRect result = widget->geometry();
  QRect parent_geo = AbsoluteGeometry(parent_widget);

  result.moveTo(parent_geo.topLeft());

  return result;
}

TutorialOverlay::~TutorialOverlay() {}

void TutorialOverlay::StartActiveStep() {
  const Step& step = GetActiveStep();
  ui_->tabWidget->setCurrentIndex(step.tab_index);

  if (step.setup.callback_init) {
    step.setup.callback_init(this, step.name);
  }

  ui_->btnPrev->setEnabled(!ActiveStepIsFirstInSection());
  const Section& section = GetActiveSection();
  const std::string text = absl::StrFormat(
      "%s: %d/%d", section.title, section.active_step_index + 1, section.step_names.size());
  ui_->labelStep->setText(QString::fromStdString(text));

  if (step.setup.anchor_widget) {
    step.setup.anchor_widget->installEventFilter(this);
  }
  showMaximized();
  UpdateOverlayLayout();
}

bool TutorialOverlay::EndActiveStep() {
  if (!HasActiveStep()) {
    return true;
  }

  UnhookActiveStep();

  return true;
}

void TutorialOverlay::StartSection(const std::string& section) {
  if (!EndActiveStep()) {
    return;
  }

  auto it = sections_.find(section);
  CHECK(it != sections_.end());

  active_section_ = it;
  it->second.active_step_index = it->second.step_names.empty() ? -1 : 0;
  StartActiveStep();
}

void TutorialOverlay::AddSection(std::string section_name, std::string title,
                                 std::vector<std::string> step_names) {
  Section section;
  for (auto& name : step_names) {
    CHECK(StepExists(name));
  }
  section.step_names = std::move(step_names);
  section.title = std::move(title);
  sections_[std::move(section_name)] = std::move(section);
}

void TutorialOverlay::NextStep() {
  if (!HasActiveStep()) {
    return;
  }
  if (!VerifyActiveStep()) {
    return;
  }
  if (!EndActiveStep()) {
    return;
  }

  auto& section = GetActiveSection();
  section.active_step_index++;
  if (static_cast<size_t>(section.active_step_index) >= section.step_names.size()) {
    section.active_step_index = -1;
  }

  if (!HasActiveStep()) {
    emit SectionCompleted(active_section_->first);
    active_section_ = sections_.end();
    close();
  } else {
    StartActiveStep();
  }
}

void TutorialOverlay::PrevStep() {
  if (!HasActiveStep()) {
    return;
  }
  if (ActiveStepIsFirstInSection()) {
    return;
  }
  if (!EndActiveStep()) {
    return;
  }
  GetActiveSection().active_step_index--;
  StartActiveStep();
}

void TutorialOverlay::closeEvent(QCloseEvent* event) {
  UnhookActiveStep();
  QDialog::closeEvent(event);

  if (parentWidget() != nullptr) {
    RecursiveUpdate(parentWidget());
    parentWidget()->removeEventFilter(this);
  }
}

void TutorialOverlay::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  if (parentWidget() != nullptr) {
    parentWidget()->installEventFilter(this);
  }
}

bool TutorialOverlay::eventFilter(QObject*, QEvent* event) {
  if (event->type() == QEvent::Resize) {
    UpdateOverlayLayout();
  }

  return false;
}

void TutorialOverlay::SetupStep(const std::string& ui_tab_name, StepSetup step_setup) {
  auto it = steps_.find(ui_tab_name);
  CHECK(it != steps_.end());
  it->second.setup = step_setup;
}

const TutorialOverlay::StepSetup& TutorialOverlay::GetStep(const std::string& name) const {
  CHECK(StepExists(name));
  auto it = steps_.find(name);
  return it->second.setup;
}

bool TutorialOverlay::StepExists(const std::string& name) const {
  auto it = steps_.find(name);
  return it != steps_.end();
}

static const auto margin = QPoint(20, 20);

void TutorialOverlay::UpdateOverlayLayout() {
  if (parentWidget() != nullptr) {
    const QRect parent_rect = parentWidget()->rect();
    setGeometry(parent_rect);
    ui_->tabWidget->setGeometry(parent_rect);
  }
  UpdateButtons();
  background_label_->setGeometry(rect());

  Step& step = GetActiveStep();

  if (step.setup.anchor_widget != nullptr && step.cutout_widget != nullptr) {
    auto target_rect = AbsoluteGeometry(step.setup.anchor_widget);
    auto outer_rect = target_rect;
    outer_rect.setTopLeft(outer_rect.topLeft() - margin);
    outer_rect.setBottomRight(outer_rect.bottomRight() + margin);

    QRegion maskedRegion(QRegion(QRect(rect())).subtracted(QRegion(target_rect)));
    setMask(maskedRegion);

    step.cutout_widget->setGeometry(outer_rect);

    for (auto& hint : step.hints) {
      UpdateHintWidgetPosition(outer_rect, hint);
    }

    background_label_->show();
  } else {
    background_label_->hide();
  }

  if (parentWidget() != nullptr) {
    RecursiveUpdate(parentWidget());
  }
}

void TutorialOverlay::UpdateButtons() {
  if (parentWidget() == nullptr) {
    return;
  }

  const int kMargin[] = {20, 20};

  const QRect parent_rect = parentWidget()->rect();

  QRect rect = ui_->btnClose->geometry();
  ui_->btnClose->setGeometry(QRect(parent_rect.width() - rect.width() - kMargin[0], kMargin[1],
                                   rect.width(), rect.height()));

  rect = ui_->titleFrame->geometry();
  ui_->titleFrame->setGeometry(QRect((parent_rect.width() - rect.width()) / 2,
                                     parent_rect.height() - rect.height() - kMargin[1],
                                     rect.width(), rect.height()));
}

bool TutorialOverlay::VerifyActiveStep() {
  const Step& step = GetActiveStep();

  if (step.setup.callback_verify) {
    auto verify_complete = step.setup.callback_verify(this, step.name);
    if (verify_complete.has_value()) {
      // This is mostly to allow unit testing...
      if (parentWidget() != nullptr && parentWidget()->isVisible()) {
        QMessageBox::critical(this, "Wizard Error",
                              QString::fromStdString(verify_complete.value()));
      }
      return false;
    }
  }

  return true;
}

TutorialOverlay::Hint TutorialOverlay::DeriveHintDescription(const QRect& anchor_rect,
                                                             QWidget* hint_widget) const {
  QRect hint_rect = hint_widget->geometry();

  // The anchor position is determined by the quadrant of anchor_rect in which the top
  // left corner of hint_rect is placed initially.
  Hint result;
  result.widget = hint_widget;

  if (hint_rect.x() < anchor_rect.x() + anchor_rect.width() / 2) {
    if (hint_rect.y() < anchor_rect.y() + anchor_rect.height() / 2) {
      result.anchor = HintAnchor::kTopLeft;
    } else {
      result.anchor = HintAnchor::kBottomLeft;
    }
  } else {
    if (hint_rect.y() < anchor_rect.y() + anchor_rect.height() / 2) {
      result.anchor = HintAnchor::kTopRight;
    } else {
      result.anchor = HintAnchor::kBottomRight;
    }
  }

  switch (result.anchor) {
    case HintAnchor::kTopLeft:
      result.offset = hint_rect.topLeft() - anchor_rect.topLeft();
      break;
    case HintAnchor::kTopRight:
      result.offset = hint_rect.topLeft() - anchor_rect.topRight();
      break;
    case HintAnchor::kBottomRight:
      result.offset = hint_rect.topLeft() - anchor_rect.bottomRight();
      break;
    case HintAnchor::kBottomLeft:
      result.offset = hint_rect.topLeft() - anchor_rect.bottomLeft();
      break;
  }

  return result;
}

void TutorialOverlay::UpdateHintWidgetPosition(const QRect& anchor_rect, Hint& hint) {
  const QPoint size(hint.widget->rect().width(), hint.widget->rect().height());
  QPoint tl, br;

  // See comments in DeriveHintDescription() on how offsets and anchors
  // are calculated
  switch (hint.anchor) {
    case HintAnchor::kTopLeft:
      tl = anchor_rect.topLeft() + hint.offset;
      break;
    case HintAnchor::kTopRight:
      tl = anchor_rect.topRight() + hint.offset;
      break;
    case HintAnchor::kBottomRight:
      tl = anchor_rect.bottomRight() + hint.offset;
      break;
    case HintAnchor::kBottomLeft:
      tl = anchor_rect.bottomLeft() + hint.offset;
      break;
  }

  br = tl + size;
  hint.widget->setGeometry(QRect(tl, br));
}

bool TutorialOverlay::HasActiveStep() const {
  if (!HasActiveSection()) {
    return false;
  }

  auto& section = GetActiveSection();
  return section.active_step_index >= 0 &&
         static_cast<size_t>(section.active_step_index) < section.step_names.size();
}

const TutorialOverlay::Step& TutorialOverlay::GetActiveStep() const {
  CHECK(HasActiveStep());
  auto& section = GetActiveSection();
  auto it = steps_.find(section.step_names[section.active_step_index]);
  CHECK(it != steps_.end());

  return it->second;
}

TutorialOverlay::Step& TutorialOverlay::GetActiveStep() {
  return const_cast<TutorialOverlay::Step&>(
      static_cast<const TutorialOverlay*>(this)->GetActiveStep());
}

const TutorialOverlay::Section& TutorialOverlay::GetActiveSection() const {
  CHECK(HasActiveSection());
  return active_section_->second;
}

TutorialOverlay::Section& TutorialOverlay::GetActiveSection() {
  return const_cast<TutorialOverlay::Section&>(
      static_cast<const TutorialOverlay*>(this)->GetActiveSection());
}

bool TutorialOverlay::ActiveStepIsFirstInSection() const {
  CHECK(HasActiveSection());
  return GetActiveSection().active_step_index == 0;
}

void TutorialOverlay::UnhookActiveStep() {
  if (HasActiveStep()) {
    Step& active_step = GetActiveStep();

    if (active_step.setup.anchor_widget != nullptr) {
      active_step.setup.anchor_widget->removeEventFilter(this);
    }
    if (active_step.setup.callback_teardown != nullptr) {
      active_step.setup.callback_teardown(this, active_step.name);
    }
  }
}