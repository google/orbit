// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CommandLineUtils/CommandLineUtils.h"

#include <absl/container/flat_hash_set.h>

namespace orbit_command_line_utils {

QStringList ExtractCommandLineFlags(const std::vector<std::string>& command_line_args,
                                    const std::vector<char*>& positional_args) {
  QStringList command_line_flags;
  absl::flat_hash_set<std::string> positional_arg_set(positional_args.begin(),
                                                      positional_args.end());
  for (const std::string& command_line_arg : command_line_args) {
    if (!positional_arg_set.contains(command_line_arg)) {
      command_line_flags << QString::fromStdString(command_line_arg);
    }
  }
  return command_line_flags;
}

QStringList RemoveFlagsNotPassedToMainWindow(const QStringList& flags) {
  QStringList result;
  for (auto flag = flags.begin(); flag != flags.end(); ++flag) {
    if (flag->startsWith("--connection_target=")) {
      continue;
    }

    result.push_back(*flag);
  }

  return result;
}

}  // namespace orbit_command_line_utils