// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_ABOUT_DIALOG_H_
#define ORBIT_QT_ORBIT_ABOUT_DIALOG_H_

#include <QDialog>
#include <QString>
#include <QWidget>
#include <memory>

namespace Ui {
class OrbitAboutDialog;
}
namespace orbit_qt {

class OrbitAboutDialog : public QDialog {
 public:
  explicit OrbitAboutDialog(QWidget* parent = nullptr);
  ~OrbitAboutDialog() override;

  void SetLicenseText(const QString& text);
  void SetVersionString(const QString& version);
  void SetBuildInformation(const QString& build_info);
  void SetOpenGlRenderer(const QString& opengl_renderer, bool software_rendering);

 private:
  std::unique_ptr<Ui::OrbitAboutDialog> ui_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_ORBIT_ABOUT_DIALOG_H_
