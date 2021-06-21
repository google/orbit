// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOVE_FILES_TO_DOCUMENTS_MOVE_FILES_PROCESS_H_
#define MOVE_FILES_TO_DOCUMENTS_MOVE_FILES_PROCESS_H_

#include <QObject>
#include <QString>
#include <QThread>
#include <atomic>
#include <filesystem>
#include <string>

namespace orbit_move_files_to_documents {

class MoveFilesProcess : public QObject {
  Q_OBJECT

 public:
  explicit MoveFilesProcess();
  ~MoveFilesProcess() noexcept override;

  void Start();

  // This method is supposed to be called from another thread in order to early interrupt the
  // migration started with Start().
  void RequestInterruption();

 signals:
  void moveDirectoryStarted(const QString& from_dir_path, const QString& to_dir_path,
                            quint64 number_of_files);
  void moveDirectoryDone();
  void moveFileStarted(const QString& from_path);
  void moveFileDone();
  void processFinished();
  void processInterrupted();
  // Called if something goes wrong before or after all files are moved.
  void generalError(const QString& error_message);

 private:
  void Run();
  void TryMoveFilesAndRemoveDirIfNeeded(const std::filesystem::path& src_dir,
                                        const std::filesystem::path& dest_dir);

  void ReportError(const std::string& error_message);

  QThread background_thread_;
  std::atomic<bool> interruption_requested_ = false;
};

}  // namespace orbit_move_files_to_documents

#endif  // MOVE_FILES_TO_DOCUMENTS_MOVE_FILES_PROCESS_H_
