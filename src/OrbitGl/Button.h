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

 private:
  [[nodiscard]] virtual std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override;

  float height_ = 0.f;
};
}  // namespace orbit_gl

#endif