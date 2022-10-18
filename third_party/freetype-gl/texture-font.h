/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __TEXTURE_FONT_H__
#define __TEXTURE_FONT_H__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "texture-atlas.h"

#ifndef __THREAD
#if defined(__GNUC__) || defined(__clang__)
#define __THREAD __thread
#elif defined(_MSC_VER)
#define __THREAD __declspec( thread )
#else
#define __THREAD
#endif
#endif

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   texture-font.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup texture-font Texture font
 *
 * Texture font.
 *
 * Example Usage:
 * @code
 * #include "texture-font.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *   return 0;
 * }
 * @endcode
 *
 * @{
 */


/**
 * A list of possible ways to render a glyph.
 */
typedef enum rendermode_t
{
    RENDER_NORMAL,
    RENDER_OUTLINE_EDGE,
    RENDER_OUTLINE_POSITIVE,
    RENDER_OUTLINE_NEGATIVE,
    RENDER_SIGNED_DISTANCE_FIELD
} rendermode_t;

/**
 * Glyph array end mark type
 */
typedef enum glyphmode_t
{
    GLYPH_END=0,
    GLYPH_CONT=1
} glyphmode_t;

/*
 * Glyph metrics:
 * --------------
 *
 *                       xmin                     xmax
 *                        |                         |
 *                        |<-------- width -------->|
 *                        |                         |
 *              |         +-------------------------+----------------- ymax
 *              |         |    ggggggggg   ggggg    |     ^        ^
 *              |         |   g:::::::::ggg::::g    |     |        |
 *              |         |  g:::::::::::::::::g    |     |        |
 *              |         | g::::::ggggg::::::gg    |     |        |
 *              |         | g:::::g     g:::::g     |     |        |
 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
 *              |         | g:::::g     g:::::g     |     |        |
 *              |         | g::::::g    g:::::g     |     |        |
 *              |         | g:::::::ggggg:::::g     |     |        |
 *              |         |  g::::::::::::::::g     |     |      height
 *              |         |   gg::::::::::::::g     |     |        |
 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
 *            / |         |             g:::::g     |              |
 *     origin   |         | gggggg      g:::::g     |              |
 *              |         | g:::::gg   gg:::::g     |              |
 *              |         |  g::::::ggg:::::::g     |              |
 *              |         |   gg:::::::::::::g      |              |
 *              |         |     ggg::::::ggg        |              |
 *              |         |         gggggg          |              v
 *              |         +-------------------------+----------------- ymin
 *              |                                   |
 *              |------------- advance_x ---------->|
 */

/**
 * A structure that describe a glyph.
 */
typedef struct texture_glyph_t
{
    /**
     * Unicode codepoint this glyph represents in UTF-32 LE encoding.
     */
    uint32_t codepoint;

    /**
     * Glyph's width in pixels.
     */
    size_t width;

    /**
     * Glyph's height in pixels.
     */
    size_t height;

    /**
     * Glyph's left bearing expressed in integer pixels.
     */
    int offset_x;

    /**
     * Glyphs's top bearing expressed in integer pixels.
     *
     * Remember that this is the distance from the baseline to the top-most
     * glyph scanline, upwards y coordinates being positive.
     */
    int offset_y;

    /**
     * For horizontal text layouts, this is the horizontal distance (in
     * fractional pixels) used to increment the pen position when the glyph is
     * drawn as part of a string of text.
     */
    float advance_x;

    /**
     * For vertical text layouts, this is the vertical distance (in fractional
     * pixels) used to increment the pen position when the glyph is drawn as
     * part of a string of text.
     */
    float advance_y;

    /**
     * First normalized texture coordinate (x) of top-left corner
     */
    float s0;

    /**
     * Second normalized texture coordinate (y) of top-left corner
     */
    float t0;

    /**
     * First normalized texture coordinate (x) of bottom-right corner
     */
    float s1;

    /**
     * Second normalized texture coordinate (y) of bottom-right corner
     */
    float t1;

    /**
     * A vector of kerning pairs relative to this glyph.
     */
    vector_t * kerning;

    /**
     * Mode this glyph was rendered
     */
    rendermode_t rendermode;

    /**
     * Glyph outline thickness
     */
    float outline_thickness;

    /**
     * Glyph scan end mark
     */
    glyphmode_t glyphmode;

} texture_glyph_t;

/**
 * Enum type for texture location
 */
typedef enum font_location_t {
    TEXTURE_FONT_FILE = 0,
    TEXTURE_FONT_MEMORY
} font_location_t;

/**
 * Enum type for automatic open/close
 */
typedef enum font_mode_t {
    MODE_AUTO_CLOSE = 0,
    MODE_GLYPHS_CLOSE,
    MODE_FREE_CLOSE,
    MODE_MANUAL_CLOSE,
    MODE_ALWAYS_OPEN
} font_mode_t;
/**
 * default mode for fonts
 */
extern __THREAD font_mode_t mode_default;

/** set defualt mode for fonts
 *
 * @param  mode  The mode for automatic open/close new fonts get
 */

void
texture_font_default_mode(font_mode_t mode);

/* If there is no Freetype included, just define that as incomplete pointer */
#if !defined(FT2BUILD_H_) && !defined(__FT2BUILD_H__) && !defined(FREETYPE_H_)
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_SizeRec_* FT_Size;
#endif

/* same for harfbuzz */
#ifndef HB_BUFFER_H
typedef struct hb_font_t hb_font_t;
#endif

/**
 *  Texture font library structure.
 */
typedef struct texture_font_library_t
{
    /**
     * Flag for mode
     */
    font_mode_t mode;

    /**
     * Freetype library pointer
     */
    FT_Library library;
} texture_font_library_t;

/**
 *  Texture font structure.
 */
typedef struct texture_font_t
{
    /**
     * Vector of glyphs contained in this font.
     * This is actually a two-stage table, indexing into 256 glyphs each
     */
    vector_t * glyphs;

    /**
     * Atlas structure to store glyphs data.
     */
    texture_atlas_t * atlas;
    
    /**
     * font location
     */
    font_location_t location;

    union {
        /**
         * Font filename, for when location == TEXTURE_FONT_FILE
         */
        char *filename;

        /**
         * Font memory address, for when location == TEXTURE_FONT_MEMORY
         */
        struct {
            const void *base;
            size_t size;
        } memory;
    };

    /**
     * Texture font library
     */

    texture_font_library_t * library;
  
    /**
     * Font size
     */
    float size;

    /**
     * Mode the font is rendering its next glyph
     */
    rendermode_t rendermode;

    /**
     * Outline thickness
     */
    float outline_thickness;

    /**
     * Whether to use our own lcd filter.
     */
    unsigned char filtering;
    /**
     * Whether to use kerning if available
     */
    unsigned char kerning;

    /**
     * Whether to use autohint when rendering font
     */
    unsigned char hinting;

    /**
     * Whether to scale texture coordinates
     */
    unsigned char scaletex;

    /**
     * LCD filter weights
     */
    unsigned char lcd_weights[5];

    /**
     * This field is simply used to compute a default line spacing (i.e., the
     * baseline-to-baseline distance) when writing text with this font. Note
     * that it usually is larger than the sum of the ascender and descender
     * taken as absolute values. There is also no guarantee that no glyphs
     * extend above or below subsequent baselines when using this distance.
     */
    float height;

    /**
     * This field is the distance that must be placed between two lines of
     * text. The baseline-to-baseline distance should be computed as:
     * ascender - descender + linegap
     */
    float linegap;

    /**
     * The ascender is the vertical distance from the horizontal baseline to
     * the highest 'character' coordinate in a font face. Unfortunately, font
     * formats define the ascender differently. For some, it represents the
     * ascent of all capital latin characters (without accents), for others it
     * is the ascent of the highest accented character, and finally, other
     * formats define it as being equal to bbox.yMax.
     */
    float ascender;

    /**
     * The descender is the vertical distance from the horizontal baseline to
     * the lowest 'character' coordinate in a font face. Unfortunately, font
     * formats define the descender differently. For some, it represents the
     * descent of all capital latin characters (without accents), for others it
     * is the ascent of the lowest accented character, and finally, other
     * formats define it as being equal to bbox.yMin. This field is negative
     * for values below the baseline.
     */
    float descender;

    /**
     * The position of the underline line for this face. It is the center of
     * the underlining stem. Only relevant for scalable formats.
     */
    float underline_position;

    /**
     * The thickness of the underline for this face. Only relevant for scalable
     * formats.
     */
    float underline_thickness;

    /**
    * The padding to be add to the glyph's texture that are loaded by this font.
    * Usefull when adding effects with shaders.
    */
    int padding;

    /**
     * Flag for mode
     */
    font_mode_t mode;

    /**
     * Freetype face pointer
     */
    FT_Face face;

    /**
     * Freetype size pointer
     */
    FT_Size ft_size;

    /**
     * Harfbuzz font pointer
     */
    hb_font_t* hb_font;

    /**
     * factor to scale font coordinates
     */
    float scale;
} texture_font_t;

/**
 * This function creates a new font library
 *
 * @return a new library (no font loaded yet)
 */
  texture_font_library_t *
      texture_library_new(void);

/**
 * This variable holds the per-thread library
 */

  extern __THREAD texture_font_library_t * freetype_gl_library;
  
/**
 * This function creates a new texture font from given filename and size.  The
 * texture atlas is used to store glyph on demand. Note the depth of the atlas
 * will determine if the font is rendered as alpha channel only (depth = 1) or
 * RGB (depth = 3) that correspond to subpixel rendering (if available on your
 * freetype implementation), or RGBA (depth = 4) for color fonts.
 *
 * @param atlas     A texture atlas
 * @param pt_size   Size of font to be created (in points)
 * @param filename  A font filename
 *
 * @return A new empty font (no glyph inside yet)
 *
 */
  texture_font_t *
  texture_font_new_from_file( texture_atlas_t * atlas,
                              const float pt_size,
                              const char * filename );


/**
 * This function creates a new texture font from a memory location and size.
 * The texture atlas is used to store glyph on demand. Note the depth of the
 * atlas will determine if the font is rendered as alpha channel only
 * (depth = 1) or RGB (depth = 3) that correspond to subpixel rendering (if
 * available on your freetype implementation).
 *
 * @param atlas       A texture atlas
 * @param pt_size     Size of font to be created (in points)
 * @param memory_base Start of the font file in memory
 * @param memory_size Size of the font file memory region, in bytes
 *
 * @return A new empty font (no glyph inside yet)
 *
 */
  texture_font_t *
  texture_font_new_from_memory( texture_atlas_t *atlas,
                                float pt_size,
                                const void *memory_base,
                                size_t memory_size );

/**
 * Clone the freetype-gl font and set a different size
 *
 * @param self         a valid texture font
 * @param size         the new size of the font
 */
  texture_font_t *
  texture_font_clone( texture_font_t *self,
		      float size);

/**
 * Close the freetype structures from a font and the associated library
 *
 * @param self         a valid texture font
 * @param face_mode    if the mode of the face is less or equal, be done with it
 * @param library_mode if the mode of the library is less or equal, be done with it
 */
  void
  texture_font_close( texture_font_t *self, font_mode_t face_mode, font_mode_t library_mode );

/**
 * Delete a texture font. Note that this does not delete the glyph from the
 * texture atlas.
 *
 * @param self a valid texture font
 */
  void
  texture_font_delete( texture_font_t * self );


/**
 * Load a texture font.
 *
 * @param self  a valid texture font
 * @param size  the size of the font
 *
 * @return 1 on success, 0 on error
 */
  int
  texture_font_load_face( texture_font_t * self, float size );


/**
 * Request a new glyph from the font. If it has not been created yet, it will
 * be.
 *
 * @param self      A valid texture font
 * @param codepoint Character codepoint to be loaded in UTF-8 encoding.
 *
 * @return A pointer on the new glyph or 0 if the texture atlas is not big
 *         enough
 *
 */
  texture_glyph_t *
  texture_font_get_glyph( texture_font_t * self,
                          const char * codepoint );

/**
 * Request an already loaded glyph from the font.
 *
 * @param self      A valid texture font
 * @param codepoint Character codepoint to be found in UTF-8 encoding.
 *
 * @return A pointer on the glyph or 0 if the glyph is not loaded
 */
 texture_glyph_t *
 texture_font_find_glyph( texture_font_t * self,
                          const char * codepoint );
    
/** 
 * Index a glyph in a font
 * 
 * @param self      A valid texture font
 * @param glyph     The glyph to index in the font
 * @param codepoint The codepoint to insert into
 *
 * @return          1 if glyph was copied, 0 if it was inserted
 */
int
texture_font_index_glyph( texture_font_t * self,
			  texture_glyph_t * glyph,
			  uint32_t codepoint );
    
/**
 * Request the loading of a given glyph.
 *
 * @param self       A valid texture font
 * @param codepoint  Character codepoint to be loaded in UTF-8 encoding.
 *
 * @return One if the glyph could be loaded, zero if not.
 */
int
texture_font_load_glyph( texture_font_t * self,
			 const char * codepoint );

/**
 * Request a new glyph from the font. If it has not been created yet, it will
 * be.
 *
 * @param self        A valid texture font
 * @param glyph_index Font's character glyph index to be obtained
 *
 * @return A pointer on the new glyph or 0 if the texture atlas is not big
 *         enough
 *
 */
texture_glyph_t *
texture_font_get_glyph_gi( texture_font_t * self,
			   uint32_t glyph_index );

/**
 * Request an already loaded glyph from the font. 
 *
 * @param self         A valid texture font
 * @param glyph_index  Font's character codepoint to be found
 *
 * @return A pointer on the glyph or 0 if the glyph is not loaded
 */
texture_glyph_t *
texture_font_find_glyph_gi( texture_font_t * self,
			    uint32_t glyph_index );

/**
 * Request the loading of a given glyph.
 *
 * @param self         A valid texture font
 * @param glyph_index  Character codepoint to be loaded in font's codepoint
 * @param ucodepoint   Character codepoint for inserting into lookup table
 *
 * @return One if the glyph could be loaded, zero if not.
 */
int
texture_font_load_glyph_gi( texture_font_t * self,
			    uint32_t glyph_index,
			    uint32_t ucodepoint);

/**
 * Request the loading of several glyphs at once.
 *
 * @param self       A valid texture font
 * @param codepoints Character codepoints to be loaded in UTF-8 encoding. May
 *                   contain duplicates.
 *
 * @return Number of missed glyph if the texture is not big enough to hold
 *         every glyphs.
 */
  size_t
  texture_font_load_glyphs( texture_font_t * self,
                            const char * codepoints );
/**
 * Increases the size of a fonts texture atlas
 * Invalidates all pointers to font->atlas->data
 * Changes the UV Coordinates of existing glyphs in the font
 *
 * @param self A valid texture font
 * @param width_new Width of the texture atlas after resizing (must be bigger
 *                  or equal to current width)
 * @param height_new Height of the texture atlas after resizing (must be bigger or
 *                   equal to current height)
 */
  void
  texture_font_enlarge_atlas( texture_font_t * self, size_t width_new,
			      size_t height_new );

/**
 * Changes the UV Coordinates of existing glyphs in the font
 *
 * @param self A valid texture font
 * @param mulw scale factor for width
 * @param mulh scale factor for height
 */
  void
  texture_font_enlarge_glyphs( texture_font_t * self, float mulw, float mulh );
  
/**
 * Increases the size of a fonts texture atlas
 *
 * @param self A valid texture font
 * @param width_new Width of the texture atlas after resizing (must be bigger
 *                  or equal to current width)
 * @param height_new Height of the texture atlas after resizing (must be bigger or
 *                   equal to current height)
 */
  void
  texture_font_enlarge_texture( texture_font_t * self, size_t width_new,
				size_t height_new );
/**
 * Get the kerning between two horizontal glyphs.
 *
 * @param self      A valid texture glyph
 * @param codepoint Character codepoint of the peceding character in UTF-8 encoding.
 *
 * @return x kerning value
 */
float
texture_glyph_get_kerning( const texture_glyph_t * self,
                           const char * codepoint );


/**
 * Creates a new empty glyph
 *
 * @return a new empty glyph (not valid)
 */
texture_glyph_t *
texture_glyph_new( void );

/**
 * Delete a glyph
 *
 * @param  self         A valid texture glyph
 */
void
texture_glyph_delete( texture_glyph_t * self );

/**
 * Clone a glyph
 *
 * @param self         A valid texture glyph
 */
texture_glyph_t*
texture_glyph_clone( texture_glyph_t* self );

/** @} */

#define GLYPHS_ITERATOR1(index, name, glyph) \
    for( index = 0; index < vector_size ( glyph ); index++ ) { \
	texture_glyph_t ** __glyphs;
#define GLYPHS_ITERATOR2(index, name, glyph) \
	if(( __glyphs = *(texture_glyph_t *** ) vector_get ( glyph, index ) )) { \
	    int __i;							\
	    for( __i = 0; __i < 0x100; __i++ ) {			\
		if(( name = __glyphs[__i] ))
#define GLYPHS_ITERATOR(index, name, glyph) \
    GLYPHS_ITERATOR1(index, name, glyph)		\
	GLYPHS_ITERATOR2(index, name, glyph)

#define GLYPHS_ITERATOR_END1 }
#define GLYPHS_ITERATOR_END2 } }
#define GLYPHS_ITERATOR_END } } }

#ifdef __cplusplus
}
}
#endif

#endif /* __TEXTURE_FONT_H__ */
