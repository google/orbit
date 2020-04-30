//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>

#include "OpenGl.h"
#include "Platform.h"
#include "TextBox.h"
#include "mat4.h"

namespace ftgl {
struct vertex_buffer_t;
struct texture_font_t;
}  // namespace ftgl

class TextRenderer {
 public:
  TextRenderer();
  ~TextRenderer();

  void Init();
  void Display();
  void AddText(const char* a_Text, float a_X, float a_Y, float a_Z,
               const Color& a_Color, float a_MaxSize = -1.f,
               bool a_RightJustified = false);
  void AddTextTrailingCharsPrioritized(const char* a_Text, float a_X, float a_Y,
                                       float a_Z, const Color& a_Color,
                                       size_t a_TrailingCharsLength,
                                       float a_MaxSize);
  int AddText2D(const char* a_Text, int a_X, int a_Y, float a_Z,
                const Color& a_Color, float a_MaxSize = -1.f,
                bool a_RightJustified = false, bool a_InvertY = true);

  void GetStringSize(const char* a_Text, int& a_Width, int& a_Height);
  int GetStringHeight(const char* a_Text);
  void Clear();
  void SetCanvas(class GlCanvas* a_Canvas) { m_Canvas = a_Canvas; }
  const GlCanvas* GetCanvas() const { return m_Canvas; }
  GlCanvas* GetCanvas() { return m_Canvas; }
  const TextBox& GetSceneBox() const;
  int GetNumCharacters() const;
  void ToggleDrawOutline() { m_DrawOutline = !m_DrawOutline; }
  void SetFontSize(int a_Size);

 protected:
  void AddTextInternal(texture_font_t* font, const char* text,
                       const vec4& color, vec2* pen, float a_MaxSize = -1.f,
                       float a_Z = -0.01f, bool a_Static = false);
  void ToScreenSpace(float a_X, float a_Y, float& o_X, float& o_Y);
  float ToScreenSpace(float a_Size);
  void DrawOutline(vertex_buffer_t* a_Buffer);

 private:
  texture_atlas_t* m_Atlas;
  vertex_buffer_t* m_Buffer;
  texture_font_t* m_Font;
  std::map<int, texture_font_t*> m_FontsBySize;
  GlCanvas* m_Canvas;
  GLuint m_Shader;
  mat4 m_Model;
  mat4 m_View;
  mat4 m_Proj;
  vec2 m_Pen;
  bool m_Initialized;
  bool m_DrawOutline;
};

//-----------------------------------------------------------------------------
inline vec4 ColorToVec4(const Color& a_Col) {
  const float coeff = 1.f / 255.f;
  vec4 color;
  color.r = a_Col[0] * coeff;
  color.g = a_Col[1] * coeff;
  color.b = a_Col[2] * coeff;
  color.a = a_Col[3] * coeff;
  return color;
}
