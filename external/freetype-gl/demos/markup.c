/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 *
 * ============================================================================
 *
 * Example demonstrating markup usage
 *
 * ============================================================================
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

#include "freetype-gl.h"
#include "font-manager.h"
#include "vertex-buffer.h"
#include "text-buffer.h"
#include "markup.h"
#include "shader.h"
#include "mat4.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>
#include <vec234.h>


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;
    float r, g, b, a;
} vertex_t;

// ------------------------------------------------------- global variables ---
font_manager_t * font_manager;
text_buffer_t * buffer;
mat4 model, view, projection;
vertex_buffer_t *lines_buffer;
GLuint bounds_shader;
GLuint text_shader;


// ------------------------------------------------------ match_description ---
char *
match_description( char * description )
{

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
    fprintf( stderr, "\"font_manager_match_description\" "
                     "not implemented for windows.\n" );
    return 0;
#endif

    char *filename = 0;
    FcInit();
    FcPattern *pattern = FcNameParse(description);
    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        fprintf( stderr, "fontconfig error: could not match description '%s'", description );
        return 0;
    }
    else
    {
        FcValue value;
        FcResult result = FcPatternGet( match, FC_FILE, 0, &value );
        if ( result )
        {
            fprintf( stderr, "fontconfig error: could not match description '%s'", description );
        }
        else
        {
            filename = strdup( (char *)(value.u.s) );
        }
    }
    FcPatternDestroy( match );
    return filename;
}

void init()
{
    text_shader = shader_load( "shaders/text.vert",
                               "shaders/text.frag" );

    font_manager = font_manager_new( 512, 512, LCD_FILTERING_ON );
    buffer = text_buffer_new( );

    vec4 black  = {{0.0, 0.0, 0.0, 1.0}};
    vec4 white  = {{1.0, 1.0, 1.0, 1.0}};
    vec4 yellow = {{1.0, 1.0, 0.0, 1.0}};
    vec4 grey   = {{0.5, 0.5, 0.5, 1.0}};
    vec4 none   = {{1.0, 1.0, 1.0, 0.0}};

    char *f_normal   = match_description("Droid Serif:size=24");
    char *f_bold     = match_description("Droid Serif:size=24:weight=bold");
    char *f_italic   = match_description("Droid Serif:size=24:slant=italic");
    char *f_japanese = match_description("Droid Sans:size=18:lang=ja");
    char *f_math     = match_description("DejaVu Sans:size=24");

    markup_t normal = {
        .family  = f_normal,
        .size    = 24.0, .bold    = 0,   .italic  = 0,
        .spacing = 0.0,  .gamma   = 2.,
        .foreground_color    = white, .background_color    = none,
        .underline           = 0,     .underline_color     = white,
        .overline            = 0,     .overline_color      = white,
        .strikethrough       = 0,     .strikethrough_color = white,
        .font = 0,
    };
    markup_t highlight = normal; highlight.background_color = grey;
    markup_t reverse   = normal; reverse.foreground_color = black;
                                 reverse.background_color = white;
                                 reverse.gamma = 1.0;
    markup_t overline  = normal; overline.overline = 1;
    markup_t underline = normal; underline.underline = 1;
    markup_t small     = normal; small.size = 10.0;
    markup_t big       = normal; big.size = 48.0;
                                 big.italic = 1;
                                 big.foreground_color = yellow;
    markup_t bold      = normal; bold.bold = 1; bold.family = f_bold;
    markup_t italic    = normal; italic.italic = 1; italic.family = f_italic;
    markup_t japanese  = normal; japanese.family = f_japanese;
                                 japanese.size = 18.0;
    markup_t math      = normal; math.family = f_math;

    normal.font = font_manager_get_from_markup( font_manager, &normal );
    highlight.font = font_manager_get_from_markup( font_manager, &highlight );
    reverse.font = font_manager_get_from_markup( font_manager, &reverse );
    overline.font = font_manager_get_from_markup( font_manager, &overline );
    underline.font = font_manager_get_from_markup( font_manager, &underline );
    small.font = font_manager_get_from_markup( font_manager, &small );
    big.font = font_manager_get_from_markup( font_manager, &big );
    bold.font = font_manager_get_from_markup( font_manager, &bold );
    italic.font = font_manager_get_from_markup( font_manager, &italic );
    japanese.font = font_manager_get_from_markup( font_manager, &japanese );
    math.font = font_manager_get_from_markup( font_manager, &math );

    vec2 pen = {{20, 200}};
    text_buffer_printf( buffer, &pen,
                        &underline, "The",
                        &normal,    " Quick",
                        &big,       " brown ",
                        &reverse,   " fox \n",
                        &italic,    "jumps over ",
                        &bold,      "the lazy ",
                        &normal,    "dog.\n",
                        &small,     "Now is the time for all good men "
                                    "to come to the aid of the party.\n",
                        &italic,    "Ég get etið gler án þess að meiða mig.\n",
                        &japanese,  "私はガラスを食べられます。 それは私を傷つけません\n",
                        &math,      "ℕ ⊆ ℤ ⊂ ℚ ⊂ ℝ ⊂ ℂ",
                        NULL );

    glGenTextures( 1, &font_manager->atlas->id );
    glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, font_manager->atlas->width,
                  font_manager->atlas->height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                  font_manager->atlas->data );

    text_buffer_align( buffer, &pen, ALIGN_CENTER );

    vec4 bounds = text_buffer_get_bounds( buffer, &pen );
    float left = bounds.left;
    float right = bounds.left + bounds.width;
    float top = bounds.top;
    float bottom = bounds.top - bounds.height;

    bounds_shader = shader_load( "shaders/v3f-c4f.vert",
                                 "shaders/v3f-c4f.frag" );

    lines_buffer = vertex_buffer_new( "vertex:3f,color:4f" );
    vertex_t vertices[] = { { left - 10,         top, 0, 0,0,0,1}, // top
                            {right + 10,         top, 0, 0,0,0,1},

                            { left - 10,      bottom, 0, 0,0,0,1}, // bottom
                            {right + 10,      bottom, 0, 0,0,0,1},

                            {      left,    top + 10, 0, 0,0,0,1}, // left
                            {      left, bottom - 10, 0, 0,0,0,1},
                            {     right,    top + 10, 0, 0,0,0,1}, // right
                            {     right, bottom - 10, 0, 0,0,0,1} };
    GLuint indices[] = { 0,1,2,3,4,5,6,7 };
    vertex_buffer_push_back( lines_buffer, vertices, 8, indices, 8);

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor(0.40,0.40,0.45,1.00);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor4f(1.00,1.00,1.00,1.00);
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

    glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    glBlendColor( 1.0, 1.0, 1.0, 1.0 );
    glUseProgram( bounds_shader );
    {
        glUniformMatrix4fv( glGetUniformLocation( bounds_shader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( bounds_shader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( bounds_shader, "projection" ),
                            1, 0, projection.data);
        vertex_buffer_render( lines_buffer, GL_LINES );
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

    window = glfwCreateWindow( 500, 220, argv[0], NULL, NULL );

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
    reshape( window, 500, 220 );

    while(!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glDeleteProgram( bounds_shader );
    glDeleteProgram( text_shader );
    glDeleteTextures( 1, &font_manager->atlas->id );
    font_manager->atlas->id = 0;
    text_buffer_delete( buffer );
    font_manager_delete( font_manager );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
