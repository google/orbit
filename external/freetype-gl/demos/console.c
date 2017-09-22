/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "markup.h"
#include "shader.h"
#include "mat4.h"
#include "screenshot-util.h"

#include <GLFW/glfw3.h>


// -------------------------------------------------------------- constants ---
const int __SIGNAL_ACTIVATE__     = 0;
const int __SIGNAL_COMPLETE__     = 1;
const int __SIGNAL_HISTORY_NEXT__ = 2;
const int __SIGNAL_HISTORY_PREV__ = 3;
#define MAX_LINE_LENGTH  511


const int MARKUP_NORMAL      = 0;
const int MARKUP_DEFAULT     = 0;
const int MARKUP_ERROR       = 1;
const int MARKUP_WARNING     = 2;
const int MARKUP_OUTPUT      = 3;
const int MARKUP_BOLD        = 4;
const int MARKUP_ITALIC      = 5;
const int MARKUP_BOLD_ITALIC = 6;
const int MARKUP_FAINT       = 7;
#define   MARKUP_COUNT         8


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;
    float s, t;
    float r, g, b, a;
} vertex_t;

struct _console_t {
    vector_t *     lines;
    char *prompt;
    char killring[MAX_LINE_LENGTH+1];
    char input[MAX_LINE_LENGTH+1];
    size_t         cursor;
    markup_t       markup[MARKUP_COUNT];
    vertex_buffer_t * buffer;
    texture_atlas_t *atlas;
    vec2           pen;
    void (*handlers[4])( struct _console_t *, char * );
};
typedef struct _console_t console_t;


// ------------------------------------------------------- global variables ---
static console_t * console;
GLuint shader;
mat4   model, view, projection;
int control_key_handled;


// ------------------------------------------------------------ console_new ---
console_t *
console_new( float font_size )
{
    console_t *self = (console_t *) malloc( sizeof(console_t) );
    if( !self )
    {
        return self;
    }
    self->lines = vector_new( sizeof(char *) );
    self->prompt = strdup( ">>> " );
    self->cursor = 0;
    self->buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    self->input[0] = '\0';
    self->killring[0] = '\0';
    self->handlers[__SIGNAL_ACTIVATE__]     = 0;
    self->handlers[__SIGNAL_COMPLETE__]     = 0;
    self->handlers[__SIGNAL_HISTORY_NEXT__] = 0;
    self->handlers[__SIGNAL_HISTORY_PREV__] = 0;
    self->pen.x = self->pen.y = 0;

    self->atlas = texture_atlas_new( 512, 512, 1 );
    glGenTextures( 1, &self->atlas->id );

    vec4 white = {{1,1,1,1}};
    vec4 black = {{0,0,0,1}};
    vec4 none = {{0,0,1,0}};

    markup_t normal;
    normal.family  = "fonts/VeraMono.ttf";
    normal.size    = font_size;
    normal.bold    = 0;
    normal.italic  = 0;
    normal.spacing = 0.0;
    normal.gamma   = 1.0;
    normal.foreground_color    = black;
    normal.background_color    = none;
    normal.underline           = 0;
    normal.underline_color     = white;
    normal.overline            = 0;
    normal.overline_color      = white;
    normal.strikethrough       = 0;
    normal.strikethrough_color = white;

    normal.font = texture_font_new_from_file( self->atlas, font_size, "fonts/VeraMono.ttf" );

    markup_t bold = normal;
    bold.bold = 1;
    bold.font = texture_font_new_from_file( self->atlas, font_size, "fonts/VeraMoBd.ttf" );

    markup_t italic = normal;
    italic.italic = 1;
    bold.font = texture_font_new_from_file( self->atlas, font_size, "fonts/VeraMoIt.ttf" );

    markup_t bold_italic = normal;
    bold.bold = 1;
    italic.italic = 1;
    italic.font = texture_font_new_from_file( self->atlas, font_size, "fonts/VeraMoBI.ttf" );

    markup_t faint = normal;
    faint.foreground_color.r = 0.35;
    faint.foreground_color.g = 0.35;
    faint.foreground_color.b = 0.35;

    markup_t error = normal;
    error.foreground_color.r = 1.00;
    error.foreground_color.g = 0.00;
    error.foreground_color.b = 0.00;

    markup_t warning = normal;
    warning.foreground_color.r = 1.00;
    warning.foreground_color.g = 0.50;
    warning.foreground_color.b = 0.50;

    markup_t output = normal;
    output.foreground_color.r = 0.00;
    output.foreground_color.g = 0.00;
    output.foreground_color.b = 1.00;

    self->markup[MARKUP_NORMAL] = normal;
    self->markup[MARKUP_ERROR] = error;
    self->markup[MARKUP_WARNING] = warning;
    self->markup[MARKUP_OUTPUT] = output;
    self->markup[MARKUP_FAINT] = faint;
    self->markup[MARKUP_BOLD] = bold;
    self->markup[MARKUP_ITALIC] = italic;
    self->markup[MARKUP_BOLD_ITALIC] = bold_italic;

    return self;
}



