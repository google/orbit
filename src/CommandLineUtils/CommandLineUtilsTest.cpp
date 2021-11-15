// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QStringList>

#include "CommandLineUtils/CommandLineUtils.h"

namespace orbit_command_line_utils {

TEST(CommandLineUtils, RemoveFlagsNotPassedToMainWindow) {
  QStringList params{"--some_bool", "-b", "--connection_target=1234@target", "--some_flag"};
  QStringList expected{"--some_bool", "-b", "--some_flag"};
  QStringList result = RemoveFlagsNotPassedToMainWindow(params);
  EXPECT_EQ(expected, result);
}

TEST(CommandLineUtils, ExtractCommandLineFlags) {
  char* pos_arg1 = strdup("pos_arg");
  char* pos_arg2 = strdup("another_pos_arg");

  std::vector<std::string> command_line_args{"-b", "--test_arg", "--another_arg=something",
                                             pos_arg1, pos_arg2};
  std::vector<char*> positional_args{pos_arg1, pos_arg2};

  auto result = ExtractCommandLineFlags(command_line_args, positional_args);
  QStringList expected{"-b", "--test_arg", "--another_arg=something"};
  EXPECT_EQ(expected, result);
}

}  // namespace orbit_command_line_utils