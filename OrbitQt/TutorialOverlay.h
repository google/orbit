// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TUTORIAL_OVERLAY_
#define ORBIT_QT_TUTORIAL_OVERLAY_

#include <QDialog>
#include <QLabel>
#include <QPoint>
#include <memory>
#include <optional>
#include <unordered_map>

#include "CutoutWidget.h"

namespace Ui {
class TutorialOverlay;
}

/**
Provides interactive tutorials as fullscreen overlays.

This Dialog relies on tutorial content defined in TutorialOverlay.ui. Open the UI file
in Qt Designer for a description on how to create new tutorial steps (check the README tab).

The Orbit tutorial consists of multiple "Sections". A section takes you through a single
workflow in Orbit (e.g. taking a capture). Each section consist of multiple "Steps",
where each step is a fullscreen overlay displaying instructions to the user. A section is
defined by a freely given name and a list of step names that will be executed
one after another.

All UI elements of a step are defined by a single tab in the main tabWidget of the tutorial UI
(again, check the UI file), where the tab objectName equals the step name. If no further
setup is done, the step will simply display this tab and wait for the user to click "next".

Usually, additional programmatical setup of a step will be performed through
TutorialOverlay::SetupStep(). Each step can receive:

* An anchor widget: This is the widget onto which you want to focus the user's attention.
  This widget will receive a prominent border, and the rest of Orbit will receive a
  dimming overlay and cannot be interacted with
* An init callback: A callback method that is executed when the step is started. This can
  be used to e.g. connect signals to the TutorialOverlay::NextStep() method to
  automatically advance
* A teardown callback: A callback method that is executed when the step ends. Use this to
  cleanup everything you set up in the init callback
* A verify callback: A callback method that is executed before a step is finished. If this
  callback returns a string, this is interpreted as an error message to be displayed to
  the user, and the tutorial will not advance to the next step.

Check the unit tests on how to use.
A very brief summary:
1) Add your step's UI to TutorialOverlay.ui
2) (optional) Setup your step with SetupStep() in code
3) Create a section that contains one or more steps with AddSection()
4) Create this dialog with the main window as a parent
5) Execute the tutorial by StartSection()
**/
class TutorialOverlay : public QDialog {
  Q_OBJECT

 public:
  using StepCallback = std::function<void(TutorialOverlay* overlay, const std::string& step_name)>;
  using VerifyStepCompleted = std::function<std::optional<std::string>(
      TutorialOverlay* overlay, const std::string& step_completed)>;

  struct StepSetup {
    QWidget* anchor_widget = nullptr;
    StepCallback callback_init = nullptr;
    StepCallback callback_teardown = nullptr;
    VerifyStepCompleted callback_verify = nullptr;
  };

  explicit TutorialOverlay(QWidget* parent = nullptr);
  ~TutorialOverlay() override;

  bool eventFilter(QObject* object, QEvent* event) override;

  void SetupStep(const std::string& ui_tab_name, StepSetup step_setup);
  [[nodiscard]] const StepSetup& GetStep(const std::string& name) const;
  [[nodiscard]] bool StepExists(const std::string& name) const;

  [[nodiscard]] std::string GetActiveStepName() const {
    return HasActiveStep() ? GetActiveStep().name : "";
  }

  [[nodiscard]] std::string GetActiveSectionName() const {
    return HasActiveSection() ? active_section_->first : "";
  }

  void AddSection(const std::string section_name, std::string title,
                  std::vector<std::string> step_names);

 public Q_SLOTS:
  void StartSection(const std::string& section_name);
  void NextStep();
  void PrevStep();

 Q_SIGNALS:
  void SectionCompleted(const std::string& section_name);

 protected:
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent* event) override;

 private:
  enum class HintAnchor { kTopLeft, kTopRight, kBottomRight, kBottomLeft };
  struct Hint {
    QWidget* widget = nullptr;
    HintAnchor anchor = HintAnchor::kTopLeft;
    QPoint offset = QPoint(0, 0);
  };

  struct Step {
    StepSetup setup;
    std::vector<Hint> hints;
    QWidget* cutout_widget = nullptr;
    int tab_index = -1;
    std::string name;
  };

  struct Section {
    std::string title;
    std::vector<std::string> step_names;
    int active_step_index = -1;
  };

  using SectionMap = std::unordered_map<std::string, Section>;

  std::unique_ptr<Ui::TutorialOverlay> ui_;
  std::unique_ptr<QWidget> background_label_;
  std::unordered_map<std::string, Step> steps_;
  SectionMap sections_ = SectionMap();

  SectionMap::iterator active_section_ = sections_.end();

 private:
  [[nodiscard]] Step InitializeStep(int tab_index);
  void InitAllStepsFromUi();

  void UpdateButtons();

  [[nodiscard]] bool HasActiveStep() const;
  [[nodiscard]] Step& GetActiveStep();
  [[nodiscard]] Section& GetActiveSection();
  [[nodiscard]] const Step& GetActiveStep() const;
  [[nodiscard]] const Section& GetActiveSection() const;
  [[nodiscard]] bool HasActiveSection() const { return active_section_ != sections_.end(); }
  [[nodiscard]] bool ActiveStepIsFirstInSection() const;

  void UnhookActiveStep();
  [[nodiscard]] bool VerifyActiveStep();

  void StartActiveStep();
  [[nodiscard]] bool EndActiveStep();

  [[nodiscard]] Hint DeriveHintDescription(const QRect& anchor_rect, QWidget* hint_widget) const;
  void UpdateHintWidgetPosition(const QRect& anchor_rect, Hint& hint);

  [[nodiscard]] QRect AbsoluteGeometry(QWidget* widget) const;

 private Q_SLOTS:
  void UpdateOverlayLayout();
};

#endif