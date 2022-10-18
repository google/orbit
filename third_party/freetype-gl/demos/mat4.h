/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#ifndef __MAT4_H__
#define __MAT4_H__

#ifdef __cplusplus
extern "C" {
namespace ftgl {
#endif


/**
 *
 */
typedef union
{
    float data[16];    /**< All compoments at once     */
    struct {
        float m00, m01, m02, m03;
        float m10, m11, m12, m13;
        float m20, m21, m22, m23;
        float m30, m31, m32, m33;
    };
} mat4;


mat4 *
mat4_new( void );

void
mat4_set_identity( mat4 *self );

void
mat4_set_zero( mat4 *self );

void
mat4_multiply( mat4 *self, mat4 *other );

void
mat4_set_orthographic( mat4 *self,
                       float left,   float right,
                       float bottom, float top,
                       float znear,  float zfar );

void
mat4_set_perspective( mat4 *self,
                      float fovy,  float aspect,
                      float zNear, float zFar);

void
mat4_set_frustum( mat4 *self,
                  float left,   float right,
                  float bottom, float top,
                  float znear,  float zfar );

void
mat4_set_rotation( mat4 *self,
                   float angle,
                   float x, float y, float z);

void
mat4_set_translation( mat4 *self,
                      float x, float y, float z);

void
mat4_set_scaling( mat4 *self,
                  float x, float y, float z);

void
mat4_rotate( mat4 *self,
             float angle,
             float x, float y, float z);

void
mat4_translate( mat4 *self,
                float x, float y, float z);

void
mat4_scale( mat4 *self,
            float x, float y, float z);


#ifdef __cplusplus
}
}
#endif

#endif /* __MAT4_H__ */
