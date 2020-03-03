//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TextRenderer.h"

#include "App.h"
#include "Core.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "Params.h"
#include "shader.h"
#include "freetype-gl/vertex-buffer.h"

//-----------------------------------------------------------------------------
typedef struct {
  float x, y, z;     // position
  float s, t;        // texture
  float r, g, b, a;  // color
} vertex_t;

//-----------------------------------------------------------------------------
TextRenderer::TextRenderer()
    : m_Atlas(NULL),
      m_Buffer(NULL),
      m_Font(NULL),
      m_Canvas(NULL),
      m_Initialized(false),
      m_DrawOutline(false) {}

//-----------------------------------------------------------------------------
TextRenderer::~TextRenderer() {
  if (m_Font) {
    texture_font_delete(m_Font);
  }

  if (m_Buffer) {
    vertex_buffer_delete(m_Buffer);
  }
}

//-----------------------------------------------------------------------------
void TextRenderer::Init() {
  if (m_Initialized) return;

  m_Font = NULL;
  int atlasSize = 2 * 1024;
  m_Atlas = texture_atlas_new(atlasSize, atlasSize, 1);

  const auto exePath = Path::GetExecutablePath();
  const auto fontFileName = exePath + "fonts/Vera.ttf";

  static float fsize = GParams.m_FontSize;
  m_Buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
  m_Font = texture_font_new_from_file(m_Atlas, fsize, fontFileName.c_str());
  for (int i = 10; i <= 100; i += 10) {
    m_FontsBySize[i] =
        texture_font_new_from_file(m_Atlas, i, fontFileName.c_str());
  }

  m_Pen.x = 0;
  m_Pen.y = 0;

  glGenTextures(1, &m_Atlas->id);

  const auto vertShaderFileName = exePath + "shaders/v3f-t2f-c4f.vert";
  const auto fragShaderFileName = exePath + "shaders/v3f-t2f-c4f.frag";
  m_Shader =
      shader_load(vertShaderFileName.c_str(), fragShaderFileName.c_str());

  mat4_set_identity(&m_Proj);
  mat4_set_identity(&m_Model);
  mat4_set_identity(&m_View);

  m_Initialized = true;
}

//-----------------------------------------------------------------------------
void TextRenderer::SetFontSize(int a_Size) {
  texture_font_t* font = m_FontsBySize[a_Size];
  if (font) {
    m_Font = font;
  }
}

//-----------------------------------------------------------------------------
void TextRenderer::Display() {
  if (m_DrawOutline) {
    DrawOutline(m_Buffer);
  }

  // Lazy init
  if (!m_Initialized) {
    Init();
  }

  glBindTexture(GL_TEXTURE_2D, m_Atlas->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)m_Atlas->width,
               (GLsizei)m_Atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE,
               m_Atlas->data);

  // Get current projection matrix
  GLfloat matrix[16];
  glGetFloatv(GL_PROJECTION_MATRIX, matrix);
  mat4* proj = (mat4*)&matrix[0];
  m_Proj = *proj;

  mat4_set_orthographic(&m_Proj, 0, (float)m_Canvas->getWidth(), 0,
                        (float)m_Canvas->getHeight(), -1, 1);

  glUseProgram(m_Shader);
  {
    glUniform1i(glGetUniformLocation(m_Shader, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "model"), 1, 0,
                       m_Model.data);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "view"), 1, 0,
                       m_View.data);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "projection"), 1, 0,
                       m_Proj.data);
    vertex_buffer_render(m_Buffer, GL_TRIANGLES);

    /*PRINT_VAR(m_Model);
    PRINT_VAR(m_View);
    PRINT_VAR(m_Proj);*/
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

//-----------------------------------------------------------------------------
void TextRenderer::DrawOutline(vertex_buffer_t* a_Buffer) {
  glBegin(GL_LINES);

  for (int i = 0; i < (int)a_Buffer->indices->size; i += 3) {
    GLuint i0 = *(GLuint*)vector_get(a_Buffer->indices, i + 0);
    GLuint i1 = *(GLuint*)vector_get(a_Buffer->indices, i + 1);
    GLuint i2 = *(GLuint*)vector_get(a_Buffer->indices, i + 2);

    vertex_t v0 = *(vertex_t*)vector_get(a_Buffer->vertices, i0);
    vertex_t v1 = *(vertex_t*)vector_get(a_Buffer->vertices, i1);
    vertex_t v2 = *(vertex_t*)vector_get(a_Buffer->vertices, i2);

    glVertex3f(v0.x, v0.y, v0.z);
    glVertex3f(v1.x, v1.y, v1.z);

    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);

    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v0.x, v0.y, v0.z);
  }

  glEnd();
}

