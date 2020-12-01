// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TextRenderer.h"

#include <freetype-gl/shader.h>
#include <freetype-gl/vertex-buffer.h>

#include "App.h"
#include "GlCanvas.h"
#include "GlUtils.h"
#include "Path.h"

typedef struct {
  float x, y, z;     // position
  float s, t;        // texture
  float r, g, b, a;  // color
} vertex_t;

bool TextRenderer::draw_outline_ = false;

TextRenderer::TextRenderer() : texture_atlas_(nullptr), canvas_(nullptr), initialized_(false) {}

TextRenderer::~TextRenderer() {
  for (const auto& pair : fonts_by_size_) {
    texture_font_delete(pair.second);
  }
  fonts_by_size_.clear();

  for (auto& [unused_layer, buffer] : vertex_buffers_by_layer_) {
    vertex_buffer_delete(buffer);
  }
  vertex_buffers_by_layer_.clear();

  if (texture_atlas_) {
    texture_atlas_delete(texture_atlas_);
  }
}

void TextRenderer::Init() {
  if (initialized_) return;

  int atlasSize = 2 * 1024;
  texture_atlas_ = texture_atlas_new(atlasSize, atlasSize, 1);

  const auto exe_dir = Path::GetExecutableDir();
  const auto fontFileName = exe_dir + "fonts/Vera.ttf";

  for (int i = 1; i <= 100; i += 1) {
    fonts_by_size_[i] = texture_font_new_from_file(texture_atlas_, i, fontFileName.c_str());
  }

  pen_.x = 0;
  pen_.y = 0;

  glGenTextures(1, &texture_atlas_->id);

  const auto vertShaderFileName = Path::JoinPath({exe_dir, "shaders", "v3f-t2f-c4f.vert"});
  const auto fragShaderFileName = Path::JoinPath({exe_dir, "shaders", "v3f-t2f-c4f.frag"});
  shader_ = shader_load(vertShaderFileName.c_str(), fragShaderFileName.c_str());

  mat4_set_identity(&projection_);
  mat4_set_identity(&model_);
  mat4_set_identity(&view_);

  initialized_ = true;
}

texture_font_t* TextRenderer::GetFont(uint32_t size) {
  CHECK(!fonts_by_size_.empty());
  if (!fonts_by_size_.count(size)) {
    auto iterator_next = fonts_by_size_.upper_bound(size);
    // If there isn't that font_size in the map, we will search for the next one or previous one
    if (iterator_next != fonts_by_size_.end()) {
      size = iterator_next->first;
    } else if (iterator_next != fonts_by_size_.begin()) {
      size = (--iterator_next)->first;
    }
  }
  return fonts_by_size_[size];
}

void TextRenderer::RenderLayer(Batcher*, float layer) {
  ORBIT_SCOPE_FUNCTION;
  if (!vertex_buffers_by_layer_.count(layer)) return;
  auto& buffer = vertex_buffers_by_layer_.at(layer);

  // Lazy init
  if (!initialized_) {
    Init();
  }

  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D, texture_atlas_->id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<GLsizei>(texture_atlas_->width),
               static_cast<GLsizei>(texture_atlas_->height), 0, GL_RED, GL_UNSIGNED_BYTE,
               texture_atlas_->data);

  // Get current projection matrix
  GLfloat matrix[16];
  glGetFloatv(GL_PROJECTION_MATRIX, matrix);
  mat4* projection = reinterpret_cast<mat4*>(&matrix[0]);
  projection_ = *projection;

  glUseProgram(shader_);
  {
    glUniform1i(glGetUniformLocation(shader_, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "model"), 1, 0, model_.data);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "view"), 1, 0, view_.data);
    glUniformMatrix4fv(glGetUniformLocation(shader_, "projection"), 1, 0, projection_.data);
    vertex_buffer_render(buffer, GL_TRIANGLES);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  glPopAttrib();
}

void TextRenderer::RenderDebug(Batcher* batcher) {
  if (!draw_outline_) return;
  for (auto& [unused_layer, buffer] : vertex_buffers_by_layer_) {
    DrawOutline(batcher, buffer);
  }
}

void TextRenderer::DrawOutline(Batcher* batcher, vertex_buffer_t* vertex_buffer) {
  if (vertex_buffer == nullptr) return;
  const Color color(255, 255, 255, 255);

  for (size_t i = 0; i < vertex_buffer->indices->size; i += 3) {
    GLuint i0 = *static_cast<const GLuint*>(vector_get(vertex_buffer->indices, i + 0));
    GLuint i1 = *static_cast<const GLuint*>(vector_get(vertex_buffer->indices, i + 1));
    GLuint i2 = *static_cast<const GLuint*>(vector_get(vertex_buffer->indices, i + 2));

    vertex_t v0 = *static_cast<const vertex_t*>(vector_get(vertex_buffer->vertices, i0));
    vertex_t v1 = *static_cast<const vertex_t*>(vector_get(vertex_buffer->vertices, i1));
    vertex_t v2 = *static_cast<const vertex_t*>(vector_get(vertex_buffer->vertices, i2));

    // TODO: This should be pickable??
    batcher->AddLine(Vec2(v0.x, v0.y), Vec2(v1.x, v1.y), GlCanvas::kZValueSlider, color);
    batcher->AddLine(Vec2(v1.x, v1.y), Vec2(v2.x, v2.y), GlCanvas::kZValueSlider, color);
    batcher->AddLine(Vec2(v2.x, v2.y), Vec2(v0.x, v0.y), GlCanvas::kZValueSlider, color);
  }
}

