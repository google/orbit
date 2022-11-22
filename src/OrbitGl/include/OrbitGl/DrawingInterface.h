// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DRAWING_INTERFACE_H_
#define ORBIT_GL_DRAWING_INTERFACE_H_

#include <vector>

namespace orbit_gl {

class DrawingInterface {
 public:
  ~DrawingInterface() = default;

  [[nodiscard]] virtual std::vector<float> GetLayers() const = 0;
  virtual void DrawLayer(float layer, bool picking) const = 0;
  virtual void Draw(bool picking) const = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_DRAWING_INTERFACE_H_
