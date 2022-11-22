// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/opengldetect.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

namespace orbit_qt {

// Detects the supported version of Desktop OpenGL by requesting the most
// recent version of OpenGL and checking what the system could provide.
std::optional<OpenGlVersion> DetectOpenGlVersion() {
  QSurfaceFormat format{};
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setVersion(4, 6);

  QOffscreenSurface surface{};
  surface.setFormat(format);
  surface.create();

  if (!surface.isValid()) {
    return std::nullopt;
  }

  QOpenGLContext gl_context{};
  gl_context.setFormat(format);
  gl_context.create();
  gl_context.makeCurrent(&surface);

  if (!gl_context.isValid()) return std::nullopt;

  return OpenGlVersion{gl_context.format().majorVersion(), gl_context.format().minorVersion(),
                       gl_context.isOpenGLES()};
}
}  // namespace orbit_qt