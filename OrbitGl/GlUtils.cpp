// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlUtils.h"

#include "Log.h"
#include "OpenGl.h"
#include "OrbitBase/Logging.h"
#include "PrintVar.h"
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

void OutputGlMatrices() {
  int i;
  glGetIntegerv(GL_MATRIX_MODE, &i);
  /*
  #define GL_MODELVIEW                      0x1700
  #define GL_PROJECTION                     0x1701
  #define GL_TEXTURE                        0x1702
  */

  ORBIT_LOG("Current matrix mode is : ");
  switch (i) {
    case GL_MODELVIEW:
      ORBIT_LOG("GL_MODELVIEW");
      break;
    case GL_PROJECTION:
      ORBIT_LOG("GL_PROJECTION");
      break;
    case GL_TEXTURE:
      ORBIT_LOG("GL_TEXTURE");
      break;
  }

  ORBIT_LOG("\n\nMatrices are: \n");

  // ModelView
  GLfloat matrix[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
  mat4* modelView = reinterpret_cast<mat4*>(&matrix[0]);
  PRINT_VAR(*modelView);

  // Projection
  glGetFloatv(GL_PROJECTION_MATRIX, matrix);
  mat4* projection = reinterpret_cast<mat4*>(&matrix[0]);
  PRINT_VAR(*projection);

  // Texture
  glGetFloatv(GL_TEXTURE_MATRIX, matrix);
  mat4* texture = reinterpret_cast<mat4*>(&matrix[0]);
  PRINT_VAR(*texture);
}
