/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include FT_TRUETYPE_TABLES_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#ifdef __APPLE__
# include <machine/endian.h>
# define __BIG_ENDIAN __ORDER_BIG_ENDIAN__
# define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
# define __BYTE_ORDER __BYTE_ORDER__
#elif defined(_WIN32) || defined(_WIN64)
# define __LITTLE_ENDIAN 1234
# define __BIG_ENDIAN 4321
# define __BYTE_ORDER __LITTLE_ENDIAN
#else
# include <endian.h>
#endif
#include "distance-field.h"
#include "texture-font.h"
#include "platform.h"
#include "utf8-utils.h"
#include "ftgl-utils.h"

#define HRES  64
#define HRESf 64.f
#define DPI   72

static float convert_F26Dot6_to_float(FT_F26Dot6 value)
{
  return ((float)value) / 64.0;
}
static FT_F26Dot6 convert_float_to_F26Dot6(float value)
{
  return (FT_F26Dot6) (value * 64.0);
}

// per-thread library

__THREAD texture_font_library_t * freetype_gl_library = NULL;
__THREAD font_mode_t mode_default=MODE_FREE_CLOSE;

// rol8 ror8

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__GNUC__)
# define inline
static inline __builtin_bswap32(uint32_t in)
{
    return ((in >> 24) & 0xFF) | ((in >> 8) & 0xFF00) | ((in & 0xFF00) << 8) | ((in & 0xFF) << 24);
}
#endif

static inline uint32_t rol(uint32_t in, uint32_t x)
{
    return (in >> (32-x)) | (in << x);
}

// ------------------------------------------------------ texture_glyph_clone ---
texture_glyph_t*
texture_glyph_clone(texture_glyph_t* self)
{
    int i;
    texture_glyph_t* new_glyph;
    float* source;
    float** target;
    assert(self);

    new_glyph = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
    if(new_glyph == NULL) {
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }
    memcpy(new_glyph, self, sizeof(texture_glyph_t));
    new_glyph->kerning = vector_new(sizeof(float**));
    vector_resize(new_glyph->kerning, self->kerning->size);
    for (i = 0; i < self->kerning->size; i++) {
        source = *(float**)vector_get(self->kerning, i);
        target = (float**)vector_get(new_glyph->kerning, i);
        *target = calloc(0x100, sizeof(float));
        memcpy(*target, source, 0x100);
    }
    return new_glyph;
}

// ------------------------------------------------------ texture_glyph_new ---
texture_glyph_t *
texture_glyph_new(void)
{
    texture_glyph_t *self = (texture_glyph_t *) malloc( sizeof(texture_glyph_t) );
    if(self == NULL) {
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }

    self->codepoint  = -1;
    self->width     = 0;
    self->height    = 0;
    /* Attributes that can have different images for the same codepoint */
    self->rendermode = RENDER_NORMAL;
    self->outline_thickness = 0.0;
    self->glyphmode = GLYPH_END;
    /* End of attribute part */
    self->offset_x  = 0;
    self->offset_y  = 0;
    self->advance_x = 0.0;
    self->advance_y = 0.0;
    self->s0        = 0.0;
    self->t0        = 0.0;
    self->s1        = 0.0;
    self->t1        = 0.0;
    self->kerning   = vector_new( sizeof(float**) );
    return self;
}

// ---------------------------------------------- texture_font_default_mode ---
void
texture_font_default_mode(font_mode_t mode)
{
    mode_default=mode;
}

// --------------------------------------------------- texture_glyph_delete ---
void
texture_glyph_delete( texture_glyph_t *self )
{
    int i;
    assert( self );
    for(i=0; i < self->kerning->size; i++)
        free( *(float **) vector_get( self->kerning, i ) );
    vector_delete( self->kerning );
    free( self );
}

// ---------------------------------------------- texture_glyph_get_kerning ---
float
texture_glyph_get_kerning( const texture_glyph_t * self,
                           const char * codepoint )
{
    uint32_t ucodepoint = utf8_to_utf32( codepoint );
    uint32_t i = ucodepoint >> 8;
    uint32_t j = ucodepoint & 0xFF;
    float *kern_index;

    assert( self );
    if(ucodepoint == -1)
        return 0;
    if(self->kerning->size <= i)
        return 0;

    kern_index = *(float **) vector_get( self->kerning, i );

    if(!kern_index)
        return 0;
    else
        return kern_index[j];
}

