// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "opengldetect.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace OrbitQt {

// Detects the supported version of Desktop OpenGL by requesting the most
// recent version of OpenGL and checking what the system could provide.
// OpenGL ES is automatically ignored.
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

  // We are trying to detect Desktop OpenGL. So if Qt falls back to OpenGL ES,
  // Desktop OpenGL will not be available.
  if (!gl_context.isValid() || gl_context.isOpenGLES()) {
    return std::nullopt;
  }

  return OpenGlVersion{gl_context.format().majorVersion(),
                       gl_context.format().minorVersion()};
}

}  // namespace OrbitQt
