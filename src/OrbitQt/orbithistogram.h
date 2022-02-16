// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_HISTOGRAM_H_
#define ORBIT_HISTOGRAM_H_

#include <qevent.h>
#include <qpainter.h>
#include <qwidget.h>

#include <optional>
#include <string>

#include "Statistics/Histogram.h"

class OrbitHistogram : public QWidget {
 public:
  explicit OrbitHistogram(QWidget* /* histogram_tab */) {}

  void SetHistogram(std::optional<orbit_statistics::Histogram> histogram,
                    std::string function_name);

 protected:
  void paintEvent(QPaintEvent* /*event*/) override;

 private:
  std::optional<orbit_statistics::Histogram> histogram_;
  std::optional<std::string> function_name_;
};

#endif  // ORBIT_HISTOGRAM_H_
