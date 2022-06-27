// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_MIZAR_MAIN_WINDOW_H_
#define MIZAR_MIZAR_MAIN_WINDOW_H_

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

namespace orbit_mizar {

class MizarMainWindow : public QMainWindow {
 public:
  MizarMainWindow();
  ~MizarMainWindow() override;
  void Setup();

 private:
  std::unique_ptr<Ui::MainWindow> ui_;
};

}  // namespace orbit_mizar

#endif  // MIZAR_MIZAR_MAIN_WINDOW_H_
