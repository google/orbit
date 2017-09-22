/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 *
 * ============================================================================
 *
 * Example showing gamma influence
 *
 * ============================================================================
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "freetype-gl.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"
#include "shader.h"
#include "mat4.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;
    float r, g, b, a;
} vertex_t;

// ------------------------------------------------------- global variables ---
font_manager_t * font_manager;
text_buffer_t *buffer;
vertex_buffer_t *background;
GLuint background_shader;
GLuint text_shader;
mat4 model, view, projection;


// ------------------------------------------------------------------- init ---
void init( void )
{
    text_shader = shader_load( "shaders/text.vert",
                               "shaders/text.frag" );

    font_manager = font_manager_new( 512, 512, LCD_FILTERING_OFF );

    buffer = text_buffer_new( );
    vec4 white = {{1.0, 1.0, 1.0, 1.0}};
    vec4 black = {{0.0, 0.0, 0.0, 1.0}};
    vec4 none  = {{1.0, 1.0, 1.0, 0.0}};

    markup_t markup;
    markup.family  = "fonts/Vera.ttf";
    markup.size    = 15.0;
    markup.bold    = 0;
    markup.italic  = 0;
    markup.spacing = 0.0;
    markup.gamma   = 1.0;
    markup.foreground_color    = white;
    markup.background_color    = none;
    markup.underline           = 0;
    markup.underline_color     = none;
    markup.overline            = 0;
    markup.overline_color      = none;
    markup.strikethrough       = 0;
    markup.strikethrough_color = none;
    markup.font = font_manager_get_from_markup( font_manager, &markup );

    size_t i;
    vec2 pen;
    pen.x = 32;
    pen.y = 508;

    char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789\n";
    for( i=0; i < 14; ++i )
    {
        markup.gamma = 0.75 + 1.5*i*(1.0/14);
        text_buffer_add_text( buffer, &pen, &markup, text, 0 );
    }
    pen.x = 32;
    pen.y = 252;
    markup.foreground_color = black;
    for( i=0; i < 14; ++i )
    {
        markup.gamma = 0.75 + 1.5*i*(1.0/14);
        text_buffer_add_text( buffer, &pen, &markup, text, 0 );
    }

    glGenTextures( 1, &font_manager->atlas->id );
    glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, font_manager->atlas->width,
        font_manager->atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE,
        font_manager->atlas->data );

    background = vertex_buffer_new( "vertex:3f,color:4f" );
    vertex_t vertices[4*2] = { {  0,  0,0, 1,1,1,1}, {  0,256,0, 1,1,1,1},
                               {512,256,0, 1,1,1,1}, {512,  0,0, 1,1,1,1},
                               {0,  256,0, 0,0,0,1}, {  0,512,0, 0,0,0,1},
                               {512,512,0, 0,0,0,1}, {512,256,0, 0,0,0,1} };
    GLuint indices[4*3] = { 0,1,2, 0,2,3, 4,5,6, 4,6,7 };
    vertex_buffer_push_back( background, vertices, 8, indices, 12 );
    background_shader = shader_load( "shaders/v3f-c4f.vert",
                                     "shaders/v3f-c4f.frag" );

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor( 1.0,1.0,1.0,1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( background_shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( background_shader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( background_shader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( background_shader, "projection" ),
                            1, 0, projection.data);
        vertex_buffer_render( background, GL_TRIANGLES );
    }

    glUseProgram( text_shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( text_shader, "projection" ),
                            1, 0, projection.data);
        glUniform1i( glGetUniformLocation( text_shader, "tex" ), 0 );
        glUniform3f( glGetUniformLocation( text_shader, "pixel" ),
                     1.0f/font_manager->atlas->width,
                     1.0f/font_manager->atlas->height,
                     (float)font_manager->atlas->depth );

        glEnable( GL_BLEND );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendColor( 1, 1, 1, 1 );

        vertex_buffer_render( buffer->buffer, GL_TRIANGLES );
        glBindTexture( GL_TEXTURE_2D, 0 );
        glBlendColor( 0, 0, 0, 0 );
        glUseProgram( 0 );
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

    glDeleteProgram( background_shader );
    glDeleteProgram( text_shader );
    glDeleteTextures( 1, &font_manager->atlas->id );
    font_manager->atlas->id = 0;
    text_buffer_delete( buffer );
    font_manager_delete( font_manager );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
