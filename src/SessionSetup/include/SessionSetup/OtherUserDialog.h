// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_OTHER_USER_DIALOG_H_
#define SESSION_SETUP_OTHER_USER_DIALOG_H_

#include <QDialog>
#include <QString>
#include <QWidget>
#include <memory>

#include "OrbitBase/Result.h"

namespace Ui {
class OtherUserDialog;  // IWYU pragma: keep
}

namespace orbit_session_setup {

class OtherUserDialog : public QDialog {
 public:
  explicit OtherUserDialog(const QString& user_name, QWidget* parent = nullptr);
  ~OtherUserDialog() override;

  ErrorMessageOr<void> Exec();

 private:
  std::unique_ptr<Ui::OtherUserDialog> ui_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_OTHER_USER_DIALOG_H_