// ---------------------------------------------- texture_font_index_kerning ---

void texture_font_index_kerning( texture_glyph_t * self,
                                 uint32_t codepoint,
                                 float kerning)
{
    uint32_t i = codepoint >> 8;
    uint32_t j = codepoint & 0xFF;
    float ** kerning_index;

    if(self->kerning->size <= i) {
        vector_resize( self->kerning, i+1);
    }

    kerning_index = (float **) vector_get( self->kerning, i );

    if(!*kerning_index) {
        *kerning_index = calloc( 0x100, sizeof(float) );
    }

    (*kerning_index)[j] = kerning;
}

// ------------------------------------------ texture_font_generate_kerning ---
void
texture_font_generate_kerning( texture_font_t *self,
                               FT_Library *library, FT_Face *face )
{
    size_t i, j, k;
    FT_UInt glyph_index, prev_index;
    texture_glyph_t *glyph, *prev_glyph;
    FT_Vector kerning;

    assert( self );

    /* For each glyph couple combination, check if kerning is necessary */
    /* Starts at index 1 since 0 is for the special background glyph */
    GLYPHS_ITERATOR(i, glyph, self->glyphs ) {
        glyph_index = FT_Get_Char_Index( *face, glyph->codepoint );
//        fprintf(stderr, "Retrieving glyph %p from index %i\n", __glyphs, __i);
//        fprintf(stderr, "Glpyh %p: Indexing %d, kerning %p\n", glyph, glyph_index, glyph->kerning);
        for(k=0; k < glyph->kerning->size; k++)
            free( *(float **) vector_get( glyph->kerning, k ) );
        vector_clear( glyph->kerning );
        
        GLYPHS_ITERATOR(j, prev_glyph, self->glyphs ) {
            prev_index = FT_Get_Char_Index( *face, prev_glyph->codepoint );
            // FT_KERNING_UNFITTED returns FT_F26Dot6 values.
            FT_Get_Kerning( *face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning );
            // printf("%c(%d)-%c(%d): %ld\n",
            //       prev_glyph->codepoint, prev_glyph->codepoint,
            //       glyph_index, glyph_index, kerning.x);
            if( kerning.x ) {
                texture_font_index_kerning( glyph,
                                            prev_glyph->codepoint,
                                            convert_F26Dot6_to_float(kerning.x) / HRESf );
            }
            // also insert kerning with the current added element
            FT_Get_Kerning( *face, glyph_index, prev_index, FT_KERNING_UNFITTED, &kerning );
            if( kerning.x ) {
                texture_font_index_kerning( prev_glyph,
                                            glyph->codepoint,
                                            kerning.x / (float)(HRESf*HRESf) );
            }
        }
        GLYPHS_ITERATOR_END
    }
    GLYPHS_ITERATOR_END
}

// -------------------------------------------------- texture_is_color_font ---

int
texture_is_color_font( texture_font_t *self) {
    static const uint32_t tag = FT_MAKE_TAG('C', 'B', 'D', 'T');
    unsigned long length = 0;
    FT_Load_Sfnt_Table(self->face, tag, 0, NULL, &length);
    return length != 0;
}

// -------------------------------------------------- texture_font_set_size ---

