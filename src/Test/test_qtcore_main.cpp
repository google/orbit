// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <cstdio>
#include <optional>

namespace {
// QCoreApplication objects needs to be on the stack or in static storage. Putting this variable
// into the QCoreApplicationStarter class leads to a crash during destruction. I'm not sure what is
// going on.
std::optional<QCoreApplication> app;

// QCoreApplicationStarter is a gtest event listener which takes care of delayed construction of the
// QCoreApplication object.
class QCoreApplicationStarter : public testing::EmptyTestEventListener {
 public:
  explicit QCoreApplicationStarter(int argc, char** argv) : argc_{argc}, argv_{argv} {}

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
  listeners.Append(new QCoreApplicationStarter{argc, argv});  // NOLINT

  int result = RUN_ALL_TESTS();
  app = std::nullopt;
  return result;
}
