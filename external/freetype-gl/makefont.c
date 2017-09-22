/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include "opengl.h"
#include "vec234.h"
#include "vector.h"
#include "freetype-gl.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>


#ifndef WIN32
#   define PRIzu "zu"
#else
#   define PRIzu "Iu"
#endif


// ------------------------------------------------------------- print help ---
void print_help()
{
    fprintf( stderr, "Usage: makefont [--help] --font <font file> "
             "--header <header file> --size <font size> "
             "--variable <variable name> --texture <texture size>"
             "--rendermode <one of 'normal', 'outline_edge', 'outline_positive', 'outline_negative' or 'sdf'>\n" );
}


// ------------------------------------------------------------------- main ---
int main( int argc, char **argv )
{
    FILE* test;
    size_t i, j;
    int arg;

    char * font_cache =
        " !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~";

    float  font_size   = 0.0;
    const char * font_filename   = NULL;
    const char * header_filename = NULL;
    const char * variable_name   = "font";
    int show_help = 0;
    size_t texture_width = 128;
    rendermode_t rendermode = RENDER_NORMAL;
    const char *rendermodes[5];
    rendermodes[RENDER_NORMAL] = "normal";
    rendermodes[RENDER_OUTLINE_EDGE] = "outline edge";
    rendermodes[RENDER_OUTLINE_POSITIVE] = "outline added";
    rendermodes[RENDER_OUTLINE_NEGATIVE] = "outline removed";
    rendermodes[RENDER_SIGNED_DISTANCE_FIELD] = "signed distance field";

    for ( arg = 1; arg < argc; ++arg )
    {
        if ( 0 == strcmp( "--font", argv[arg] ) || 0 == strcmp( "-f", argv[arg] ) )
        {
            ++arg;

            if ( font_filename )
            {
                fprintf( stderr, "Multiple --font parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No font file given.\n" );
                print_help();
                exit( 1 );
            }

            font_filename = argv[arg];
            continue;
        }

        if ( 0 == strcmp( "--header", argv[arg] ) || 0 == strcmp( "-o", argv[arg] )  )
        {
            ++arg;

            if ( header_filename )
            {
                fprintf( stderr, "Multiple --header parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No header file given.\n" );
                print_help();
                exit( 1 );
            }

            header_filename = argv[arg];
            continue;
        }

        if ( 0 == strcmp( "--help", argv[arg] ) || 0 == strcmp( "-h", argv[arg] ) )
        {
            show_help = 1;
            break;
        }

        if ( 0 == strcmp( "--size", argv[arg] ) || 0 == strcmp( "-s", argv[arg] ) )
        {
            ++arg;

            if ( 0.0 != font_size )
            {
                fprintf( stderr, "Multiple --size parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No font size given.\n" );
                print_help();
                exit( 1 );
            }

            errno = 0;

            font_size = atof( argv[arg] );

            if ( errno )
            {
                fprintf( stderr, "No valid font size given.\n" );
                print_help();
                exit( 1 );
            }

            continue;
        }

        if ( 0 == strcmp( "--variable", argv[arg] ) || 0 == strcmp( "-a", argv[arg] )  )
        {
            ++arg;

            if ( 0 != strcmp( "font", variable_name ) )
            {
                fprintf( stderr, "Multiple --variable parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No variable name given.\n" );
                print_help();
                exit( 1 );
            }

            variable_name = argv[arg];
            continue;
        }

        if ( 0 == strcmp( "--texture", argv[arg] ) || 0 == strcmp( "-t", argv[arg] ) )
        {
            ++arg;

            if ( 128.0 != texture_width )
            {
                fprintf( stderr, "Multiple --texture parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No texture size given.\n" );
                print_help();
                exit( 1 );
            }

            errno = 0;

            texture_width = atof( argv[arg] );

            if ( errno )
            {
                fprintf( stderr, "No valid texture size given.\n" );
                print_help();
                exit( 1 );
            }

            continue;
        }

        if ( 0 == strcmp( "--rendermode", argv[arg] ) || 0 == strcmp( "-r", argv[arg] ) )
        {
            ++arg;

            if ( 128.0 != texture_width )
            {
                fprintf( stderr, "Multiple --texture parameters.\n" );
                print_help();
                exit( 1 );
            }

            if ( arg >= argc )
            {
                fprintf( stderr, "No texture size given.\n" );
                print_help();
                exit( 1 );
            }

            errno = 0;

            if( 0 == strcmp( "normal", argv[arg] ) )
            {
                rendermode = RENDER_NORMAL;
            }
            else if( 0 == strcmp( "outline_edge", argv[arg] ) )
            {
                rendermode = RENDER_OUTLINE_EDGE;
            }
            else if( 0 == strcmp( "outline_positive", argv[arg] ) )
            {
                rendermode = RENDER_OUTLINE_POSITIVE;
            }
            else if( 0 == strcmp( "outline_negative", argv[arg] ) )
            {
                rendermode = RENDER_OUTLINE_NEGATIVE;
            }
            else if( 0 == strcmp( "sdf", argv[arg] ) )
            {
                rendermode = RENDER_SIGNED_DISTANCE_FIELD;
            }
            else
            {
                fprintf( stderr, "No valid render mode given.\n" );
                print_help();
                exit( 1 );
            }

            continue;
        }

        fprintf( stderr, "Unknown parameter %s\n", argv[arg] );
        print_help();
        exit( 1 );
    }

    if ( show_help )
    {
        print_help();
        exit( 1 );
    }

    if ( !font_filename )
    {
        fprintf( stderr, "No font file given.\n" );
        print_help();
        exit( 1 );
    }

    if ( !( test = fopen( font_filename, "r" ) ) )
    {
        fprintf( stderr, "Font file \"%s\" does not exist.\n", font_filename );
    }

    fclose( test );

    if ( 4.0 > font_size )
    {
        fprintf( stderr, "Font size too small, expected at least 4 pt.\n" );
        print_help();
        exit( 1 );
    }

    if ( !header_filename )
    {
        fprintf( stderr, "No header file given.\n" );
        print_help();
        exit( 1 );
    }

    texture_atlas_t * atlas = texture_atlas_new( texture_width, texture_width, 1 );
    texture_font_t  * font  = texture_font_new_from_file( atlas, font_size, font_filename );
    font->rendermode = rendermode;

    size_t missed = texture_font_load_glyphs( font, font_cache );

    printf( "Font filename           : %s\n"
            "Font size               : %.1f\n"
            "Number of glyphs        : %ld\n"
            "Number of missed glyphs : %ld\n"
            "Texture size            : %ldx%ldx%ld\n"
            "Texture occupancy       : %.2f%%\n"
            "\n"
            "Header filename         : %s\n"
            "Variable name           : %s\n"
            "Render mode             : %s\n",
            font_filename,
            font_size,
            strlen(font_cache),
            missed,
            atlas->width, atlas->height, atlas->depth,
            100.0 * atlas->used / (float)(atlas->width * atlas->height),
            header_filename,
            variable_name,
            rendermodes[rendermode] );

    size_t texture_size = atlas->width * atlas->height *atlas->depth;
    size_t glyph_count = font->glyphs->size;
    size_t max_kerning_count = 1;
    for( i=0; i < glyph_count; ++i )
    {
        texture_glyph_t *glyph = *(texture_glyph_t **) vector_get( font->glyphs, i );

        if( vector_size(glyph->kerning) > max_kerning_count )
        {
            max_kerning_count = vector_size(glyph->kerning);
        }
    }


    FILE *file = fopen( header_filename, "w" );


    // -------------
    // Header
    // -------------
    fprintf( file,
        "/* ============================================================================\n"
        " * Freetype GL - A C OpenGL Freetype engine\n"
        " * Platform:    Any\n"
        " * WWW:         https://github.com/rougier/freetype-gl\n"
        " * ----------------------------------------------------------------------------\n"
        " * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.\n"
        " *\n"
        " * Redistribution and use in source and binary forms, with or without\n"
        " * modification, are permitted provided that the following conditions are met:\n"
        " *\n"
        " *  1. Redistributions of source code must retain the above copyright notice,\n"
        " *     this list of conditions and the following disclaimer.\n"
        " *\n"
        " *  2. Redistributions in binary form must reproduce the above copyright\n"
        " *     notice, this list of conditions and the following disclaimer in the\n"
        " *     documentation and/or other materials provided with the distribution.\n"
        " *\n"
        " * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR\n"
        " * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
        " * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO\n"
        " * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,\n"
        " * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
        " * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
        " * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n"
        " * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
        " * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n"
        " * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
        " *\n"
        " * The views and conclusions contained in the software and documentation are\n"
        " * those of the authors and should not be interpreted as representing official\n"
        " * policies, either expressed or implied, of Nicolas P. Rougier.\n"
        " * ============================================================================\n"
        " */\n");


    // ----------------------
    // Structure declarations
    // ----------------------
    fprintf( file,
        "#include <stddef.h>\n"
        "#include <stdint.h>\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "\n"
        "typedef struct\n"
        "{\n"
        "    uint32_t codepoint;\n"
        "    float kerning;\n"
        "} kerning_t;\n\n" );

    fprintf( file,
        "typedef struct\n"
        "{\n"
        "    uint32_t codepoint;\n"
        "    int width, height;\n"
        "    int offset_x, offset_y;\n"
        "    float advance_x, advance_y;\n"
        "    float s0, t0, s1, t1;\n"
        "    size_t kerning_count;\n"
        "    kerning_t kerning[%" PRIzu "];\n"
        "} texture_glyph_t;\n\n", max_kerning_count );

    fprintf( file,
        "typedef struct\n"
        "{\n"
        "    size_t tex_width;\n"
        "    size_t tex_height;\n"
        "    size_t tex_depth;\n"
        "    char tex_data[%" PRIzu "];\n"
        "    float size;\n"
        "    float height;\n"
        "    float linegap;\n"
        "    float ascender;\n"
        "    float descender;\n"
        "    size_t glyphs_count;\n"
        "    texture_glyph_t glyphs[%" PRIzu "];\n"
        "} texture_font_t;\n\n", texture_size, glyph_count );



    fprintf( file, "texture_font_t %s = {\n", variable_name );


    // ------------
    // Texture data
    // ------------
    fprintf( file, " %" PRIzu ", %" PRIzu ", %" PRIzu ", \n", atlas->width, atlas->height, atlas->depth );
    fprintf( file, " {" );
    for( i=0; i < texture_size; i+= 32 )
    {
        for( j=0; j < 32 && (j+i) < texture_size ; ++ j)
        {
            if( (j+i) < (texture_size-1) )
            {
                fprintf( file, "%d,", atlas->data[i+j] );
            }
            else
            {
                fprintf( file, "%d", atlas->data[i+j] );
            }
        }
        if( (j+i) < texture_size )
        {
            fprintf( file, "\n  " );
        }
    }
    fprintf( file, "}, \n" );


    // -------------------
    // Texture information
    // -------------------
    fprintf( file, " %ff, %ff, %ff, %ff, %ff, %" PRIzu ", \n",
             font->size, font->height,
             font->linegap,font->ascender, font->descender,
             glyph_count );

    // --------------
    // Texture glyphs
    // --------------
    fprintf( file, " {\n" );
    for( i=0; i < glyph_count; ++i )
    {
        texture_glyph_t * glyph = *(texture_glyph_t **) vector_get( font->glyphs, i );

/*
        // Debugging information
        printf( "glyph : '%lc'\n",
                 glyph->codepoint );
        printf( "  size       : %dx%d\n",
                 glyph->width, glyph->height );
        printf( "  offset     : %+d%+d\n",
                 glyph->offset_x, glyph->offset_y );
        printf( "  advance    : %ff, %ff\n",
                 glyph->advance_x, glyph->advance_y );
        printf( "  tex coords.: %ff, %ff, %ff, %ff\n",
                 glyph->u0, glyph->v0, glyph->u1, glyph->v1 );

        printf( "  kerning    : " );
        if( glyph->kerning_count )
        {
            for( j=0; j < glyph->kerning_count; ++j )
            {
                printf( "('%lc', %ff)",
                         glyph->kerning[j].codepoint, glyph->kerning[j].kerning );
                if( j < (glyph->kerning_count-1) )
                {
                    printf( ", " );
                }
            }
        }
        else
        {
            printf( "None" );
        }
        printf( "\n\n" );
*/


        // TextureFont
        fprintf( file, "  {%u, ", glyph->codepoint );
        fprintf( file, "%" PRIzu ", %" PRIzu ", ", glyph->width, glyph->height );
        fprintf( file, "%d, %d, ", glyph->offset_x, glyph->offset_y );
        fprintf( file, "%ff, %ff, ", glyph->advance_x, glyph->advance_y );
        fprintf( file, "%ff, %ff, %ff, %ff, ", glyph->s0, glyph->t0, glyph->s1, glyph->t1 );
        fprintf( file, "%" PRIzu ", ", vector_size(glyph->kerning) );
        if (vector_size(glyph->kerning) == 0) {
            fprintf( file, "0" );
        }
        else {
            fprintf( file, "{ " );
            for( j=0; j < vector_size(glyph->kerning); ++j )
            {
                kerning_t *kerning = (kerning_t *) vector_get( glyph->kerning, j);

                fprintf( file, "{%u, %ff}", kerning->codepoint, kerning->kerning );
                if( j < (vector_size(glyph->kerning)-1))
                {
                    fprintf( file, ", " );
                }
            }
            fprintf( file, "}" );
        }
        fprintf( file, " },\n" );
    }
    fprintf( file, " }\n};\n" );

    fprintf( file,
        "#ifdef __cplusplus\n"
        "}\n"
        "#endif\n" );

    return 0;
}
