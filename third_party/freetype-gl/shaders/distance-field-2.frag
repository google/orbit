/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform sampler2D u_texture;

void main(void)
{
    float dist = texture2D(u_texture, gl_TexCoord[0].st).r;
    float width = fwidth(dist);
    float alpha = smoothstep(0.5-width, 0.5+width, dist);
    gl_FragColor = vec4(gl_Color.rgb, alpha*gl_Color.a);
}