int
texture_font_set_size ( texture_font_t *self, float size )
{
    FT_Error error=0;
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    if( FT_HAS_FIXED_SIZES( self->face ) ) {
        /* Select best size */
        if (self->face->num_fixed_sizes == 0) {
            freetype_gl_error( No_Fixed_Size_In_Color_Font );
            return 0;
        }
        
        int best_match = 0;
        float diff = 1e20;
        int i;

        for (i = 0; i < self->face->num_fixed_sizes; ++i) {
	    float new_size = convert_F26Dot6_to_float(self->face->available_sizes[i].size);
	    float ndiff = size > new_size ? size / new_size : new_size / size;
	    if(freetype_gl_warnings)
		log_error("candiate: size[%i]=%f %d*%d\n", i, new_size,
			  self->face->available_sizes[i].width,
			  self->face->available_sizes[i].height);
            if (ndiff < diff) {
                best_match = i;
                diff = ndiff;
            }
        }
	if(freetype_gl_warnings)
	    log_error("selected: size[%i] for %f\n", best_match, size);
        error = FT_Select_Size(self->face, best_match);
        if(error) {
            freetype_error( error );
            return 0;
        }
        self->scale = self->size / convert_F26Dot6_to_float(self->face->available_sizes[best_match].size);
    } else {
        /* Set char size */
        error = FT_Set_Char_Size(self->face, convert_float_to_F26Dot6(size), 0, DPI * HRES, DPI);
        
        if(error) {
            freetype_error( error );
            return 0;
        }
    }
    /* Set transform matrix */
    FT_Set_Transform(self->face, &matrix, NULL);

    return 1;
}

// --------------------------------------------------

void
texture_font_init_size( texture_font_t * self)
{
    FT_Size_Metrics metrics;
    
    self->underline_position = self->face->underline_position / (float)(HRESf*HRESf) * self->size;
    self->underline_position = roundf( self->underline_position );
    if( self->underline_position > -2 )
    {
        self->underline_position = -2.0;
    }

    self->underline_thickness = self->face->underline_thickness / (float)(HRESf*HRESf) * self->size;
    self->underline_thickness = roundf( self->underline_thickness );
    if( self->underline_thickness < 1 )
    {
        self->underline_thickness = 1.0;
    }

    metrics = self->face->size->metrics;
    self->ascender  = metrics.ascender  >> 6;
    self->descender = metrics.descender >> 6;
    self->height    = metrics.height    >> 6;
    self->linegap = self->height - self->ascender + self->descender;
}

// ------------------------------------------------------ texture_font_init ---
static int
texture_font_init(texture_font_t *self)
{
    assert(self->atlas);
    assert(self->size > 0);
    assert((self->location == TEXTURE_FONT_FILE && self->filename)
        || (self->location == TEXTURE_FONT_MEMORY
            && self->memory.base && self->memory.size));

    self->glyphs = vector_new(sizeof(texture_glyph_t *));
    self->height = 0;
    self->ascender = 0;
    self->descender = 0;
    self->linegap = 0;
    self->rendermode = RENDER_NORMAL;
    self->outline_thickness = 0.0;
    self->hinting = 1;
    self->kerning = 1;
    self->filtering = 1;
    self->scaletex = 1;
    self->scale = 1.0;

    // FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
    // FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
    self->lcd_weights[0] = 0x10;
    self->lcd_weights[1] = 0x40;
    self->lcd_weights[2] = 0x70;
    self->lcd_weights[3] = 0x40;
    self->lcd_weights[4] = 0x10;

    if (!texture_font_load_face(self, self->size))
        return -1;

    texture_font_init_size( self );

    if (!texture_font_set_size(self, self->size))
        return -1;
    
    /* NULL is a special glyph */
    texture_font_get_glyph( self, NULL );

    return 0;
}

// ---------------------------------------------------- texture_library_new ---
texture_font_library_t *
texture_library_new(void)
{
    texture_font_library_t *self = calloc(1, sizeof(*self));
    
    self->mode = MODE_ALWAYS_OPEN;
    
    return self;
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
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_FILE;
    self->filename = strdup(filename);
    self->mode = mode_default;
    
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
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }

    self->atlas = atlas;
    self->size  = pt_size;

    self->location = TEXTURE_FONT_MEMORY;
    self->memory.base = memory_base;
    self->memory.size = memory_size;
    self->mode = mode_default;
    
    if (texture_font_init(self)) {
        texture_font_delete(self);
        return NULL;
    }

    return self;
}

