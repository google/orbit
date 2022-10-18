/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __VERTEX_BUFFER_H__
#define __VERTEX_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "opengl.h"
#include "vector.h"
#include "vertex-attribute.h"

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   vertex-buffer.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 * @date   April, 2012
 *
 * @defgroup vertex-buffer Vertex buffer
 *
 * @{
 */


/**
 * Generic vertex buffer.
 */
typedef struct vertex_buffer_t
{
    /** Format of the vertex buffer. */
    char * format;

    /** Vector of vertices. */
    vector_t * vertices;

#ifdef FREETYPE_GL_USE_VAO
    /** GL identity of the Vertex Array Object */
    GLuint VAO_id;
#endif

    /** GL identity of the vertices buffer. */
    GLuint vertices_id;

    /** Vector of indices. */
    vector_t * indices;

    /** GL identity of the indices buffer. */
    GLuint indices_id;

    /** Current size of the vertices buffer in GPU */
    size_t GPU_vsize;

    /** Current size of the indices buffer in GPU*/
    size_t GPU_isize;

    /** GL primitives to render. */
    GLenum mode;

    /** Whether the vertex buffer needs to be uploaded to GPU memory. */
    char state;

    /** Individual items */
    vector_t * items;

    /** Array of attributes. */
    vertex_attribute_t *attributes[MAX_VERTEX_ATTRIBUTE];
} vertex_buffer_t;


/**
 * Creates an empty vertex buffer.
 *
 * @param  format a string describing vertex format.
 * @return        an empty vertex buffer.
 */
  vertex_buffer_t *
  vertex_buffer_new( const char *format );


/**
 * Deletes vertex buffer and releases GPU memory.
 *
 * @param  self  a vertex buffer
 */
  void
  vertex_buffer_delete( vertex_buffer_t * self );


/**
 *  Returns the number of items in the vertex buffer
 *
 *  @param  self  a vertex buffer
 *  @return       number of items
 */
  size_t
  vertex_buffer_size( const vertex_buffer_t *self );


/**
 *  Returns vertex format
 *
 *  @param  self  a vertex buffer
 *  @return       vertex format
 */
  const char *
  vertex_buffer_format( const vertex_buffer_t *self );


/**
 * Print information about a vertex buffer
 *
 * @param  self  a vertex buffer
 */
  void
  vertex_buffer_print( vertex_buffer_t * self );


/**
 * Prepare vertex buffer for render.
 *
 * @param  self  a vertex buffer
 * @param  mode  render mode
 */
  void
  vertex_buffer_render_setup ( vertex_buffer_t *self,
                               GLenum mode );


/**
 * Finish rendering by setting back modified states
 *
 * @param  self  a vertex buffer
 */
  void
  vertex_buffer_render_finish ( vertex_buffer_t *self );


/**
 * Render vertex buffer.
 *
 * @param  self  a vertex buffer
 * @param  mode  render mode
 */
  void
  vertex_buffer_render ( vertex_buffer_t *self,
                         GLenum mode );


/**
 * Render a specified item from the vertex buffer.
 *
 * @param  self   a vertex buffer
 * @param  index index of the item to be rendered
 */
  void
  vertex_buffer_render_item ( vertex_buffer_t *self,
                              size_t index );


/**
 * Upload buffer to GPU memory.
 *
 * @param  self  a vertex buffer
 */
  void
  vertex_buffer_upload( vertex_buffer_t *self );


/**
 * Clear all items.
 *
 * @param  self  a vertex buffer
 */
  void
  vertex_buffer_clear( vertex_buffer_t *self );


/**
 * Appends indices at the end of the buffer.
 *
 * @param  self     a vertex buffer
 * @param  indices  indices to be appended
 * @param  icount   number of indices to be appended
 *
 * @private
 */
  void
  vertex_buffer_push_back_indices ( vertex_buffer_t *self,
                                    const GLuint * indices,
                                    const size_t icount );


/**
 * Appends vertices at the end of the buffer.
 *
 * @note Internal use
 *
 * @param  self     a vertex buffer
 * @param  vertices vertices to be appended
 * @param  vcount   number of vertices to be appended
 *
 * @private
 */
  void
  vertex_buffer_push_back_vertices ( vertex_buffer_t *self,
                                     const void * vertices,
                                     const size_t vcount );


/**
 * Insert indices in the buffer.
 *
 * @param  self    a vertex buffer
 * @param  index   location before which to insert indices
 * @param  indices indices to be appended
 * @param  icount  number of indices to be appended
 *
 * @private
 */
  void
  vertex_buffer_insert_indices ( vertex_buffer_t *self,
                                 const size_t index,
                                 const GLuint *indices,
                                 const size_t icount );


/**
 * Insert vertices in the buffer.
 *
 * @param  self     a vertex buffer
 * @param  index    location before which to insert vertices
 * @param  vertices vertices to be appended
 * @param  vcount   number of vertices to be appended
 *
 * @private
 */
  void
  vertex_buffer_insert_vertices ( vertex_buffer_t *self,
                                  const size_t index,
                                  const void *vertices,
                                  const size_t vcount );

/**
 * Erase indices in the buffer.
 *
 * @param  self   a vertex buffer
 * @param  first  the index of the first index to be erased
 * @param  last   the index of the last index to be erased
 *
 * @private
 */
  void
  vertex_buffer_erase_indices ( vertex_buffer_t *self,
                                const size_t first,
                                const size_t last );

/**
 * Erase vertices in the buffer.
 *
 * @param  self   a vertex buffer
 * @param  first  the index of the first vertex to be erased
 * @param  last   the index of the last vertex to be erased
 *
 * @private
 */
  void
  vertex_buffer_erase_vertices ( vertex_buffer_t *self,
                                 const size_t first,
                                 const size_t last );


/**
 * Append a new item to the collection.
 *
 * @param  self   a vertex buffer
 * @param  vcount   number of vertices
 * @param  vertices raw vertices data
 * @param  icount   number of indices
 * @param  indices  raw indices data
 */
  size_t
  vertex_buffer_push_back( vertex_buffer_t * self,
                           const void * vertices, const size_t vcount,
                           const GLuint * indices, const size_t icount );


/**
 * Insert a new item into the vertex buffer.
 *
 * @param  self      a vertex buffer
 * @param  index     location before which to insert item
 * @param  vertices  raw vertices data
 * @param  vcount    number of vertices
 * @param  indices   raw indices data
 * @param  icount    number of indices
 */
  size_t
  vertex_buffer_insert( vertex_buffer_t * self,
                        const size_t index,
                        const void * vertices, const size_t vcount,
                        const GLuint * indices, const size_t icount );

/**
 * Erase an item from the vertex buffer.
 *
 * @param  self     a vertex buffer
 * @param  index    index of the item to be deleted
 */
  void
  vertex_buffer_erase( vertex_buffer_t * self,
                       const size_t index );

/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __VERTEX_BUFFER_H__ */