// -------------------------------------------------------- console_delete ---
void
console_delete( console_t *self )
{
    glDeleteTextures( 1, &self->atlas->id );
    self->atlas->id = 0;
    texture_atlas_delete( self->atlas );
}



// ----------------------------------------------------- console_add_glyph ---
void
console_add_glyph( console_t *self,
                   char* current,
                   char* previous,
                   markup_t *markup )
{
    texture_glyph_t *glyph  = texture_font_get_glyph( markup->font, current );
    if( previous )
    {
        self->pen.x += texture_glyph_get_kerning( glyph, previous );
    }
    float r = markup->foreground_color.r;
    float g = markup->foreground_color.g;
    float b = markup->foreground_color.b;
    float a = markup->foreground_color.a;
    int x0  = self->pen.x + glyph->offset_x;
    int y0  = self->pen.y + glyph->offset_y;
    int x1  = x0 + glyph->width;
    int y1  = y0 - glyph->height;
    float s0 = glyph->s0;
    float t0 = glyph->t0;
    float s1 = glyph->s1;
    float t1 = glyph->t1;

    GLuint indices[] = {0,1,2, 0,2,3};
    vertex_t vertices[] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                            { x0,y1,0,  s0,t1,  r,g,b,a },
                            { x1,y1,0,  s1,t1,  r,g,b,a },
                            { x1,y0,0,  s1,t0,  r,g,b,a } };
    vertex_buffer_push_back( self->buffer, vertices, 4, indices, 6 );

    self->pen.x += glyph->advance_x;
    self->pen.y += glyph->advance_y;
}



