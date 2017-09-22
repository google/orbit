/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#if 0
#  if !defined(_WIN32) && !defined(_WIN64)
#    include <fontconfig/fontconfig.h>
#  endif
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font-manager.h"


// ------------------------------------------------------------ file_exists ---
static int
file_exists( const char * filename )
{
    FILE * file = fopen( filename, "r" );
    if ( file )
    {
        fclose(file);
        return 1;
    }
    return 0;
}


// ------------------------------------------------------- font_manager_new ---
font_manager_t *
font_manager_new( size_t width, size_t height, size_t depth )
{
    font_manager_t *self;
    texture_atlas_t *atlas = texture_atlas_new( width, height, depth );
    self = (font_manager_t *) malloc( sizeof(font_manager_t) );
    if( !self )
    {
        fprintf( stderr,
                 "line %d: No more memory for allocating data\n", __LINE__ );
        exit( EXIT_FAILURE );
    }
    self->atlas = atlas;
    self->fonts = vector_new( sizeof(texture_font_t *) );
    self->cache = strdup( " " );
    return self;
}


// ---------------------------------------------------- font_manager_delete ---
void
font_manager_delete( font_manager_t * self )
{
    size_t i;
    texture_font_t *font;
    assert( self );

    for( i=0; i<vector_size( self->fonts ); ++i)
    {
        font = *(texture_font_t **) vector_get( self->fonts, i );
        texture_font_delete( font );
    }
    vector_delete( self->fonts );
    texture_atlas_delete( self->atlas );
    if( self->cache )
    {
        free( self->cache );
    }
    free( self );
}



// ----------------------------------------------- font_manager_delete_font ---
void
font_manager_delete_font( font_manager_t * self,
                          texture_font_t * font)
{
    size_t i;
    texture_font_t *other;

    assert( self );
    assert( font );

    for( i=0; i<self->fonts->size;++i )
    {
        other = (texture_font_t *) vector_get( self->fonts, i );
        if ( (strcmp(font->filename, other->filename) == 0)
               && ( font->size == other->size) )
        {
            vector_erase( self->fonts, i);
            break;
        }
    }
    texture_font_delete( font );
}



// ----------------------------------------- font_manager_get_from_filename ---
texture_font_t *
font_manager_get_from_filename( font_manager_t *self,
                                const char * filename,
                                const float size )
{
    size_t i;
    texture_font_t *font;

    assert( self );
    for( i=0; i<vector_size(self->fonts); ++i )
    {
        font = * (texture_font_t **) vector_get( self->fonts, i );
        if( (strcmp(font->filename, filename) == 0) && ( font->size == size) )
        {
            return font;
        }
    }
    font = texture_font_new_from_file( self->atlas, size, filename );
    if( font )
    {
        vector_push_back( self->fonts, &font );
        texture_font_load_glyphs( font, self->cache );
        return font;
    }
    fprintf( stderr, "Unable to load \"%s\" (size=%.1f)\n", filename, size );
    return 0;
}


// ----------------------------------------- font_manager_get_from_description ---
texture_font_t *
font_manager_get_from_description( font_manager_t *self,
                                   const char * family,
                                   const float size,
                                   const int bold,
                                   const int italic )
{
    texture_font_t *font;
    char *filename = 0;

    assert( self );

    if( file_exists( family ) )
    {
        filename = strdup( family );
    }
    else
    {
#if defined(_WIN32) || defined(_WIN64)
        fprintf( stderr, "\"font_manager_get_from_description\" not implemented yet.\n" );
        return 0;
#endif
        filename = font_manager_match_description( self, family, size, bold, italic );
        if( !filename )
        {
            fprintf( stderr, "No \"%s (size=%.1f, bold=%d, italic=%d)\" font available.\n",
                     family, size, bold, italic );
            return 0;
        }
    }
    font = font_manager_get_from_filename( self, filename, size );

    free( filename );
    return font;
}

// ------------------------------------------- font_manager_get_from_markup ---
texture_font_t *
font_manager_get_from_markup( font_manager_t *self,
                              const markup_t *markup )
{
    assert( self );
    assert( markup );

    return font_manager_get_from_description( self, markup->family, markup->size,
                                              markup->bold,   markup->italic );
}

// ----------------------------------------- font_manager_match_description ---
char *
font_manager_match_description( font_manager_t * self,
                                const char * family,
                                const float size,
                                const int bold,
                                const int italic )
{
// Use of fontconfig is disabled by default.
#if 1
    return 0;
#else
#  if defined _WIN32 || defined _WIN64
      fprintf( stderr, "\"font_manager_match_description\" not implemented for windows.\n" );
      return 0;
#  endif
    char *filename = 0;
    int weight = FC_WEIGHT_REGULAR;
    int slant = FC_SLANT_ROMAN;
    if ( bold )
    {
        weight = FC_WEIGHT_BOLD;
    }
    if( italic )
    {
        slant = FC_SLANT_ITALIC;
    }
    FcInit();
    FcPattern *pattern = FcPatternCreate();
    FcPatternAddDouble( pattern, FC_SIZE, size );
    FcPatternAddInteger( pattern, FC_WEIGHT, weight );
    FcPatternAddInteger( pattern, FC_SLANT, slant );
    FcPatternAddString( pattern, FC_FAMILY, (FcChar8*) family );
    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        fprintf( stderr, "fontconfig error: could not match family '%s'", family );
        return 0;
    }
    else
    {
        FcValue value;
        FcResult result = FcPatternGet( match, FC_FILE, 0, &value );
        if ( result )
        {
            fprintf( stderr, "fontconfig error: could not match family '%s'", family );
        }
        else
        {
            filename = strdup( (char *)(value.u.s) );
        }
    }
    FcPatternDestroy( match );
    return filename;
#endif
}
