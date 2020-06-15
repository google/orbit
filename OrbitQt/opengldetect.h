// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_OPEN_GL_DETECT__H_
#define ORBIT_QT_OPEN_GL_DETECT__H_

#include <optional>

namespace OrbitQt {

struct OpenGlVersion {
  int major;
  int minor;
};

std::optional<OpenGlVersion> DetectOpenGlVersion();

}  // namespace OrbitQt

#endif  // ORBIT_QT_OPEN_GL_DETECT__H_