void TextRenderer::AddTextInternal(texture_font_t* font, const char* text, const vec4& color,
                                   vec2* pen, float max_size, float z, vec2* out_text_pos,
                                   vec2* out_text_size) {
  float r = color.red, g = color.green, b = color.blue, a = color.alpha;

  float max_width = max_size == -1.f ? FLT_MAX : ToScreenSpace(max_size);
  float str_width = 0.f;
  float min_x = FLT_MAX;
  float max_x = -FLT_MAX;
  float min_y = FLT_MAX;
  float max_y = -FLT_MAX;
  constexpr std::array<GLuint, 6> indices = {0, 1, 2, 0, 2, 3};
  vec2 initial_pen = *pen;

  for (size_t i = 0; i < strlen(text); ++i) {
    if (text[i] == '\n') {
      pen->x = initial_pen.x;
      pen->y -= font->height;
      continue;
    }

    if (!texture_font_find_glyph(font, text + i)) {
      texture_font_load_glyph(font, text + i);
    }

    texture_glyph_t* glyph = texture_font_get_glyph(font, text + i);
    if (glyph != nullptr) {
      float kerning = (i == 0) ? 0.0f : texture_glyph_get_kerning(glyph, text + i - 1);
      pen->x += kerning;

      float x0 = floorf(pen->x + glyph->offset_x);
      float y0 = floorf(pen->y + glyph->offset_y);
      float x1 = floorf(x0 + glyph->width);
      float y1 = floorf(y0 - glyph->height);

      float s0 = glyph->s0;
      float t0 = glyph->t0;
      float s1 = glyph->s1;
      float t1 = glyph->t1;

      vertex_t vertices[4] = {{x0, y0, z, s0, t0, r, g, b, a},
                              {x0, y1, z, s0, t1, r, g, b, a},
                              {x1, y1, z, s1, t1, r, g, b, a},
                              {x1, y0, z, s1, t0, r, g, b, a}};

      min_x = std::min(min_x, x0);
      max_x = std::max(max_x, x1);
      min_y = std::min(min_y, y1);
      max_y = std::max(max_y, y0);

      str_width = max_x - min_x;

      if (str_width > max_width) {
        break;
      }
      if (!vertex_buffers_by_layer_.count(z)) {
        vertex_buffers_by_layer_[z] = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
      }
      vertex_buffer_push_back(vertex_buffers_by_layer_.at(z), vertices, 4, indices.data(), 6);
      pen->x += glyph->advance_x;
    }
  }

  if (out_text_pos) {
    out_text_pos->x = min_x;
    out_text_pos->y = min_y;
  }

  if (out_text_size) {
    out_text_size->x = max_x - min_x;
    out_text_size->y = max_y - min_y;
  }
}

void TextRenderer::AddText(const char* text, float x, float y, float z, const Color& color,
                           uint32_t font_size, float max_size, bool right_justified,
                           Vec2* out_text_pos, Vec2* out_text_size) {
  if (!font_size) return;
  ToScreenSpace(x, y, pen_.x, pen_.y);

  if (right_justified) {
    max_size = FLT_MAX;
    int string_width = GetStringWidthScreenSpace(text, font_size);
    pen_.x -= string_width;
  }

  vec2 out_screen_pos;
  vec2 out_screen_size;
  AddTextInternal(GetFont(font_size), text, ColorToVec4(color), &pen_, max_size, z, &out_screen_pos,
                  &out_screen_size);
  if (out_text_pos) {
    float inv_y = canvas_->GetHeight() - out_screen_pos.y;
    (*out_text_pos) = canvas_->ScreenToWorld(Vec2(out_screen_pos.x, inv_y));
  }

  if (out_text_size) {
    (*out_text_size)[0] = canvas_->ScreenToWorldWidth(static_cast<int>(out_screen_size.x));
    (*out_text_size)[1] = canvas_->ScreenToWorldHeight(static_cast<int>(out_screen_size.y));
  }
}

