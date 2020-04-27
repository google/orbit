// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITQT_ORBIT_STARTUP_WINDOW_H_
#define ORBITQT_ORBIT_STARTUP_WINDOW_H_

#include <QDialog>
#include <QPointer>
#include <QWidget>
#include <optional>

#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpInstanceItemModel.h"

class OrbitStartupWindow : public QDialog {
 public:
  explicit OrbitStartupWindow(QWidget* parent = nullptr);
  int Run(std::string* ip_address);

 private:
  void ReloadInstances();

  std::optional<GgpClient> ggp_client_;
  std::optional<GgpInstance> chosen_instance_;
  QPointer<GgpInstanceItemModel> model_;
};

#endif  // ORBITQT_ORBIT_STARTUP_WINDOW_H_