/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __DISTANCE_FIELD_H__
#define __DISTANCE_FIELD_H__

#ifdef __cplusplus
extern "C" {
namespace ftgl {
#endif

/**
 * @file   shader.h
 * @author Nicolas Rougier (Nicolas.Rougier@inria.fr)
 *
 * @defgroup distance-field Distance Field
 *
 * Functions to calculate signed distance fields for bitmaps.
 *
 * <b>Example Usage</b>:
 * @code
 * #include "distance-field.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *     int width = 512;
 *     int height = 512;
 *     unsigned char *image = create_greyscale_image(width, height);
 *
 *     image = make_distance_map( image, width, height );
 *
 *     return 0;
 * }
 * @endcode
 *
 * @{
 */

/**
 * Create a distance file from the given image.
 *
 * @param img     A greyscale image.
 * @param width   The width of the given image.
 * @param height  The height of the given image.
 *
 * @return        A newly allocated distance field.  This image must
 *                be freed after usage.
 *
 */
double *
make_distance_mapd( double *img,
                    unsigned int width, unsigned int height );

unsigned char *
make_distance_mapb( unsigned char *img,
                    unsigned int width, unsigned int height );

/** @} */

#ifdef __cplusplus
}
}
#endif

#endif /* __DISTANCE_FIELD_H__ */
