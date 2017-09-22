/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __SHADER_H__
#define __SHADER_H__

#include "opengl.h"

#ifdef __cplusplus
extern "C" {
namespace ftgl {
#endif

/**
 * @file   shader.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup shader Shader
 *
 * Simple functions for loading/building a shader from sources.
 *
 * <b>Example Usage</b>:
 * @code
 * #include "shader.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *     GLuint shader = shader_load("shader.vert", "shader.frag");
 *
 *     return 0;
 * }
 * @endcode
 *
 * @{
 */

/**
 * Read a fragment or vertex shader from a file
 *
 * @param filename file to read shader from
 * @return         a newly-allocated text buffer containing code. This buffer
 *                 must be freed after usage.
 *
 */
  char *
  shader_read( const char *filename );


/**
 * Compile a shader from a text buffer.
 *
 * @param source code of the shader
 * @param type   type of the shader
 *
 * @return a handle on the compiled program
 *
 */
  GLuint
  shader_compile( const char* source,
                  const GLenum type );


/**
 * Load a vertex and fragment shader sources and build program
 *
 * @param  vert_filename vertex shader filename
 * @param  frag_filename fragment shader filename
 *
 * @return a handle on the built program
 *
 */
  GLuint
  shader_load( const char * vert_filename,
               const char * frag_filename );


/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __SHADER_H__ */
