// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CommandLineUtils/CommandLineUtils.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/commandlineflag.h>
#include <absl/flags/flag.h>

#include <QString>
#include <array>
#include <string_view>

#include "ClientFlags/ClientFlags.h"

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
  const std::array kDoNotPassTheseFlags{
      absl::GetFlagReflectionHandle(FLAGS_ssh_hostname).Name(),
      absl::GetFlagReflectionHandle(FLAGS_ssh_port).Name(),
      absl::GetFlagReflectionHandle(FLAGS_ssh_user).Name(),
      absl::GetFlagReflectionHandle(FLAGS_ssh_known_host_path).Name(),
      absl::GetFlagReflectionHandle(FLAGS_ssh_key_path).Name(),
      absl::GetFlagReflectionHandle(FLAGS_ssh_target_process).Name()};

  for (const auto& flag : flags) {
    bool ignore_this_flag = false;

    for (auto ignore_flag : kDoNotPassTheseFlags) {
      const QString flag_cmd_format =
          QString("-%1=").arg(QString::fromStdString(std::string{ignore_flag}));
      if (flag.contains(flag_cmd_format)) {
        ignore_this_flag = true;
      }
    }

    if (!ignore_this_flag) {
      result.push_back(flag);
    }
  }

  return result;
}

}  // namespace orbit_command_line_utils