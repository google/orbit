/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "texture-atlas.h"
#include "texture-font.h"
#include "ftgl-utils.h"

// -------------------------------------------------- texture_atlas_special ---

void texture_atlas_special ( texture_atlas_t * self )
{
    ivec4 region = texture_atlas_get_region( self, 5, 5 );
    texture_glyph_t * glyph = texture_glyph_new( );
    static unsigned char data[4*4*3] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    if ( region.x < 0 ) {
        freetype_gl_error( Texture_Atlas_Full );
    }
    
    texture_atlas_set_region( self, region.x, region.y, 4, 4, data, 0 );
    glyph->codepoint = -1;
    glyph->s0 = (region.x+2)/(float)self->width;
    glyph->t0 = (region.y+2)/(float)self->height;
    glyph->s1 = (region.x+3)/(float)self->width;
    glyph->t1 = (region.y+3)/(float)self->height;

    self->special = (void*)glyph;
}

// ------------------------------------------------------ texture_atlas_new ---
texture_atlas_t *
texture_atlas_new( const size_t width,
                   const size_t height,
                   const size_t depth )
{
    texture_atlas_t *self = (texture_atlas_t *) malloc( sizeof(texture_atlas_t) );

    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    ivec3 node = {{1,1,width-2}};

    assert( (depth == 1) || (depth == 3) || (depth == 4) );
    if( self == NULL)
    {
        freetype_gl_error( Out_Of_Memory );
        return NULL;
        /* exit( EXIT_FAILURE ); */ /* Never exit from a library */
    }
    self->nodes = vector_new( sizeof(ivec3) );
    self->used = 0;
    self->width = width;
    self->height = height;
    self->depth = depth;
    self->id = 0;
    self->modified = 1;

    vector_push_back( self->nodes, &node );
    self->data = (unsigned char *)
        calloc( width*height*depth, sizeof(unsigned char) );

    if( self->data == NULL)
    {
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }

    texture_atlas_special( self );
    
    return self;
}


// --------------------------------------------------- texture_atlas_delete ---
void
texture_atlas_delete( texture_atlas_t *self )
{
    assert( self );
    vector_delete( self->nodes );
    texture_glyph_delete( self->special );
    if( self->data )
    {
        free( self->data );
    }
    free( self );
}


// ----------------------------------------------- texture_atlas_set_region ---
void
texture_atlas_set_region( texture_atlas_t * self,
                          const size_t x,
                          const size_t y,
                          const size_t width,
                          const size_t height,
                          const unsigned char * data,
                          const size_t stride )
{
    size_t i;
    size_t depth;
    size_t charsize;

    assert( self );
    assert( x > 0);
    assert( y > 0);
    assert( x < (self->width-1));
    assert( (x + width) <= (self->width-1));
    assert( y < (self->height-1));
    assert( (y + height) <= (self->height-1));
        
    // prevent copying data from undefined position
    // and prevent memcpy's undefined behavior when count is zero
    assert(height == 0 || (data != NULL && width > 0));

    depth = self->depth;
    charsize = sizeof(char);
    for( i=0; i<height; ++i )
    {
        memcpy( self->data+((y+i)*self->width + x ) * charsize * depth,
                data + (i*stride) * charsize, width * charsize * depth  );
    }
    self->modified = 1;
}


// ------------------------------------------------------ texture_atlas_fit ---
int
texture_atlas_fit( texture_atlas_t * self,
                   const size_t index,
                   const size_t width,
                   const size_t height )
{
    ivec3 *node;
    int x, y, width_left;
    size_t i;

    assert( self );

    node = (ivec3 *) (vector_get( self->nodes, index ));
    x = node->x;
    y = node->y;
    width_left = width;
    i = index;

    if ( (x + width) > (self->width-1) )
    {
	return -1;
    }
    y = node->y;
    while( width_left > 0 )
    {
	node = (ivec3 *) (vector_get( self->nodes, i ));
	if( node->y > y )
	{
	    y = node->y;
	}
	if( (y + height) > (self->height-1) )
	{
	    return -1;
	}
	width_left -= node->z;
	++i;
    }
    return y;
}


