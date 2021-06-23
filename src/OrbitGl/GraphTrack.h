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
  explicit GraphTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                      orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                      std::array<std::string, Dimension> series_names,
                      const orbit_client_model::CaptureData* capture_data,
                      uint32_t indentation_level = 0);

  [[nodiscard]] Type GetType() const override { return Type::kGraphTrack; }
  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] float GetLegendHeight() const;

  [[nodiscard]] bool IsCollapsible() const override { return true; }
  [[nodiscard]] bool IsEmpty() const override { return series_.IsEmpty(); }

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

  void AddValues(uint64_t timestamp_ns, const std::array<double, Dimension>& values) {
    series_.AddValues(timestamp_ns, values);
  }
  void SetLabelUnit(std::string label_unit) { series_.SetValueUnit(label_unit); }
  void SetNumberOfDecimalDigits(uint8_t value_decimal_digits) {
    series_.SetNumberOfDecimalDigits(value_decimal_digits);
  }
  void SetSeriesColors(const std::array<Color, Dimension>& series_colors) {
    series_colors_ = series_colors;
  }

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllChains() const override;
  [[nodiscard]] std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() const override {
    return GetAllChains();
  }

 protected:
  [[nodiscard]] virtual Color GetColor(size_t index) const;
  [[nodiscard]] virtual double GetGraphMaxValue() const { return series_.GetMax(); }
  [[nodiscard]] virtual double GetGraphMinValue() const { return series_.GetMin(); }
  [[nodiscard]] double GetGraphInverseValueRange() const {
    if (GetGraphMaxValue() <= GetGraphMinValue()) return 0;
    return 1.0 / (GetGraphMaxValue() - GetGraphMinValue());
  }

  [[nodiscard]] virtual float GetLabelYFromValues(
      const std::array<double, Dimension>& values) const;
  [[nodiscard]] virtual std::string GetLabelTextFromValues(
      const std::array<double, Dimension>& values) const;

  virtual void DrawLabel(Batcher& batcher, TextRenderer& text_renderer, Vec2 target_pos,
                         const std::string& text, const Color& text_color, const Color& font_color,
                         float z);
  virtual void DrawLegend(Batcher& batcher, TextRenderer& text_renderer,
                          const std::array<std::string, Dimension>& series_names,
                          const Color& legend_text_color, float z);
  virtual void DrawSeries(Batcher* batcher, uint64_t min_tick, uint64_t max_tick, float z);

  MultivariateTimeSeries<Dimension> series_;

 private:
  void DrawSingleSeriesEntry(Batcher* batcher, uint64_t start_tick, uint64_t end_tick,
                             const std::array<float, Dimension>& normalized_values, float z);

  std::optional<std::array<Color, Dimension>> series_colors_;
};

#endif  // ORBIT_GL_GRAPH_TRACK_H_