// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
#define MIZAR_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_

#include <QWidget>
#include <memory>
#include <string_view>

#include "MizarData/MizarPairedData.h"

namespace Ui {
class SamplingWithFrameTrackInputWidget;
}

namespace orbit_mizar {

template <typename PairedData>
class SamplingWithFrameTrackInputWidgetTmpl : public QWidget {
 public:
  explicit SamplingWithFrameTrackInputWidgetTmpl(QWidget* parent);
  ~SamplingWithFrameTrackInputWidgetTmpl() override;

  void Init(const PairedData* data, const QString& name);

 private:
  std::unique_ptr<Ui::SamplingWithFrameTrackInputWidget> ui_;
};

using SamplingWithFrameTrackInputWidget =
    SamplingWithFrameTrackInputWidgetTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar

#endif  // MIZAR_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