float TextRenderer::AddTextTrailingCharsPrioritized(const char* text, float x, float y, float z,
                                                    const Color& color,
                                                    size_t trailing_chars_length,
                                                    uint32_t font_size, float max_size) {
  if (!initialized_) {
    Init();
  }

  float temp_pen_x = ToScreenSpace(x);
  float max_width = max_size == -1.f ? FLT_MAX : ToScreenSpace(max_size);
  float string_width = 0.f;
  int min_x = INT_MAX;
  int max_x = -INT_MAX;

  const size_t text_length = strlen(text);
  texture_font_t* font = GetFont(font_size);
  size_t i;
  for (i = 0; i < text_length; ++i) {
    if (!texture_font_find_glyph(font, text + i)) {
      texture_font_load_glyph(font, text + i);
    }

    texture_glyph_t* glyph = texture_font_get_glyph(font, text + i);
    if (glyph != NULL) {
      float kerning = 0.0f;
      if (i > 0) {
        kerning = texture_glyph_get_kerning(glyph, text + i - 1);
      }
      temp_pen_x += kerning;
      int x0 = static_cast<int>(temp_pen_x + glyph->offset_x);
      int x1 = static_cast<int>(x0 + glyph->width);

      min_x = std::min(min_x, x0);
      max_x = std::max(max_x, x1);
      string_width = float(max_x - min_x);

      if (string_width > max_width) {
        break;
      }

      temp_pen_x += glyph->advance_x;
    }
  }

  // TODO: Technically, we'd want the size of "... <TIME>" + remaining
  // characters

  auto fitting_chars_count = i;

  static const char* ELLIPSIS_TEXT = "... ";
  static const size_t ELLIPSIS_TEXT_LEN = strlen(ELLIPSIS_TEXT);
  static const size_t LEADING_CHARS_COUNT = 1;
  static const size_t ELLIPSIS_BUFFER_SIZE = ELLIPSIS_TEXT_LEN + LEADING_CHARS_COUNT;

  bool use_ellipsis_text = (fitting_chars_count < text_length) &&
                           (fitting_chars_count > (trailing_chars_length + ELLIPSIS_BUFFER_SIZE));

  if (!use_ellipsis_text) {
    AddText(text, x, y, z, color, font_size, max_size);
    return GetStringWidth(text, font_size);
  } else {
    auto leading_char_count = fitting_chars_count - (trailing_chars_length + ELLIPSIS_TEXT_LEN);

    std::string modified_text(text, leading_char_count);
    modified_text.append(ELLIPSIS_TEXT);

    auto timePosition = text_length - trailing_chars_length;
    modified_text.append(&text[timePosition], trailing_chars_length);

    AddText(modified_text.c_str(), x, y, z, color, font_size, max_size);
    return GetStringWidth(modified_text.c_str(), font_size);
  }
}

float TextRenderer::GetStringWidth(const char* text, uint32_t font_size) {
  return canvas_->ScreenToWorldWidth(GetStringWidthScreenSpace(text, font_size));
}

float TextRenderer::GetStringHeight(const char* text, uint32_t font_size) {
  return canvas_->ScreenToWorldHeight(GetStringHeightScreenSpace(text, font_size));
}

int TextRenderer::GetStringWidthScreenSpace(const char* text, uint32_t font_size) {
  float string_width = 0;

  std::size_t len = strlen(text);
  for (std::size_t i = 0; i < len; ++i) {
    texture_glyph_t* glyph = texture_font_get_glyph(GetFont(font_size), text + i);
    if (glyph != NULL) {
      float kerning = 0.0f;
      if (i > 0) {
        kerning = texture_glyph_get_kerning(glyph, text + i - 1);
      }

      string_width += kerning;
      string_width += glyph->advance_x;
    }

    // Only return width of first line.
    if (text[i] == '\n') break;
  }

  return static_cast<int>(ceil(string_width));
}

int TextRenderer::GetStringHeightScreenSpace(const char* text, uint32_t font_size) {
  int max_height = 0.f;
  texture_font_t* font = GetFont(font_size);
  for (std::size_t i = 0; i < strlen(text); ++i) {
    if (!texture_font_find_glyph(font, text + i)) {
      texture_font_load_glyph(font, text + i);
    }

    texture_glyph_t* glyph = texture_font_get_glyph(font, text + i);
    if (glyph != nullptr) {
      max_height = std::max(max_height, glyph->offset_y);
    }

    // Only return height of first line.
    if (text[i] == '\n') break;
  }
  return max_height;
}

std::vector<float> TextRenderer::GetLayers() const {
  std::vector<float> layers;
  for (auto& [layer, unused_buffer] : vertex_buffers_by_layer_) {
    layers.push_back(layer);
  }
  return layers;
};

void TextRenderer::ToScreenSpace(float x, float y, float& o_x, float& o_y) {
  float world_width = canvas_->GetWorldWidth();
  float world_height = canvas_->GetWorldHeight();
  float world_top_left_x = canvas_->GetWorldTopLeftX();
  float world_min_left_y = canvas_->GetWorldTopLeftY() - world_height;

  o_x = ((x - world_top_left_x) / world_width) * canvas_->GetWidth();
  o_y = ((y - world_min_left_y) / world_height) * canvas_->GetHeight();
}

float TextRenderer::ToScreenSpace(float width) {
  return (width / canvas_->GetWorldWidth()) * canvas_->GetWidth();
}

void TextRenderer::Clear() {
  pen_.x = 0.f;
  pen_.y = 0.f;
  for (auto& [unused_layer, buffer] : vertex_buffers_by_layer_) {
    vertex_buffer_clear(buffer);
  }
}
