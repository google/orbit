// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_
#define MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_

#include <QMainWindow>
#include <QString>
#include <QWidget>
#include <memory>
#include <string_view>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"

namespace Ui {
class MainWindow;
}

namespace orbit_mizar_widgets {

class MizarMainWindow : public QMainWindow {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;

 public:
  explicit MizarMainWindow(const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison,
                           const Baseline<QString>& baseline_file_name,
                           const Comparison<QString>& comparison_file_name,
                           QWidget* parent = nullptr);
  ~MizarMainWindow() override;

 private:
  std::unique_ptr<Ui::MainWindow> ui_;
};

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_MIZAR_MAIN_WINDOW_H_
