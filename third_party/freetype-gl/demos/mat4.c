/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "mat4.h"

mat4 *
mat4_new( void )
{
    mat4 *self = (mat4 *) malloc( sizeof(mat4) );
    return self;

}

void
mat4_set_zero( mat4 *self )
{
    if (!self)
        return;

    memset( self, 0, sizeof( mat4 ));
}

void
mat4_set_identity( mat4 *self )
{
    if (!self)
        return;

    memset( self, 0, sizeof( mat4 ));
    self->m00 = 1.0;
    self->m11 = 1.0;
    self->m22 = 1.0;
    self->m33 = 1.0;
}

void
mat4_multiply( mat4 *self, mat4 *other )
{
    mat4 m;
    size_t i;

    if (!self || !other)
        return;

    for( i=0; i<4; ++i )
    {
        m.data[i*4+0] =
            (self->data[i*4+0] * other->data[0*4+0]) +
            (self->data[i*4+1] * other->data[1*4+0]) +
            (self->data[i*4+2] * other->data[2*4+0]) +
            (self->data[i*4+3] * other->data[3*4+0]) ;

        m.data[i*4+1] =
            (self->data[i*4+0] * other->data[0*4+1]) +
            (self->data[i*4+1] * other->data[1*4+1]) +
            (self->data[i*4+2] * other->data[2*4+1]) +
            (self->data[i*4+3] * other->data[3*4+1]) ;

        m.data[i*4+2] =
            (self->data[i*4+0] * other->data[0*4+2]) +
            (self->data[i*4+1] * other->data[1*4+2]) +
            (self->data[i*4+2] * other->data[2*4+2]) +
            (self->data[i*4+3] * other->data[3*4+2]) ;

        m.data[i*4+3] =
            (self->data[i*4+0] * other->data[0*4+3]) +
            (self->data[i*4+1] * other->data[1*4+3]) +
            (self->data[i*4+2] * other->data[2*4+3]) +
            (self->data[i*4+3] * other->data[3*4+3]) ;
    }
    memcpy( self, &m, sizeof( mat4 ) );

}


void
mat4_set_orthographic( mat4 *self,
                       float left,   float right,
                       float bottom, float top,
                       float znear,  float zfar )
{
    if (!self)
        return;

    if (left == right || bottom == top || znear == zfar)
        return;

    mat4_set_zero( self );

    self->m00 = +2.0f/(right-left);
    self->m30 = -(right+left)/(right-left);
    self->m11 = +2.0f/(top-bottom);
    self->m31 = -(top+bottom)/(top-bottom);
    self->m22 = -2.0f/(zfar-znear);
    self->m32 = -(zfar+znear)/(zfar-znear);
    self->m33 = 1.0f;
}

void
mat4_set_perspective( mat4 *self,
                      float fovy,  float aspect,
                      float znear, float zfar)
{
    float h, w;

    if (!self)
        return;

    if (znear == zfar)
        return;

    h = (float)tan(fovy / 360.0 * M_PI) * znear;
    w = h * aspect;

    mat4_set_frustum( self, -w, w, -h, h, znear, zfar );
}

void
mat4_set_frustum( mat4 *self,
                  float left,   float right,
                  float bottom, float top,
                  float znear,  float zfar )
{

    if (!self)
        return;

    if (left == right || bottom == top || znear == zfar)
        return;

    mat4_set_zero( self );

    self->m00 = (2.0f*znear)/(right-left);
    self->m20 = (right+left)/(right-left);

    self->m11 = (2.0f*znear)/(top-bottom);
    self->m21 = (top+bottom)/(top-bottom);

    self->m22 = -(zfar+znear)/(zfar-znear);
    self->m32 = -(2.0f*zfar*znear)/(zfar-znear);

    self->m23 = -1.0f;
}

void
mat4_set_rotation( mat4 *self,
                   float angle,
                   float x, float y, float z)
{
    float c, s, norm;

    if (!self)
        return;

    c = (float)cos( M_PI*angle/180.0 );
    s = (float)sin( M_PI*angle/180.0 );
    norm = (float)sqrt(x*x+y*y+z*z);

    x /= norm; y /= norm; z /= norm;

    mat4_set_identity( self );

    self->m00 = x*x*(1-c)+c;
    self->m10 = y*x*(1-c)-z*s;
    self->m20 = z*x*(1-c)+y*s;

    self->m01 =  x*y*(1-c)+z*s;
    self->m11 =  y*y*(1-c)+c;
    self->m21 =  z*y*(1-c)-x*s;

    self->m02 = x*z*(1-c)-y*s;
    self->m12 = y*z*(1-c)+x*s;
    self->m22 = z*z*(1-c)+c;
}

void
mat4_set_translation( mat4 *self,
                      float x, float y, float z)
{
    if (!self)
        return;

    mat4_set_identity( self );
    self-> m30 = x;
    self-> m31 = y;
    self-> m32 = z;
}

void
mat4_set_scaling( mat4 *self,
                  float x, float y, float z)
{
    if (!self)
        return;

    mat4_set_identity( self );
    self-> m00 = x;
    self-> m11 = y;
    self-> m22 = z;
}

void
mat4_rotate( mat4 *self,
             float angle,
             float x, float y, float z)
{
    mat4 m;

    if (!self)
        return;

    mat4_set_rotation( &m, angle, x, y, z);
    mat4_multiply( self, &m );
}

void
mat4_translate( mat4 *self,
                float x, float y, float z)
{
    mat4 m;

    if (!self)
        return;

    mat4_set_translation( &m, x, y, z);
    mat4_multiply( self, &m );
}

void
mat4_scale( mat4 *self,
            float x, float y, float z)
{
    mat4 m;

    if (!self)
        return;

    mat4_set_scaling( &m, x, y, z);
    mat4_multiply( self, &m );
}
