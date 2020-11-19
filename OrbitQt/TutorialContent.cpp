// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TutorialContent.h"

#include <QMenu>

#include "App.h"
#include "TutorialOverlay.h"
#include "ui_orbitmainwindow.h"

std::unique_ptr<TutorialOverlay> overlay;

TutorialOverlay::StepSetup CreateTakeACaptureStepSetup(OrbitMainWindow* main_window) {
  TutorialOverlay::StepSetup setup;

  setup.anchor_widget = main_window->GetUi()->capture_toolbar;
  return setup;
}

TutorialOverlay::StepSetup CreateAnalyzeResultsStepSetup(OrbitMainWindow* main_window) {
  TutorialOverlay::StepSetup setup;
  setup.anchor_widget = main_window->GetUi()->RightTabWidget;
  return setup;
}

void SetupAllSteps(OrbitMainWindow* main_window) {
  overlay->SetupStep("capture", CreateTakeACaptureStepSetup(main_window));
  overlay->SetupStep("analyze", CreateAnalyzeResultsStepSetup(main_window));
}

void SetupDynamicInstrumentationTutorial(QMenu* menu) {
  auto action = menu->addAction("Dynamic Instrumentation");

  overlay->AddSection("dynamicInstrumentation", "Dynamic Instrumentation", {"capture", "analyze"});
  QObject::connect(action, &QAction::triggered,
                   []() { overlay->StartSection("dynamicInstrumentation"); });
}

void InitTutorials(OrbitMainWindow* main_window) {
  auto tutorials_menu = main_window->GetUi()->menuHelp->addMenu("Tutorials");

  overlay = std::make_unique<TutorialOverlay>(main_window);
  static const int margin = 60;
  QObject::connect(overlay.get(), &TutorialOverlay::Shown, [main_window]() {
    int top, left, right, bottom;
    main_window->centralWidget()->layout()->getContentsMargins(&left, &top, &right, &bottom);
    main_window->centralWidget()->layout()->setContentsMargins(left, top, right, bottom + margin);
  });
  QObject::connect(overlay.get(), &TutorialOverlay::Hidden, [main_window]() {
    int top, left, right, bottom;
    main_window->centralWidget()->layout()->getContentsMargins(&left, &top, &right, &bottom);
    main_window->centralWidget()->layout()->setContentsMargins(left, top, right, bottom - margin);
  });

  SetupAllSteps(main_window);
  SetupDynamicInstrumentationTutorial(tutorials_menu);
}

void DeinitTutorials() { overlay.reset(); }