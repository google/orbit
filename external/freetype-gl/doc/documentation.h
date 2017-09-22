/**
@example ansi.c
@example font.c
@example gamma.c
@example markup.c
@example makefont.c
@example console.c
@example texture.c
@example outline.c
@example cartoon.c
@example benchmark.c
@example subpixel.c
@example atb-agg.c
@example distance-field.c

@page documentation Documentation

\tableofcontents

@section introduction Introduction

Freetype GL is a small library for displaying unicode text using a (single)
vertex buffer and a single texture where necessary glyphs are tighly packed.

<b>Example Usage</b>:
@code
   #include "freetype-gl.h"

   ...

   // Text to be printed
   char *text = "A Quick Brown Fox Jumps Over The Lazy Dog 0123456789";

   // Texture atlas to store individual glyphs
   texture_atlas_t *atlas = texture_atlas_new( 512, 512, 1 );

   // Build a new texture font from its description and size
   texture_font_t *font = texture_font_new( atlas, "./Vera.ttf", 16 );

   // Build a new vertex buffer (position, texture & color)
   vertex_buffer_t *buffer= vertex_buffer_new( "v3i:t2f:c4f" );

   // Where to start printing on screen
   vec2 pen = {0,0};

   // Text color
   vec4 black = {0,0,0,1};

   // Add text to the buffer
   add_text( buffer, font, "Hello World !", text, &black, &pen );

   ...

   vertex_buffer_render( buffer, GL_TRIANGLES, "vtc");

   ...

@endcode

-------------------------------------------------------------------------------
@section API Application Programming Interface

The code is fairly simple and organized as follow:

@subsection mandatory Mandatory

These are the minimum set of structures and files that are required to load a
font and pack glyphs into a texture.

- @ref vector<br/>
  This structure loosely mimics the std::vector class from c++.

- @ref texture-atlas<br/>
  Texture atlas is used to pack several small regions into a single texture.

- @ref texture-font<br/>
  The texture-font structure is in charge of creating bitmap glyphs and to
  upload them to a texture atlas.


@subsection optional Optional

These are convenient stuctures and files that facilitate text rendering.

- @ref shader<br/>
  Shader loading/compiling/linking.

- @ref vertex-buffer<br/>
  Generic vertex buffer structure inspired by pyglet (python).

- @ref markup<br/>
  Simple structure that describes text properties.

- @ref font-manager<br/>
  Structure in charge of caching fonts.

- @ref text-buffer<br/>
  Convenient structure for manipulating and rendering text.


*/
