/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "vec234.h"
#include "platform.h"
#include "vertex-attribute.h"



// ----------------------------------------------------------------------------
vertex_attribute_t *
vertex_attribute_new( GLchar * name,
                      GLint size,
                      GLenum type,
                      GLboolean normalized,
                      GLsizei stride,
                      GLvoid *pointer )
{
    vertex_attribute_t *attribute =
        (vertex_attribute_t *) malloc (sizeof(vertex_attribute_t));

    assert( size > 0 );

    attribute->name       = (GLchar *) strdup( name );
    attribute->index      = -1;
    attribute->size       = size;
    attribute->type       = type;
    attribute->normalized = normalized;
    attribute->stride     = stride;
    attribute->pointer    = pointer;
    return attribute;
}



// ----------------------------------------------------------------------------
void
vertex_attribute_delete( vertex_attribute_t * self )
{
    assert( self );

    free( self->name );
    free( self );
}



// ----------------------------------------------------------------------------
vertex_attribute_t *
vertex_attribute_parse( char *format )
{
    GLenum type = 0;
    int size;
    int normalized = 0;
    char ctype;
    char *name;
    vertex_attribute_t *attr;
    char *p = strchr(format, ':');
    if( p != NULL)
    {
        name = strndup(format, p-format);
        if( *(++p) == '\0' )
        {
            fprintf( stderr, "No size specified for '%s' attribute\n", name );
            free( name );
            return 0;
        }
        size = *p - '0';

        if( *(++p) == '\0' )
        {
            fprintf( stderr, "No format specified for '%s' attribute\n", name );
            free( name );
            return 0;
        }
        ctype = *p;

        if( *(++p) != '\0' )
        {
            if( *p == 'n' )
            {
                normalized = 1;
            }
        }

    }
    else
    {
        fprintf(stderr, "Vertex attribute format not understood ('%s')\n", format );
        return 0;
    }

    switch( ctype )
    {
    case 'b': type = GL_BYTE;           break;
    case 'B': type = GL_UNSIGNED_BYTE;  break;
    case 's': type = GL_SHORT;          break;
    case 'S': type = GL_UNSIGNED_SHORT; break;
    case 'i': type = GL_INT;            break;
    case 'I': type = GL_UNSIGNED_INT;   break;
    case 'f': type = GL_FLOAT;          break;
    default:  type = 0;                 break;
    }


    attr = vertex_attribute_new( name, size, type, normalized, 0, 0 );
    free( name );
    return attr;
}



// ----------------------------------------------------------------------------
void
vertex_attribute_enable( vertex_attribute_t *attr )
{
    if( attr->index == -1 )
    {
        GLint program;
        glGetIntegerv( GL_CURRENT_PROGRAM, &program );
        if( program == 0)
        {
            return;
        }
        attr->index = glGetAttribLocation( program, attr->name );
        if( attr->index == -1 )
        {
            return;
        }
    }
    glEnableVertexAttribArray( attr->index );
    glVertexAttribPointer( attr->index, attr->size, attr->type,
                           attr->normalized, attr->stride, attr->pointer );
}
