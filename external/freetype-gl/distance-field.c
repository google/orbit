/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "edtaa3func.h"


double *
make_distance_mapd( double *data, unsigned int width, unsigned int height )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    double vmin = DBL_MAX;
    unsigned int i;

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0.0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset( gx, 0, sizeof(double)*width*height );
    memset( gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, width, height, gx, gy );
    edtaa3( data, gx, gy, width, height, xdist, ydist, inside );
    for( i=0; i<width*height; ++i )
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        if( outside[i] < vmin )
            vmin = outside[i];
    }

    vmin = fabs(vmin);

    for( i=0; i<width*height; ++i)
    {
        double v = outside[i];
        if     ( v < -vmin) outside[i] = -vmin;
        else if( v > +vmin) outside[i] = +vmin;
        data[i] = (outside[i]+vmin)/(2*vmin);
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( outside );
    free( inside );
    return data;
}

unsigned char *
make_distance_mapb( unsigned char *img,
                    unsigned int width, unsigned int height )
{
    double * data    = (double *) calloc( width * height, sizeof(double) );
    unsigned char *out = (unsigned char *) malloc( width * height * sizeof(unsigned char) );
    unsigned int i;

    // find minimimum and maximum values
    double img_min = DBL_MAX;
    double img_max = DBL_MIN;

    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max)
            img_max = v;
        if (v < img_min)
            img_min = v;
    }

    // Map values from 0 - 255 to 0.0 - 1.0
    for( i=0; i<width*height; ++i)
        data[i] = (img[i]-img_min)/img_max;

    data = make_distance_mapd(data, width, height);

    // map values from 0.0 - 1.0 to 0 - 255
    for( i=0; i<width*height; ++i)
        out[i] = (unsigned char)(255*(1-data[i]));

    free( data );

    return out;
}
