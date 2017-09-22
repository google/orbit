/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vector.h"



// ------------------------------------------------------------- vector_new ---
vector_t *
vector_new( size_t item_size )
{
    vector_t *self = (vector_t *) malloc( sizeof(vector_t) );
    assert( item_size );

    if( !self )
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    self->item_size = item_size;
    self->size      = 0;
    self->capacity  = 1;
    self->items     = malloc( self->item_size * self->capacity );
    return self;
}



// ---------------------------------------------------------- vector_delete ---
void
vector_delete( vector_t *self )
{
    assert( self );

    free( self->items );
    free( self );
}



// ------------------------------------------------------------- vector_get ---
const void *
vector_get( const vector_t *self,
            size_t index )
{
    assert( self );
    assert( self->size );
    assert( index  < self->size );

    return (char*)(self->items) + index * self->item_size;
}



// ----------------------------------------------------------- vector_front ---
const void *
vector_front( const vector_t *self )
{
    assert( self );
    assert( self->size );

    return vector_get( self, 0 );
}


// ------------------------------------------------------------ vector_back ---
const void *
vector_back( const vector_t *self )
{
    assert( self );
    assert( self->size );

    return vector_get( self, self->size-1 );
}


// -------------------------------------------------------- vector_contains ---
int
vector_contains( const vector_t *self,
                 const void *item,
                 int (*cmp)(const void *, const void *) )
{
    size_t i;
    assert( self );

    for( i=0; i<self->size; ++i )
    {
        if( (*cmp)(item, vector_get(self,i) ) == 0 )
        {
            return 1;
        }
    }
   return 0;
}


// ----------------------------------------------------------- vector_empty ---
int
vector_empty( const vector_t *self )
{
    assert( self );

    return self->size == 0;
}


// ------------------------------------------------------------ vector_size ---
size_t
vector_size( const vector_t *self )
{
    assert( self );

    return self->size;
}


// --------------------------------------------------------- vector_reserve ---
void
vector_reserve( vector_t *self,
                const size_t size )
{
    assert( self );

    if( self->capacity < size)
    {
        self->items = realloc( self->items, size * self->item_size );
        self->capacity = size;
    }
}


// -------------------------------------------------------- vector_capacity ---
size_t
vector_capacity( const vector_t *self )
{
    assert( self );

    return self->capacity;
}


// ---------------------------------------------------------- vector_shrink ---
void
vector_shrink( vector_t *self )
{
    assert( self );

    if( self->capacity > self->size )
    {
        self->items = realloc( self->items, self->size * self->item_size );
    }
    self->capacity = self->size;
}


// ----------------------------------------------------------- vector_clear ---
void
vector_clear( vector_t *self )
{
    assert( self );

    self->size = 0;
}


// ------------------------------------------------------------- vector_set ---
void
vector_set( vector_t *self,
            const size_t index,
            const void *item )
{
    assert( self );
    assert( self->size );
    assert( index  < self->size );

    memcpy( (char *)(self->items) + index * self->item_size,
            item, self->item_size );
}


// ---------------------------------------------------------- vector_insert ---
void
vector_insert( vector_t *self,
               const size_t index,
               const void *item )
{
    assert( self );
    assert( index <= self->size);

    if( self->capacity <= self->size )
    {
        vector_reserve(self, 2 * self->capacity );
    }
    if( index < self->size )
    {
        memmove( (char *)(self->items) + (index + 1) * self->item_size,
                 (char *)(self->items) + (index + 0) * self->item_size,
                 (self->size - index)  * self->item_size);
    }
    self->size++;
    vector_set( self, index, item );
}


// ----------------------------------------------------- vector_erase_range ---
void
vector_erase_range( vector_t *self,
                    const size_t first,
                    const size_t last )
{
    assert( self );
    assert( first < self->size );
    assert( last  < self->size+1 );
    assert( first < last );

    memmove( (char *)(self->items) + first * self->item_size,
             (char *)(self->items) + last  * self->item_size,
             (self->size - last)   * self->item_size);
    self->size -= (last-first);
}


// ----------------------------------------------------------- vector_erase ---
void
vector_erase( vector_t *self,
              const size_t index )
{
    assert( self );
    assert( index < self->size );

    vector_erase_range( self, index, index+1 );
}


// ------------------------------------------------------- vector_push_back ---
void
vector_push_back( vector_t *self,
                  const void *item )
{
    vector_insert( self, self->size, item );
}


// -------------------------------------------------------- vector_pop_back ---
void
vector_pop_back( vector_t *self )
{
    assert( self );
    assert( self->size );

    self->size--;
}


// ---------------------------------------------------------- vector_resize ---
void
vector_resize( vector_t *self,
               const size_t size )
{
    assert( self );

    if( size > self->capacity)
    {
        vector_reserve( self, size );
        self->size = self->capacity;
    }
    else
    {
        self->size = size;
    }
}


// -------------------------------------------------- vector_push_back_data ---
void
vector_push_back_data( vector_t *self,
                       const void * data,
                       const size_t count )
{
    assert( self );
    assert( data );
    assert( count );

    if( self->capacity < (self->size+count) )
    {
        vector_reserve(self, self->size+count);
    }
    memmove( (char *)(self->items) + self->size * self->item_size, data,
             count*self->item_size );
    self->size += count;
}


// ----------------------------------------------------- vector_insert_data ---
void
vector_insert_data( vector_t *self,
                    const size_t index,
                    const void * data,
                    const size_t count )
{
    assert( self );
    assert( index < self->size );
    assert( data );
    assert( count );

    if( self->capacity < (self->size+count) )
    {
        vector_reserve(self, self->size+count);
    }
    memmove( (char *)(self->items) + (index + count ) * self->item_size,
             (char *)(self->items) + (index ) * self->item_size,
             count*self->item_size );
    memmove( (char *)(self->items) + index * self->item_size, data,
             count*self->item_size );
    self->size += count;
}


// ------------------------------------------------------------ vector_sort ---
void
vector_sort( vector_t *self,
             int (*cmp)(const void *, const void *) )
{
    assert( self );
    assert( self->size );

    qsort(self->items, self->size, self->item_size, cmp);
}
