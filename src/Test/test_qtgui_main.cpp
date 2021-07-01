// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <cstdio>
#include <optional>

namespace {
// QApplication objects needs to be on the stack or in static storage. Putting this variable into
// the QApplicationStarter class leads to a crash during destruction. I'm not sure what is going on.
std::optional<QApplication> app;

// QApplicationStarter is a gtest event listener which takes care of delayed construction of the
// QApplication object.
class QApplicationStarter : public testing::EmptyTestEventListener {
 public:
  explicit QApplicationStarter(int argc, char** argv) : argc_{argc}, argv_{argv} {}

 private:
  void OnTestStart(const ::testing::TestInfo& /*test_info*/) override {
    if (!app.has_value()) app.emplace(argc_, argv_);
  }

  int argc_;
  char** argv_;
};

}  // namespace

// This main function is trying hard to create the QApplication object only if it is really needed
// (i.e. when tests are actually meant to be executed). This allows the test executable to be called
// in a headless setup since the QApplication constructor would abort if no screen was available.
// That's important for listing tests (./executable --gtest_list_tests) or for just showing the help
// text for example.
int main(int argc, char* argv[]) {
  const bool in_death_test_run = [&]() {
    for (int i = 0; i < argc; ++i) {
      if (absl::StartsWith(argv[i], "--gtest_internal_run_death_test")) return true;
    }
    return false;
  }();

  printf("Running main() from %s\n", __FILE__);
  ::testing::InitGoogleTest(&argc, argv);

  ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new QApplicationStarter{argc, argv});  // NOLINT

  // In a normal test run the event listener above will create the QApplication object right before
  // the first test case gets executed. In a death test the event listener won't receive any events
  // so we are going a different way to instantiate the QApplication object.
  if (in_death_test_run) app.emplace(argc, argv);

  int result = RUN_ALL_TESTS();
  app = std::nullopt;
  return result;
}