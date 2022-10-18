/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "shader.h"


// ------------------------------------------------------------ shader_read ---
char *
shader_read( const char *filename )
{
    FILE * file;
    char * buffer;
    size_t size;

    file = fopen( filename, "rb" );
    if( !file )
    {
        fprintf( stderr, "Unable to open file \"%s\".\n", filename );
        return 0;
    }
    fseek( file, 0, SEEK_END );
    size = ftell( file );
    fseek(file, 0, SEEK_SET );
    buffer = (char *) malloc( (size+1) * sizeof( char *) );
    fread( buffer, sizeof(char), size, file );
    buffer[size] = 0;
    fclose( file );
    return buffer;
}



// --------------------------------------------------------- shader_compile ---
GLuint
shader_compile( const char* source,
                const GLenum type )
{
    GLint compile_status;
    GLuint handle = glCreateShader( type );
    glShaderSource( handle, 1, &source, 0 );
    glCompileShader( handle );

    glGetShaderiv( handle, GL_COMPILE_STATUS, &compile_status );
    if( compile_status == GL_FALSE )
    {
        GLchar messages[256];
        glGetShaderInfoLog( handle, sizeof(messages), 0, &messages[0] );
        fprintf( stderr, "%s\n", messages );
        exit( EXIT_FAILURE );
    }
    return handle;
}


// ------------------------------------------------------------ shader_load ---
GLuint
shader_load( const char * vert_filename,
              const char * frag_filename )
{
    GLuint handle = glCreateProgram( );
    GLint link_status;

    if( vert_filename && strlen( vert_filename ) )
    {
        char *vert_source = shader_read( vert_filename );
        GLuint vert_shader = shader_compile( vert_source, GL_VERTEX_SHADER);
        glAttachShader( handle, vert_shader);
        glDeleteShader( vert_shader );
        free( vert_source );
    }
    if( frag_filename && strlen( frag_filename ) )
    {
        char *frag_source = shader_read( frag_filename );
        GLuint frag_shader = shader_compile( frag_source, GL_FRAGMENT_SHADER);
        glAttachShader( handle, frag_shader);
        glDeleteShader( frag_shader );
        free( frag_source );
    }

    glLinkProgram( handle );

    glGetProgramiv( handle, GL_LINK_STATUS, &link_status );
    if (link_status == GL_FALSE)
    {
        GLchar messages[256];
        glGetProgramInfoLog( handle, sizeof(messages), 0, &messages[0] );
        fprintf( stderr, "%s\n", messages );
        exit(1);
    }
    return handle;
}
