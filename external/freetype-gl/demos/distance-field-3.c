/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <math.h>

#include "freetype-gl.h"
#include "distance-field.h"
#include "vertex-buffer.h"
#include "shader.h"
#include "mat4.h"
#include "texture-font.h"
#include "texture-atlas.h"
#include "platform.h"
#include "utf8-utils.h"
#include "screenshot-util.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <GLFW/glfw3.h>


#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

double total_time = 0.0;


// ------------------------------------------------------- typedef & struct ---
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;


// ------------------------------------------------------- global variables ---
float angle = 0;
GLuint program = 0;
vertex_buffer_t *buffer;
texture_font_t * font = 0;
texture_atlas_t *atlas;
mat4  model, view, projection;


// ------------------------------------------------------ MitchellNetravali ---
// Mitchell Netravali reconstruction filter
float
MitchellNetravali( float x )
{
    const float B = 1/3.0, C = 1/3.0; // Recommended
    // const float B =   1.0, C =   0.0; // Cubic B-spline (smoother results)
    // const float B =   0.0, C = 1/2.0; // Catmull-Rom spline (sharper results)
    x = fabs(x);
    if( x < 1 )
         return ( ( 12 -  9 * B - 6 * C) * x * x * x
                + (-18 + 12 * B + 6 * C) * x * x
                + (  6 -  2 * B) ) / 6;
    else if( x < 2 )
        return ( (     -B -  6 * C) * x * x * x
               + (  6 * B + 30 * C) * x * x
               + (-12 * B - 48 * C) * x
               + (  8 * B + 24 * C) ) / 6;
    else
        return 0;
}


// ------------------------------------------------------------ interpolate ---
float
interpolate( float x, float y0, float y1, float y2, float y3 )
{
    float c0 = MitchellNetravali(x-1);
    float c1 = MitchellNetravali(x  );
    float c2 = MitchellNetravali(x+1);
    float c3 = MitchellNetravali(x+2);
    float r =  c0*y0 + c1*y1 + c2*y2 + c3*y3;
    return min( max( r, 0.0 ), 1.0 );
}


// ------------------------------------------------------------------ scale ---
int
resize( double *src_data, size_t src_width, size_t src_height,
        double *dst_data, size_t dst_width, size_t dst_height )
{
    if( (src_width == dst_width) && (src_height == dst_height) )
    {
        memcpy( dst_data, src_data, src_width*src_height*sizeof(double));
        return 0;
    }
    size_t i,j;
    float xscale = src_width / (float) dst_width;
    float yscale = src_height / (float) dst_height;
    for( j=0; j < dst_height; ++j )
    {
        for( i=0; i < dst_width; ++i )
        {
            int src_i = (int) floor( i * xscale );
            int src_j = (int) floor( j * yscale );
            int i0 = min( max( 0, src_i-1 ), src_width-1 );
            int i1 = min( max( 0, src_i   ), src_width-1 );
            int i2 = min( max( 0, src_i+1 ), src_width-1 );
            int i3 = min( max( 0, src_i+2 ), src_width-1 );
            int j0 = min( max( 0, src_j-1 ), src_height-1 );
            int j1 = min( max( 0, src_j   ), src_height-1 );
            int j2 = min( max( 0, src_j+1 ), src_height-1 );
            int j3 = min( max( 0, src_j+2 ), src_height-1 );
            float t0 = interpolate( i / (float) dst_width,
                                    src_data[j0*src_width+i0],
                                    src_data[j0*src_width+i1],
                                    src_data[j0*src_width+i2],
                                    src_data[j0*src_width+i3] );
            float t1 = interpolate( i / (float) dst_width,
                                    src_data[j1*src_width+i0],
                                    src_data[j1*src_width+i1],
                                    src_data[j1*src_width+i2],
                                    src_data[j1*src_width+i3] );
            float t2 = interpolate( i / (float) dst_width,
                                    src_data[j2*src_width+i0],
                                    src_data[j2*src_width+i1],
                                    src_data[j2*src_width+i2],
                                    src_data[j2*src_width+i3] );
            float t3 = interpolate( i / (float) dst_width,
                                    src_data[j3*src_width+i0],
                                    src_data[j3*src_width+i1],
                                    src_data[j3*src_width+i2],
                                    src_data[j3*src_width+i3] );
            float y =  interpolate( j / (float) dst_height, t0, t1, t2, t3 );
            dst_data[j*dst_width+i] = y;
        }
    }
    return 0;
}


