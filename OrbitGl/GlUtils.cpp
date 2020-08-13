// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlUtils.h"

#include "OrbitBase/Logging.h"
#include "absl/strings/str_format.h"
#include "freetype-gl/freetype-gl.h"

void CheckGlError() {
  GLenum errCode = glGetError();
  const char* errString;
  if (errCode != GL_NO_ERROR) {
    errString = reinterpret_cast<const char*>(gluErrorString(errCode));
    LOG("OpenGL ERROR : %s", errString);
  }
}
