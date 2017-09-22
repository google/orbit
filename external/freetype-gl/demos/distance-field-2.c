/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"
#include "shader.h"
#include "mat4.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

double total_time = 0.0;


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;


// ------------------------------------------------------- global variables ---
GLuint shader;
vertex_buffer_t *buffer;
texture_atlas_t *atlas;
mat4  model, view, projection;


// --------------------------------------------------------------- add_text ---
vec4
add_text( vertex_buffer_t * buffer, texture_font_t * font,
          char *text, vec4 * color, vec2 * pen )
{
    vec4 bbox = {{0,0,0,0}};
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i = 0; i < strlen(text); ++i )
    {
        glfwSetTime(total_time);
        texture_glyph_t *glyph = texture_font_get_glyph( font, text + i );
        total_time += glfwGetTime();
        if( glyph != NULL )
        {
            float kerning = 0.0f;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text + i - 1 );
            }
            pen->x += kerning;
            int x0  = (int)( pen->x + glyph->offset_x );
            int y0  = (int)( pen->y + glyph->offset_y );
            int x1  = (int)( x0 + glyph->width );
            int y1  = (int)( y0 - glyph->height );
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            GLuint indices[6] = {0,1,2, 0,2,3};
            vertex_t vertices[4] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                                     { x0,y1,0,  s0,t1,  r,g,b,a },
                                     { x1,y1,0,  s1,t1,  r,g,b,a },
                                     { x1,y0,0,  s1,t0,  r,g,b,a } };
            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            pen->x += glyph->advance_x;

            if  (x0 < bbox.x)                bbox.x = x0;
            if  (y1 < bbox.y)                bbox.y = y1;
            if ((x1 - bbox.x) > bbox.width)  bbox.width  = x1-bbox.x;
            if ((y0 - bbox.y) > bbox.height) bbox.height = y0-bbox.y;
        }
    }
    return bbox;
}


// ------------------------------------------------------------------- init ---
void init( void )
{
    texture_font_t *font = 0;
    atlas = texture_atlas_new( 512, 512, 1 );
    const char * filename = "fonts/Vera.ttf";
    char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";
    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vec2 pen = {{0,0}};
    vec4 black = {{1,1,1,1}};
    font = texture_font_new_from_file( atlas, 48, filename );
    font->rendermode = RENDER_SIGNED_DISTANCE_FIELD;
    vec4 bbox = add_text( buffer, font, text, &black, &pen );
    size_t i;
    vector_t * vertices = buffer->vertices;
    for( i=0; i< vector_size(vertices); ++i )
    {
        vertex_t * vertex = (vertex_t *) vector_get(vertices,i);
        vertex->x -= (int)(bbox.x + bbox.width/2);
        vertex->y -= (int)(bbox.y + bbox.height/2);
    }

    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
                  0, GL_RED, GL_UNSIGNED_BYTE, atlas->data );

    shader = shader_load( "shaders/distance-field.vert",
                          "shaders/distance-field-2.frag" );
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor( 1, 1, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLint viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );
    GLint width  = viewport[2];
    GLint height = viewport[3];

    srand(4);
    vec4 color = {{0.067,0.333, 0.486, 1.0}};
    size_t i;
    for( i=0; i<40; ++i)
    {
        float scale = .25 + 4.75 * pow(rand()/(float)(RAND_MAX),2);
        float angle = 90*(rand()%2);
        float x = (.05 + .9*(rand()/(float)(RAND_MAX)))*width;
        float y = (-.05 + .9*(rand()/(float)(RAND_MAX)))*height;
        float a =  0.1+.8*(pow((1.0-scale/5),2));

        mat4_set_identity( &model );
        mat4_rotate( &model, angle,0,0,1);
        mat4_scale( &model, scale, scale, 1);
        mat4_translate( &model, x, y, 0);

        glUseProgram( shader );
        {
            glUniform1i( glGetUniformLocation( shader, "u_texture" ),
                         0 );
            glUniform4f( glGetUniformLocation( shader, "u_color" ),
                         color.r, color.g, color.b, a);
            glUniformMatrix4fv( glGetUniformLocation( shader, "u_model" ),
                                1, 0, model.data);
            glUniformMatrix4fv( glGetUniformLocation( shader, "u_view" ),
                                1, 0, view.data);
            glUniformMatrix4fv( glGetUniformLocation( shader, "u_projection" ),
                                1, 0, projection.data);
            vertex_buffer_render( buffer, GL_TRIANGLES );
        }
    }

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    fputs( description, stderr );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    GLFWwindow* window;
    char* screenshot_path = NULL;

    if (argc > 1)
    {
        if (argc == 3 && 0 == strcmp( "--screenshot", argv[1] ))
            screenshot_path = argv[2];
        else
        {
            fprintf( stderr, "Unknown or incomplete parameters given\n" );
            exit( EXIT_FAILURE );
        }
    }

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    window = glfwCreateWindow( 800, 600, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

    init();

    fprintf(stderr, "Total time to generate distance map: %fs\n", total_time);

    glfwShowWindow( window );
    {
        int pixWidth, pixHeight;
        glfwGetFramebufferSize( window, &pixWidth, &pixHeight );
        reshape( window, pixWidth, pixHeight );
    }

    glfwSetTime(0.0);

    while (!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glDeleteTextures( 1, &atlas->id );
    atlas->id = 0;
    texture_atlas_delete( atlas );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
