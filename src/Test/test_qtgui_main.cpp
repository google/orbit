// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

int main(int argc, char* argv[]) {
  printf("Running main() from %s\n", __FILE__);
  ::testing::InitGoogleTest(&argc, argv);

  ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new QApplicationStarter{argc, argv});  // NOLINT

  int result = RUN_ALL_TESTS();
  app = std::nullopt;
  return result;
}