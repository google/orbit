// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

#include <stdint.h>

class TimeGraphLayout {
 public:
  [[nodiscard]] virtual float GetTextBoxHeight() const = 0;
  [[nodiscard]] virtual float GetTextCoresHeight() const = 0;
  [[nodiscard]] virtual float GetThreadStateTrackHeight() const = 0;
  [[nodiscard]] virtual float GetEventTrackHeightFromTid(uint32_t tid) const = 0;
  [[nodiscard]] virtual float GetTrackContentBottomMargin() const = 0;
  [[nodiscard]] virtual float GetTrackContentTopMargin() const = 0;
  [[nodiscard]] virtual float GetTrackLabelOffsetX() const = 0;
  [[nodiscard]] virtual float GetSliderWidth() const = 0;
  [[nodiscard]] virtual float GetMinSliderLength() const = 0;
  [[nodiscard]] virtual float GetSliderResizeMargin() const = 0;
  [[nodiscard]] virtual float GetTimeBarHeight() const = 0;
  [[nodiscard]] virtual float GetTrackTabWidth() const = 0;
  [[nodiscard]] virtual float GetTrackTabHeight() const = 0;
  [[nodiscard]] virtual float GetTrackTabOffset() const = 0;
  [[nodiscard]] virtual float GetTrackIndentOffset() const = 0;
  [[nodiscard]] virtual float GetCollapseButtonSize(int indentation_level) const = 0;
  [[nodiscard]] virtual float GetCollapseButtonOffset() const = 0;
  [[nodiscard]] virtual float GetRoundingRadius() const = 0;
  [[nodiscard]] virtual float GetRoundingNumSides() const = 0;
  [[nodiscard]] virtual float GetTextOffset() const = 0;
  [[nodiscard]] virtual float GetLeftMargin() const = 0;
  [[nodiscard]] virtual float GetRightMargin() const = 0;
  [[nodiscard]] virtual float GetMinButtonSize() const = 0;
  [[nodiscard]] virtual float GetButtonWidth() const = 0;
  [[nodiscard]] virtual float GetButtonHeight() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenTracks() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenTracksAndTimeline() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenCores() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenGpuDepths() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenThreadPanes() const = 0;
  [[nodiscard]] virtual float GetSpaceBetweenSubtracks() const = 0;
  [[nodiscard]] virtual float GetGenericFixedSpacerWidth() const = 0;
  [[nodiscard]] virtual float GetThreadDependencyArrowHeadWidth() const = 0;
  [[nodiscard]] virtual float GetThreadDependencyArrowHeadHeight() const = 0;
  [[nodiscard]] virtual float GetThreadDependencyArrowBodyWidth() const = 0;
  [[nodiscard]] virtual float GetScale() const = 0;
  [[nodiscard]] virtual bool GetDrawTrackBackground() const = 0;
  [[nodiscard]] virtual uint32_t GetFontSize() const = 0;
  [[nodiscard]] virtual int GetMaxLayoutingLoops() const = 0;
  virtual void SetScale(float value) = 0;

  ~TimeGraphLayout() = default;
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_
