// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_MOVE_FILES_PROCESS_H_
#define ORBIT_QT_MOVE_FILES_PROCESS_H_

#include <QObject>
#include <QThread>
#include <filesystem>

namespace orbit_qt {

class MoveFilesProcess : public QObject {
  Q_OBJECT

 public:
  explicit MoveFilesProcess();
  ~MoveFilesProcess() noexcept override;

  void Start();

 signals:
  void moveStarted(const std::filesystem::path& from_dir, const std::filesystem::path& to_dir,
                   size_t number_of_files);
  void moveDone();
  void processFinished();
  // Called if something goes wrong before or after all files are moved.
  void generalError(const std::string& error_message);

 private:
  void Run();
  void TryMoveFilesAndRemoveDirIfNeeded(const std::filesystem::path& src_dir,
                                        const std::filesystem::path& dest_dir);

  void ReportError(const std::string& error_message);

  QThread background_thread_;
};

}  // namespace orbit_qt
#endif  // ORBIT_QT_MOVE_FILES_PROCESS_H_