// ---------------------------------------------------- texture_atlas_merge ---
void
texture_atlas_merge( texture_atlas_t * self )
{
    ivec3 *node, *next;
    size_t i;

    assert( self );

    for( i=0; i< self->nodes->size-1; ++i )
    {
        node = (ivec3 *) (vector_get( self->nodes, i ));
        next = (ivec3 *) (vector_get( self->nodes, i+1 ));
        if( node->y == next->y )
	{
            node->z += next->z;
            vector_erase( self->nodes, i+1 );
            --i;
        }
    }
}


// ----------------------------------------------- texture_atlas_get_region ---
ivec4
texture_atlas_get_region( texture_atlas_t * self,
                          const size_t width,
                          const size_t height )
{
    int y, best_index;
    size_t best_height, best_width;
    ivec3 *node, *prev;
    ivec4 region = {{0,0,width,height}};
    size_t i;

    assert( self );

    best_height = UINT_MAX;
    best_index  = -1;
    best_width = UINT_MAX;
    for( i=0; i<self->nodes->size; ++i )
    {
        y = texture_atlas_fit( self, i, width, height );
        if( y >= 0 )
	{
            node = (ivec3 *) vector_get( self->nodes, i );
            if( ( (y + height) < best_height ) ||
                ( ((y + height) == best_height) && (node->z > 0 && (size_t)node->z < best_width)) ) {
                best_height = y + height;
                best_index = i;
                best_width = node->z;
                region.x = node->x;
                region.y = y;
            }
        }
    }
    
    if( best_index == -1 )
    {
        region.x = -1;
        region.y = -1;
        region.width = 0;
        region.height = 0;
        return region;
    }

    node = (ivec3 *) malloc( sizeof(ivec3) );
    if( node == NULL) {
        freetype_gl_error( Out_Of_Memory );
        return (ivec4){{-1,-1,0,0}};
        /* exit( EXIT_FAILURE ); */ /* Never exit from a library */
    }
    node->x = region.x;
    node->y = region.y + height;
    node->z = width;
    vector_insert( self->nodes, best_index, node );
    free( node );

    for( i = best_index+1; i < self->nodes->size; ++i )
    {
        node = (ivec3 *) vector_get( self->nodes, i );
        prev = (ivec3 *) vector_get( self->nodes, i-1 );

        if (node->x < (prev->x + prev->z) )
	{
            int shrink = prev->x + prev->z - node->x;
            node->x += shrink;
            node->z -= shrink;
            if (node->z <= 0) {
                vector_erase( self->nodes, i );
                --i;
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    texture_atlas_merge( self );
    self->used += width * height;
    self->modified = 1;    
    return region;
}


// ---------------------------------------------------- texture_atlas_clear ---
void
texture_atlas_clear( texture_atlas_t * self )
{
    ivec3 node = {{1,1,1}};

    assert( self );
    assert( self->data );

    vector_clear( self->nodes );
    self->used = 0;
    // We want a one pixel border around the whole atlas to avoid any artefact when
    // sampling texture
    node.z = self->width-2;

    vector_push_back( self->nodes, &node );
    memset( self->data, 0, self->width*self->height*self->depth );
}

// -------------------------------------------- texture_atlas_enlarge_atlas ---

void texture_atlas_enlarge_texture ( texture_atlas_t* self, size_t width_new, size_t height_new)
{
    assert(self);
    //ensure size increased
    assert(width_new >= self->width);
    assert(height_new >= self->height);
    assert(width_new + height_new > self->width + self->height);

    size_t width_old = self->width;
    size_t height_old = self->height;    
    //allocate new buffer
    unsigned char* data_old = self->data;
    self->data = calloc(1,width_new*height_new * sizeof(char)*self->depth);    
    //update atlas size
    self->width = width_new;
    self->height = height_new;
    //add node reflecting the gained space on the right
    if( width_new>width_old )
    {
        ivec3 node;
        node.x = width_old - 1;
        node.y = 1;
        node.z = width_new - width_old;
        vector_push_back(self->nodes, &node);    
    }
    //copy over data from the old buffer, skipping first row and column because of the margin
    size_t pixel_size = sizeof(char) * self->depth;
    size_t old_row_size = width_old * pixel_size;
    texture_atlas_set_region(self, 1, 1, width_old - 2, height_old - 2, data_old + old_row_size + pixel_size, old_row_size);
    free(data_old);    
}