// ------------------------------------------------------------- load_glyph ---
texture_glyph_t *
load_glyph( const char *  filename,     const char* codepoint,
            const float   highres_size, const float   lowres_size,
            const float   padding )
{
    size_t i, j;
    FT_Library library;
    FT_Face face;

    FT_Init_FreeType( &library );
    FT_New_Face( library, filename, 0, &face );
    FT_Select_Charmap( face, FT_ENCODING_UNICODE );
    FT_UInt glyph_index = FT_Get_Char_Index( face, utf8_to_utf32( codepoint ) );

    // Render glyph at high resolution (highres_size points)
    FT_Set_Char_Size( face, highres_size*64, 0, 72, 72 );
    FT_Load_Glyph( face, glyph_index,
                   FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap bitmap = slot->bitmap;

    // Allocate high resolution buffer
    size_t highres_width  = bitmap.width + 2*padding*highres_size;
    size_t highres_height = bitmap.rows + 2*padding*highres_size;
    double * highres_data = (double *) malloc( highres_width*highres_height*sizeof(double) );
    memset( highres_data, 0, highres_width*highres_height*sizeof(double) );

    // Copy high resolution bitmap with padding and normalize values
    for( j=0; j < bitmap.rows; ++j )
    {
        for( i=0; i < bitmap.width; ++i )
        {
            int x = i + padding;
            int y = j + padding;
            highres_data[y*highres_width+x] = bitmap.buffer[j*bitmap.width+i]/255.0;
        }
    }

    // Compute distance map
    glfwSetTime(total_time);
    highres_data = make_distance_mapd( highres_data, highres_width, highres_height );
    total_time += glfwGetTime();

    // Allocate low resolution buffer
    size_t lowres_width  = round(highres_width * lowres_size/highres_size);
    size_t lowres_height = round(highres_height * lowres_width/(float) highres_width);
    double * lowres_data = (double *) malloc( lowres_width*lowres_height*sizeof(double) );
    memset( lowres_data, 0, lowres_width*lowres_height*sizeof(double) );

    // Scale down highres buffer into lowres buffer
    resize( highres_data, highres_width, highres_height,
            lowres_data,  lowres_width,  lowres_height );

    // Convert the (double *) lowres buffer into a (unsigned char *) buffer and
    // rescale values between 0 and 255.
    unsigned char * data =
        (unsigned char *) malloc( lowres_width*lowres_height*sizeof(unsigned char) );
    for( j=0; j < lowres_height; ++j )
    {
        for( i=0; i < lowres_width; ++i )
        {
            double v = lowres_data[j*lowres_width+i];
            data[j*lowres_width+i] = (int) (255*(1-v));
        }
    }

    // Compute new glyph information from highres value
    float ratio = lowres_size / highres_size;
    size_t pitch  = lowres_width * sizeof( unsigned char );

    // Create glyph
    texture_glyph_t * glyph = texture_glyph_new( );
    glyph->offset_x = (slot->bitmap_left + padding*highres_width) * ratio;
    glyph->offset_y = (slot->bitmap_top + padding*highres_height) * ratio;
    glyph->width    = lowres_width;
    glyph->height   = lowres_height;
    glyph->codepoint = utf8_to_utf32( codepoint );
    /*
    printf( "Glyph width:  %ld\n", glyph->width );
    printf( "Glyph height: %ld\n", glyph->height );
    printf( "Glyph offset x: %d\n", glyph->offset_x );
    printf( "Glyph offset y: %d\n", glyph->offset_y );
    */
    ivec4 region = texture_atlas_get_region( atlas, glyph->width, glyph->height );
    /*
    printf( "Region x : %d\n", region.x );
    printf( "Region y : %d\n", region.y );
    printf( "Region width : %d\n", region.width );
    printf( "Region height : %d\n", region.height );
    */
    texture_atlas_set_region( atlas, region.x, region.y, glyph->width, glyph->height, data, pitch );
    glyph->s0       = region.x/(float)atlas->width;
    glyph->t0       = region.y/(float)atlas->height;
    glyph->s1       = (region.x + glyph->width)/(float)atlas->width;
    glyph->t1       = (region.y + glyph->height)/(float)atlas->height;

    FT_Load_Glyph( face, glyph_index,
                   FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
    glyph->advance_x = ratio * face->glyph->advance.x/64.0;
    glyph->advance_y = ratio * face->glyph->advance.y/64.0;
    /*
    printf( "Advance x : %f\n", glyph->advance_x );
    printf( "Advance y : %f\n", glyph->advance_y );
    */
    free( highres_data );
    free( lowres_data );
    free( data );

    return glyph;
}


// ------------------------------------------------------------------- init ---
void init( void )
{
    atlas = texture_atlas_new( 512, 512, 1 );
    font = texture_font_new_from_file( atlas, 32, "fonts/Vera.ttf" );

    texture_glyph_t *glyph;

    // Generate the glyp at 512 points, compute distance field and scale it
    // back to 32 points
    // Just load another glyph if you want to see difference (draw render a '@')
    glyph = load_glyph( "fonts/Vera.ttf", "@", 512, 64, 0.1);
    vector_push_back( font->glyphs, &glyph );

    glyph = texture_font_get_glyph( font, "@");

    GLuint indices[6] = {0,1,2, 0,2,3};
    vertex_t vertices[4] = { { -.5,-.5,0,  glyph->s0,glyph->t1,  0,0,0,1 },
                             { -.5, .5,0,  glyph->s0,glyph->t0,  0,0,0,1 },
                             {  .5, .5,0,  glyph->s1,glyph->t0,  0,0,0,1 },
                             {  .5,-.5,0,  glyph->s1,glyph->t1,  0,0,0,1 } };
    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );

    glActiveTexture( GL_TEXTURE0 );
    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
                  0, GL_RED, GL_UNSIGNED_BYTE, atlas->data );

    program = shader_load( "shaders/distance-field.vert",
                           "shaders/distance-field-2.frag" );
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}


