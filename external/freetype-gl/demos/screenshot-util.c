/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdlib.h>
#include <stdio.h>

#include "screenshot-util.h"


// ------------------------------------------------------------- screenshot ---
void screenshot( GLFWwindow* window, const char* path )
{
    int width, height;

    FILE* out = fopen( path, "wb" );

    glfwGetWindowSize( window, &width, &height );

    uint8_t* buffer = (uint8_t*)calloc( width * height * 3, sizeof(uint8_t) );

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels( 0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer );

    uint8_t tga_header[18] = { 0 };
    // Data code type -- 2 - uncompressed RGB image.
    tga_header[2] = 2;
    // Image width - low byte
    tga_header[12] = width & 0xFF;
    // Image width - high byte
    tga_header[13] = (width >> 8) & 0xFF;
    // Image height - low byte
    tga_header[14] = height & 0xFF;
    // Image height - high byte
    tga_header[15] = (height >> 8) & 0xFF;
    // Color bit depth
    tga_header[16] = 24;

    fwrite( tga_header, sizeof(uint8_t), 18, out );
    fwrite( buffer, sizeof(uint8_t), width * height * 3, out );

    fclose( out );
    free( buffer );
}
