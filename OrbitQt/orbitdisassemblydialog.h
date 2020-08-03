// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITDISASSEMBLYDIALOG_H
#define ORBITDISASSEMBLYDIALOG_H

#include <DisassemblyReport.h>

#include <QDialog>
#include <functional>
#include <string>

namespace Ui {
class OrbitDisassemblyDialog;
}

class OrbitDisassemblyDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OrbitDisassemblyDialog(QWidget* parent = nullptr);
  ~OrbitDisassemblyDialog() override;

  void SetText(std::string a_Text);
  void SetDisassemblyReport(DisassemblyReport report);

 private:
  Ui::OrbitDisassemblyDialog* ui;
};

#endif  // ORBITDISASSEMBLYDIALOG_H
