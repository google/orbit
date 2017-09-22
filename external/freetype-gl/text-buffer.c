/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "opengl.h"
#include "text-buffer.h"
#include "utf8-utils.h"

#define SET_GLYPH_VERTEX(value,x0,y0,z0,s0,t0,r,g,b,a,sh,gm) { \
	glyph_vertex_t *gv=&value;                                 \
	gv->x=x0; gv->y=y0; gv->z=z0;                              \
	gv->u=s0; gv->v=t0;                                        \
	gv->r=r; gv->g=g; gv->b=b; gv->a=a;                        \
	gv->shift=sh; gv->gamma=gm;}

// ----------------------------------------------------------------------------

text_buffer_t *
text_buffer_new( )
{
    text_buffer_t *self = (text_buffer_t *) malloc (sizeof(text_buffer_t));
    self->buffer = vertex_buffer_new(
                                     "vertex:3f,tex_coord:2f,color:4f,ashift:1f,agamma:1f" );
    self->line_start = 0;
    self->line_ascender = 0;
    self->base_color.r = 0.0;
    self->base_color.g = 0.0;
    self->base_color.b = 0.0;
    self->base_color.a = 1.0;
    self->line_descender = 0;
    self->lines = vector_new( sizeof(line_info_t) );
    self->bounds.left   = 0.0;
    self->bounds.top    = 0.0;
    self->bounds.width  = 0.0;
    self->bounds.height = 0.0;
    return self;
}

// ----------------------------------------------------------------------------
void
text_buffer_delete( text_buffer_t * self )
{
    vector_delete( self->lines );
    vertex_buffer_delete( self->buffer );
    free( self );
}

// ----------------------------------------------------------------------------
void
text_buffer_clear( text_buffer_t * self )
{
    assert( self );

    vertex_buffer_clear( self->buffer );
    self->line_start = 0;
    self->line_ascender = 0;
    self->line_descender = 0;
    vector_clear( self->lines );
    self->bounds.left   = 0.0;
    self->bounds.top    = 0.0;
    self->bounds.width  = 0.0;
    self->bounds.height = 0.0;
}

// ----------------------------------------------------------------------------
void
text_buffer_printf( text_buffer_t * self, vec2 *pen, ... )
{
    markup_t *markup;
    char *text;
    va_list args;

    if( vertex_buffer_size( self->buffer ) == 0 )
    {
        self->origin = *pen;
    }

    va_start ( args, pen );
    do {
        markup = va_arg( args, markup_t * );
        if( markup == NULL )
        {
            return;
        }
        text = va_arg( args, char * );
        text_buffer_add_text( self, pen, markup, text, 0 );
    } while( markup != 0 );
    va_end ( args );
}

// ----------------------------------------------------------------------------
void
text_buffer_move_last_line( text_buffer_t * self, float dy )
{
    size_t i;
    int j;
    for( i=self->line_start; i < vector_size( self->buffer->items ); ++i )
    {
        ivec4 *item = (ivec4 *) vector_get( self->buffer->items, i);
        for( j=item->vstart; j<item->vstart+item->vcount; ++j)
        {
            glyph_vertex_t * vertex =
                (glyph_vertex_t *)  vector_get( self->buffer->vertices, j );
            vertex->y -= dy;
        }
    }
}


// ----------------------------------------------------------------------------
// text_buffer_finish_line (internal use only)
// 
//  Performs calculations needed at the end of each line of text 
//  and prepares for the next line if necessary
//
//  advancePen: if true, advance the pen to the next line
//
static void
text_buffer_finish_line( text_buffer_t * self, vec2 * pen, bool advancePen )
{
    float line_left = self->line_left;
    float line_right = pen->x;
    float line_width  = line_right - line_left;
    float line_top = pen->y + self->line_ascender;
    float line_height = self->line_ascender - self->line_descender;
    float line_bottom = line_top - line_height;

    line_info_t line_info;
    line_info.line_start = self->line_start;
    line_info.bounds.left = line_left;
    line_info.bounds.top = line_top;
    line_info.bounds.width = line_width;
    line_info.bounds.height = line_height;

    vector_push_back( self->lines,  &line_info);


    if (line_left < self->bounds.left)
    {
        self->bounds.left = line_left;
    }
    if (line_top > self->bounds.top)
    {
        self->bounds.top = line_top;
    }

    float self_right = self->bounds.left + self->bounds.width;
    float self_bottom = self->bounds.top - self->bounds.height;

    if (line_right > self_right)
    {
        self->bounds.width = line_right - self->bounds.left;
    }
    if (line_bottom < self_bottom)
    {
        self->bounds.height = self->bounds.top - line_bottom;
    }

    if ( advancePen )
    {
        pen->x = self->origin.x;
        pen->y += (int)(self->line_descender);
    }

    self->line_descender = 0;
    self->line_ascender = 0;
    self->line_start = vector_size( self->buffer->items );
    self->line_left = pen->x;
}

