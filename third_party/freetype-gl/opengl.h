/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __OPEN_GL_H__
#define __OPEN_GL_H__
#include "config.h"

#if defined(GL_WITH_GLAD)
#   include <glad/glad.h>
#else
#if defined(__APPLE__)
#   include "TargetConditionals.h"
#   if TARGET_OS_SIMULATOR || TARGET_OS_IPHONE
#     if defined(FREETYPE_GL_ES_VERSION_3_0)
#       include <OpenGLES/ES3/gl.h>
#     else
#       include <OpenGLES/ES2/gl.h>
#     endif
#   else
#     include <OpenGL/gl.h>
#   endif
#elif defined(_WIN32) || defined(_WIN64)
#  include <GL/glew.h>
#  include <GL/wglew.h>
#elif defined(__ANDROID_API__)
#  if defined(FREETYPE_GL_ES_VERSION_3_0)
#    include <GLES3/gl3.h>
#  else
#    include <GLES2/gl2.h>
#  endif
#else
#if defined(FREETYPE_GL_USE_GLEW)
#  include <GL/glew.h>
#endif
#  include <GL/gl.h>
#endif
#endif /* GL_WITH_GLAD */

#endif /* OPEN_GL_H */
