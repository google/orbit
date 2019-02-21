/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 Color;
attribute vec3 vertex;
attribute vec4 color;
void main()
{
    gl_FrontColor = color*Color;
    gl_Position = projection*(view*(model*vec4(vertex,1.0)));
}
