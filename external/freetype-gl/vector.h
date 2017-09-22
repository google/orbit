/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   vector.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup vector Vector
 *
 * The vector structure and accompanying functions loosely mimic the STL C++
 * vector class. It is used by @ref texture-atlas (for storing nodes), @ref
 * texture-font (for storing glyphs) and @ref font-manager (for storing fonts).
 * More information at http://www.cppreference.com/wiki/container/vector/start
 *
 * <b>Example Usage</b>:
 * @code
 * #include "vector.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *   int i,j = 1;
 *   vector_t * vector = vector_new( sizeof(int) );
 *   vector_push_back( &i );
 *
 *   j = * (int *) vector_get( vector, 0 );
 *   vector_delete( vector);
 *
 *   return 0;
 * }
 * @endcode
 *
 * @{
 */

/**
 *  Generic vector structure.
 *
 * @memberof vector
 */
typedef struct vector_t
 {
     /** Pointer to dynamically allocated items. */
     void * items;

     /** Number of items that can be held in currently allocated storage. */
     size_t capacity;

     /** Number of items. */
     size_t size;

     /** Size (in bytes) of a single item. */
     size_t item_size;
} vector_t;


/**
 * Creates a new empty vector.
 *
 * @param   item_size    item size in bytes
 * @return               a new empty vector
 *
 */
  vector_t *
  vector_new( size_t item_size );


/**
 *  Deletes a vector.
 *
 *  @param self a vector structure
 *
 */
  void
  vector_delete( vector_t *self );


/**
 *  Returns a pointer to the item located at specified index.
 *
 *  @param  self  a vector structure
 *  @param  index the index of the item to be returned
 *  @return       pointer on the specified item
 */
  const void *
  vector_get( const vector_t *self,
              size_t index );


/**
 *  Returns a pointer to the first item.
 *
 *  @param  self  a vector structure
 *  @return       pointer on the first item
 */
  const void *
  vector_front( const vector_t *self );


/**
 *  Returns a pointer to the last item
 *
 *  @param  self  a vector structure
 *  @return pointer on the last item
 */
  const void *
  vector_back( const vector_t *self );


/**
 *  Check if an item is contained within the vector.
 *
 *  @param  self  a vector structure
 *  @param  item  item to be searched in the vector
 *  @param  cmp   a pointer a comparison function
 *  @return       1 if item is contained within the vector, 0 otherwise
 */
  int
  vector_contains( const vector_t *self,
                   const void *item,
                   int (*cmp)(const void *, const void *) );


/**
 *  Checks whether the vector is empty.
 *
 *  @param  self  a vector structure
 *  @return       1 if the vector is empty, 0 otherwise
 */
  int
  vector_empty( const vector_t *self );


/**
 *  Returns the number of items
 *
 *  @param  self  a vector structure
 *  @return       number of items
 */
  size_t
  vector_size( const vector_t *self );


/**
 *  Reserve storage such that it can hold at last size items.
 *
 *  @param  self  a vector structure
 *  @param  size  the new storage capacity
 */
  void
  vector_reserve( vector_t *self,
                  const size_t size );


/**
 *  Returns current storage capacity
 *
 *  @param  self  a vector structure
 *  @return       storage capacity
 */
  size_t
  vector_capacity( const vector_t *self );


/**
 *  Decrease capacity to fit actual size.
 *
 *  @param  self  a vector structure
 */
  void
  vector_shrink( vector_t *self );


/**
 *  Removes all items.
 *
 *  @param  self  a vector structure
 */
  void
  vector_clear( vector_t *self );


/**
 *  Replace an item.
 *
 *  @param  self  a vector structure
 *  @param  index the index of the item to be replaced
 *  @param  item  the new item
 */
  void
  vector_set( vector_t *self,
              const size_t index,
              const void *item );


/**
 *  Erase an item.
 *
 *  @param  self  a vector structure
 *  @param  index the index of the item to be erased
 */
  void
  vector_erase( vector_t *self,
                const size_t index );


/**
 *  Erase a range of items.
 *
 *  @param  self  a vector structure
 *  @param  first the index of the first item to be erased
 *  @param  last  the index of the last item to be erased
 */
  void
  vector_erase_range( vector_t *self,
                      const size_t first,
                      const size_t last );


/**
 *  Appends given item to the end of the vector.
 *
 *  @param  self a vector structure
 *  @param  item the item to be inserted
 */
  void
  vector_push_back( vector_t *self,
                    const void *item );


/**
 *  Removes the last item of the vector.
 *
 *  @param  self a vector structure
 */
  void
  vector_pop_back( vector_t *self );


/**
 *  Resizes the vector to contain size items
 *
 *  If the current size is less than size, additional items are appended and
 *  initialized with value. If the current size is greater than size, the
 *  vector is reduced to its first size elements.
 *
 *  @param  self a vector structure
 *  @param  size the new size
 */
  void
  vector_resize( vector_t *self,
                 const size_t size );


/**
 *  Insert a single item at specified index.
 *
 *  @param  self  a vector structure
 *  @param  index location before which to insert item
 *  @param  item  the item to be inserted
 */
  void
  vector_insert( vector_t *self,
                 const size_t index,
                 const void *item );


/**
 *  Insert raw data at specified index.
 *
 *  @param  self  a vector structure
 *  @param  index location before which to insert item
 *  @param  data  a pointer to the items to be inserted
 *  @param  count the number of items to be inserted
 */
  void
  vector_insert_data( vector_t *self,
                      const size_t index,
                      const void * data,
                      const size_t count );


/**
 *  Append raw data to the end of the vector.
 *
 *  @param  self  a vector structure
 *  @param  data  a pointer to the items to be inserted
 *  @param  count the number of items to be inserted
 */
  void
  vector_push_back_data( vector_t *self,
                         const void * data,
                         const size_t count );


/**
 *  Sort vector items according to cmp function.
 *
 *  @param  self  a vector structure
 *  @param  cmp   a pointer a comparison function
 */
  void
  vector_sort( vector_t *self,
               int (*cmp)(const void *, const void *) );


/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __VECTOR_H__ */
