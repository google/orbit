/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include "ftgl-utils.h"

error_callback_t log_error = error_callback_default;

// ------------------------------------------------- error_callback_default ---
#ifdef __ANDROID__
#include <android/log.h>
#define  LOG_TAG    "freetype-gl"
void error_callback_default(const char* fmt, ...)
{
  va_list myargs;
  va_start(myargs, fmt);
  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
		      "Freetype GL Error %03x %s:\n", freetype_gl_errno, freetype_gl_message);
  __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG,
		       fmt, myargs);
  va_end(myargs);
}
#else
void error_callback_default(const char* fmt, ...)
{
  va_list myargs;
  va_start(myargs, fmt);
  vfprintf(stderr, fmt, myargs);
  va_end(myargs);
}
#endif

// ----------------------------------------------------- set_error_callback ---
void
set_error_callback(error_callback_t error_cb)
{
    log_error = error_cb;
}

__THREAD int freetype_gl_errno=0;
__THREAD int freetype_gl_warnings=0;
__THREAD const char * freetype_gl_message=NULL;

#undef FTERRORS_H_
#define FT_NOERRORDEF_( e, v, s )  [v] = s,
#define FT_ERRORDEF_( e, v, s )  [v] = s,
#define FTGL_ERRORDEF_( e, v, s )  [v+FTGL_ERR_BASE] = s,
#define FT_ERROR_START_LIST
#define FT_ERROR_END_LIST
#define FTGL_ERROR_START_LIST
#define FTGL_ERROR_END_LIST

const char* freetype_gl_errstrs[] = {
  #include <freetype/fterrdef.h>
  #include "freetype-gl-errdef.h"
  [FTGL_ERR_MAX+1] = NULL
};

const char* FTGL_Error_String( unsigned int error_code )
{
    if( error_code > FTGL_ERR_MAX) return NULL;
    return freetype_gl_errstrs[error_code];
}

#undef FTGL_ERRORDEF_
#undef __FREETYPE_GL_ERRORS_H__
#undef FT_ERROR_START_LIST
#undef FT_ERROR_END_LIST
#undef FTGL_ERROR_START_LIST
#undef FTGL_ERROR_END_LIST
