/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __MARKUP_H__
#define __MARKUP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "texture-font.h"
#include "vec234.h"

#ifdef __cplusplus
namespace ftgl {
#endif

/**
 * @file   markup.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup markup Markup
 *
 * Simple structure that describes text properties.
 *
 * <b>Example Usage</b>:
 * @code
 * #include "markup.h"
 *
 * ...
 *
 * vec4 black  = {{0.0, 0.0, 0.0, 1.0}};
 * vec4 white  = {{1.0, 1.0, 1.0, 1.0}};
 * vec4 none   = {{1.0, 1.0, 1.0, 0.0}};
 *
 * markup_t normal = {
 *     .family  = "Droid Serif",
 *     .size = 24.0,
 *     .bold = 0,
 *     .italic = 0,
 *     .spacing = 1.0,
 *     .gamma = 1.0,
 *     .foreground_color = black, .background_color    = none,
 *     .underline        = 0,     .underline_color     = black,
 *     .overline         = 0,     .overline_color      = black,
 *     .strikethrough    = 0,     .strikethrough_color = black,
 *     .font = 0,
 * };
 *
 * ...
 *
 * @endcode
 *
 * @{
 */


/**
 * Simple structure that describes text properties.
 */
typedef struct markup_t
{
    /**
     * A font family name such as "normal", "sans", "serif" or "monospace".
     */
    char * family;

    /**
     * Font size.
     */
    float size;

    /**
     * Whether text is bold.
     */
    int bold;

    /**
     * Whether text is italic.
     */
    int italic;

    /**
     * Spacing between letters.
     */
    float spacing;

    /**
     * Gamma correction.
     */
    float gamma;

    /**
     * Text color.
     */
    vec4 foreground_color;

    /**
     * Background color.
     */
    vec4 background_color;

    /**
     * Whether outline is active.
     */
    int outline;

    /**
     * Outline color.
     */
    vec4 outline_color;

    /**
     * Whether underline is active.
     */
    int underline;

    /**
     * Underline color.
     */
    vec4 underline_color;

    /**
     * Whether overline is active.
     */
    int overline;

    /**
     * Overline color.
     */
    vec4 overline_color;

    /**
     * Whether strikethrough is active.
     */
    int strikethrough;

    /**
     * Strikethrough color.
     */
    vec4 strikethrough_color;

    /**
     * Pointer on the corresponding font (family/size/bold/italic)
     */
    texture_font_t * font;

} markup_t;

/**
  * Default markup
  */
extern markup_t default_markup;


/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __MARKUP_H__ */