// -------------------------------------------------------- console_render ---
void
console_render( console_t *self )
{
    char* cur_char;
    char* prev_char;

    int viewport[4];
    glGetIntegerv( GL_VIEWPORT, viewport );

    size_t i, index;
    self->pen.x = 0;
    self->pen.y = viewport[3];
    vertex_buffer_clear( console->buffer );

    int cursor_x = self->pen.x;
    int cursor_y = self->pen.y;

    markup_t markup;

    // console_t buffer
    markup = self->markup[MARKUP_FAINT];
    self->pen.y -= markup.font->height;

    for( i=0; i<self->lines->size; ++i )
    {
        char *text = * (char **) vector_get( self->lines, i ) ;
        if( strlen(text) > 0 )
        {
            cur_char = text;
            prev_char = NULL;
            console_add_glyph( console, cur_char, prev_char, &markup );
            prev_char = cur_char;
            for( index=1; index < strlen(text)-1; ++index )
            {
                cur_char = text + index;
                console_add_glyph( console, cur_char, prev_char, &markup );
                prev_char = cur_char;
            }
        }
        self->pen.y -= markup.font->height - markup.font->linegap;
        self->pen.x = 0;
        cursor_x = self->pen.x;
        cursor_y = self->pen.y;
    }

    // Prompt
    markup = self->markup[MARKUP_BOLD];
    if( strlen( self->prompt ) > 0 )
    {
        cur_char = self->prompt;
        prev_char = NULL;
        console_add_glyph( console, cur_char, prev_char, &markup );
        prev_char = cur_char;
        for( index=1; index < strlen(self->prompt); ++index )
        {
            cur_char = self->prompt + index;
            console_add_glyph( console, cur_char, prev_char, &markup );
            prev_char = cur_char;
        }
    }
    cursor_x = (int) self->pen.x;

    // Input
    markup = self->markup[MARKUP_NORMAL];
    if( strlen(self->input) > 0 )
    {
        cur_char = self->input;
        prev_char = NULL;
        console_add_glyph( console, cur_char, prev_char, &markup );
        prev_char = cur_char;
        if( self->cursor > 0)
        {
            cursor_x = (int) self->pen.x;
        }
        for( index=1; index < strlen(self->input); ++index )
        {
            cur_char = self->input + index;
            console_add_glyph( console, cur_char, prev_char, &markup );
            prev_char = cur_char;
            if( index < self->cursor )
            {
                cursor_x = (int) self->pen.x;
            }
        }
    }

    if( self->lines->size || self->prompt[0] != '\0' || self->input[0] != '\0' )
    {
        glBindTexture( GL_TEXTURE_2D, self->atlas->id );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, self->atlas->width,
                      self->atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE,
                      self->atlas->data );
    }

    // Cursor (we use the black character (NULL) as texture )
    texture_glyph_t *glyph  = texture_font_get_glyph( markup.font, NULL );
    float r = markup.foreground_color.r;
    float g = markup.foreground_color.g;
    float b = markup.foreground_color.b;
    float a = markup.foreground_color.a;
    int x0  = cursor_x+1;
    int y0  = cursor_y + markup.font->descender;
    int x1  = cursor_x+2;
    int y1  = y0 + markup.font->height - markup.font->linegap;
    float s0 = glyph->s0;
    float t0 = glyph->t0;
    float s1 = glyph->s1;
    float t1 = glyph->t1;
    GLuint indices[] = {0,1,2, 0,2,3};
    vertex_t vertices[] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                            { x0,y1,0,  s0,t1,  r,g,b,a },
                            { x1,y1,0,  s1,t1,  r,g,b,a },
                            { x1,y0,0,  s1,t0,  r,g,b,a } };
    vertex_buffer_push_back( self->buffer, vertices, 4, indices, 6 );
    glEnable( GL_TEXTURE_2D );

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
        vertex_buffer_render( console->buffer, GL_TRIANGLES );
    }
}


// ------------------------------------------------------- console_connect ---
void
console_connect( console_t *self,
                  const char *signal,
                  void (*handler)(console_t *, char *))
{
    if( strcmp( signal,"activate" ) == 0 )
    {
        self->handlers[__SIGNAL_ACTIVATE__] = handler;
    }
    else if( strcmp( signal,"complete" ) == 0 )
    {
        self->handlers[__SIGNAL_COMPLETE__] = handler;
    }
    else if( strcmp( signal,"history-next" ) == 0 )
    {
        self->handlers[__SIGNAL_HISTORY_NEXT__] = handler;
    }
    else if( strcmp( signal,"history-prev" ) == 0 )
    {
        self->handlers[__SIGNAL_HISTORY_PREV__] = handler;
    }
}


// --------------------------------------------------------- console_print ---
void
console_print( console_t *self, char *text )
{
    // Make sure there is at least one line
    if( self->lines->size == 0 )
    {
        char *line = strdup( "" );
        vector_push_back( self->lines, &line );
    }

    // Make sure last line does not end with '\n'
    char *last_line = *(char **) vector_get( self->lines, self->lines->size-1 ) ;
    if( strlen( last_line ) != 0 )
    {
        if( last_line[strlen( last_line ) - 1] == '\n' )
        {
            char *line = strdup( "" );
            vector_push_back( self->lines, &line );
        }
    }
    last_line = *(char **) vector_get( self->lines, self->lines->size-1 ) ;

    char *start = text;
    char *end   = strchr(start, '\n');
    size_t len = strlen( last_line );
    if( end != NULL)
    {
        char *line = (char *) malloc( (len + end - start + 2)*sizeof( char ) );
        strncpy( line, last_line, len );
        strncpy( line + len, text, end-start+1 );

        line[len+end-start+1] = '\0';

        vector_set( self->lines, self->lines->size-1, &line );
        free( last_line );
        if( (end-start)  < (strlen( text )-1) )
        {
            console_print(self, end+1 );
        }
        return;
    }
    else
    {
        char *line = (char *) malloc( (len + strlen(text) + 1) * sizeof( char ) );
        strncpy( line, last_line, len );
        strcpy( line + len, text );
        vector_set( self->lines, self->lines->size-1, &line );
        free( last_line );
        return;
    }
}


