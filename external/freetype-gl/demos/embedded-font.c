/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "opengl.h"
#include "vec234.h"
#include "vector.h"
#include "utf8-utils.h"
#include "screenshot-util.h"


#include "vera-16.h"

#include <GLFW/glfw3.h>

// --------------------------------------------------------------- print_at ---
void print_at( int pen_x, int pen_y, char *text )
{
    size_t i, j;
    for( i=0; i < strlen(text); ++i)
    {
        texture_glyph_t *glyph = 0;
        for( j=0; j<font.glyphs_count; ++j)
        {
            if( font.glyphs[j].codepoint == utf8_to_utf32( text + i ) )
            {
                glyph = &font.glyphs[j];
                break;
            }
        }
        if( !glyph )
        {
            continue;
        }
        int x = pen_x + glyph->offset_x;
        int y = pen_y + glyph->offset_y;
        int w  = glyph->width;
        int h  = glyph->height;
        glBegin( GL_TRIANGLES );
        {
            glTexCoord2f( glyph->s0, glyph->t0 ); glVertex2i( x,   y   );
            glTexCoord2f( glyph->s0, glyph->t1 ); glVertex2i( x,   y-h );
            glTexCoord2f( glyph->s1, glyph->t1 ); glVertex2i( x+w, y-h );
            glTexCoord2f( glyph->s0, glyph->t0 ); glVertex2i( x,   y   );
            glTexCoord2f( glyph->s1, glyph->t1 ); glVertex2i( x+w, y-h );
            glTexCoord2f( glyph->s1, glyph->t0 ); glVertex2i( x+w, y   );
        }
        glEnd();
        pen_x += glyph->advance_x;
        pen_y += glyph->advance_y;

    }
}


// ------------------------------------------------------------------- init ---
void init( void )
{
    GLuint texid;
    glGenTextures( 1, &texid );
    glBindTexture( GL_TEXTURE_2D, texid );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, font.tex_width, font.tex_height,
                  0, GL_ALPHA, GL_UNSIGNED_BYTE, font.tex_data );
    glBindTexture( GL_TEXTURE_2D, texid );

    glClearColor( 1, 1, 1, 1 );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_TEXTURE_2D );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glColor4f(0,0,0,1);
    print_at( 100, 100, "Hello World !" );

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
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

    window = glfwCreateWindow( 640, 480, argv[0], NULL, NULL );

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

    init();

    glfwShowWindow( window );
    reshape( window, 640, 480 );

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

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
