// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "opengldetect.h"

// This needs to be first because if it is not GL/glew.h
// complains about being included after gl.h
// clang-format off
#include "OpenGl.h" // IWYU pragma: keep
// clang-format on

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>

#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

namespace orbit_qt {
namespace {

// Must have created an OpenGl context before calling this. Assumes that both
// major and minor versions of the string are single digits.
std::optional<OpenGlVersion> GetOpenGlVersionViaDirectCall() {
  std::string gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  if (gl_version.size() < 3) {
    return std::nullopt;
  }
  int major_version;
  if (!absl::SimpleAtoi(gl_version.substr(0, 1), &major_version)) {
    return std::nullopt;
  }
  int minor_version;
  if (!absl::SimpleAtoi(gl_version.substr(2, 1), &minor_version)) {
    return std::nullopt;
  }
  return OpenGlVersion{major_version, minor_version};
}

std::optional<OpenGlVersion> ComputeMinimumVersion(std::optional<OpenGlVersion> version_a,
                                                   std::optional<OpenGlVersion> version_b) {
  if (!version_a || !version_b) {
    return std::nullopt;
  }
  if (version_a.value().major < version_b.value().major) {
    return version_a;
  }
  if (version_b.value().major < version_a.value().major) {
    return version_b;
  }
  if (version_a.value().minor < version_b.value().minor) {
    return version_a;
  }
  return version_b;
}
}  // namespace

// Detects the supported version of Desktop OpenGL by (1) requesting the most
// recent version of OpenGL and checking what the system could provide and (2)
// calling glGetString(GL_VERSION). The versions obtained via (1) and (2) are
// compared and the smaller of the two is returned.
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

  std::optional<OpenGlVersion> gl_version_direct = GetOpenGlVersionViaDirectCall();
  if (!gl_version_direct) {
    return std::nullopt;
  }
  OpenGlVersion qt_gl_version{surface.format().majorVersion(), surface.format().minorVersion()};

  return ComputeMinimumVersion(gl_version_direct, qt_gl_version);
}
}  // namespace orbit_qt