// ------------------------------------------------------- console_process ---
void
console_process( console_t *self,
                  const char *action,
                  const unsigned char key )
{
    size_t len = strlen(self->input);

    if( strcmp(action, "type") == 0 )
    {
        if( len < MAX_LINE_LENGTH )
        {
            memmove( self->input + self->cursor+1,
                     self->input + self->cursor,
                     (len - self->cursor+1)*sizeof(char) );
            self->input[self->cursor] = key;
            self->cursor++;
        }
        else
        {
            fprintf( stderr, "Input buffer is full\n" );
        }
    }
    else
    {
        if( strcmp( action, "enter" ) == 0 )
        {
            if( console->handlers[__SIGNAL_ACTIVATE__] )
            {
                (*console->handlers[__SIGNAL_ACTIVATE__])(console, console->input);
            }
            console_print( self, self->prompt );
            console_print( self, self->input );
            console_print( self, "\n" );
            self->input[0] = '\0';
            self->cursor = 0;
        }
        else if( strcmp( action, "right" ) == 0 )
        {
            if( self->cursor < strlen(self->input) )
            {
                self->cursor += 1;
            }
        }
        else if( strcmp( action, "left" ) == 0 )
        {
            if( self->cursor > 0 )
            {
                self->cursor -= 1;
            }
        }
        else if( strcmp( action, "delete" ) == 0 )
        {
            memmove( self->input + self->cursor,
                     self->input + self->cursor+1,
                     (len - self->cursor)*sizeof(char) );
        }
        else if( strcmp( action, "backspace" ) == 0 )
        {
            if( self->cursor > 0 )
            {
                memmove( self->input + self->cursor-1,
                         self->input + self->cursor,
                         (len - self->cursor+1)*sizeof(char) );
                self->cursor--;
            }
        }
        else if( strcmp( action, "kill" ) == 0 )
        {
            if( self->cursor < len )
            {
                strcpy(self->killring, self->input);
                self->input[self->cursor] = '\0';
                fprintf(stderr, "Kill ring: %s\n", self->killring);
            }

        }
        else if( strcmp( action, "yank" ) == 0 )
        {
            size_t l = strlen(self->killring);
            if( (len + l) < MAX_LINE_LENGTH )
            {
                memmove( self->input + self->cursor + l,
                         self->input + self->cursor,
                         (len - self->cursor)*sizeof(char) );
                memcpy( self->input + self->cursor,
                        self->killring, l*sizeof(char));
                self->cursor += l;
            }
        }
        else if( strcmp( action, "home" ) == 0 )
        {
            self->cursor = 0;
        }
        else if( strcmp( action, "end" ) == 0 )
        {
            self->cursor = strlen( self->input );
        }
        else if( strcmp( action, "clear" ) == 0 )
        {
        }
        else if( strcmp( action, "history-prev" ) == 0 )
        {
            if( console->handlers[__SIGNAL_HISTORY_PREV__] )
            {
                (*console->handlers[__SIGNAL_HISTORY_PREV__])( console, console->input );
            }
        }
        else if( strcmp( action, "history-next" ) == 0 )
        {
            if( console->handlers[__SIGNAL_HISTORY_NEXT__] )
            {
                (*console->handlers[__SIGNAL_HISTORY_NEXT__])( console, console->input );
            }
        }
        else if( strcmp( action, "complete" ) == 0 )
        {
            if( console->handlers[__SIGNAL_COMPLETE__] )
            {
                (*console->handlers[__SIGNAL_COMPLETE__])( console, console->input );
            }
        }
    }
}


