/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
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

typedef struct {
    float x, y, zoom;
} viewport_t;


// ------------------------------------------------------- global variables ---
GLuint shader;
vertex_buffer_t *buffer;
texture_atlas_t *atlas;
mat4  model, view, projection;
viewport_t viewport = {0,0,1};


// ------------------------------------------------------------------- init ---
void init( void )
{
    texture_font_t * font;
    const char *filename = "fonts/Vera.ttf";
    const char * cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
                         "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                         "`abcdefghijklmnopqrstuvwxyz{|}~";

    atlas = texture_atlas_new( 512, 512, 1 );
    font = texture_font_new_from_file( atlas, 72, filename );
    font->rendermode = RENDER_SIGNED_DISTANCE_FIELD;

    glfwSetTime(total_time);
    texture_font_load_glyphs( font, cache );
    total_time += glfwGetTime();

    texture_font_delete( font );

    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
                      0, GL_RED, GL_UNSIGNED_BYTE, atlas->data );

    GLuint indices[6] = {0,1,2, 0,2,3};
    vertex_t vertices[4] = { { 0,0,0,  0,1,  1,1,1,1 },
                             { 0,1,0,  0,0,  1,1,1,1 },
                             { 1,1,0,  1,0,  1,1,1,1 },
                             { 1,0,0,  1,1,  1,1,1,1 } };
    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );

    shader = shader_load( "shaders/distance-field.vert",
                           "shaders/distance-field.frag" );
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor(0.5,0.5,0.5,1.00);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->id);
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GLint v[4];
    glGetIntegerv( GL_VIEWPORT, v );
    GLint width  = v[2];
    GLint height = v[3];

    vec4 color = {{1.0, 1.0, 1.0, 1.0 }};

    mat4_set_identity( &model );
    mat4_scale( &model, width * viewport.zoom, height * viewport.zoom, 0 );
    mat4_translate( &model, viewport.x, viewport.y, 0);

    glUseProgram( shader );
    {
        glUniform1i( glGetUniformLocation( shader, "u_texture" ),
                     0);
        glUniform4f( glGetUniformLocation( shader, "u_color" ),
                     color.r, color.g, color.b, color.a);
        glUniformMatrix4fv( glGetUniformLocation( shader, "u_model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "u_view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "u_projection" ),
                            1, 0, projection.data);

        vertex_buffer_render( buffer, GL_TRIANGLES );
    }

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------- cursor_motion ---
void cursor_motion( GLFWwindow* window, double x, double y )
{
    int v[4];
    static int _x=-1, _y=-1;

    if( GLFW_PRESS == glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) )
    {
        if( (_x == -1) && (_y == -1) )
        {
            _x = x; _y = y;
            return;
        }
        int dy = y - _y;
        if (dy < 0)
        {
            viewport.zoom *= 1.05;
        }
        else
        {
            viewport.zoom /= 1.05;
        }
        _x = x; _y = y;
    }

    glGetIntegerv( GL_VIEWPORT, v );
    GLfloat width = v[2], height = v[3];
    float nx = min( max( x/width, 0.0), 1.0 );
    float ny = 1-min( max( y/height, 0.0), 1.0 );
    viewport.x = nx*width*(1-viewport.zoom);
    viewport.y = ny*height*(1-viewport.zoom);
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

    window = glfwCreateWindow( 512, 512, argv[0], NULL, NULL );

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
    glfwSetCursorPosCallback( window, cursor_motion );

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
