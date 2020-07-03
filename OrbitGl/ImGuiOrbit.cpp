// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ImGui GLFW binding with OpenGL
// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include "ImGuiOrbit.h"

#include <imgui.h>

#include "Capture.h"
#include "Core.h"
#include "GlCanvas.h"
#include "Images.h"
#include "OpenGl.h"
#include "Params.h"
#include "Pdb.h"
#include "OrbitBase/Logging.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


ImFont* GOrbitImguiFont;

namespace {

// Data
static bool g_MousePressed[3] = {false, false, false};
static float g_MouseWheel = 0.0f;
static GLuint g_FontTexture = 0;

GLuint GTextureInjected = 0;
GLuint GTextureTimer = 0;
GLuint GTextureHelp = 0;
GLuint GTextureRecord = 0;

static const char g_GlslVersionString[32] = "#version 100\n";
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;
static GLuint g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int g_AttribLocationPosition = 0, g_AttribLocationUV = 0,
           g_AttribLocationColor = 0;

void Orbit_ImGui_InvalidateDeviceObjects() {
	if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
	if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
	g_VboHandle = g_ElementsHandle = 0;

	if (g_ShaderHandle && g_VertHandle)
		glDetachShader(g_ShaderHandle, g_VertHandle);
	if (g_VertHandle) glDeleteShader(g_VertHandle);
	g_VertHandle = 0;

	if (g_ShaderHandle && g_FragHandle)
		glDetachShader(g_ShaderHandle, g_FragHandle);
	if (g_FragHandle) glDeleteShader(g_FragHandle);
	g_FragHandle = 0;

	if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
	g_ShaderHandle = 0;

	if (g_FontTexture) {
		glDeleteTextures(1, &g_FontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

// If you get an error please report on github. You may try different GL context
// version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc) {
  GLint status = 0, log_length = 0;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
  glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 1) {
    ImVector<char> buf;
    buf.resize((int)(log_length + 1));
    glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
    LOG("Log from shader compilation: %s", buf.begin());
  }
  if ((GLboolean)status == GL_FALSE) {
    FATAL("Orbit_ImGui_CreateDeviceObjects: failed to compile %s!", desc);
  }
  return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context
// version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc) {
  GLint status = 0, log_length = 0;
  glGetProgramiv(handle, GL_LINK_STATUS, &status);
  glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length > 1) {
    ImVector<char> buf;
    buf.resize((int)(log_length + 1));
    glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
    LOG("Log from shader program linking: %s", buf.begin());
  }
  if ((GLboolean)status == GL_FALSE) {
    FATAL("Orbit_ImGui_CreateDeviceObjects: failed to link %s!", desc);
  }
  return (GLboolean)status == GL_TRUE;
}

bool Orbit_ImGui_CreateTextures() {
  // Build texture atlas
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);  

  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

  glGenTextures(1, &g_FontTexture);
  glBindTexture(GL_TEXTURE_2D, g_FontTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, pixels);

  glGenTextures(1, &GTextureInjected);
  glBindTexture(GL_TEXTURE_2D, GTextureInjected);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               inject_image.pixel_data);

  glGenTextures(1, &GTextureTimer);
  glBindTexture(GL_TEXTURE_2D, GTextureTimer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               timer_image.pixel_data);

  glGenTextures(1, &GTextureHelp);
  glBindTexture(GL_TEXTURE_2D, GTextureHelp);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, help_image.width, help_image.height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, help_image.pixel_data);

  glGenTextures(1, &GTextureRecord);
  glBindTexture(GL_TEXTURE_2D, GTextureRecord);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, record_image.width,
               record_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               record_image.pixel_data);

  // Store our identifier
  io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);

  return true;
}

