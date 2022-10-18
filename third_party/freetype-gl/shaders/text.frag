/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
vec3
energy_distribution( vec4 previous, vec4 current, vec4 next )
{
    float primary   = 1.0/3.0;
    float secondary = 1.0/3.0;
    float tertiary  = 0.0;

    // Energy distribution as explained on:
    // http://www.grc.com/freeandclear.htm
    //
    //  .. v..
    // RGB RGB RGB
    // previous.g + previous.b + current.r + current.g + current.b
    //
    //   . .v. .
    // RGB RGB RGB
    // previous.b + current.r + current.g + current.b + next.r
    //
    //     ..v ..
    // RGB RGB RGB
    // current.r + current.g + current.b + next.r + next.g

    float r =
        tertiary  * previous.g +
        secondary * previous.b +
        primary   * current.r  +
        secondary * current.g  +
        tertiary  * current.b;

    float g =
        tertiary  * previous.b +
        secondary * current.r +
        primary   * current.g  +
        secondary * current.b  +
        tertiary  * next.r;

    float b =
        tertiary  * current.r +
        secondary * current.g +
        primary   * current.b +
        secondary * next.r    +
        tertiary  * next.g;

    return vec3(r,g,b);
}

uniform sampler2D tex;
uniform vec3 pixel;

varying vec4 vcolor;
varying vec2 vtex_coord;
varying float vshift;
varying float vgamma;

void main()
{
    // LCD Off
    if( pixel.z == 1.0)
    {
        float a = texture2D(tex, vtex_coord).r;
        gl_FragColor = vcolor * pow( a, 1.0/vgamma );
        return;
    }

    // LCD On
    vec4 current = texture2D(tex, vtex_coord);
    vec4 previous= texture2D(tex, vtex_coord+vec2(-1.,0.)*pixel.xy);
    vec4 next    = texture2D(tex, vtex_coord+vec2(+1.,0.)*pixel.xy);

    current = pow(current, vec4(1.0/vgamma));
    previous= pow(previous, vec4(1.0/vgamma));

    float r = current.r;
    float g = current.g;
    float b = current.b;

    if( vshift <= 0.333 )
    {
        float z = vshift/0.333;
        r = mix(current.r, previous.b, z);
        g = mix(current.g, current.r,  z);
        b = mix(current.b, current.g,  z);
    }
    else if( vshift <= 0.666 )
    {
        float z = (vshift-0.33)/0.333;
        r = mix(previous.b, previous.g, z);
        g = mix(current.r,  previous.b, z);
        b = mix(current.g,  current.r,  z);
    }
   else if( vshift < 1.0 )
    {
        float z = (vshift-0.66)/0.334;
        r = mix(previous.g, previous.r, z);
        g = mix(previous.b, previous.g, z);
        b = mix(current.r,  previous.b, z);
    }

   float t = max(max(r,g),b);
   vec4 color = vec4(vcolor.rgb, (r+g+b)/3.0);
   color = t*color + (1.0-t)*vec4(r,g,b, min(min(r,g),b));
   gl_FragColor = vec4( color.rgb, vcolor.a*color.a);


//    gl_FragColor = vec4(pow(vec3(r,g,b),vec3(1.0/vgamma)),a);

    /*
    vec3 color = energy_distribution(previous, vec4(r,g,b,1), next);
    color = pow( color, vec3(1.0/vgamma));

    vec3 color = vec3(r,g,b); //pow( vec3(r,g,b), vec3(1.0/vgamma));
    gl_FragColor.rgb = color;
    gl_FragColor.a = (color.r+color.g+color.b)/3.0 * vcolor.a;
    */

//    gl_FragColor = vec4(pow(vec3(r,g,b),vec3(1.0/vgamma)),a);
    //gl_FragColor = vec4(r,g,b,a);
}