// ----------------------------------------------------- texture_font_clone ---
texture_font_t *
texture_font_clone( texture_font_t *old, float pt_size)
{
    texture_font_t *self;
    FT_Error error = 0;
    float native_size = old->size / old->scale; // unscale fonts
    
    self = calloc(1, sizeof(*self));
    if (!self) {
        freetype_gl_error( Out_Of_Memory );
        return NULL;
    }

    memcpy(self, old, sizeof(*self));
    self->size  = pt_size;

    error = FT_New_Size( self->face, &self->ft_size );
    if(error) {
        freetype_error( error );
        return NULL;
    }

    error = FT_Activate_Size( self->ft_size );
    if(error) {
        freetype_error( error );
        return NULL;
    }
    
    if(!texture_font_set_size ( self, pt_size ))
        return NULL;

    texture_font_init_size( self );
    
    if(self->size / self->scale != native_size)
        self->glyphs = vector_new(sizeof(texture_glyph_t *));
    return self;
}
// ----------------------------------------------------- texture_font_close ---

void
texture_font_close( texture_font_t *self, font_mode_t face_mode, font_mode_t library_mode )
{
    if( self->face && self->mode <= face_mode ) {
        FT_Done_Face( self->face );
        self->face = NULL;
    } else {
        return; // never close the library when the face stays open
    }

    if( self->library && self->library->library && self->library->mode <= library_mode ) {
        FT_Done_FreeType( self->library->library );
        self->library->library = NULL;
    }
}

// ------------------------------------------------- texture_font_load_face ---

int
texture_font_load_face( texture_font_t *self, float size )
{
    FT_Error error;

    if ( !self->library ) {
        if ( !freetype_gl_library ) {
            freetype_gl_library = texture_library_new();
        }
        self->library = freetype_gl_library;
    }
    
    if( !self->library->library ) {
        error = FT_Init_FreeType( &self->library->library );
        if(error) {
            freetype_error( error );
            goto cleanup;
        }
    }
    
    if( !self->face ) {
        switch (self->location) {
        case TEXTURE_FONT_FILE:
            error = FT_New_Face(self->library->library, self->filename, 0, &self->face);
            if(error) {
                freetype_error( error );
                goto cleanup_library;
            }
            break;

        case TEXTURE_FONT_MEMORY:
            error = FT_New_Memory_Face(self->library->library,
                                       self->memory.base, self->memory.size, 0, &self->face);
            if(error) {
                freetype_error( error );
                goto cleanup_library;
            }
            break;
        }

        /* Select charmap */
        error = FT_Select_Charmap(self->face, FT_ENCODING_UNICODE);
        if(error) {
            freetype_error( error );
            goto cleanup_face;
        }

        error = FT_New_Size( self->face, &self->ft_size );
        if(error) {
            freetype_error( error );
            goto cleanup_face;
        }

        error = FT_Activate_Size( self->ft_size );
        if(error) {
            freetype_error( error );
            goto cleanup_face;
        }
        
        if(!texture_font_set_size ( self, size ))
            goto cleanup_face;
    }
    
    return 1;
    
  cleanup_face:
    texture_font_close( self, MODE_ALWAYS_OPEN, MODE_FREE_CLOSE );
    return 0;
  cleanup_library:
    texture_font_close( self, MODE_ALWAYS_OPEN, MODE_ALWAYS_OPEN );
  cleanup:
    return 0;
}

// ---------------------------------------------------- texture_font_delete ---
void
texture_font_delete( texture_font_t *self )
{
    size_t i;
    texture_glyph_t *glyph;
    FT_Error error=0;

    assert( self );

    error = FT_Done_Size( self->ft_size );
    if(error) {
        freetype_error( error );
    }

    texture_font_close( self, MODE_ALWAYS_OPEN, MODE_FREE_CLOSE );

    if(self->location == TEXTURE_FONT_FILE && self->filename)
        free( self->filename );
        
    GLYPHS_ITERATOR(i, glyph, self->glyphs) {
        texture_glyph_delete( glyph );
    } GLYPHS_ITERATOR_END1
        free( __glyphs );
    GLYPHS_ITERATOR_END2;

    vector_delete( self->glyphs );
    free( self );
}

// ------------------------------------------------ texture_font_find_glyph ---
texture_glyph_t *
texture_font_find_glyph( texture_font_t * self,
                         const char * codepoint )
{
    if(!codepoint)
        return (texture_glyph_t *)self->atlas->special;
    
    return texture_font_find_glyph_gi(self, utf8_to_utf32( codepoint ));
}

