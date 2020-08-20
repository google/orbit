// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QPushButton>

#include "TutorialOverlay.h"
#include "gtest/gtest.h"

static int argc = 0;

// TODO: These tests use the actual Tutorial UI and can be broken
// by updating this UI... it should use a dedicated unit testing UI

TEST(TutorialOverlay, StepExists) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);

  // Check if StepExists works, and implicitely check if the UI file
  // is correctly setup for unit tests. If this failes, a lot of other
  // tests will also fail!
  EXPECT_TRUE(overlay->StepExists("unitTest1"));
  EXPECT_TRUE(overlay->StepExists("unitTest2"));
  EXPECT_FALSE(overlay->StepExists("unitTest3"));
}

TEST(TutorialOverlay, SetupSections) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);

  overlay->AddSection("unitTest", "Unit Testing", {"unitTest1", "unitTest2"});
  overlay->StartSection("unitTest");
  EXPECT_EQ(overlay->GetActiveSectionName(), "unitTest");
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");
}

TEST(TutorialOverlay, callbackSuccess) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);

  std::map<std::string, int> init_count{std::make_pair<std::string, int>("unitTest1", 0),
                                        std::make_pair<std::string, int>("unitTest2", 0)};
  std::map<std::string, int> verify_count = init_count;
  std::map<std::string, int> teardown_count = init_count;

  TutorialOverlay::StepSetup setup;
  setup.callback_init = [&](TutorialOverlay*, const std::string& step) { init_count[step]++; };
  setup.callback_verify = [&](TutorialOverlay*,
                              const std::string& step) -> std::optional<std::string> {
    verify_count[step]++;
    return std::nullopt;
  };
  setup.callback_teardown = [&](TutorialOverlay*, const std::string& step) {
    teardown_count[step]++;
  };

  overlay->SetupStep("unitTest1", setup);
  overlay->SetupStep("unitTest2", setup);

  overlay->AddSection("unitTest", "Unit Testing", {"unitTest1", "unitTest2"});
  overlay->StartSection("unitTest");

  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");
  EXPECT_EQ(init_count["unitTest1"], 1);
  EXPECT_EQ(verify_count["unitTest1"], 0);

  overlay->NextStep();

  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest2");
  EXPECT_EQ(init_count["unitTest2"], 1);
  EXPECT_EQ(verify_count["unitTest1"], 1);
  EXPECT_EQ(teardown_count["unitTest1"], 1);

  overlay->PrevStep();

  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");
  EXPECT_EQ(init_count["unitTest2"], 1);
  EXPECT_EQ(teardown_count["unitTest2"], 1);
  EXPECT_EQ(init_count["unitTest1"], 2);

  overlay->close();
  EXPECT_EQ(teardown_count["unitTest1"], 2);
}

TEST(TutorialOverlay, CallbackFail) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);

  TutorialOverlay::StepSetup setup{
      nullptr, nullptr, nullptr,
      [&](TutorialOverlay*, const std::string&) -> std::optional<std::string> {
        return "You did not complete this step correctly.";
      }};
  overlay->SetupStep("unitTest1", setup);

  overlay->AddSection("unitTest", "Unit Testing", {"unitTest1", "unitTest2"});
  overlay->StartSection("unitTest");
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");
  overlay->NextStep();
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");
}

TEST(TutorialOverlay, SignalsAndSlots) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);

  auto button = std::make_unique<QPushButton>();

  TutorialOverlay::StepSetup setup;
  setup.callback_init = [&button](TutorialOverlay* overlay, const std::string&) {
    QObject::connect(button.get(), &QPushButton::clicked, overlay, &TutorialOverlay::NextStep);
  };
  setup.callback_teardown = [&button](TutorialOverlay* overlay, const std::string&) {
    QObject::disconnect(button.get(), &QPushButton::clicked, overlay, &TutorialOverlay::NextStep);
  };
  overlay->SetupStep("unitTest1", setup);

  // Clicking the button initially should not do anything - connect() happens on
  // initialization of step 0
  emit button->clicked();
  EXPECT_EQ(overlay->GetActiveStepName(), "");

  overlay->AddSection("unitTest", "Unit Testing", {"unitTest1", "unitTest2"});
  overlay->StartSection("unitTest");
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest1");

  emit button->clicked();
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest2");

  // Clicking the button again should not do anything - the slot is disconnected
  // on teardown of step 0
  emit button->clicked();
  EXPECT_EQ(overlay->GetActiveStepName(), "unitTest2");

  bool completed = false;
  QObject::connect(overlay.get(), &TutorialOverlay::SectionCompleted,
                   [&completed] { completed = true; });
  overlay->NextStep();
  EXPECT_TRUE(completed);
}

TEST(TutorialOverlay, FailTests) {
  QApplication app(argc, nullptr);
  auto overlay = std::make_unique<TutorialOverlay>(nullptr);
  EXPECT_DEATH({ overlay->SetupStep("xx_invalid_step_name_xx", {}); }, "Check failed");
  EXPECT_DEATH({ overlay->AddSection("test", "Unit Testing", {"xx_invalid_step_name_xx"}); },
               "Check failed");
  EXPECT_DEATH({ overlay->StartSection("invalid_section"); }, "Check failed");

  overlay->NextStep();
  EXPECT_EQ(overlay->GetActiveStepName(), "");
  overlay->PrevStep();
  EXPECT_EQ(overlay->GetActiveStepName(), "");
}