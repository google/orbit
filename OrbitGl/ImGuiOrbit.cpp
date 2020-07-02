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

void Orbit_ImGui_InvalidateDeviceObjects() {
  if (g_FontTexture) {
    glDeleteTextures(1, &g_FontTexture);
    ImGui::GetIO().Fonts->TexID = 0;
    g_FontTexture = 0;
  }
}

bool Orbit_ImGui_CreateDeviceObjects() {
  // Build texture atlas
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &g_FontTexture);
  glBindTexture(GL_TEXTURE_2D, g_FontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
               GL_UNSIGNED_BYTE, pixels);

  glGenTextures(1, &GTextureInjected);
  glBindTexture(GL_TEXTURE_2D, GTextureInjected);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               inject_image.pixel_data);

  glGenTextures(1, &GTextureTimer);
  glBindTexture(GL_TEXTURE_2D, GTextureTimer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
               inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               timer_image.pixel_data);

  glGenTextures(1, &GTextureHelp);
  glBindTexture(GL_TEXTURE_2D, GTextureHelp);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, help_image.width, help_image.height,
               0, GL_RGBA, GL_UNSIGNED_BYTE, help_image.pixel_data);

  glGenTextures(1, &GTextureRecord);
  glBindTexture(GL_TEXTURE_2D, GTextureRecord);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, record_image.width,
               record_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               record_image.pixel_data);

  // Restore state
  glBindTexture(GL_TEXTURE_2D, last_texture);

  return true;
}

// Simple helper function to load an image into a OpenGL texture with common
// settings
bool LoadTextureFromFile(const char* filename, uint32_t* out_texture,
                         int* out_width, int* out_height) {
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
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_data);
  stbi_image_free(image_data);

  *out_texture = image_texture;
  *out_width = image_width;
  *out_height = image_height;

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

// This is the main rendering function that you have to implement and provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure) If text
// or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by
// (0.5f,0.5f) or (0.375f,0.375f)
void Orbit_ImGui_RenderDrawLists(ImDrawData* draw_data) {
  // We are using the OpenGL fixed pipeline to make the example code simpler to
  // read! Setup render state: alpha-blending enabled, no face culling, no depth
  // testing, scissor enabled, vertex/texcoord/color pointers.
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  GLint last_viewport[4];
  glGetIntegerv(GL_VIEWPORT, last_viewport);
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  // glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glEnable(GL_TEXTURE_2D);
  glUseProgram(
      0);  // You may want this if using this code in an OpenGL 3+ context

  // Handle cases of screen coordinates != from framebuffer coordinates (e.g.
  // retina displays)
  ImGuiIO& io = ImGui::GetIO();
  int fb_width =
      static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height =
      static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, static_cast<GLsizei>(fb_width),
             static_cast<GLsizei>(fb_height));
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // Render command lists
  for (int n = 0; n < draw_data->CmdListsCount; n++) {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const uint8_t* vtx_buffer =
        reinterpret_cast<const uint8_t*>(&cmd_list->VtxBuffer.front());
    const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
    glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                    vtx_buffer + offsetof(ImDrawVert, pos));
    glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert),
                      vtx_buffer + offsetof(ImDrawVert, uv));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert),
                   vtx_buffer + offsetof(ImDrawVert, col));

    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      } else {
        glBindTexture(
            GL_TEXTURE_2D,
            static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));
        glScissor(static_cast<int>(pcmd->ClipRect.x),
                  static_cast<int>(fb_height - pcmd->ClipRect.w),
                  static_cast<int>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  static_cast<int>(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(
            GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount),
            sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
            idx_buffer);
      }
      idx_buffer += pcmd->ElemCount;
    }
  }

  // Restore modified state
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPopAttrib();
  glViewport(last_viewport[0], last_viewport[1],
             static_cast<GLsizei>(last_viewport[2]),
             static_cast<GLsizei>(last_viewport[3]));
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

  static volatile bool doSsetTexture = true;
  if (doSsetTexture) {
    // SCOPE_TIMER_LOG( "glTexImage2D" );
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
                 GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, GTextureInjected);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, inject_image.width,
                 inject_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 inject_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureTimer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, timer_image.width,
                 timer_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 timer_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureHelp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, help_image.width, help_image.height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, help_image.pixel_data);

    glBindTexture(GL_TEXTURE_2D, GTextureRecord);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, record_image.width,
                 record_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 record_image.pixel_data);
  }

  // Store our identifier
  io.Fonts->TexID =
      reinterpret_cast<void*>(static_cast<intptr_t>(g_FontTexture));

  // Setup display size (every frame to accommodate for window resizing)
  int w = a_Canvas->getWidth();
  int h = a_Canvas->getHeight();
  // int display_w;
  // int display_h;
  io.DisplaySize = ImVec2(w, h);
  // io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h
  // / h);

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