// ---------------------------------------------- texture_font_find_glyph_gi ---
texture_glyph_t *
texture_font_find_glyph_gi( texture_font_t * self,
                            uint32_t codepoint )
{
    uint32_t i = codepoint >> 8;
    uint32_t j = codepoint & 0xFF;
    texture_glyph_t **glyph_index1, *glyph;

    if(self->glyphs->size <= i)
        return NULL;

    glyph_index1 = *(texture_glyph_t ***) vector_get( self->glyphs, i );

    if(!glyph_index1)
        return NULL;
    else
        glyph = glyph_index1[j];

    while( glyph && // if no glyph is there, we are done here
           (glyph->rendermode != self->rendermode ||
            glyph->outline_thickness != self->outline_thickness) ) {
        if( glyph->glyphmode != GLYPH_CONT)
            return NULL;
        glyph++;
    }
    return glyph;
}

int
texture_font_index_glyph( texture_font_t * self,
                          texture_glyph_t *glyph,
                          uint32_t codepoint)
{
    uint32_t i = codepoint >> 8;
    uint32_t j = codepoint & 0xFF;
    texture_glyph_t ***glyph_index1, *glyph_insert;

    if(self->glyphs->size <= i) {
        vector_resize( self->glyphs, i+1);
    }

    glyph_index1 = (texture_glyph_t ***) vector_get( self->glyphs, i );

    if(!*glyph_index1) {
        *glyph_index1 = calloc( 0x100, sizeof(texture_glyph_t*) );
    }

    if(( glyph_insert = (*glyph_index1)[j] )) {
        int i = 0;
        // fprintf(stderr, "glyph already there\n");
        while (glyph_insert[i].glyphmode != GLYPH_END)
            i++;
        // fprintf(stderr, "Insert a glyph after position %d\n", i);
        glyph_insert[i].glyphmode = GLYPH_CONT;
        (*glyph_index1)[j] = glyph_insert = realloc( glyph_insert, sizeof(texture_glyph_t)*(i+2) );
        memcpy( glyph_insert+(i+1), glyph, sizeof(texture_glyph_t) );
        return 1;
    } else {
        (*glyph_index1)[j] = glyph;
        return 0;
    }
}

