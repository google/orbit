//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

// ImGui GLFW binding with OpenGL
// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#include <map>
#include <string>

#include "OrbitType.h"
#include "ProcessUtils.h"
#include "imgui.h"

class GlCanvas;

IMGUI_API bool Orbit_ImGui_Init();
IMGUI_API void Orbit_ImGui_Shutdown();
IMGUI_API void Orbit_ImGui_NewFrame(GlCanvas* a_Canvas);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void Orbit_ImGui_InvalidateDeviceObjects();
IMGUI_API bool Orbit_ImGui_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during
// initialization) Provided here if you want to chain callbacks. You can also
// handle inputs yourself and use those as a reference.
IMGUI_API void Orbit_ImGui_MouseButtonCallback(GlCanvas* a_Canvas, int button,
                                               bool down);
IMGUI_API void Orbit_ImGui_ScrollCallback(GlCanvas* a_Canvas, int scroll);
IMGUI_API void Orbit_ImGui_KeyCallback(GlCanvas* a_Canvas, int key, bool down);
IMGUI_API void Orbit_ImGui_CharCallback(GlCanvas* a_Canvas, unsigned int c);

void SetupImGuiStyle(bool bStyleDark_, float alpha_);

extern ImFont* GOrbitImguiFont;

struct ScopeImguiContext {
  explicit ScopeImguiContext(ImGuiContext* a_State) : m_ImGuiContext(nullptr) {
    ImGuiContext* state = (ImGuiContext*)ImGui::GetCurrentContext();
    if (state != a_State) {
      m_ImGuiContext = state;
      ImGui::SetCurrentContext(a_State);
    }
  }
  ~ScopeImguiContext() {
    if (m_ImGuiContext) {
      ImGui::SetCurrentContext(m_ImGuiContext);
    }
  }

  ImGuiContext* m_ImGuiContext;
};

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct DebugWindow {
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets;  // Index to lines offset
  bool ScrollToBottom;

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
  }

  void AddLog(const char* fmt, ...) {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n') LineOffsets.push_back(old_size);
    // ScrollToBottom = true;
  }

  void Draw(const char* title, bool* p_opened = nullptr) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin(title, p_opened);
    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    if (copy) ImGui::LogToClipboard();

    if (Filter.IsActive()) {
      const char* buf_begin = Buf.begin();
      const char* line = buf_begin;
      for (int line_no = 0; line != nullptr; line_no++) {
        const char* line_end = (line_no < LineOffsets.Size)
                                   ? buf_begin + LineOffsets[line_no]
                                   : nullptr;
        if (Filter.PassFilter(line, line_end))
          ImGui::TextUnformatted(line, line_end);
        line = line_end && line_end[1] ? line_end + 1 : nullptr;
      }
    } else {
      ImGui::TextUnformatted(Buf.begin());
    }

    if (ScrollToBottom) ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;

    ImGui::EndChild();
    ImGui::End();
  }
};

//-----------------------------------------------------------------------------
class WatchWindow {
 public:
  WatchWindow() {}
  void Draw(const char* title, bool* p_opened = nullptr);

 protected:
  ImGuiTextFilter m_Filter;
};

//-----------------------------------------------------------------------------
class LogWindow {
 public:
  LogWindow() {}
  ImGuiTextFilter Filter;
  bool ScrollToBottom = false;
  bool m_Open = false;

  void Draw(const char* title, const std::vector<std::string>& lines,
            bool* p_opened = nullptr) {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin(title, p_opened);

    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    if (copy) ImGui::LogToClipboard();

    if (Filter.IsActive()) {
      for (const std::string& line : lines) {
        if (Filter.PassFilter(line.c_str()))
          ImGui::TextUnformatted(line.c_str());
      }
    } else {
      for (const std::string& line : lines) {
        ImGui::TextUnformatted(line.c_str());
      }
    }

    if (ScrollToBottom) ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;
    ImGui::EndChild();
    ImGui::End();
  }
};

struct VizWindow {
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets;  // Index to lines offset
  bool ScrollToBottom;
  ImGuiWindowFlags WindowFlags;

  VizWindow() : ScrollToBottom(false), WindowFlags(0) {}

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
  }

  void AddLog(const char* fmt, ...) {
    int old_size = Buf.size();
    va_list args;
    va_start(args, fmt);
    Buf.appendfv(fmt, args);
    va_end(args);
    for (int new_size = Buf.size(); old_size < new_size; old_size++)
      if (Buf[old_size] == '\n') LineOffsets.push_back(old_size);
    // ScrollToBottom = true;
  }

  void FitCanvas() {
    WindowFlags |= ImGuiWindowFlags_NoTitleBar;
    // WindowFlags |= ImGuiWindowFlags_ShowBorders;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    WindowFlags |= ImGuiWindowFlags_NoMove;
    // WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;
    // WindowFlags |= ImGuiWindowFlags_MenuBar;
  }

  void Draw(const char* title, bool* p_opened = nullptr,
            ImVec2* a_Size = nullptr) {
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

    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    if (copy) ImGui::LogToClipboard();

    if (Filter.IsActive()) {
      const char* buf_begin = Buf.begin();
      const char* line = buf_begin;
      for (int line_no = 0; line != nullptr; line_no++) {
        const char* line_end = (line_no < LineOffsets.Size)
                                   ? buf_begin + LineOffsets[line_no]
                                   : nullptr;
        if (Filter.PassFilter(line, line_end))
          ImGui::TextUnformatted(line, line_end);
        line = line_end && line_end[1] ? line_end + 1 : nullptr;
      }
    } else {
      ImGui::TextUnformatted(Buf.begin());
    }

    if (ScrollToBottom) ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
  }
};

struct OutputWindow {
  ImGuiTextBuffer Buf;
  ImVector<int> LineOffsets;  // Index to lines offset
  ImGuiWindowFlags WindowFlags;

  OutputWindow() : WindowFlags(0) {}

  void Clear() {
    Buf.clear();
    LineOffsets.clear();
  }
  void AddLine(const std::string& a_String);
  void AddLog(const char* fmt, ...);
  void Draw(const char* title, bool* p_opened = nullptr,
            ImVec2* a_Size = nullptr);
};
