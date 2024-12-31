// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>
#include <string>
#include <vector>

#include "CommandLineUtils/CommandLineUtils.h"

namespace {

std::vector<std::string> QStringListToStdVector(const QStringList& qlist) {
  std::vector<std::string> result;
  result.reserve(qlist.size());

  for (const QString& str : qlist) {
    result.push_back(str.toStdString());
  }

  return result;
}

}  // namespace

namespace orbit_command_line_utils {

TEST(CommandLineUtils, RemoveFlagsNotPassedToMainWindow) {
  QStringList params{"--some_bool",
                     "-b",
                     "--ssh_hostname=1.1.1.1",
                     "--ssh_target_process=ssh_target_process",
                     "--some_flag",
                     "--ssh_port=300",
                     "--ssh_user=username",
                     "--ssh_known_host_path=path_placeholder",
                     "--ssh_key_path=another_path"};
  QStringList expected{"--some_bool", "-b", "--some_flag"};
  QStringList result = RemoveFlagsNotPassedToMainWindow(params);
  EXPECT_EQ(QStringListToStdVector(expected), QStringListToStdVector(result));
}

TEST(CommandLineUtils, ExtractCommandLineFlags) {
  std::string pos_arg1{"pos_arg"};
  std::string pos_arg2{"another_pos_arg"};

  std::vector<std::string> command_line_args{"-b", "--test_arg", "--another_arg=something",
                                             pos_arg1, pos_arg2};
  std::vector<char*> positional_args{pos_arg1.data(), pos_arg2.data()};

  auto result = ExtractCommandLineFlags(command_line_args, positional_args);
  QStringList expected{"-b", "--test_arg", "--another_arg=something"};
  EXPECT_EQ(QStringListToStdVector(expected), QStringListToStdVector(result));
}

}  // namespace orbit_command_line_utils