// ------------------------------------------------ texture_font_load_glyph ---
int
texture_font_load_glyph( texture_font_t * self,
                         const char * codepoint )
{
    /* codepoint NULL is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if( !codepoint ) {
        return 1;
    }
    uint32_t ucodepoint = utf8_to_utf32(codepoint);

    return texture_font_load_glyph_gi( self,
                                       FT_Get_Char_Index( self->face, ucodepoint),
                                       ucodepoint);
}

// ------------------------------------------------ texture_font_load_glyph ---
int
texture_font_load_glyph_gi( texture_font_t * self,
                            uint32_t glyph_index,
                            uint32_t ucodepoint )
{
    size_t i, x, y;

    FT_Error error;
    FT_Face face;
    FT_Glyph ft_glyph = NULL;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;

    texture_glyph_t *glyph;
    FT_Int32 flags = 0;
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;

    ivec4 region;
    size_t missed = 0;

    /* Check if codepoint has been already loaded */
    if (texture_font_find_glyph_gi(self, ucodepoint)) {
        return 1;
    }

    if (!texture_font_load_face(self, self->size))
        return 0;

    flags = 0;
    ft_glyph_top = 0;
    ft_glyph_left = 0;
    if(!glyph_index) {
        texture_glyph_t * glyph;
        if ((glyph = texture_font_find_glyph(self, "\0"))) {
            texture_font_index_glyph( self, glyph, ucodepoint );
            texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );
            return 1;
        }
    }
    // WARNING: We use texture-atlas depth to guess if user wants
    //          LCD subpixel rendering

    if( self->rendermode != RENDER_NORMAL && self->rendermode != RENDER_SIGNED_DISTANCE_FIELD )
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

    if( self->atlas->depth == 3 )
    {
        FT_Library_SetLcdFilter( self->library->library, FT_LCD_FILTER_LIGHT );
        flags |= FT_LOAD_TARGET_LCD;

        if( self->filtering )
        {
            FT_Library_SetLcdFilterWeights( self->library->library, self->lcd_weights );
        }
    }
    else if (HRES == 1)
    {
        /* “FT_LOAD_TARGET_LIGHT
         *  A lighter hinting algorithm for gray-level modes. Many generated
         *  glyphs are fuzzier but better resemble their original shape.
         *  This is achieved by snapping glyphs to the pixel grid
         *  only vertically (Y-axis), as is done by FreeType's new CFF engine
         *  or Microsoft's ClearType font renderer.”
         * https://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#ft_load_target_xxx
         */
        flags |= FT_LOAD_TARGET_LIGHT;
    }

    if( self->atlas->depth == 4 )
    {
#ifdef FT_LOAD_COLOR
        flags |= FT_LOAD_COLOR;
#else
        freetype_gl_error( Load_Color_Not_Available );
#endif
    }

    error = FT_Activate_Size( self->ft_size );
    if(error) {
        freetype_error( error );
        return 0;
    }

    error = FT_Load_Glyph( self->face, glyph_index, flags );
    if( error )
    {
        freetype_error( error );
        texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );
        return 0;
    }

    if( self->rendermode == RENDER_NORMAL || self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        slot            = self->face->glyph;
        ft_bitmap       = slot->bitmap;
        ft_glyph_top    = slot->bitmap_top;
        ft_glyph_left   = slot->bitmap_left;
    }
    else
    {
        FT_Stroker stroker;
        FT_BitmapGlyph ft_bitmap_glyph;

        error = FT_Stroker_New( self->library->library, &stroker );

        if( error )
        {
            freetype_error( error );
            goto cleanup_stroker;
        }

        FT_Stroker_Set(stroker,
                        (int)(self->outline_thickness * HRES),
                        FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND,
                        0);

        error = FT_Get_Glyph( self->face->glyph, &ft_glyph);

        if( error )
        {
            freetype_error( error );
            goto cleanup_stroker;
        }

        if( self->rendermode == RENDER_OUTLINE_EDGE )
            error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
        else if ( self->rendermode == RENDER_OUTLINE_POSITIVE )
            error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
        else if ( self->rendermode == RENDER_OUTLINE_NEGATIVE )
            error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );

        if( error )
        {
            freetype_error( error );
            goto cleanup_stroker;
        }

        switch( self->atlas->depth ) {
        case 1:
            error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
            break;
        case 3:
            error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
            break;
        case 4:
            error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
            break;
        }

        if( error )
        {
            freetype_error( error );
            goto cleanup_stroker;
        }

        ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
        ft_bitmap       = ft_bitmap_glyph->bitmap;
        ft_glyph_top    = ft_bitmap_glyph->top;
        ft_glyph_left   = ft_bitmap_glyph->left;

