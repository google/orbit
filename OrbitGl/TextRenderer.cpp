// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TextRenderer.h"

#include <freetype-gl/shader.h>
#include <freetype-gl/vertex-buffer.h>

#include "App.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "Params.h"
#include "Path.h"

typedef struct {
  float x, y, z;     // position
  float s, t;        // texture
  float r, g, b, a;  // color
} vertex_t;

TextRenderer::TextRenderer()
    : m_Atlas(nullptr),
      m_Buffer(nullptr),
      m_Font(nullptr),
      m_Canvas(nullptr),
      m_Initialized(false),
      m_DrawOutline(false) {}

TextRenderer::~TextRenderer() {
  for (const auto& pair : m_FontsBySize) {
    texture_font_delete(pair.second);
  }
  m_FontsBySize.clear();

  if (m_Buffer) {
    vertex_buffer_delete(m_Buffer);
  }

  if (m_Atlas) {
    texture_atlas_delete(m_Atlas);
  }
}

void TextRenderer::Init() {
  if (m_Initialized) return;

  int atlasSize = 2 * 1024;
  m_Atlas = texture_atlas_new(atlasSize, atlasSize, 1);

  const auto exe_dir = Path::GetExecutableDir();
  const auto fontFileName = exe_dir + "fonts/Vera.ttf";

  m_Buffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
  for (int i = 1; i <= 100; i += 1) {
    m_FontsBySize[i] = texture_font_new_from_file(m_Atlas, i, fontFileName.c_str());
  }
  current_font_size_ = GParams.font_size;
  SetFontSize(current_font_size_);

  m_Pen.x = 0;
  m_Pen.y = 0;

  glGenTextures(1, &m_Atlas->id);

  const auto vertShaderFileName = Path::JoinPath({exe_dir, "shaders", "v3f-t2f-c4f.vert"});
  const auto fragShaderFileName = Path::JoinPath({exe_dir, "shaders", "v3f-t2f-c4f.frag"});
  m_Shader = shader_load(vertShaderFileName.c_str(), fragShaderFileName.c_str());

  mat4_set_identity(&m_Proj);
  mat4_set_identity(&m_Model);
  mat4_set_identity(&m_View);

  m_Initialized = true;
}

void TextRenderer::SetFontSize(uint32_t size) {
  CHECK(!m_FontsBySize.empty());
  if (!m_FontsBySize.count(size)) {
    auto iterator_next = m_FontsBySize.upper_bound(size);
    // If there isn't that font_size in the map, we will search for the next one or previous one
    if (iterator_next != m_FontsBySize.end()) {
      size = iterator_next->first;
    } else if (iterator_next != m_FontsBySize.begin()) {
      size = (--iterator_next)->first;
    }
  }
  texture_font_t* font = m_FontsBySize[size];
  m_Font = font;
  current_font_size_ = size;
}

uint32_t TextRenderer::GetFontSize() const { return current_font_size_; }

void TextRenderer::Display(Batcher* batcher) {
  if (m_DrawOutline) {
    DrawOutline(batcher, m_Buffer);
  }

  // Lazy init
  if (!m_Initialized) {
    Init();
  }

  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, m_Atlas->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(m_Atlas->width),
               static_cast<GLsizei>(m_Atlas->height), 0, GL_RED, GL_UNSIGNED_BYTE, m_Atlas->data);

  // Get current projection matrix
  GLfloat matrix[16];
  glGetFloatv(GL_PROJECTION_MATRIX, matrix);
  mat4* proj = reinterpret_cast<mat4*>(&matrix[0]);
  m_Proj = *proj;

  mat4_set_orthographic(&m_Proj, 0, m_Canvas->GetWidth(), 0, m_Canvas->GetHeight(), -1, 1);

  glUseProgram(m_Shader);
  {
    glUniform1i(glGetUniformLocation(m_Shader, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "model"), 1, 0, m_Model.data);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "view"), 1, 0, m_View.data);
    glUniformMatrix4fv(glGetUniformLocation(m_Shader, "projection"), 1, 0, m_Proj.data);
    vertex_buffer_render(m_Buffer, GL_TRIANGLES);

    /*PRINT_VAR(m_Model);
    PRINT_VAR(m_View);
    PRINT_VAR(m_Proj);*/
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  glPopAttrib();
}

void TextRenderer::DrawOutline(Batcher* batcher, vertex_buffer_t* a_Buffer) {
  // TODO: No color was set here before.
  Color color(255, 255, 255, 255);

  for (size_t i = 0; i < a_Buffer->indices->size; i += 3) {
    GLuint i0 = *static_cast<const GLuint*>(vector_get(a_Buffer->indices, i + 0));
    GLuint i1 = *static_cast<const GLuint*>(vector_get(a_Buffer->indices, i + 1));
    GLuint i2 = *static_cast<const GLuint*>(vector_get(a_Buffer->indices, i + 2));

    vertex_t v0 = *static_cast<const vertex_t*>(vector_get(a_Buffer->vertices, i0));
    vertex_t v1 = *static_cast<const vertex_t*>(vector_get(a_Buffer->vertices, i1));
    vertex_t v2 = *static_cast<const vertex_t*>(vector_get(a_Buffer->vertices, i2));

    // TODO: This should be pickable??
    batcher->AddLine(Vec2(v0.x, v0.y), Vec2(v1.x, v1.y), v0.z, color);
    batcher->AddLine(Vec2(v1.x, v1.y), Vec2(v2.x, v2.y), v1.z, color);
    batcher->AddLine(Vec2(v2.x, v2.y), Vec2(v0.x, v0.y), v2.z, color);
  }
}

