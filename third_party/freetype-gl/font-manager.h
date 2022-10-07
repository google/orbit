/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "markup.h"
#include "texture-font.h"
#include "texture-atlas.h"

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   font-manager.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup font-manager Font manager
 *
 * Structure in charge of caching fonts.
 *
 * <b>Example Usage</b>:
 * @code
 * #include "font-manager.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *     font_manager_t * manager = manager_new( 512, 512, 1 );
 *     texture_font_t * font = font_manager_get( manager, "Mono", 12, 0, 0 );
 *
 *     return 0;
 * }
 * @endcode
 *
 * @{
 */


/**
 * Structure in charge of caching fonts.
 */
typedef struct font_manager_t {
    /**
     * Texture atlas to hold font glyphs.
     */
    texture_atlas_t * atlas;

    /**
     * Cached textures.
     */
    vector_t * fonts;

    /**
     * Default glyphs to be loaded when loading a new font.
     */
    char * cache;

} font_manager_t;



/**
 * Creates a new empty font manager.
 *
 * @param   width   width of the underlying atlas
 * @param   height  height of the underlying atlas
 * @param   depth   bit depth of the underlying atlas
 *
 * @return          a new font manager.
 *
 */
  font_manager_t *
  font_manager_new( size_t width,
                    size_t height,
                    size_t depth );


/**
 *  Deletes a font manager.
 *
 *  @param self a font manager.
 */
  void
  font_manager_delete( font_manager_t *self );


/**
 *  Deletes a font from the font manager.
 *
 *  Note that font glyphs are not removed from the atlas.
 *
 *  @param self a font manager.
 *  @param font font to be deleted
 *
 */
  void
  font_manager_delete_font( font_manager_t * self,
                            texture_font_t * font );


/**
 *  Request for a font based on a filename.
 *
 *  @param self     a font manager.
 *  @param filename font filename
 *  @param size     font size
 *
 *  @return Requested font
 */
  texture_font_t *
  font_manager_get_from_filename( font_manager_t * self,
                                  const char * filename,
                                  const float size );


/**
 *  Request for a font based on a description
 *
 *  @param self     a font manager
 *  @param family   font family
 *  @param size     font size
 *  @param bold     whether font is bold
 *  @param italic   whether font is italic
 *
 *  @return Requested font
 */
  texture_font_t *
  font_manager_get_from_description( font_manager_t * self,
                                     const char * family,
                                     const float size,
                                     const int bold,
                                     const int italic );


/**
 *  Request for a font based on a markup
 *
 *  @param self    a font manager
 *  @param markup  Markup describing a font
 *
 *  @return Requested font
 */
  texture_font_t *
  font_manager_get_from_markup( font_manager_t *self,
                                const markup_t *markup );


/**
 *  Search for a font filename that match description.
 *
 *  @param self    a font manager
 *  @param family   font family
 *  @param size     font size
 *  @param bold     whether font is bold
 *  @param italic   whether font is italic
 *
 *  @return Requested font filename
 */
  char *
  font_manager_match_description( font_manager_t * self,
                                  const char * family,
                                  const float size,
                                  const int bold,
                                  const int italic );

/** @} */

#ifdef __cplusplus
}
}
#endif // ifdef __cplusplus

#endif /* __FONT_MANAGER_H__ */
