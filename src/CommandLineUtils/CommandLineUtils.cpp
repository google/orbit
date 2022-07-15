// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CommandLineUtils/CommandLineUtils.h"

#include <absl/container/flat_hash_set.h>

#include "ClientFlags/ClientFlags.h"
#include "absl/flags/flag.h"

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

// TODO(b/239181650): Add unit tests for this.
QStringList RemoveFlagsNotPassedToMainWindow(const QStringList& flags) {
  QStringList result;
  const std::array kDoNotPassTheseFlags{absl::GetFlagReflectionHandle(FLAGS_target_instance).Name(),
                                        absl::GetFlagReflectionHandle(FLAGS_target_process).Name(),
                                        absl::GetFlagReflectionHandle(FLAGS_target_uri).Name()};

  for (auto flag = flags.begin(); flag != flags.end(); ++flag) {
    bool ignore_this_flag = false;

    for (auto ignore_flag : kDoNotPassTheseFlags) {
      const QString flag_cmd_format =
          QString("-%1=").arg(QString::fromStdString(std::string{ignore_flag}));
      if (flag->contains(flag_cmd_format)) {
        ignore_this_flag = true;
      }
    }

    if (!ignore_this_flag) {
      result.push_back(*flag);
    }
  }

  return result;
}

}  // namespace orbit_command_line_utils