//-----------------------------------------------------------------------------
void TextRenderer::AddTextInternal(texture_font_t* font, const char* text,
                                   const vec4& color, vec2* pen,
                                   float a_MaxSize, float a_Z, bool a_Static) {
  size_t i;
  float r = color.red, g = color.green, b = color.blue, a = color.alpha;
  float textZ = a_Z;

  float maxWidth = a_MaxSize == -1.f ? FLT_MAX : ToScreenSpace(a_MaxSize);
  float strWidth = 0.f;
  int minX = INT_MAX;
  int maxX = -INT_MAX;

  for (i = 0; i < strlen(text); ++i) {
    if (!texture_font_find_glyph(font, text + i)) {
      texture_font_load_glyph(font, text + i);
    }

    texture_glyph_t* glyph = texture_font_get_glyph(font, text + i);
    if (glyph != NULL) {
      float kerning = 0.0f;
      if (i > 0) {
        kerning = texture_glyph_get_kerning(glyph, text + i - 1);
      }
      pen->x += kerning;
      int x0 = (int)(pen->x + glyph->offset_x);
      int y0 = (int)(pen->y + glyph->offset_y);
      int x1 = (int)(x0 + glyph->width);
      int y1 = (int)(y0 - glyph->height);
      float s0 = glyph->s0;
      float t0 = glyph->t0;
      float s1 = glyph->s1;
      float t1 = glyph->t1;
      GLuint indices[6] = {0, 1, 2, 0, 2, 3};
      vertex_t vertices[4] = {
          {(float)x0, (float)y0, textZ, s0, t0, r, g, b, a},
          {(float)x0, (float)y1, textZ, s0, t1, r, g, b, a},
          {(float)x1, (float)y1, textZ, s1, t1, r, g, b, a},
          {(float)x1, (float)y0, textZ, s1, t0, r, g, b, a}};

      minX = std::min(minX, x0);
      maxX = std::max(maxX, x1);
      strWidth = float(maxX - minX);

      if (strWidth > maxWidth) {
        break;
      }

      vertex_buffer_push_back(m_Buffer, vertices, 4, indices, 6);
      pen->x += glyph->advance_x;
    }
  }
}

//-----------------------------------------------------------------------------
void TextRenderer::AddText(const char* a_Text, float a_X, float a_Y, float a_Z,
                           const Color& a_Color, float a_MaxSize,
                           bool a_RightJustified) {
  ToScreenSpace(a_X, a_Y, m_Pen.x, m_Pen.y);

  if (a_RightJustified) {
    a_MaxSize = FLT_MAX;
    int stringWidth, stringHeight;
    GetStringSize(a_Text, stringWidth, stringHeight);
    m_Pen.x -= stringWidth;
  }

  AddTextInternal(m_Font, a_Text, ColorToVec4(a_Color), &m_Pen, a_MaxSize, a_Z);
}

//-----------------------------------------------------------------------------
void TextRenderer::AddTextTrailingCharsPrioritized(
    const char* a_Text, float a_X, float a_Y, float a_Z, const Color& a_Color,
    size_t a_TrailingCharsLength, float a_MaxSize) {
  float tempPenX = ToScreenSpace(a_X);
  float maxWidth = a_MaxSize == -1.f ? FLT_MAX : ToScreenSpace(a_MaxSize);
  float strWidth = 0.f;
  int minX = INT_MAX;
  int maxX = -INT_MAX;

  const size_t textLen = strlen(a_Text);

  size_t i;
  for (i = 0; i < textLen; ++i) {
    if (!texture_font_find_glyph(m_Font, a_Text + i)) {
      texture_font_load_glyph(m_Font, a_Text + i);
    }

    texture_glyph_t* glyph = texture_font_get_glyph(m_Font, a_Text + i);
    if (glyph != NULL) {
      float kerning = 0.0f;
      if (i > 0) {
        kerning = texture_glyph_get_kerning(glyph, a_Text + i - 1);
      }
      tempPenX += kerning;
      int x0 = (int)(tempPenX + glyph->offset_x);
      int x1 = (int)(x0 + glyph->width);

      minX = std::min(minX, x0);
      maxX = std::max(maxX, x1);
      strWidth = float(maxX - minX);

      if (strWidth > maxWidth) {
        break;
      }

      tempPenX += glyph->advance_x;
    }
  }

  // TODO: Technically, we'd want the size of "... <TIME>" + remaining
  // characters

  auto fittingCharsCount = i;

  static const char* ELLIPSIS_TEXT = "... ";
  static const size_t ELLIPSIS_TEXT_LEN = strlen(ELLIPSIS_TEXT);
  static const size_t LEADING_CHARS_COUNT = 1;
  static const size_t ELLIPSIS_BUFFER_SIZE =
      ELLIPSIS_TEXT_LEN + LEADING_CHARS_COUNT;

  bool useEllipsisText =
      (fittingCharsCount < textLen) &&
      (fittingCharsCount > (a_TrailingCharsLength + ELLIPSIS_BUFFER_SIZE));

  if (!useEllipsisText) {
    AddText(a_Text, a_X, a_Y, a_Z, a_Color, a_MaxSize);
  } else {
    auto leadingCharCount =
        fittingCharsCount - (a_TrailingCharsLength + ELLIPSIS_TEXT_LEN);

    std::string modifiedText(a_Text, leadingCharCount);
    modifiedText.append(ELLIPSIS_TEXT);

    auto timePosition = textLen - a_TrailingCharsLength;
    modifiedText.append(&a_Text[timePosition], a_TrailingCharsLength);

    AddText(modifiedText.c_str(), a_X, a_Y, a_Z, a_Color, a_MaxSize);
  }
}

