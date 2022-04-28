// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BUTTON_H_
#define ORBIT_GL_BUTTON_H_

#include <functional>

#include "CaptureViewElement.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

namespace orbit_gl {
class Button : public CaptureViewElement {
 public:
  explicit Button(CaptureViewElement* parent, const Viewport* viewport,
                  const TimeGraphLayout* layout);

  [[nodiscard]] float GetHeight() const override { return height_; }
  [[nodiscard]] uint32_t GetLayoutFlags() const override { return LayoutFlags::kNone; }

  void SetHeight(float height);

  void SetLabel(const std::string& label);
  [[nodiscard]] const std::string& GetLabel() const { return label_; }

  using MouseReleaseCallback = std::function<void(Button*)>;
  void SetMouseReleaseCallback(MouseReleaseCallback callback);

  void OnRelease() override;

  static const Color kHighlightColor;
  static const Color kBaseColor;
  static const Color kTextColor;

 protected:
  void DoUpdateLayout() override;
  void DoDraw(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
              const DrawContext& draw_context) override;

 private:
  [[nodiscard]] virtual std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

  float height_ = 0.f;
  std::string label_;

  MouseReleaseCallback mouse_release_callback_ = nullptr;
};
}  // namespace orbit_gl

#endif