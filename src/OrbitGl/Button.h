// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BUTTON_H_
#define ORBIT_GL_BUTTON_H_

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

 protected:
  void DoUpdateLayout() override;

 private:
  [[nodiscard]] virtual std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

  float height_ = 0.f;
  std::string label_;
};
}  // namespace orbit_gl

#endif