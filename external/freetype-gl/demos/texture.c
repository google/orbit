/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 *
 * ============================================================================
 *
 * Example showing texture atlas packing
 *
 * ============================================================================
 */
#include <stdio.h>
#include <string.h>

#include "freetype-gl.h"
#include "mat4.h"
#include "shader.h"
#include "vertex-buffer.h"
#include "utf8-utils.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


// ------------------------------------------------------- global variables ---
GLuint shader;
texture_atlas_t *atlas;
vertex_buffer_t * buffer;
mat4 model, view, projection;


// ------------------------------------------------------------------- init ---
void init( void )
{
    atlas = texture_atlas_new( 512, 512, 1 );
    const char *filename = "fonts/Vera.ttf";
    const char * cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
                         "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                         "`abcdefghijklmnopqrstuvwxyz{|}~";
    size_t minsize = 8, maxsize = 27;
    size_t count = maxsize - minsize;
    size_t i, missed = 0;

    for( i=minsize; i < maxsize; ++i )
    {
        texture_font_t * font = texture_font_new_from_file( atlas, i, filename );
        missed += texture_font_load_glyphs( font, cache );
        texture_font_delete( font );
    }

    printf( "Matched font               : %s\n", filename );
    printf( "Number of fonts            : %ld\n", count );
    printf( "Number of glyphs per font  : %ld\n", utf8_strlen(cache) );
    printf( "Number of missed glyphs    : %ld\n", missed );
    printf( "Total number of glyphs     : %ld/%ld\n",
            utf8_strlen(cache)*count - missed, utf8_strlen(cache)*count );
    printf( "Texture size               : %ldx%ld\n", atlas->width, atlas->height );
    printf( "Texture occupancy          : %.2f%%\n",
            100.0*atlas->used/(float)(atlas->width*atlas->height) );

    glClearColor(1,1,1,1);
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
                  0, GL_RED, GL_UNSIGNED_BYTE, atlas->data );

    typedef struct { float x,y,z, u,v, r,g,b,a; } vertex_t;
    vertex_t vertices[4] =  {
        {  0,  0,0, 0,1, 0,0,0,1},
        {  0,512,0, 0,0, 0,0,0,1},
        {512,512,0, 1,0, 0,0,0,1},
        {512,  0,0, 1,1, 0,0,0,1} };
    GLuint indices[6] = { 0, 1, 2, 0,2,3 };
    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );

    shader = shader_load("shaders/v3f-t2f-c4f.vert",
                         "shaders/v3f-t2f-c4f.frag");
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram( shader );
    {
        glUniform1i( glGetUniformLocation( shader, "texture" ),
                     0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ),
                            1, 0, projection.data);
        vertex_buffer_render( buffer, GL_TRIANGLES );
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

    glfwShowWindow( window );
    reshape( window, 512, 512 );

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
