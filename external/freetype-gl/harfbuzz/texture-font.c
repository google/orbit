/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "texture-font.h"
#include "platform.h"

#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

// ------------------------------------------------- texture_font_load_face ---
static int
texture_font_load_face(texture_font_t *self, float size,
        FT_Library *library)
{
    FT_Error error;
    FT_Matrix matrix = {
        (int)((1.0/self->hres) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    assert(library);
    assert(size);

    /* Initialize library */
    error = FT_Init_FreeType(library);
    if(error) {
        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                FT_Errors[error].code, FT_Errors[error].message);
        return 0;
    }

    /* Load face */
    switch (self->location) {
    case TEXTURE_FONT_FILE:
        error = FT_New_Face(*library, self->filename, 0, &self->ft_face);
        break;

    case TEXTURE_FONT_MEMORY:
        error = FT_New_Memory_Face(*library,
            self->memory.base, self->memory.size, 0, &self->ft_face);
        break;
    }

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Select charmap */
    error = FT_Select_Charmap(self->ft_face, FT_ENCODING_UNICODE);
    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
    }

    /* Set char size */
    error = FT_Set_Char_Size(self->ft_face, 0, (int)(self->size*64), DPI * self->hres, DPI);

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        FT_Done_Face(self->ft_face);
        FT_Done_FreeType(*library);
        return 0;
    }

    /* Set transform matrix */
    FT_Set_Transform(self->ft_face, &matrix, NULL);

    return 1;
}