bool Orbit_ImGui_CreateDeviceObjects() {
  // Backup GL state
  GLint last_texture, last_array_buffer;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
  GLint last_vertex_array;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

  const GLchar* vertex_shader =
      "uniform mat4 ProjMtx;\n"
      "attribute vec2 Position;\n"
      "attribute vec2 UV;\n"
      "attribute vec4 Color;\n"
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Color;\n"
      "void main()\n"
      "{\n"
      "    Frag_UV = UV;\n"
      "    Frag_Color = Color;\n"
      "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
      "}\n";
  const GLchar* fragment_shader =
      "#ifdef GL_ES\n"
      "    precision mediump float;\n"
      "#endif\n"
      "uniform sampler2D Texture;\n"
      "varying vec2 Frag_UV;\n"
      "varying vec4 Frag_Color;\n"
      "void main()\n"
      "{\n"
      "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
      "}\n";
  // Create shaders
  const GLchar* vertex_shader_with_version[2] = {g_GlslVersionString,
                                                 vertex_shader};
  g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
  glCompileShader(g_VertHandle);
  CheckShader(g_VertHandle, "vertex shader");

  const GLchar* fragment_shader_with_version[2] = {g_GlslVersionString,
                                                   fragment_shader};
  g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
  glCompileShader(g_FragHandle);
  CheckShader(g_FragHandle, "fragment shader");

  g_ShaderHandle = glCreateProgram();
  glAttachShader(g_ShaderHandle, g_VertHandle);
  glAttachShader(g_ShaderHandle, g_FragHandle);
  glLinkProgram(g_ShaderHandle);
  CheckProgram(g_ShaderHandle, "shader program");

  g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
  g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
  g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
  g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
  g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

  // Create buffers
  glGenBuffers(1, &g_VboHandle);
  glGenBuffers(1, &g_ElementsHandle);

  Orbit_ImGui_CreateTextures();
  
   // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
  glBindVertexArray(last_vertex_array);
#endif

  return true;
}

// Simple helper function to load an image into a OpenGL texture with common
// settings
bool LoadTextureFromFile(const char* filename, uint32_t* out_texture,
                         int* out_width, int* out_height) {
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char* image_data =
      stbi_load(filename, &image_width, &image_height, nullptr, 4);
  if (image_data == nullptr) return false;

  // Create an OpenGL texture identifier
  GLuint image_texture;
  glGenTextures(1, &image_texture);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
#ifdef GL_UNPACK_ROW_LENGTH
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

  glBindTexture(GL_TEXTURE_2D, last_texture);

  return true;
}

// From Avoid/Doug Binks
void SetupImGuiStyle(bool bStyleDark_, float alpha_) {
  ImGuiStyle& style = ImGui::GetStyle();

  // light style from Pacome Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
  style.Alpha = 1.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  // style.Colors[ImGuiCol_PopupBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] =
      ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  // style.Colors[ImGuiCol_ComboBg] = ImVec4( 0.86f, 0.86f, 0.86f, 0.99f );
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  // style.Colors[ImGuiCol_CloseButton] = ImVec4( 0.59f, 0.59f, 0.59f, 0.50f );
  // style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4( 0.98f, 0.39f,
  // 0.36f, 1.00f ); style.Colors[ImGuiCol_CloseButtonActive] = ImVec4( 0.98f,
  // 0.39f, 0.36f, 1.00f );
  style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] =
      ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_ModalWindowDarkening] =
      ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

  if (bStyleDark_) {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4& col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

      if (S < 0.1f) {
        V = 1.0f - V;
      }
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f) {
        col.w *= alpha_;
      }
    }
  } else {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4& col = style.Colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}

