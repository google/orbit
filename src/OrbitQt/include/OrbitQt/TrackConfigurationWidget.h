// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TRACK_CONFIGURATION_WIDGET_H_
#define ORBIT_QT_TRACK_CONFIGURATION_WIDGET_H_

#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>

#include "OrbitGl/TrackManager.h"
#include "OrbitQt/TrackTypeItemModel.h"

namespace Ui {
class TrackConfigurationWidget;
}

namespace orbit_qt {

class TrackConfigurationWidget : public QWidget {
  Q_OBJECT

 public:
  explicit TrackConfigurationWidget(QWidget* parent = nullptr);
  ~TrackConfigurationWidget() override;
  void SetTrackManager(orbit_gl::TrackManager* track_manager);

 private:
  std::unique_ptr<Ui::TrackConfigurationWidget> ui_;
  TrackTypeItemModel track_type_item_model_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_TRACK_CONFIGURATION_WIDGET_H_