cleanup_stroker:
        FT_Stroker_Done( stroker );

        if( error )
        {
            texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );
            return 0;
        }
    }

    struct {
        int left;
        int top;
        int right;
        int bottom;
    } padding = { 0, 0, 1, 1 };

    if( self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        padding.top = 1;
        padding.left = 1;
    }

    if( self->padding != 0 )
    {
        padding.top += self->padding;
        padding.left += self->padding;
        padding.right += self->padding;
        padding.bottom += self->padding;
    }

    size_t src_w = self->atlas->depth == 3 ? ft_bitmap.width/3 : ft_bitmap.width;
    size_t src_h = ft_bitmap.rows;

    size_t tgt_w = src_w + padding.left + padding.right;
    size_t tgt_h = src_h + padding.top + padding.bottom;

    region = texture_atlas_get_region( self->atlas, tgt_w, tgt_h );

    if ( region.x < 0 )
    {
        freetype_gl_warning( Texture_Atlas_Full );
        texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );
        return 0;
    }

    x = region.x;
    y = region.y;

    // Copy pixel data over
    unsigned char *buffer = calloc( tgt_w * tgt_h * self->atlas->depth, sizeof(unsigned char) );

    unsigned char *dst_ptr = buffer + (padding.top * tgt_w + padding.left) * self->atlas->depth;
    unsigned char *src_ptr = ft_bitmap.buffer;
    if( ft_bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && self->atlas->depth == 4 )
    {
        // BGRA in, RGBA out
        for( i = 0; i < src_h; i++ ) {
            int j;
            for( j = 0; j < ft_bitmap.width; j++ ) {
                uint32_t bgra, rgba;
                bgra = ((uint32_t*)src_ptr)[j];
#if __BYTE_ORDER == __BIG_ENDIAN
                rgba = rol(__builtin_bswap32(bgra), 8);
#else
                rgba = rol(__builtin_bswap32(bgra), 24);
#endif
                ((uint32_t*)dst_ptr)[j] = rgba;
            }
            dst_ptr += tgt_w * self->atlas->depth;
            src_ptr += ft_bitmap.pitch;
        }
    }
    else if( ft_bitmap.pixel_mode == FT_PIXEL_MODE_BGRA && self->atlas->depth == 1 )
    {
        // BGRA in, grey out: Use weighted sum for luminosity, and multiply by alpha
        struct src_pixel_t { uint8_t b; uint8_t g; uint8_t r; uint8_t a; } * src = (struct src_pixel_t *)ft_bitmap.buffer;
        for( int row = 0; row < src_h; row++, dst_ptr += tgt_w * self->atlas->depth ) {
            for( int col = 0; col < src_w; col++, src++ ) {
                dst_ptr[col] = (0.3*src->r + 0.59*src->g + 0.11*src->b) * (src->a/255.0);
            }
        }
    }
    else if( ft_bitmap.pixel_mode == FT_PIXEL_MODE_GRAY && self->atlas->depth == 4 ) {
        // Grey in, RGBA out: Use grey level for alpha channel, with white color
        struct dst_pixel_t { uint8_t r; uint8_t g; uint8_t b; uint8_t a; } * dst = (struct dst_pixel_t *)dst_ptr;
        for( int row = 0; row < src_h; row++, dst += tgt_w ) {
            for( int col = 0; col < src_w; col++, src_ptr++ ) {
                dst[col] = (struct dst_pixel_t){ 255, 255, 255, *src_ptr };
            }
        }
    }
    else
    {
        // Straight copy, per row
        for( i = 0; i < src_h; i++ ) {
            //difference between width and pitch: https://www.freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_Bitmap
            memcpy( dst_ptr, src_ptr, ft_bitmap.width);
            dst_ptr += tgt_w * self->atlas->depth;
            src_ptr += ft_bitmap.pitch;
        }
    }

    if( self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        unsigned char *sdf = make_distance_mapb( buffer, tgt_w, tgt_h );
        free( buffer );
        buffer = sdf;
    }

    texture_atlas_set_region( self->atlas, x, y, tgt_w, tgt_h, buffer, tgt_w * self->atlas->depth);

    free( buffer );

    glyph = texture_glyph_new( );
    glyph->codepoint = glyph_index ? ucodepoint : 0;
;
    glyph->width    = tgt_w;
    glyph->height   = tgt_h;
    glyph->rendermode = self->rendermode;
    glyph->outline_thickness = self->outline_thickness;
    glyph->offset_x = ft_glyph_left;
    glyph->offset_y = ft_glyph_top;
    if(self->scaletex) {
        glyph->s0       = x/(float)self->atlas->width;
        glyph->t0       = y/(float)self->atlas->height;
        glyph->s1       = (x + glyph->width)/(float)self->atlas->width;
        glyph->t1       = (y + glyph->height)/(float)self->atlas->height;
    } else {
        // fix up unscaled coordinates by subtracting 0.5
        // this avoids drawing pixels from neighboring characters
        // note that you also have to paint these glyphs with an offset of
        // half a pixel each to get crisp rendering
        glyph->s0       = x - 0.5;
        glyph->t0       = y - 0.5;
        glyph->s1       = x + tgt_w - 0.5;
        glyph->t1       = y + tgt_h - 0.5;
    }
    slot = self->face->glyph;
    if( FT_HAS_FIXED_SIZES( self->face ) ) {
        // color fonts use actual pixels, not subpixels
        glyph->advance_x = slot->advance.x;
        glyph->advance_y = slot->advance.y;
    } else {
	glyph->advance_x = convert_F26Dot6_to_float(slot->advance.x) * self->scale;
        glyph->advance_y = convert_F26Dot6_to_float(slot->advance.y) * self->scale;
    }

    int free_glyph = texture_font_index_glyph(self, glyph, ucodepoint);
    if(!glyph_index) {
        if(!free_glyph) {
            glyph = texture_glyph_clone(glyph);
        }
        free_glyph = texture_font_index_glyph(self, glyph, 0);
    }
    if(free_glyph) {
        // fprintf(stderr, "Free glyph\n");
        free(glyph);
    }
    
    if( self->rendermode != RENDER_NORMAL && self->rendermode != RENDER_SIGNED_DISTANCE_FIELD )
        FT_Done_Glyph( ft_glyph );

    texture_font_generate_kerning( self, &self->library->library, &self->face );

    texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );

    return 1;
}

