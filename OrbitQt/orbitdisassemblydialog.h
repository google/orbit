// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITDISASSEMBLYDIALOG_H
#define ORBITDISASSEMBLYDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class OrbitDisassemblyDialog;
}

class OrbitDisassemblyDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OrbitDisassemblyDialog(QWidget* parent = nullptr);
  ~OrbitDisassemblyDialog() override;

  void SetText(const std::string& a_Text);
  void SetLineToHitRatio(
      const std::function<double(size_t)>& line_to_hit_ratio);

 private:
  Ui::OrbitDisassemblyDialog* ui;
};

#endif  // ORBITDISASSEMBLYDIALOG_H
