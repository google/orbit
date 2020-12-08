// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_ACCESSIBILITY_H_
#define ORBIT_GL_TRACK_ACCESSIBILITY_H_

#include "OrbitGlAccessibility.h"

class Track;

namespace orbit_gl {

class TrackContentAccessibility : public GlA11yControlInterface {
 public:
  TrackContentAccessibility(Track* track) : track_(track){};

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const GlA11yControlInterface* AccessibleChild(int) const override {
    return nullptr;
  }
  [[nodiscard]] const GlA11yControlInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_gl::A11yRole AccessibleRole() const override {
    return orbit_gl::A11yRole::Grouping;
  }
  [[nodiscard]] orbit_gl::A11yRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_gl::A11yState AccessibleState() const override;

 private:
  Track* track_;
};

class TrackTabAccessibility : public GlA11yControlInterface {
 public:
  TrackTabAccessibility(Track* track) : track_(track){};

  [[nodiscard]] int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] const GlA11yControlInterface* AccessibleChild(int) const override {
    return nullptr;
  }
  [[nodiscard]] const GlA11yControlInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_gl::A11yRole AccessibleRole() const override {
    return orbit_gl::A11yRole::PageTab;
  }
  [[nodiscard]] orbit_gl::A11yRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_gl::A11yState AccessibleState() const override;

 private:
  Track* track_;
};

class TrackAccessibility : public GlA11yControlInterface {
 public:
  TrackAccessibility(Track* track) : track_(track), content_(track_), tab_(track_){};

  [[nodiscard]] int AccessibleChildCount() const override { return 2; }
  [[nodiscard]] const GlA11yControlInterface* AccessibleChild(int index) const override {
    if (index == 0) {
      return &tab_;
    } else {
      return &content_;
    }
  }
  [[nodiscard]] const GlA11yControlInterface* AccessibleParent() const override;

  [[nodiscard]] std::string AccessibleName() const override;
  [[nodiscard]] orbit_gl::A11yRole AccessibleRole() const override {
    return orbit_gl::A11yRole::Grouping;
  }
  [[nodiscard]] orbit_gl::A11yRect AccessibleLocalRect() const override;
  [[nodiscard]] orbit_gl::A11yState AccessibleState() const override;

 private:
  Track* track_;
  TrackContentAccessibility content_;
  TrackTabAccessibility tab_;
};

}  // namespace orbit_gl

#endif