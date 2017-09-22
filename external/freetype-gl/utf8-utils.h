/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __UTF8_UTILS_H__
#define __UTF8_UTILS_H__

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {

namespace ftgl {
#endif

/**
 * @file    utf8-utils.h
 * @author  Marcel Metz <mmetz@adrian-broher.net>
 *
 * defgroup utf8-utils UTF-8 Utilities
 *
 * @{
 */

  /**
   * Returns the size in bytes of a given UTF-8 encoded character surrogate
   *
   * @param character  An UTF-8 encoded character
   *
   * @return  The length of the surrogate in bytes.
   */
  size_t
  utf8_surrogate_len( const char* character );

  /**
   * Return the length of the given UTF-8 encoded and
   * NULL terminated string.
   *
   * @param string  An UTF-8 encoded string
   *
   * @return  The length of the string in characters.
   */
  size_t
  utf8_strlen( const char* string );

  /**
   * Converts a given UTF-8 encoded character to its UTF-32 LE equivalent
   *
   * @param character  An UTF-8 encoded character
   *
   * @return  The equivalent of the given character in UTF-32 LE
   *          encoding.
   */
  uint32_t
  utf8_to_utf32( const char * character );

/**
 * @}
 */

#ifdef __cplusplus
}
}
#endif

#endif /* #define __UTF8_UTILS_H__ */