// ------------------------------------------------------- console activate ---
void console_activate( console_t *self, char *input )
{
    //console_print( self, "Activate callback\n" );
    fprintf( stderr, "Activate callback : %s\n", input );
}


// ------------------------------------------------------- console complete ---
void console_complete( console_t *self, char *input )
{
    // console_print( self, "Complete callback\n" );
    fprintf( stderr, "Complete callback : %s\n", input );
}


// ----------------------------------------------- console previous history ---
void console_history_prev( console_t *self, char *input )
{
    // console_print( self, "History prev callback\n" );
    fprintf( stderr, "History prev callback : %s\n", input );
}


// --------------------------------------------------- console next history ---
void console_history_next( console_t *self, char *input )
{
    // console_print( self, "History next callback\n" );
    fprintf( stderr, "History next callback : %s\n", input );
}


// ------------------------------------------------------------------- init ---
void init( float font_size )
{
    control_key_handled = 0;

    console = console_new( font_size );
    console_print( console,
                   "OpenGL Freetype console\n"
                   "Copyright 2011 Nicolas P. Rougier. All rights reserved.\n \n" );
    console_connect( console, "activate",     console_activate );
    console_connect( console, "complete",     console_complete );
    console_connect( console, "history-prev", console_history_prev );
    console_connect( console, "history-next", console_history_next );

    glClearColor( 1.00, 1.00, 1.00, 1.00 );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );

    shader = shader_load("shaders/v3f-t2f-c4f.vert",
                         "shaders/v3f-t2f-c4f.frag");
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    console_render( console );
    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}


// ----------------------------------------------------------- on char input ---
void char_input( GLFWwindow* window, unsigned int cp )
{
    if( control_key_handled )
    {
        control_key_handled = 0;
        return;
    }

    console_process( console, "type", cp );
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if( GLFW_PRESS != action && GLFW_REPEAT != action )
    {
        return;
    }

    switch( key )
    {
        case GLFW_KEY_HOME:
            console_process( console, "home", 0 );
            break;
        case GLFW_KEY_DELETE:
            console_process( console, "delete", 0 );
            break;
        case GLFW_KEY_END:
            console_process( console, "end", 0 );
            break;
        case GLFW_KEY_BACKSPACE:
            console_process( console, "backspace", 0 );
            break;
        case GLFW_KEY_TAB:
            console_process( console, "complete", 0 );
            break;
        case GLFW_KEY_ENTER:
            console_process( console, "enter", 0 );
            break;
        case GLFW_KEY_ESCAPE:
            console_process( console, "escape", 0 );
            break;
        case GLFW_KEY_UP:
            console_process( console, "history-prev", 0 );
            break;
        case GLFW_KEY_DOWN:
            console_process( console, "history-next", 0 );
            break;
        case GLFW_KEY_LEFT:
            console_process( console,  "left", 0 );
            break;
        case GLFW_KEY_RIGHT:
            console_process( console, "right", 0 );
            break;
        default:
            break;
    }

    if( ( GLFW_MOD_CONTROL & mods ) == 0 )
    {
        return;
    }

    switch( key )
    {
        case GLFW_KEY_K:
            control_key_handled = 1;
            console_process( console, "kill", 0 );
            break;
        case GLFW_KEY_L:
            control_key_handled = 1;
            console_process( console, "clear", 0 );
            break;
        case GLFW_KEY_Y:
            control_key_handled = 1;
            console_process( console, "yank", 0 );
            break;
        default:
            break;
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

    window = glfwCreateWindow( 600,400, argv[0], NULL, NULL );

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
    glfwSetCharCallback( window, char_input );

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

    glfwShowWindow( window );
    int pixWidth, pixHeight;
    glfwGetFramebufferSize( window, &pixWidth, &pixHeight );

    init( 13.0 * pixWidth / 600 );

    reshape( window, pixWidth, pixHeight );

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

    console_delete( console );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