// ----------------------------------------------- texture_font_load_glyphs ---
size_t
texture_font_load_glyphs( texture_font_t * self,
                          const char * codepoints )
{
    size_t i;

    self->mode++;

    /* Load each glyph */
    for( i = 0; i < strlen(codepoints); i += utf8_surrogate_len(codepoints + i) ) {
        if( !texture_font_load_glyph( self, codepoints + i ) ) {
            self->mode--;
            texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );

            return utf8_strlen( codepoints + i );
        }
    }

    self->mode--;
    texture_font_close( self, MODE_AUTO_CLOSE, MODE_AUTO_CLOSE );

    return 0;
}


// ------------------------------------------------- texture_font_get_glyph ---
texture_glyph_t *
texture_font_get_glyph( texture_font_t * self,
                        const char * codepoint )
{
    texture_glyph_t *glyph;

    assert( self );
    assert( self->filename );
    assert( self->atlas );

    /* Check if codepoint has been already loaded */
    if( !(glyph = texture_font_find_glyph( self, codepoint )) )
    /* Glyph has not been already loaded */
        if( texture_font_load_glyph( self, codepoint ) )
            glyph = texture_font_find_glyph( self, codepoint );

    return glyph;
}

// ----------------------------------------------- texture_font_get_glyph_gi ---
texture_glyph_t *
texture_font_get_glyph_gi( texture_font_t * self,
                           uint32_t glyph_index )
{
    texture_glyph_t *glyph;

    assert( self );
    assert( self->filename );
    assert( self->atlas );

    /* Check if glyph_index has been already loaded */
    if( (glyph = texture_font_find_glyph_gi( self, glyph_index )) )
        return glyph;

    /* Glyph has not been already loaded */
    if( texture_font_load_glyph_gi( self, glyph_index, glyph_index ) )
        return texture_font_find_glyph_gi( self, glyph_index );

    return NULL;
}

// ------------------------------------------  texture_font_enlarge_texture ---
void
texture_font_enlarge_texture( texture_font_t * self, size_t width_new,
                              size_t height_new)
{
    assert(self);

    texture_atlas_enlarge_texture ( self->atlas, width_new, height_new);
}
// -------------------------------------------- texture_font_enlarge_glyphs ---
void
texture_font_enlarge_glyphs( texture_font_t * self, float mulw, float mulh)
{
    size_t i;
    texture_glyph_t* g;
    GLYPHS_ITERATOR(i, g, self->glyphs) {
        g->s0 *= mulw;
        g->s1 *= mulw;
        g->t0 *= mulh;
        g->t1 *= mulh;
    } GLYPHS_ITERATOR_END
}

// -------------------------------------------  texture_font_enlarge_atlas ---
void
texture_font_enlarge_atlas( texture_font_t * self, size_t width_new,
                            size_t height_new )
{
    assert(self);
    assert(self->atlas);
    //ensure size increased
    assert(width_new >= self->atlas->width);
    assert(height_new >= self->atlas->height);
    assert(width_new + height_new > self->atlas->width + self->atlas->height);    
    assert(width_new + height_new > self->atlas->width + self->atlas->height);
    texture_atlas_t* ta = self->atlas;
    size_t width_old = ta->width;
    size_t height_old = ta->height;    

    texture_font_enlarge_texture( self, width_new, height_new);
    if( self->scaletex ) {
        float mulw = (float)width_old / width_new;
        float mulh = (float)height_old / height_new;
        texture_font_enlarge_glyphs( self, mulw, mulh );
    }
}