// ---------------------------------------------------------------- display ---
void display( GLFWwindow* window )
{
    glClearColor(1.0,1.0,1.0,1.0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    texture_glyph_t * glyph = texture_font_get_glyph( font, "@");

    int width, height;
    glfwGetFramebufferSize( window, &width, &height );

    float glyph_height = glyph->height * width/(float)glyph->width;
    float glyph_width  = glyph->width * height/(float)glyph->height;
    int x = -glyph_width/2 + width/2.;
    int y = -glyph_height/2 + height/2.;

    float s = .025+.975*(1+cos(angle/100.0))/2.;

    vec4 color = {{1.0, 1.0, 1.0, 1.0 }};

    mat4_set_identity( &model );
    mat4_scale( &model, width * s, width * s, 1 );
    mat4_rotate( &model, angle, 0, 0, 1 );
    mat4_translate( &model, width/2., height/2., 0 );

    glUseProgram( program );
    {
        glUniform1i( glGetUniformLocation( program, "u_texture" ),
                     0);
        glUniform4f( glGetUniformLocation( program, "u_color" ),
                     color.r, color.g, color.b, color.a);
        glUniformMatrix4fv( glGetUniformLocation( program, "u_model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "u_view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( program, "u_projection" ),
                            1, 0, projection.data);

        vertex_buffer_render( buffer, GL_TRIANGLES );
    }

    glfwSwapBuffers( window );
}


// ---------------------------------------------------------------- reshape ---
void reshape( GLFWwindow* window, int width, int height )
{
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}


// --------------------------------------------------------------- keyboard ---
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    fputs( description, stderr );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    GLFWwindow* window;
    char* screenshot_path = NULL;

    if (argc > 1)
    {
        if (argc == 3 && 0 == strcmp( "--screenshot", argv[1] ))
            screenshot_path = argv[2];
        else
        {
            fprintf( stderr, "Unknown or incomplete parameters given\n" );
            exit( EXIT_FAILURE );
        }
    }

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    window = glfwCreateWindow( 512, 512, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

    init();

    fprintf(stderr, "Total time to generate distance map: %fs\n", total_time);

    glfwShowWindow( window );
    {
        int pixWidth, pixHeight;
        glfwGetFramebufferSize( window, &pixWidth, &pixHeight );
        reshape( window, pixWidth, pixHeight );
    }

    glfwSetTime(1.0);

    while (!glfwWindowShouldClose( window ))
    {
        display( window );

        angle += 30 * glfwGetTime();
        glfwSetTime(0.0);

        glfwPollEvents( );

        if (screenshot_path)
        {
            screenshot( window, screenshot_path );
            glfwSetWindowShouldClose( window, 1 );
        }
    }

    glDeleteTextures( 1, &atlas->id );
    atlas->id = 0;
    texture_atlas_delete( atlas );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
