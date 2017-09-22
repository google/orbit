/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __TEXT_BUFFER_H__
#define __TEXT_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vertex-buffer.h"
#include "markup.h"

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * Use LCD filtering
 */
#define LCD_FILTERING_ON    3

/**
 * Do not use LCD filtering
 */
#define LCD_FILTERING_OFF 1

/**
 * @file   text-buffer.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup text-buffer Text buffer
 *
 *
 * <b>Example Usage</b>:
 * @code
 *
 * int main( int arrgc, char *argv[] )
 * {
 *
 *     return 0;
 * }
 * @endcode
 *
 * @{
 */

/**
 * Text buffer structure
 */
typedef struct  text_buffer_t {
    /**
     * Vertex buffer
     */
    vertex_buffer_t *buffer;

    /**
     * Base color for text
     */
    vec4 base_color;


    /**
     * Pen origin
     */
    vec2 origin;

    /**
     * Last pen y location
     */
    float last_pen_y;

    /**
     * Total bounds
     */
    vec4 bounds;

    /**
     * Index (in the vertex buffer) of the current line start
     */
    size_t line_start;

    /**
     * Location of the start of the line
     */
    float line_left;

    /**
     * Vector of line information
     */
    vector_t * lines;

    /**
     * Current line ascender
     */
    float line_ascender;

    /**
     * Current line decender
     */
    float line_descender;
} text_buffer_t;



/**
 * Glyph vertex structure
 */
typedef struct glyph_vertex_t {
    /**
     * Vertex x coordinates
     */
    float x;

    /**
     * Vertex y coordinates
     */
    float y;

    /**
     * Vertex z coordinates
     */
    float z;

    /**
     * Texture first coordinate
     */
    float u;

    /**
     * Texture second coordinate
     */
    float v;

    /**
     * Color red component
     */
    float r;

    /**
     * Color green component
     */
    float g;

    /**
     * Color blue component
     */
    float b;

    /**
     * Color alpha component
     */
    float a;

    /**
     * Shift along x
     */
    float shift;

    /**
     * Color gamma correction
     */
    float gamma;

} glyph_vertex_t;


/**
 * Line structure
 */
typedef struct line_info_t {
    /**
     * Index (in the vertex buffer) where this line starts
     */
    size_t line_start;

    /**
     * bounds of this line
     */
    vec4 bounds;

} line_info_t;

/**
 * Align enumeration
 */
typedef enum Align
{
    /**
     * Align text to the left hand side
     */
    ALIGN_LEFT,

    /**
     * Align text to the center
     */
    ALIGN_CENTER,

    /**
     * Align text to the right hand side
     */
    ALIGN_RIGHT
} Align;


/**
 * Creates a new empty text buffer.
 *
 * @return  a new empty text buffer.
 *
 */
  text_buffer_t *
  text_buffer_new( );

/**
 * Deletes texture buffer and its associated vertex buffer.
 *
 * @param  self  texture buffer to delete
 *
 */
  void
  text_buffer_delete( text_buffer_t * self );


 /**
  * Print some text to the text buffer
  *
  * @param self a text buffer
  * @param pen  position of text start
  * @param ...  a series of markup_t *, char * ended by NULL
  *
  */
  void
  text_buffer_printf( text_buffer_t * self, vec2 * pen, ... );


 /**
  * Add some text to the text buffer
  *
  * @param self   a text buffer
  * @param pen    position of text start
  * @param markup Markup to be used to add text
  * @param text   Text to be added
  * @param length Length of text to be added
  */
  void
  text_buffer_add_text( text_buffer_t * self,
                        vec2 * pen, markup_t * markup,
                        const char * text, size_t length );

 /**
  * Add a char to the text buffer
  *
  * @param self     a text buffer
  * @param pen      position of text start
  * @param markup   markup to be used to add text
  * @param current  charactr to be added
  * @param previous previous character (if any)
  */
  void
  text_buffer_add_char( text_buffer_t * self,
                        vec2 * pen, markup_t * markup,
                        const char * current, const char * previous );

 /**
  * Align all the lines of text already added to the buffer
  * This alignment will be relative to the overall bounds of the
  * text which can be queried by text_buffer_get_bounds
  *
  * @param self      a text buffer
  * @param pen       pen used in last call (must be unmodified)
  * @param alignment desired alignment of text
  */
  void
  text_buffer_align( text_buffer_t * self, vec2 * pen,
                     enum Align alignment );

 /**
  * Get the rectangle surrounding the text
  *
  * @param self      a text buffer
  * @param pen       pen used in last call (must be unmodified)
  */
  vec4
  text_buffer_get_bounds( text_buffer_t * self, vec2 * pen );

/**
  * Clear text buffer
  *
  * @param self a text buffer
 */
  void
  text_buffer_clear( text_buffer_t * self );


/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* #define __TEXT_BUFFER_H__ */
