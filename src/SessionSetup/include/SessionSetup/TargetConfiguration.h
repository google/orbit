// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_TARGET_CONFIGURATION_H_
#define SESSION_SETUP_TARGET_CONFIGURATION_H_

#include <utility>

#include "ClientData/ProcessData.h"
#include "ClientServices/ProcessManager.h"
#include "SessionSetup/Connections.h"

namespace orbit_session_setup {

/*
 * `SshTarget` describes a successful ssh connection to a machine and a selected process. The
 * class is built in `SessionSetupDialog` and mainly used in `OrbitMainWindow`. This class is meant
 * to be constructed and then not modified anymore. Only `SessionSetupDialog` is allowed to modify
 * the members, which is used to move out members for reusing them.
 */
class SshTarget {
  friend class SessionSetupDialog;

 public:
  explicit SshTarget(SshConnection&& connection,
                     std::unique_ptr<orbit_client_data::ProcessData> process)
      : connection_(std::move(connection)), process_(std::move(process)) {
    ORBIT_CHECK(process_ != nullptr);
  }
  [[nodiscard]] const SshConnection* GetConnection() const { return &connection_; }
  [[nodiscard]] orbit_client_data::ProcessData* GetProcess() const { return process_.get(); }

 private:
  SshConnection connection_;
  std::unique_ptr<orbit_client_data::ProcessData> process_;
};

/*
 * LocalTarget describes a successful connection to an OrbitService running on the local machine
 * and a selected process. The class is built in SessionSetupDialog and mainly used in
 * OrbitMainWindow. This class is meant to be constructed and then not modified anymore. Only
 * SessionSetupDialog is allowed to modify the members, which is used to move out members for
 * reusing them.
 */
class LocalTarget {
  friend class SessionSetupDialog;

 public:
  explicit LocalTarget(LocalConnection&& connection,
                       std::unique_ptr<orbit_client_data::ProcessData> process)
      : connection_(std::move(connection)), process_(std::move(process)) {
    ORBIT_CHECK(process_ != nullptr);
  }
  [[nodiscard]] const LocalConnection* GetConnection() const { return &connection_; }
  [[nodiscard]] orbit_client_data::ProcessData* GetProcess() const { return process_.get(); }

 private:
  LocalConnection connection_;
  std::unique_ptr<orbit_client_data::ProcessData> process_;
};

/*
 * FileTarget is a wrapper around a file path and is used to open a capture in OrbitMainWindow. This
 * class is meant to be constructed and then not modified anymore.
 */
class FileTarget {
 public:
  explicit FileTarget(std::filesystem::path capture_file_path)
      : capture_file_path_(std::move(capture_file_path)) {}
  [[nodiscard]] const std::filesystem::path& GetCaptureFilePath() const {
    return capture_file_path_;
  }

 private:
  std::filesystem::path capture_file_path_;
};

using TargetConfiguration = std::variant<SshTarget, LocalTarget, FileTarget>;

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_TARGET_CONFIGURATION_H_