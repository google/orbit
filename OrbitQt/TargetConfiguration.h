// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TARGET_CONFIGURATION_H_
#define ORBIT_QT_TARGET_CONFIGURATION_H_

#include <utility>

#include "Connections.h"

namespace orbit_qt {

/*
 * StadiaTarget describes a successful connection to an instance and a selected process. The
 * class is built in ProfilingTargetDialog and mainly used in OrbitMainWindow. This class is meant
 * to be constructed and then not modified anymore. Only ProfilingTargetDialog is allowed to modify
 * the members, which is used to move out members for reusing them.
 */
class StadiaTarget {
  friend class ProfilingTargetDialog;

 public:
  explicit StadiaTarget(StadiaConnection&& connection,
                        std::unique_ptr<ProcessManager> process_manager,
                        std::unique_ptr<ProcessData> process)
      : connection_(std::move(connection)),
        process_manager_(std::move(process_manager)),
        process_(std::move(process)) {
    CHECK(process_manager_ != nullptr);
    CHECK(process_ != nullptr);
  }
  [[nodiscard]] const StadiaConnection* GetConnection() const { return &connection_; }
  [[nodiscard]] ProcessManager* GetProcessManager() const { return process_manager_.get(); }
  [[nodiscard]] ProcessData* GetProcess() const { return process_.get(); }

 private:
  StadiaConnection connection_;
  std::unique_ptr<ProcessManager> process_manager_;
  std::unique_ptr<ProcessData> process_;
};

/*
 * LocalTarget describes a successful connection to an OrbitService running on the local machine
 * and a selected process. The class is built in ProfilingTargetDialog and mainly used in
 * OrbitMainWindow. This class is meant to be constructed and then not modified anymore. Only
 * ProfilingTargetDialog is allowed to modify the members, which is used to move out members for
 * reusing them.
 */
class LocalTarget {
  friend class ProfilingTargetDialog;

 public:
  explicit LocalTarget(LocalConnection&& connection,
                       std::unique_ptr<ProcessManager> process_manager,
                       std::unique_ptr<ProcessData> process)
      : connection_(connection),
        process_manager_(std::move(process_manager)),
        process_(std::move(process)) {
    CHECK(process_manager_ != nullptr);
    CHECK(process_ != nullptr);
  }
  [[nodiscard]] const LocalConnection* GetConnection() const { return &connection_; }
  [[nodiscard]] ProcessManager* GetProcessManager() const { return process_manager_.get(); }
  [[nodiscard]] ProcessData* GetProcess() const { return process_.get(); }

 private:
  LocalConnection connection_;
  std::unique_ptr<ProcessManager> process_manager_;
  std::unique_ptr<ProcessData> process_;
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

using TargetConfiguration = std::variant<StadiaTarget, LocalTarget, FileTarget>;

}  // namespace orbit_qt

#endif  // ORBIT_QT_TARGET_CONFIGURATION_H_