// ------------------------------------------------------ texture_glyph_new ---
texture_glyph_t *
texture_glyph_new(void)
{
    texture_glyph_t *self = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
    if(self == NULL) {
        fprintf( stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->codepoint  = -1;
    self->width     = 0;
    self->height    = 0;
    self->outline_type = 0;
    self->outline_thickness = 0.0;
    self->offset_x  = 0;
    self->offset_y  = 0;
    self->s0        = 0.0;
    self->t0        = 0.0;
    self->s1        = 0.0;
    self->t1        = 0.0;
    return self;
}


// --------------------------------------------------- texture_glyph_delete ---
void
texture_glyph_delete( texture_glyph_t *self )
{
    assert( self );
    free( self );
}

// ------------------------------------------------------ texture_font_init ---
static int
texture_font_init(texture_font_t *self)
{
    FT_Library library;

    assert(self->atlas);
    assert(self->size > 0);
    assert((self->location == TEXTURE_FONT_FILE && self->filename)
        || (self->location == TEXTURE_FONT_MEMORY
            && self->memory.base && self->memory.size));

    self->glyphs = vector_new(sizeof(texture_glyph_t *));
    self->height = 0;
    self->ascender = 0;
    self->descender = 0;
    self->outline_type = 0;
    self->outline_thickness = 0.0;
    self->hres = 100;
    self->hinting = 1;
    self->filtering = 1;
    self->ft_face = 0;
    self->hb_ft_font = 0;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    self->lcd_weights[0] = 0x10;
    self->lcd_weights[1] = 0x40;
    self->lcd_weights[2] = 0x70;
    self->lcd_weights[3] = 0x40;
    self->lcd_weights[4] = 0x10;

    if (!texture_font_load_face(self, self->size * 100.f, &library))
        return -1;

    /* Set harfbuzz font */
    self->hb_ft_font = hb_ft_font_create( self->ft_face, NULL );

    return 0;
}

// --------------------------------------------- texture_font_new_from_file ---
texture_font_t *
texture_font_new_from_file(texture_atlas_t *atlas, const float pt_size,
        const char *filename)
{
    texture_font_t *self;

    assert(filename);

    self = calloc(1, sizeof(*self));
    if (!self) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_FILE;
    self->filename = strdup(filename);

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return NULL;
    }

    return self;
}

// ------------------------------------------- texture_font_new_from_memory ---
texture_font_t *
texture_font_new_from_memory(texture_atlas_t *atlas, float pt_size,
        const void *memory_base, size_t memory_size)
{
    texture_font_t *self;

    assert(memory_base);
    assert(memory_size);

    self = calloc(1, sizeof(*self));
    if (!self) {
        fprintf(stderr,
                "line %d: No more memory for allocating data\n", __LINE__);
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_MEMORY;
    self->memory.base = memory_base;
    self->memory.size = memory_size;

    if (texture_font_init(self)) {
        texture_font_delete(self);
        return NULL;
    }

    return self;
}

// ---------------------------------------------------- texture_font_delete ---
void
texture_font_delete( texture_font_t *self )
{
    size_t i;
    texture_glyph_t *glyph;

    assert( self );

    if(self->location == TEXTURE_FONT_FILE && self->filename)
        free( self->filename );

    for( i=0; i<vector_size( self->glyphs ); ++i)
    {
        glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
        texture_glyph_delete( glyph);
    }

    vector_delete( self->glyphs );

    FT_Done_Face( self->ft_face );
    hb_font_destroy( self->hb_ft_font );

    free( self );
}

texture_glyph_t *
texture_font_find_glyph( texture_font_t * self,
                         uint32_t codepoint )
{
    size_t i;
    texture_glyph_t *glyph;

    for( i = 0; i < self->glyphs->size; ++i )
    {
        glyph = *(texture_glyph_t **) vector_get( self->glyphs, i );
        // If codepoint is -1, we don't care about outline type or thickness
        if( (glyph->codepoint == codepoint) &&
            ((codepoint == -1) ||
             ((glyph->outline_type == self->outline_type) &&
              (glyph->outline_thickness == self->outline_thickness)) ))
        {
            return glyph;
        }
    }

    return NULL;
}

// ----------------------------------------------- texture_font_load_glyphs ---
size_t
texture_font_load_glyphs( texture_font_t * self,
                          const char * codepoints,
                          const char *language )
{
    size_t i, x, y, width, height, depth, w, h;

    FT_Library library;
    FT_Error error;
    FT_Glyph ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;

    unsigned int glyph_count;
    FT_UInt glyph_index;
    texture_glyph_t *glyph;
    FT_Int32 flags = 0;
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;

    hb_buffer_t *buffer;
    hb_glyph_info_t *glyph_info;

    ivec4 region;
    size_t missed = 0;

    assert( self );
    assert( codepoints );

    width  = self->atlas->width;
    height = self->atlas->height;
    depth  = self->atlas->depth;

    FT_Init_FreeType(&library);

    /* Create a buffer for harfbuzz to use */
    buffer = hb_buffer_create();

    hb_buffer_set_language( buffer,
                            hb_language_from_string(language, strlen(language)) );

    /* Layout the text */
    hb_buffer_add_utf8( buffer, codepoints, strlen(codepoints), 0, strlen(codepoints) );
    /* Guess text script and direction */
    hb_buffer_guess_segment_properties( buffer );
    hb_shape( self->hb_ft_font, buffer, NULL, 0 );

    glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

    for( i = 0; i < glyph_count; ++i ) {
        /* Check if codepoint has been already loaded */
        if( texture_font_find_glyph( self, glyph_info[i].codepoint ) )
            continue;

        flags = 0;
        ft_glyph_top = 0;
        ft_glyph_left = 0;
        // WARNING: We use texture-atlas depth to guess if user wants
        //          LCD subpixel rendering

        if( self->outline_type > 0 )
        {
            flags |= FT_LOAD_NO_BITMAP;
        }
        else
        {
            flags |= FT_LOAD_RENDER;
        }

        if( !self->hinting )
        {
            flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
        }
        else
        {
            flags |= FT_LOAD_FORCE_AUTOHINT;
        }

        if( depth == 3 )
        {
            FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT );
            flags |= FT_LOAD_TARGET_LCD;

            if( self->filtering )
            {
                FT_Library_SetLcdFilterWeights( library, self->lcd_weights );
            }
        }

        error = FT_Load_Glyph( self->ft_face, glyph_info[i].codepoint, flags );
        if( error )
        {
            fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                     __LINE__, FT_Errors[error].code, FT_Errors[error].message );
            FT_Done_FreeType( library );
            return glyph_count - i;
        }


        if( self->outline_type == 0 )
        {
            slot            = self->ft_face->glyph;
            ft_bitmap       = slot->bitmap;
            ft_glyph_top    = slot->bitmap_top;
            ft_glyph_left   = slot->bitmap_left;
        }
        else
        {
            FT_Stroker stroker;
            FT_BitmapGlyph ft_bitmap_glyph;
            error = FT_Stroker_New( library, &stroker );
            if( error )
            {
                fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                        FT_Errors[error].code, FT_Errors[error].message);
                FT_Stroker_Done( stroker );
                FT_Done_FreeType( library );
                return 0;
            }
            FT_Stroker_Set(stroker,
                            (int)(self->outline_thickness * self->hres),
                            FT_STROKER_LINECAP_ROUND,
                            FT_STROKER_LINEJOIN_ROUND,
                            0);
            error = FT_Get_Glyph( self->ft_face->glyph, &ft_glyph);
            if( error )
            {
                fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                        FT_Errors[error].code, FT_Errors[error].message);
                FT_Stroker_Done( stroker );
                FT_Done_FreeType( library );
                return 0;
            }

            if( self->outline_type == 1 )
            {
                error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
            }
            else if ( self->outline_type == 2 )
            {
                error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
            }
            else if ( self->outline_type == 3 )
            {
                error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
            }
            if( error )
            {
                fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                        FT_Errors[error].code, FT_Errors[error].message);
                FT_Stroker_Done( stroker );
                FT_Done_FreeType( library );
                return 0;
            }

            if( depth == 1 )
            {
                error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                if( error )
                {
                    fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                            FT_Errors[error].code, FT_Errors[error].message);
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
                    return 0;
                }
            }
            else
            {
                error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
                if( error )
                {
                    fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                            FT_Errors[error].code, FT_Errors[error].message);
                    FT_Stroker_Done( stroker );
                    FT_Done_FreeType( library );
                    return 0;
                }
            }

            ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
            ft_bitmap       = ft_bitmap_glyph->bitmap;
            ft_glyph_top    = ft_bitmap_glyph->top;
            ft_glyph_left   = ft_bitmap_glyph->left;
            FT_Stroker_Done(stroker);
        }

        // We want each glyph to be separated by at least one black pixel
        w = ft_bitmap.width/depth;
        h = ft_bitmap.rows;
        region = texture_atlas_get_region( self->atlas, w+1, h+1 );
        if ( region.x < 0 )
        {
            missed++;
            fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
            continue;
        }
        x = region.x;
        y = region.y;
        texture_atlas_set_region( self->atlas, x, y, w, h,
                                  ft_bitmap.buffer, ft_bitmap.pitch );

        glyph = texture_glyph_new( );
        glyph->codepoint = glyph_info[i].codepoint;
        glyph->width    = w;
        glyph->height   = h;
        glyph->outline_type = self->outline_type;
        glyph->outline_thickness = self->outline_thickness;
        glyph->offset_x = ft_glyph_left;
        glyph->offset_y = ft_glyph_top;
        glyph->s0       = x/(float)width;
        glyph->t0       = y/(float)height;
        glyph->s1       = (x + glyph->width)/(float)width;
        glyph->t1       = (y + glyph->height)/(float)height;
        vector_push_back( self->glyphs, &glyph );

        if( self->outline_type > 0 )
        {
            FT_Done_Glyph( ft_glyph );
        }
    }

    /* clean up the buffer, but don't kill it just yet */
    hb_buffer_reset(buffer);

    /* Cleanup */
    hb_buffer_destroy( buffer );

    return missed;
}


// ------------------------------------------------- texture_font_get_glyph ---
texture_glyph_t *
texture_font_get_glyph( texture_font_t * self,
                        uint32_t codepoint )
{
    texture_glyph_t *glyph;

    assert( self );
    assert( self->filename );
    assert( self->atlas );

    /* Check if codepoint has been already loaded */
    return texture_font_find_glyph(self, codepoint );
}
