//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlUtils.h"

#include "Log.h"
#include "OrbitCore/Core.h"
#include "freetype-gl.h"

void CheckGlError() {
  GLenum errCode = glGetError();
  const GLubyte* errString;
  if (errCode != GL_NO_ERROR) {
    errString = gluErrorString(errCode);
    ORBIT_LOG(Format("OpenGL ERROR : %s\n", errString));
    PRINT(Format("OpenGL ERROR : %s\n", errString));
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
  mat4* modelView = (mat4*)&matrix[0];
  PRINT_VAR(*modelView);

  // Projection
  glGetFloatv(GL_PROJECTION_MATRIX, matrix);
  mat4* projection = (mat4*)&matrix[0];
  PRINT_VAR(*projection);

  // Texture
  glGetFloatv(GL_TEXTURE_MATRIX, matrix);
  mat4* texture = (mat4*)&matrix[0];
  PRINT_VAR(*texture);
}
