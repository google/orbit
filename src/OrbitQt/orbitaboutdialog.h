// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_ABOUT_DIALOG_H_
#define ORBIT_QT_ORBIT_ABOUT_DIALOG_H_

#include <QDialog>
#include <QString>
#include <QWidget>
#include <memory>

#include "ui_orbitaboutdialog.h"

namespace orbit_qt {

class OrbitAboutDialog : public QDialog {
 public:
  explicit OrbitAboutDialog(QWidget* parent = nullptr);

  void SetLicenseText(const QString& text);
  void SetVersionString(const QString& version);
  void SetBuildInformation(const QString& build_info);

  ~OrbitAboutDialog() noexcept override = default;

 private:
  std::unique_ptr<Ui::OrbitAboutDialog> ui_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_ORBIT_ABOUT_DIALOG_H_
