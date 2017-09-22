/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __VERTEX_ATTRIBUTE_H__
#define __VERTEX_ATTRIBUTE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "opengl.h"
#include "vector.h"

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   vertex-attribute.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup vertex-attribut Vertex attribute
 *
 * Besides the required vertex position, vertices can have several other
 * numeric attributes. Each is specified in the format string with a letter,
 * the number of components and the data type.
 *
 * Each of the attributes is described in the table below with the set of valid
 * format strings written as a regular expression (for example, "v[234][if]"
 * means "v2f", "v3i", "v4f", etc. are all valid formats).
 *
 * Some attributes have a "recommended" format string, which is the most
 * efficient form for the video driver as it requires less conversion.
 *
 * <table>
 *   <tr>
 *     <th>Attribute</th>
 *     <th>Formats</th>
 *     <th>Recommended</th>
 *   </tr>
 *   <tr>
 *     <td>Vertex position</td>
 *     <td>"v[234][sifd]"</td>
 *     <td>"v[234]f"</td>
 *   </tr>
 *   <tr>
 *     <td> Color             </td>
 *     <td> "c[34][bBsSiIfd]" </td>
 *     <td> "c[34]B"          </td>
 *   </tr>
 *   <tr>
 *     <td> Edge flag </td>
 *     <td> "e1[bB]"  </td>
 *     <td>           </td>
 *   </tr>
 *   <tr>
 *    <td> Fog coordinate     </td>
 *    <td> "f[1234][bBsSiIfd]"</td>
 *    <td>                    </td>
 *   </tr>
 *   <tr>
 *     <td> Normal      </td>
 *     <td> "n3[bsifd]" </td>
 *     <td> "n3f"       </td>
 *   </tr>
 *   <tr>
 *     <td> Secondary color   </td>
 *     <td> "s[34][bBsSiIfd]" </td>
 *     <td> "s[34]B"          </td>
 *   </tr>
 *   <tr>
 *     <td> Texture coordinate </td>
 *     <td> "t[234][sifd]"     </td>
 *     <td> "t[234]f"          </td>
 *   </tr>
 *   <tr>
 *     <td> Generic attribute             </td>
 *     <td> "[0-15]g(n)?[1234][bBsSiIfd]" </td>
 *     <td>                               </td>
 *   </tr>
 * </table>
 *
 * The possible data types that can be specified in the format string are described below.
 *
 * <table>
 *   <tr>
 *     <th> Format   </th>
 *     <th> Type     </th>
 *     <th> GL Type  </th>
 *   </tr>
 *   <tr>
 *     <td> "b"               </td>
 *     <td> Signed byte       </td>
 *     <td> GL_BYTE           </td>
 *   </tr>
 *   <tr>
 *     <td> "B"               </td>
 *     <td> Unsigned byte     </td>
 *     <td> GL_UNSIGNED_BYTE  </td>
 *   </tr>
 *   <tr>
 *     <td> "s"               </td>
 *     <td> Signed short      </td>
 *     <td> GL_SHORT          </td>
 *   </tr>
 *   <tr>
 *     <td> "S"               </td>
 *     <td> Unsigned short    </td>
 *     <td> GL_UNSIGNED_SHORT </td>
 *   </tr>
 *   <tr>
 *     <td> "i"               </td>
 *     <td> Signed int        </td>
 *     <td> GL_INT            </td>
 *   </tr>
 *   <tr>
 *     <td> "I"               </td>
 *     <td> Unsigned int      </td>
 *     <td> GL_UNSIGNED_INT   </td>
 *   </tr>
 *   <tr>
 *     <td> "f"               </td>
 *     <td> Float             </td>
 *     <td> GL_FLOAT          </td>
 *   </tr>
 *   <tr>
 *     <td> "d"               </td>
 *     <td> Double            </td>
 *     <td> GL_DOUBLE     T   </td>
 *   </tr>
 * </table>
 *
 * The following attributes are normalised to the range [0, 1]. The value is
 * used as-is if the data type is floating-point. If the data type is byte,
 * short or int, the value is divided by the maximum value representable by
 * that type. For example, unsigned bytes are divided by 255 to get the
 * normalised value.
 *
 *  - Color
 *  - Secondary color
 *  - Generic attributes with the "n" format given.
 *
 * Up to 16 generic attributes can be specified per vertex, and can be used by
 * shader programs for any purpose (they are ignored in the fixed-function
 * pipeline). For the other attributes, consult the OpenGL programming guide
 * for details on their effects.
 *
 * When using the draw and related functions, attribute data is specified
 * alongside the vertex position data. The following example reproduces the two
 * points from the previous page, except that the first point is blue and the
 * second green:
 *
 * It is an error to provide more than one set of data for any attribute, or to
 * mismatch the size of the initial data with the number of vertices specified
 * in the first argument.
 *
 * @{
 */


/**
 * Maximum number of attributes per vertex
 *
 * @private
 */
#define MAX_VERTEX_ATTRIBUTE 16


/**
 *  Generic vertex attribute.
 */
typedef struct vertex_attribute_t
{
    /**
     *  atribute name
     */
    GLchar * name;

    /**
     * index of the generic vertex attribute to be modified.
     */
    GLuint index;

    /**
     * Number of components per generic vertex attribute.
     *
     * Must be 1, 2, 3, or 4. The initial value is 4.
     */
    GLint size;

    /**
     *  data type of each component in the array.
     *
     *  Symbolic constants GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT,
     *  GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_FLOAT, or GL_DOUBLE are
     *  accepted. The initial value is GL_FLOAT.
     */
    GLenum type;

    /**
     *  whether fixed-point data values should be normalized (GL_TRUE) or
     *  converted directly as fixed-point values (GL_FALSE) when they are
     *  accessed.
     */
    GLboolean normalized;

    /**
     *  byte offset between consecutive generic vertex attributes.
     *
     *  If stride is 0, the generic vertex attributes are understood to be
     *  tightly packed in the array. The initial value is 0.
     */
    GLsizei stride;

    /**
     *  pointer to the first component of the first attribute element in the
     *  array.
     */
    GLvoid * pointer;

    /**
     * pointer to the function that enable this attribute.
     */
    void ( * enable )(void *);

} vertex_attribute_t;



/**
 * Create an attribute from the given parameters.
 *
 * @param size       number of component
 * @param type       data type
 * @param normalized Whether fixed-point data values should be normalized
                     (GL_TRUE) or converted directly as fixed-point values
                     (GL_FALSE) when they are  accessed.
 * @param stride     byte offset between consecutive attributes.
 * @param pointer    pointer to the first component of the first attribute
 *                   element in the array.
 * @return           a new initialized vertex attribute.
 *
 * @private
 */
vertex_attribute_t *
vertex_attribute_new( GLchar * name,
                      GLint size,
                      GLenum type,
                      GLboolean normalized,
                      GLsizei stride,
                      GLvoid *pointer );



/**
 * Delete a vertex attribute.
 *
 * @param  self a vertex attribute
 *
 */
void
vertex_attribute_delete( vertex_attribute_t * self );


/**
 * Create an attribute from the given description.
 *
 * @param  format Format string specifies the format of a vertex attribute.
 * @return        an initialized vertex attribute
 *
 * @private
 */
  vertex_attribute_t *
  vertex_attribute_parse( char *format );

/**
 * Enable a vertex attribute.
 *
 * @param attr  a vertex attribute
 *
 * @private
 */
  void
  vertex_attribute_enable( vertex_attribute_t *attr );


/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __VERTEX_ATTRIBUTE_H__ */
