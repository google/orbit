// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_H_
#define ORBIT_GL_GRAPH_TRACK_H_

#include <array>
#include <map>
#include <optional>
#include <string>

#include "Batcher.h"
#include "CoreMath.h"
#include "MultivariateTimeSeries.h"
#include "PickingManager.h"
#include "Timer.h"
#include "Track.h"
#include "Viewport.h"

template <size_t Dimension>
class GraphTrack : public Track {
 public:
  explicit GraphTrack(CaptureViewElement* parent,
                      const orbit_gl::TimelineInfoInterface* timeline_info,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                      std::array<std::string, Dimension> series_names,
                      uint8_t series_value_decimal_digits, std::string series_value_unit,
                      const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] Type GetType() const override { return Type::kGraphTrack; }
  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] float GetLegendHeight() const;
  [[nodiscard]] uint8_t GetNumberOfDecimalDigits() const { return series_.GetValueDecimalDigits(); }

  [[nodiscard]] bool IsCollapsible() const override { return true; }
  [[nodiscard]] bool IsEmpty() const override { return series_.IsEmpty(); }

  virtual void AddValues(uint64_t timestamp_ns, const std::array<double, Dimension>& values) {
    series_.AddValues(timestamp_ns, values);
  }

  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

  void SetSeriesColors(const std::array<Color, Dimension>& series_colors) {
    series_colors_ = series_colors;
  }

  // These are not supported in GraphTracks
  const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }

 protected:
  void DoDraw(Batcher& batcher, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

  void DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer, uint64_t min_tick,
                          uint64_t max_tick, PickingMode picking_mode) override;

  [[nodiscard]] virtual Color GetColor(size_t index) const;
  [[nodiscard]] virtual double GetGraphMaxValue() const { return series_.GetMax(); }
  [[nodiscard]] virtual double GetGraphMinValue() const { return series_.GetMin(); }
  [[nodiscard]] double GetInverseOfGraphValueRange() const {
    if (GetGraphMaxValue() <= GetGraphMinValue()) return 0;
    return 1.0 / (GetGraphMaxValue() - GetGraphMinValue());
  }

  [[nodiscard]] float GetGraphContentBottomY() const {
    return GetPos()[1] + GetSize()[1] - layout_->GetTrackContentBottomMargin();
  }
  [[nodiscard]] float GetGraphContentHeight() const;

  [[nodiscard]] virtual float GetLabelYFromValues(
      const std::array<double, Dimension>& values) const;
  [[nodiscard]] virtual std::string GetLabelTextFromValues(
      const std::array<double, Dimension>& values) const;
  [[nodiscard]] uint32_t GetLegendFontSize(uint32_t indentation_level = 0) const;

  virtual void DrawLabel(Batcher& batcher, TextRenderer& text_renderer, Vec2 target_pos,
                         const std::string& text, const Color& text_color, const Color& font_color);
  virtual void DrawLegend(Batcher& batcher, TextRenderer& text_renderer,
                          const std::array<std::string, Dimension>& series_names,
                          const Color& legend_text_color);
  virtual void DrawSeries(Batcher& batcher, uint64_t min_tick, uint64_t max_tick, float z);

  [[nodiscard]] double RoundPrecision(double value) {
    return std::round(value * std::pow(10, GetNumberOfDecimalDigits())) /
           std::pow(10, GetNumberOfDecimalDigits());
  }

  MultivariateTimeSeries<Dimension> series_;

 private:
  [[nodiscard]] virtual std::string GetLegendTooltips(size_t legend_index) const = 0;
  void DrawSingleSeriesEntry(Batcher& batcher, uint64_t start_tick, uint64_t end_tick,
                             const std::array<float, Dimension>& normalized_values, float z);

  std::optional<std::array<Color, Dimension>> series_colors_;
};

#endif  // ORBIT_GL_GRAPH_TRACK_H_
