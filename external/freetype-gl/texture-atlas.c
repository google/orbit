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
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    self->nodes = vector_new( sizeof(ivec3) );
    self->used = 0;
    self->width = width;
    self->height = height;
    self->depth = depth;
    self->id = 0;

    vector_push_back( self->nodes, &node );
    self->data = (unsigned char *)
        calloc( width*height*depth, sizeof(unsigned char) );

    if( self->data == NULL)
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }

    return self;
}


// --------------------------------------------------- texture_atlas_delete ---
void
texture_atlas_delete( texture_atlas_t *self )
{
    assert( self );
    vector_delete( self->nodes );
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
	
    //prevent copying data from undefined position 
    //and prevent memcpy's undefined behavior when count is zero
    assert(height == 0 || (data != NULL && width > 0));

    depth = self->depth;
    charsize = sizeof(char);
    for( i=0; i<height; ++i )
    {
        memcpy( self->data+((y+i)*self->width + x ) * charsize * depth,
                data + (i*stride) * charsize, width * charsize * depth  );
    }
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
                ( ((y + height) == best_height) && (node->z > 0 && (size_t)node->z < best_width)) )
			{
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
    if( node == NULL)
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    node->x = region.x;
    node->y = region.y + height;
    node->z = width;
    vector_insert( self->nodes, best_index, node );
    free( node );

    for(i = best_index+1; i < self->nodes->size; ++i)
    {
        node = (ivec3 *) vector_get( self->nodes, i );
        prev = (ivec3 *) vector_get( self->nodes, i-1 );

        if (node->x < (prev->x + prev->z) )
        {
            int shrink = prev->x + prev->z - node->x;
            node->x += shrink;
            node->z -= shrink;
            if (node->z <= 0)
            {
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
