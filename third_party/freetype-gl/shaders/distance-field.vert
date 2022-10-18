/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec4 u_color;

attribute vec3 vertex;
attribute vec2 tex_coord;
attribute vec4 color;

void main(void)
{
    gl_TexCoord[0].xy = tex_coord.xy;
    gl_FrontColor     = color * u_color;
    gl_Position       = u_projection*(u_view*(u_model*vec4(vertex,1.0)));
}
