/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
namespace ftgl {
#endif

#ifdef __APPLE__
    /* strndup() was only added in OSX lion */
    char * strndup( const char *s1, size_t n);
#elif defined(_WIN32) || defined(_WIN64)
    /* does not exist on windows */
    char * strndup( const char *s1, size_t n);
#    pragma warning (disable: 4244) // suspend warnings
#endif // _WIN32 || _WIN64

#ifdef __cplusplus
}
}
#endif // __cplusplus

#endif /* __PLATFORM_H__ */