void Orbit_ImGui_RenderDrawLists(ImDrawData* draw_data) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  // (screen coordinates != framebuffer coordinates)
  int fb_width =
      (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
  int fb_height =
      (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
  if (fb_width <= 0 || fb_height <= 0) return;

    // Backup GL state
  GLenum last_active_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
  glActiveTexture(GL_TEXTURE0);
  GLint last_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
  GLint last_sampler;
  glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
  GLint last_array_buffer;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
  GLint last_vertex_array;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif
#ifdef GL_POLYGON_MODE
  GLint last_polygon_mode[2];
  glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4];
  glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLenum last_blend_src_rgb;
  glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
  GLenum last_blend_dst_rgb;
  glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
  GLenum last_blend_src_alpha;
  glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
  GLenum last_blend_dst_alpha;
  glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
  GLenum last_blend_equation_rgb;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
  GLenum last_blend_equation_alpha;
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
  bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
  GLenum last_clip_origin = 0;
  glGetIntegerv(GL_CLIP_ORIGIN,
                (GLint*)&last_clip_origin);  // Support for GL 4.5's
                                             // glClipControl(GL_UPPER_LEFT)
  if (last_clip_origin == GL_UPPER_LEFT) clip_origin_lower_left = false;
#endif

  // Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, polygon fill
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

  // Setup viewport, orthographic projection matrix
  // Our visible imgui space lies from draw_data->DisplayPos (top left) to
  // draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is
  // typically (0,0) for single viewport apps.
  glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
  float L = draw_data->DisplayPos.x;
  float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
  float T = draw_data->DisplayPos.y;
  float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
  const float ortho_projection[4][4] = {
      {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
      {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
      {0.0f, 0.0f, -1.0f, 0.0f},
      {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
  };
  glUseProgram(g_ShaderHandle);
  glUniform1i(g_AttribLocationTex, 0);
  glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE,
                     &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
  glBindSampler(0, 0);  // We use combined texture/sampler state. Applications
                        // using GL 3.3 may set that otherwise.
#endif

#ifndef IMGUI_IMPL_OPENGL_ES2
  // Recreate the VAO every time
  // (This is to easily allow multiple GL contexts. VAO are not shared among GL
  // contexts, and we don't track creation/deletion of windows so we don't have
  // an obvious key to use to cache them.)
  GLuint vao_handle = 0;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);
#endif
  glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
  glEnableVertexAttribArray(g_AttribLocationPosition);
  glEnableVertexAttribArray(g_AttribLocationUV);
  glEnableVertexAttribArray(g_AttribLocationColor);
  glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
  glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
  glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                        sizeof(ImDrawVert),
                        (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

  // Will project scissor/clipping rectangles into framebuffer space
  ImVec2 clip_off =
      draw_data->DisplayPos;  // (0,0) unless using multi-viewports
  ImVec2 clip_scale =
      draw_data->FramebufferScale;  // (1,1) unless using retina display which
                                    // are often (2,2)

    // Render command lists
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    size_t idx_buffer_offset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                 (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                 (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        // User callback (registered via ImDrawList::AddCallback)
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        // Project scissor/clipping rectangles into framebuffer space
        ImVec4 clip_rect;
        clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
        clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
        clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
        clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

        if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
            clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
          // Apply scissor/clipping rectangle
          if (clip_origin_lower_left)
            glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w),
                      (int)(clip_rect.z - clip_rect.x),
                      (int)(clip_rect.w - clip_rect.y));
          else
            glScissor(
                (int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z,
                (int)clip_rect
                    .w);  // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)

          // Bind texture, Draw
          glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
          glDrawElements(
              GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
              sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
              (void*)idx_buffer_offset);
        }
      }
      idx_buffer_offset += pcmd->ElemCount * sizeof(ImDrawIdx);
    }
  }
#ifndef IMGUI_IMPL_OPENGL_ES2
  glDeleteVertexArrays(1, &vao_handle);
#endif
  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
  glBindSampler(0, last_sampler);
#endif
  glActiveTexture(last_active_texture);
#ifndef IMGUI_IMPL_OPENGL_ES2
  glBindVertexArray(last_vertex_array);
#endif
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                      last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (last_enable_cull_face)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (last_enable_depth_test)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
  glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2],
             (GLsizei)last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1],
            (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

ImFont* AddOrbitFont(float pixel_size) {
  const auto exe_path = Path::GetExecutablePath();
  const auto font_file_name = exe_path + "fonts/Vera.ttf";
  return ImGui::GetIO().Fonts->AddFontFromFileTTF(font_file_name.c_str(),
                                                  pixel_size);
}

}  // namespace


void Orbit_ImGui_MouseButtonCallback(GlCanvas* a_GlCanvas, int button,
                                     bool down) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  if (button >= 0 && button < 3) {
    g_MousePressed[button] = down;
  }
}

void Orbit_ImGui_ScrollCallback(GlCanvas* a_GlCanvas, int scroll) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  g_MouseWheel += scroll;  // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void Orbit_ImGui_KeyCallback(GlCanvas* a_Canvas, int key, bool down) {
  // Convert "enter" into "return"
  if (key == 5) key = 4;

  ScopeImguiContext state(a_Canvas->GetImGuiContext());
  if (key >= 0 && key < 512) {
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = down;
  }
}

void Orbit_ImGui_CharCallback(GlCanvas* a_GlCanvas, unsigned int c) {
  ScopeImguiContext state(a_GlCanvas->GetImGuiContext());
  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) io.AddInputCharacter(static_cast<uint16_t>(c));
}

uint32_t LoadTextureFromFile(const char* file_name) {
  uint32_t texture_id = 0;
  int image_width = 0;
  int image_height = 0;
  if (!LoadTextureFromFile(file_name, &texture_id, &image_width,
                           &image_height)) {
    LOG("ERROR, could not load texture %s", file_name);
  }

  return texture_id;
}

