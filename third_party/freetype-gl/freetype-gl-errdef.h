/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */

#ifndef __FREETYPE_GL_ERRORS_H__
#define __FREETYPE_GL_ERRORS_H__

FTGL_ERROR_START_LIST

FTGL_ERRORDEF_( Texture_Atlas_Full,			0x00,
		"Texture atlas is full" )
FTGL_ERRORDEF_( Cannot_Load_File,			0x01,
		"unable to load file" )
FTGL_ERRORDEF_( Font_Unavailable,			0x02,
		"no font available" )
FTGL_ERRORDEF_( No_Font_File_Given,			0x03,
		"no font file given" )
FTGL_ERRORDEF_( Out_Of_Memory,				0x04,
		"out of memory" )
FTGL_ERRORDEF_( Unimplemented_Function,			0x05,
		"unimplemented function" )
FTGL_ERRORDEF_( Cant_Match_Family,			0x06,
		"fontconfig error: could not match family" )
FTGL_ERRORDEF_( No_Font_In_Markup,			0x07,
		"Markup doesn't have a font" )
FTGL_ERRORDEF_( No_Size_Specified,			0x08,
		"No size specified for attribute" )
FTGL_ERRORDEF_( No_Format_Specified,			0x09,
		"No format specified for attribute" )
FTGL_ERRORDEF_( Vertex_Attribute_Format_Wrong,		0x0A,
		"Vertex attribute format not understood" )
FTGL_ERRORDEF_( Load_Color_Not_Available,		0x0B,
		"FT_LOAD_COLOR not available" )
FTGL_ERRORDEF_( No_Fixed_Size_In_Color_Font,		0x0C,
		"No fixed size in color font" )

FTGL_ERROR_END_LIST

#endif
