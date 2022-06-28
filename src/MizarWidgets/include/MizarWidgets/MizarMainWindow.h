// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_
#define MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

namespace orbit_mizar_widgets {

class MizarMainWindow : public QMainWindow {
 public:
  MizarMainWindow();
  ~MizarMainWindow() override;

 private:
  std::unique_ptr<Ui::MainWindow> ui_;
};

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_
