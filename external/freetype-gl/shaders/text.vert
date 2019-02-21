/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform sampler2D tex;
uniform vec3 pixel;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 vertex;
attribute vec4 color;
attribute vec2 tex_coord;
attribute float ashift;
attribute float agamma;

varying vec4 vcolor;
varying vec2 vtex_coord;
varying float vshift;
varying float vgamma;

void main()
{
    vshift = ashift;
    vgamma = agamma;
    vcolor = color;
    vtex_coord = tex_coord;
    gl_Position = projection*(view*(model*vec4(vertex,1.0)));
}
