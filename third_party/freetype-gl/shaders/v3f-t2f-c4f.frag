/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform sampler2D texture;
void main()
{
    float a = texture2D(texture, gl_TexCoord[0].xy).r;
    gl_FragColor = vec4(gl_Color.rgb, gl_Color.a*a);
}