//-----------------------------------------------------------------------------
int TextRenderer::AddText2D(const char* a_Text, int a_X, int a_Y, float a_Z,
                            const Color& a_Color, float a_MaxSize,
                            bool a_RightJustified, bool a_InvertY) {
  if (a_RightJustified) {
    int stringWidth, stringHeight;
    GetStringSize(a_Text, stringWidth, stringHeight);
    a_X -= stringWidth;
  }

  m_Pen.x = (float)a_X;
  m_Pen.y = a_InvertY ? (float)(m_Canvas->getHeight() - a_Y) : (float)a_Y;

  AddTextInternal(m_Font, a_Text, ColorToVec4(a_Color), &m_Pen, a_MaxSize, a_Z);

  return a_X;
}

//-----------------------------------------------------------------------------
void TextRenderer::GetStringSize(const char* a_Text, int& a_Width,
                                 int& a_Height) {
  float stringWidth = 0;
  float stringHeight = 0;

  std::size_t len = strlen(a_Text);
  for (std::size_t i = 0; i < len; ++i) {
    texture_glyph_t* glyph = texture_font_get_glyph(m_Font, a_Text + i);
    if (glyph != NULL) {
      float kerning = 0.0f;
      if (i > 0) {
        kerning = texture_glyph_get_kerning(glyph, a_Text + i - 1);
      }

      stringWidth += kerning;
      stringWidth += glyph->advance_x;

      if (glyph->height > stringHeight) stringHeight = (float)glyph->height;
    }
  }

  a_Width = (int)stringWidth;
  a_Height = (int)stringHeight;
}

//-----------------------------------------------------------------------------
int TextRenderer::GetStringHeight(const char* a_Text) {
  int w, h;
  GetStringSize(a_Text, w, h);
  return h;
}

//-----------------------------------------------------------------------------
void TextRenderer::ToScreenSpace(float a_X, float a_Y, float& o_X, float& o_Y) {
  float WorldWidth = m_Canvas->GetWorldWidth();
  float WorldHeight = m_Canvas->GetWorldHeight();
  float WorldTopLeftX = m_Canvas->GetWorldTopLeftX();
  float WorldMinLeftY = m_Canvas->GetWorldTopLeftY() - WorldHeight;

  o_X = ((a_X - WorldTopLeftX) / WorldWidth) * (float)m_Canvas->getWidth();
  o_Y = ((a_Y - WorldMinLeftY) / WorldHeight) * (float)m_Canvas->getHeight();
}

//-----------------------------------------------------------------------------
float TextRenderer::ToScreenSpace(float a_Size) {
  return (a_Size / m_Canvas->GetWorldWidth()) * (float)m_Canvas->getWidth();
}

//-----------------------------------------------------------------------------
void TextRenderer::Clear() {
  m_Pen.x = 0.f;
  m_Pen.y = 0.f;
  if (m_Buffer) {
    vertex_buffer_clear(m_Buffer);
  }
}

//-----------------------------------------------------------------------------
const TextBox& TextRenderer::GetSceneBox() const {
  return m_Canvas->GetSceneBox();
}

//-----------------------------------------------------------------------------
int TextRenderer::GetNumCharacters() const {
  return int(m_Buffer->vertices->size) / 4;
}
