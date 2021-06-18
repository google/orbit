// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_MOVE_FILES_DIALOG_H_
#define ORBIT_QT_MOVE_FILES_DIALOG_H_

#include <QCloseEvent>
#include <QDialog>
#include <QWidget>
#include <memory>

namespace Ui {
// forward declaration of class with same name in a different namespace
class MoveFilesDialog;  // NOLINT
}  // namespace Ui

namespace orbit_qt {

class MoveFilesDialog : public QDialog {
 public:
  explicit MoveFilesDialog();
  ~MoveFilesDialog() noexcept override;

  void AddText(std::string_view text);
  void EnableCloseButton();

 protected:
  void closeEvent(QCloseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  std::unique_ptr<class Ui::MoveFilesDialog> ui_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_MOVE_FILES_DIALOG_H_