// ----------------------------------------------------------------------------
void
text_buffer_add_text( text_buffer_t * self,
                      vec2 * pen, markup_t * markup,
                      const char * text, size_t length )
{
    size_t i;
    const char * prev_character = NULL;

    if( markup == NULL )
    {
        return;
    }

    if( !markup->font )
    {
        fprintf( stderr, "Houston, we've got a problem !\n" );
        return;
    }

    if( length == 0 )
    {
        length = utf8_strlen(text);
    }
    if( vertex_buffer_size( self->buffer ) == 0 )
    {
        self->origin = *pen;
        self->line_left = pen->x;
        self->bounds.left = pen->x;
        self->bounds.top = pen->y;
    }
    else
    {
        if (pen->x < self->origin.x)
        {
            self->origin.x = pen->x;
        }
        if (pen->y != self->last_pen_y)
        {
            text_buffer_finish_line(self, pen, false);
        }
    }

    for( i = 0; utf8_strlen( text + i ) && length; i += utf8_surrogate_len( text + i ) )
    {
        text_buffer_add_char( self, pen, markup, text + i, prev_character );
        prev_character = text + i;
        length--;
    }

    self->last_pen_y = pen->y;
}

// ----------------------------------------------------------------------------
void
text_buffer_add_char( text_buffer_t * self,
                      vec2 * pen, markup_t * markup,
                      const char * current, const char * previous )
{
    size_t vcount = 0;
    size_t icount = 0;
    vertex_buffer_t * buffer = self->buffer;
    texture_font_t * font = markup->font;
    float gamma = markup->gamma;

    // Maximum number of vertices is 20 (= 5x2 triangles) per glyph:
    //  - 2 triangles for background
    //  - 2 triangles for overline
    //  - 2 triangles for underline
    //  - 2 triangles for strikethrough
    //  - 2 triangles for glyph
    glyph_vertex_t vertices[4*5];
    GLuint indices[6*5];
    texture_glyph_t *glyph;
    texture_glyph_t *black;
    float kerning = 0.0f;

    if( markup->font->ascender > self->line_ascender )
    {
        float y = pen->y;
        pen->y -= (markup->font->ascender - self->line_ascender);
        text_buffer_move_last_line( self, (float)(int)(y-pen->y) );
        self->line_ascender = markup->font->ascender;
    }
    if( markup->font->descender < self->line_descender )
    {
        self->line_descender = markup->font->descender;
    }

    if( *current == '\n' )
    {
        text_buffer_finish_line(self, pen, true);
        return;
    }

    glyph = texture_font_get_glyph( font, current );
    black = texture_font_get_glyph( font, NULL );

    if( glyph == NULL )
    {
        return;
    }

    if( previous && markup->font->kerning )
    {
        kerning = texture_glyph_get_kerning( glyph, previous );
    }
    pen->x += kerning;

    // Background
    if( markup->background_color.alpha > 0 )
    {
        float r = markup->background_color.r;
        float g = markup->background_color.g;
        float b = markup->background_color.b;
        float a = markup->background_color.a;
        float x0 = ( pen->x -kerning );
        float y0 = (float)(int)( pen->y + font->descender );
        float x1 = ( x0 + glyph->advance_x );
        float y1 = (float)(int)( y0 + font->height + font->linegap );
        float s0 = black->s0;
        float t0 = black->t0;
        float s1 = black->s1;
        float t1 = black->t1;

        SET_GLYPH_VERTEX(vertices[vcount+0],
                         (float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+1],
                         (float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+2],
                         (float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+3],
                         (float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
        indices[icount + 0] = vcount+0;
        indices[icount + 1] = vcount+1;
        indices[icount + 2] = vcount+2;
        indices[icount + 3] = vcount+0;
        indices[icount + 4] = vcount+2;
        indices[icount + 5] = vcount+3;
        vcount += 4;
        icount += 6;
    }

    // Underline
    if( markup->underline )
    {
        float r = markup->underline_color.r;
        float g = markup->underline_color.g;
        float b = markup->underline_color.b;
        float a = markup->underline_color.a;
        float x0 = ( pen->x - kerning );
        float y0 = (float)(int)( pen->y + font->underline_position );
        float x1 = ( x0 + glyph->advance_x );
        float y1 = (float)(int)( y0 + font->underline_thickness );
        float s0 = black->s0;
        float t0 = black->t0;
        float s1 = black->s1;
        float t1 = black->t1;

        SET_GLYPH_VERTEX(vertices[vcount+0],
                         (float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+1],
                         (float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+2],
                         (float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+3],
                         (float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
        indices[icount + 0] = vcount+0;
        indices[icount + 1] = vcount+1;
        indices[icount + 2] = vcount+2;
        indices[icount + 3] = vcount+0;
        indices[icount + 4] = vcount+2;
        indices[icount + 5] = vcount+3;
        vcount += 4;
        icount += 6;
    }

    // Overline
    if( markup->overline )
    {
        float r = markup->overline_color.r;
        float g = markup->overline_color.g;
        float b = markup->overline_color.b;
        float a = markup->overline_color.a;
        float x0 = ( pen->x -kerning );
        float y0 = (float)(int)( pen->y + (int)font->ascender );
        float x1 = ( x0 + glyph->advance_x );
        float y1 = (float)(int)( y0 + (int)font->underline_thickness );
        float s0 = black->s0;
        float t0 = black->t0;
        float s1 = black->s1;
        float t1 = black->t1;
        SET_GLYPH_VERTEX(vertices[vcount+0],
                         (float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+1],
                         (float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+2],
                         (float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+3],
                         (float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
        indices[icount + 0] = vcount+0;
        indices[icount + 1] = vcount+1;
        indices[icount + 2] = vcount+2;
        indices[icount + 3] = vcount+0;
        indices[icount + 4] = vcount+2;
        indices[icount + 5] = vcount+3;
        vcount += 4;
        icount += 6;
    }

    /* Strikethrough */
    if( markup->strikethrough )
    {
        float r = markup->strikethrough_color.r;
        float g = markup->strikethrough_color.g;
        float b = markup->strikethrough_color.b;
        float a = markup->strikethrough_color.a;
        float x0  = ( pen->x -kerning );
        float y0  = (float)(int)( pen->y + (int)font->ascender*.33f);
        float x1  = ( x0 + glyph->advance_x );
        float y1  = (float)(int)( y0 + (int)font->underline_thickness );
        float s0 = black->s0;
        float t0 = black->t0;
        float s1 = black->s1;
        float t1 = black->t1;
        SET_GLYPH_VERTEX(vertices[vcount+0],
                         (float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+1],
                         (float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+2],
                         (float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+3],
                         (float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
        indices[icount + 0] = vcount+0;
        indices[icount + 1] = vcount+1;
        indices[icount + 2] = vcount+2;
        indices[icount + 3] = vcount+0;
        indices[icount + 4] = vcount+2;
        indices[icount + 5] = vcount+3;
        vcount += 4;
        icount += 6;
    }
    {
        // Actual glyph
        float r = markup->foreground_color.red;
        float g = markup->foreground_color.green;
        float b = markup->foreground_color.blue;
        float a = markup->foreground_color.alpha;
        float x0 = ( pen->x + glyph->offset_x );
        float y0 = (float)(int)( pen->y + glyph->offset_y );
        float x1 = ( x0 + glyph->width );
        float y1 = (float)(int)( y0 - glyph->height );
        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;

        SET_GLYPH_VERTEX(vertices[vcount+0],
                         (float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+1],
                         (float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+2],
                         (float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
        SET_GLYPH_VERTEX(vertices[vcount+3],
                         (float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
        indices[icount + 0] = vcount+0;
        indices[icount + 1] = vcount+1;
        indices[icount + 2] = vcount+2;
        indices[icount + 3] = vcount+0;
        indices[icount + 4] = vcount+2;
        indices[icount + 5] = vcount+3;
        vcount += 4;
        icount += 6;

        vertex_buffer_push_back( buffer, vertices, vcount, indices, icount );
        pen->x += glyph->advance_x * (1.0f + markup->spacing);
    }
}

// ----------------------------------------------------------------------------
void
text_buffer_align( text_buffer_t * self, vec2 * pen,
                   enum Align alignment )
{
    if (ALIGN_LEFT == alignment)
    {
        return;
    }

    size_t total_items = vector_size( self->buffer->items );
    if ( self->line_start != total_items )
    {
        text_buffer_finish_line( self, pen, false );
    }


    size_t i, j;
    int k;
    float self_left, self_right, self_center;
    float line_left, line_right, line_center;
    float dx;

    self_left = self->bounds.left;
    self_right = self->bounds.left + self->bounds.width;
    self_center = (self_left + self_right) / 2;

    line_info_t* line_info;
    size_t lines_count, line_end;

    lines_count = vector_size( self->lines );
    for ( i = 0; i < lines_count; ++i )
    {
        line_info = (line_info_t*)vector_get( self->lines, i );

        if ( i + 1 < lines_count )
        {
            line_end = ((line_info_t*)vector_get( self->lines, i + 1 ))->line_start;
        }
        else
        {
            line_end = vector_size( self->buffer->items );
        }

        line_right = line_info->bounds.left + line_info->bounds.width;

        if ( ALIGN_RIGHT == alignment )
        {
            dx = self_right - line_right;
        }
        else // ALIGN_CENTER
        {
            line_left = line_info->bounds.left;
            line_center = (line_left + line_right) / 2;
            dx = self_center - line_center;
        }

        dx = roundf( dx );

        for( j=line_info->line_start; j < line_end; ++j )
        {
            ivec4 *item = (ivec4 *) vector_get( self->buffer->items, j);
            for( k=item->vstart; k<item->vstart+item->vcount; ++k)
            {
                glyph_vertex_t * vertex =
                                   (glyph_vertex_t *)vector_get( self->buffer->vertices, k );
                vertex->x += dx;
            }
        }
        
    }
}

vec4
text_buffer_get_bounds( text_buffer_t * self, vec2 * pen )
{
    size_t total_items = vector_size( self->buffer->items );
    if ( self->line_start != total_items )
    {
        text_buffer_finish_line( self, pen, false );
    }

    return self->bounds;
}