bool Orbit_ImGui_Init() {
  ImGuiIO& io = ImGui::GetIO();

  // http://doc.qt.io/qt-4.8/qt.html#Key-enum

  io.KeyMap[ImGuiKey_Tab] =
      0x00000001;  // Keyboard mapping. ImGui will use those indices to peek
                   // into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_LeftArrow] = 0x00000012;
  io.KeyMap[ImGuiKey_RightArrow] = 0x00000014;
  io.KeyMap[ImGuiKey_UpArrow] = 0x00000013;
  io.KeyMap[ImGuiKey_DownArrow] = 0x00000015;
  io.KeyMap[ImGuiKey_PageUp] = 0x00000016;
  io.KeyMap[ImGuiKey_PageDown] = 0x00000017;
  io.KeyMap[ImGuiKey_Home] = 0x00000010;
  io.KeyMap[ImGuiKey_End] = 0x00000011;
  io.KeyMap[ImGuiKey_Delete] = 0x00000007;
  io.KeyMap[ImGuiKey_Backspace] = 0x00000003;
  io.KeyMap[ImGuiKey_Enter] = 0x00000004;
  io.KeyMap[ImGuiKey_Escape] = 0x00000000;
  io.KeyMap[ImGuiKey_A] = 65;
  io.KeyMap[ImGuiKey_C] = 67;
  io.KeyMap[ImGuiKey_V] = 86;
  io.KeyMap[ImGuiKey_X] = 88;
  io.KeyMap[ImGuiKey_Y] = 89;
  io.KeyMap[ImGuiKey_Z] = 90;

  io.RenderDrawListsFn =
      Orbit_ImGui_RenderDrawLists;  // Alternatively you can set this to NULL
                                    // and call ImGui::GetDrawData() after
                                    // ImGui::Render() to get the same
                                    // ImDrawData pointer.

  SetupImGuiStyle(true, 1.f);

  const float kImguiFontOffset = 10.f;
  GOrbitImguiFont = AddOrbitFont(GParams.font_size + kImguiFontOffset);
  ImGui::GetIO().Fonts->Build();

  return true;
}

void Orbit_ImGui_Shutdown() { Orbit_ImGui_InvalidateDeviceObjects(); }

void Orbit_ImGui_NewFrame(GlCanvas* a_Canvas) {
  if (!g_FontTexture) Orbit_ImGui_CreateDeviceObjects();

  ImGuiIO& io = ImGui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  const int w = a_Canvas->getWidth();
  const int h = a_Canvas->getHeight();
  io.DisplaySize = ImVec2(w, h);

  // Setup time step
  // TODO: Does this make sense?
  if (a_Canvas->GetDeltaTimeSeconds() > 0) {
    io.DeltaTime = a_Canvas->GetDeltaTimeSeconds();
  }

  // Setup inputs
  io.MousePos = ImVec2(
      a_Canvas->GetMousePosX(),
      a_Canvas
          ->GetMousePosY());  // Mouse position in screen coordinates (set to
                              // -1,-1 if no mouse / on another screen, etc.)

  for (int i = 0; i < 3; i++) {
    io.MouseDown[i] = g_MousePressed[i];
    // g_MousePressed[i] = false;
  }

  io.MouseWheel = g_MouseWheel;
  g_MouseWheel = 0.0f;

  // Start the frame
  ImGui::NewFrame();
}

void OutputWindow::AddLine(const std::string& a_String) {
  int old_size = Buf.size();
  Buf.append(a_String.c_str());
  Buf.append("\n");
  for (int new_size = Buf.size(); old_size < new_size; old_size++)
    if (Buf[old_size] == '\n') LineOffsets.push_back(old_size);
}

void OutputWindow::Draw(const char* title, bool* p_opened, ImVec2* a_Size) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  if (a_Size) {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImVec2 CanvasSize = *a_Size;
    CanvasSize.x -= 20;
    CanvasSize.y -= 20;
    ImGui::SetNextWindowSize(CanvasSize, ImGuiCond_Always);
    ImGui::Begin(title, p_opened, CanvasSize, 1.f, WindowFlags);
  } else {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImVec2 size(400, 400);
    ImGui::Begin(title, p_opened, size, 1.f, WindowFlags);
  }

  ImGui::TextUnformatted(Buf.begin());

  static bool checked = true;
  ImGui::Checkbox("blah", &checked);

  static int i1 = 0;
  if (ImGui::SliderInt("slider int", &i1, 10, 100)) {
    GCurrentTimeGraph->SetFontSize(i1);
  }

  ImGui::End();
  ImGui::PopStyleVar();
}