void TextRenderer::AddTextInternal(texture_font_t* font, const char* text, const vec4& color,
                                   vec2* pen, float a_MaxSize, float a_Z, bool) {
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
      float x0 = static_cast<int>(pen->x + glyph->offset_x);
      float y0 = static_cast<int>(pen->y + glyph->offset_y);
      float x1 = static_cast<int>(x0 + glyph->width);
      float y1 = static_cast<int>(y0 - glyph->height);
      float s0 = glyph->s0;
      float t0 = glyph->t0;
      float s1 = glyph->s1;
      float t1 = glyph->t1;
      GLuint indices[6] = {0, 1, 2, 0, 2, 3};
      vertex_t vertices[4] = {{x0, y0, textZ, s0, t0, r, g, b, a},
                              {x0, y1, textZ, s0, t1, r, g, b, a},
                              {x1, y1, textZ, s1, t1, r, g, b, a},
                              {x1, y0, textZ, s1, t0, r, g, b, a}};

      minX = std::min(minX, static_cast<int>(x0));
      maxX = std::max(maxX, static_cast<int>(x1));
      strWidth = maxX - minX;

      if (strWidth > maxWidth) {
        break;
      }

      vertex_buffer_push_back(m_Buffer, vertices, 4, indices, 6);
      pen->x += glyph->advance_x;
    }
  }
}

void TextRenderer::AddText(const char* a_Text, float a_X, float a_Y, float a_Z,
                           const Color& a_Color, uint32_t font_size, float a_MaxSize,
                           bool a_RightJustified) {
  if (!font_size) return;
  ToScreenSpace(a_X, a_Y, m_Pen.x, m_Pen.y);

  if (a_RightJustified) {
    a_MaxSize = FLT_MAX;
    int stringWidth = GetStringWidth(a_Text);
    m_Pen.x -= stringWidth;
  }
  SetFontSize(font_size);

  AddTextInternal(m_Font, a_Text, ColorToVec4(a_Color), &m_Pen, a_MaxSize, a_Z);
}

void TextRenderer::AddTextTrailingCharsPrioritized(const char* a_Text, float a_X, float a_Y,
                                                   float a_Z, const Color& a_Color,
                                                   size_t a_TrailingCharsLength, uint32_t font_size,
                                                   float a_MaxSize) {
  if (!m_Initialized) {
    Init();
  }

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
      int x0 = static_cast<int>(tempPenX + glyph->offset_x);
      int x1 = static_cast<int>(x0 + glyph->width);

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
  static const size_t ELLIPSIS_BUFFER_SIZE = ELLIPSIS_TEXT_LEN + LEADING_CHARS_COUNT;

  bool useEllipsisText = (fittingCharsCount < textLen) &&
                         (fittingCharsCount > (a_TrailingCharsLength + ELLIPSIS_BUFFER_SIZE));

  if (!useEllipsisText) {
    AddText(a_Text, a_X, a_Y, a_Z, a_Color, font_size, a_MaxSize);
  } else {
    auto leadingCharCount = fittingCharsCount - (a_TrailingCharsLength + ELLIPSIS_TEXT_LEN);

    std::string modifiedText(a_Text, leadingCharCount);
    modifiedText.append(ELLIPSIS_TEXT);

    auto timePosition = textLen - a_TrailingCharsLength;
    modifiedText.append(&a_Text[timePosition], a_TrailingCharsLength);

    AddText(modifiedText.c_str(), a_X, a_Y, a_Z, a_Color, font_size, a_MaxSize);
  }
}

int TextRenderer::AddText2D(const char* a_Text, int a_X, int a_Y, float a_Z, const Color& a_Color,
                            float a_MaxSize, bool a_RightJustified, bool a_InvertY) {
  if (a_RightJustified) {
    int stringWidth = GetStringWidth(a_Text);
    a_X -= stringWidth;
  }

  m_Pen.x = a_X;
  m_Pen.y = a_InvertY ? m_Canvas->GetHeight() - a_Y : a_Y;

  AddTextInternal(m_Font, a_Text, ColorToVec4(a_Color), &m_Pen, a_MaxSize, a_Z);

  return a_X;
}

int TextRenderer::GetStringWidth(const char* a_Text) const {
  float stringWidth = 0;

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
    }
  }

  return static_cast<int>(ceil(stringWidth));
}

void TextRenderer::ToScreenSpace(float a_X, float a_Y, float& o_X, float& o_Y) {
  float WorldWidth = m_Canvas->GetWorldWidth();
  float WorldHeight = m_Canvas->GetWorldHeight();
  float WorldTopLeftX = m_Canvas->GetWorldTopLeftX();
  float WorldMinLeftY = m_Canvas->GetWorldTopLeftY() - WorldHeight;

  o_X = ((a_X - WorldTopLeftX) / WorldWidth) * m_Canvas->GetWidth();
  o_Y = ((a_Y - WorldMinLeftY) / WorldHeight) * m_Canvas->GetHeight();
}

float TextRenderer::ToScreenSpace(float a_Size) {
  return (a_Size / m_Canvas->GetWorldWidth()) * m_Canvas->GetWidth();
}

void TextRenderer::Clear() {
  m_Pen.x = 0.f;
  m_Pen.y = 0.f;
  if (m_Buffer) {
    vertex_buffer_clear(m_Buffer);
  }
}

int TextRenderer::GetNumCharacters() const { return int(m_Buffer->vertices->size) / 4; }
