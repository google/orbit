// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureWindow.h"

#include "../OrbitPlugin/OrbitSDK.h"
#include "App.h"
#include "Capture.h"
#include "EventTracer.h"
#include "GlUtils.h"
#include "PluginManager.h"
#include "Serialization.h"
#include "Systrace.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TimerManager.h"
#include "absl/flags/flag.h"
#include "absl/strings/str_format.h"

#ifdef _WIN32
#include "SymbolUtils.h"
#else
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "LinuxUtils.h"
#endif

ABSL_DECLARE_FLAG(bool, enable_stale_features);

//-----------------------------------------------------------------------------
CaptureWindow::CaptureWindow() {
  GCurrentTimeGraph = &time_graph_;
  time_graph_.SetTextRenderer(&m_TextRenderer);
  time_graph_.SetPickingManager(&m_PickingManager);
  time_graph_.SetCanvas(this);
  m_DrawUI = false;
  m_DrawHelp = true;
  m_DrawFilter = false;
  m_DrawMemTracker = false;
  m_FirstHelpDraw = true;
  m_DrawStats = false;
  m_Picking = false;
  m_WorldTopLeftX = 0;
  m_WorldTopLeftY = 0;
  m_WorldMaxY = 0;
  m_ProcessX = 0;

  GTimerManager->m_TimerAddedCallbacks.emplace_back(
      [this](Timer& a_Timer) { this->OnTimerAdded(a_Timer); });
  GTimerManager->m_ContextSwitchAddedCallback =
      [this](const ContextSwitch& a_CS) { this->OnContextSwitchAdded(a_CS); };

  m_HoverDelayMs = 300;
  m_CanHover = false;
  m_IsHovering = false;
  ResetHoverTimer();

  m_Slider.SetCanvas(this);
  m_Slider.SetDragCallback([&](float a_Ratio) { this->OnDrag(a_Ratio); });

  m_VerticalSlider.SetCanvas(this);
  m_VerticalSlider.SetVertical();
  m_VerticalSlider.SetDragCallback(
      [&](float a_Ratio) { this->OnVerticalDrag(a_Ratio); });

  GOrbitApp->RegisterCaptureWindow(this);
}

//-----------------------------------------------------------------------------
CaptureWindow::~CaptureWindow() {
  if (GCurrentTimeGraph == &time_graph_) GCurrentTimeGraph = nullptr;
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnTimer() { GlCanvas::OnTimer(); }

//-----------------------------------------------------------------------------
void CaptureWindow::ZoomAll() {
  time_graph_.ZoomAll();
  m_WorldTopLeftY = m_WorldMaxY;
  ResetHoverTimer();
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::UpdateWheelMomentum(float a_DeltaTime) {
  GlCanvas::UpdateWheelMomentum(a_DeltaTime);

  bool zoomWidth = true;  // TODO: !wxGetKeyState(WXK_CONTROL);
  if (zoomWidth && m_WheelMomentum != 0.f) {
    time_graph_.ZoomTime(m_WheelMomentum, m_MouseRatio);
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::MouseMoved(int a_X, int a_Y, bool a_Left, bool /*a_Right*/,
                               bool /*a_Middle*/) {
  int mousex = a_X;
  int mousey = a_Y;

  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);

  m_MouseX = worldx;
  m_MouseY = worldy;
  m_MousePosX = mousex;
  m_MousePosY = mousey;

  // Pan
  if (a_Left && !m_ImguiActive && !m_PickingManager.IsDragging() &&
      !Capture::IsCapturing()) {
    float worldMin;
    float worldMax;

    time_graph_.GetWorldMinMax(worldMin, worldMax);

    m_WorldTopLeftX =
        m_WorldClickX - static_cast<float>(mousex) / getWidth() * m_WorldWidth;
    m_WorldTopLeftY = m_WorldClickY +
                      static_cast<float>(mousey) / getHeight() * m_WorldHeight;

    m_WorldTopLeftX = clamp(m_WorldTopLeftX, worldMin, worldMax - m_WorldWidth);
    m_WorldTopLeftY =
        clamp(m_WorldTopLeftY,
              m_WorldHeight - time_graph_.GetThreadTotalHeight(), m_WorldMaxY);
    UpdateSceneBox();

    time_graph_.PanTime(m_ScreenClickX, a_X, getWidth(),
                        static_cast<double>(m_RefTimeClick));
    UpdateVerticalSlider();
    NeedsUpdate();
  }

  if (m_IsSelecting) {
    m_SelectStop = Vec2(worldx, worldy);
    m_TimeStop = time_graph_.GetTickFromWorld(worldx);
  }

  if (a_Left) {
    m_PickingManager.Drag(a_X, a_Y);
  }

  ResetHoverTimer();

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftDown(int a_X, int a_Y) {
  // Store world clicked pos for panning
  ScreenToWorld(a_X, a_Y, m_WorldClickX, m_WorldClickY);
  m_ScreenClickX = a_X;
  m_ScreenClickY = a_Y;
  m_RefTimeClick = static_cast<TickType>(
      time_graph_.GetTime(static_cast<double>(a_X) / getWidth()));

  m_IsSelecting = false;

  Orbit_ImGui_MouseButtonCallback(this, 0, true);

  m_Picking = true;
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftUp() {
  GlCanvas::LeftUp();
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::LeftDoubleClick() {
  GlCanvas::LeftDoubleClick();
  m_DoubleClicking = true;
  m_Picking = true;
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick() {
  m_Picking = true;
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick(int a_X, int a_Y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::vector<uint8_t> pixels(1 * 1 * 4);
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);

  PickingID pickId = PickingID::Get(*reinterpret_cast<uint32_t*>(&pixels[0]));

  Capture::GSelectedTextBox = nullptr;
  Capture::GSelectedThreadId = 0;

  Pick(pickId, a_X, a_Y);

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pick(PickingID a_PickingID, int a_X, int a_Y) {
  uint32_t type = a_PickingID.m_Type;
  uint32_t id = a_PickingID.m_Id;

  switch (type) {
    case PickingID::BOX: {
      void** textBoxPtr =
          time_graph_.GetBatcher().GetBoxBuffer().m_UserData.SlowAt(id);
      if (textBoxPtr) {
        TextBox* textBox = static_cast<TextBox*>(*textBoxPtr);
        SelectTextBox(textBox);
      }
      break;
    }
    case PickingID::LINE: {
      void** textBoxPtr =
          time_graph_.GetBatcher().GetLineBuffer().m_UserData.SlowAt(id);
      if (textBoxPtr) {
        TextBox* textBox = static_cast<TextBox*>(*textBoxPtr);
        SelectTextBox(textBox);
      }
      break;
    }
    case PickingID::PICKABLE:
      m_PickingManager.Pick(a_PickingID.m_Id, a_X, a_Y);
      break;
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::SelectTextBox(class TextBox* a_TextBox) {
  if (a_TextBox == nullptr) return;
  Capture::GSelectedTextBox = a_TextBox;
  Capture::GSelectedThreadId = a_TextBox->GetTimer().m_TID;
  Capture::GSelectedCallstack =
      Capture::GetCallstack(a_TextBox->GetTimer().m_CallstackHash);
  GOrbitApp->SetCallStack(Capture::GSelectedCallstack);

  const Timer& a_Timer = a_TextBox->GetTimer();
  DWORD64 address = a_Timer.m_FunctionAddress;
  if (a_Timer.IsType(Timer::ZONE)) {
    std::shared_ptr<CallStack> callStack =
        Capture::GetCallstack(a_Timer.m_CallstackHash);
    if (callStack && callStack->m_Depth > 1) {
      address = callStack->m_Data[1];
    }
  }

  FindCode(address);

  if (m_DoubleClicking && a_TextBox) {
    time_graph_.Zoom(a_TextBox);
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::Hover(int a_X, int a_Y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::vector<uint8_t> pixels(1 * 1 * 4);
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);

  PickingID pickId = *reinterpret_cast<PickingID*>(&pixels[0]);

  TextBox* textBox = time_graph_.GetBatcher().GetTextBox(pickId);
  if (textBox) {
    if (!textBox->GetTimer().IsType(Timer::CORE_ACTIVITY)) {
      Function* func =
          Capture::GSelectedFunctionsMap[textBox->GetTimer().m_FunctionAddress];
      m_ToolTip = absl::StrFormat("%s %s", func ? func->PrettyName() : "",
                                  textBox->GetText());
      GOrbitApp->SendToUiAsync("tooltip:" + m_ToolTip);
      NeedsRedraw();
    }
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::FindCode(DWORD64 address) {
#ifdef _WIN32
  SCOPE_TIMER_LOG("FindCode");

  LineInfo lineInfo;

  if (SymUtils::GetLineInfo(address, lineInfo) ||
      (Capture::GSamplingProfiler &&
       Capture::GSamplingProfiler->GetLineInfo(address, lineInfo))) {
    --lineInfo.m_Line;

    // File mapping
    const std::map<std::string, std::string>& fileMap =
        GOrbitApp->GetFileMapping();
    for (const auto& pair : fileMap) {
      ReplaceStringInPlace(lineInfo.m_File, pair.first, pair.second);
    }

    if (lineInfo.m_Address != 0) {
      GOrbitApp->SendToUiAsync(
          absl::StrFormat("code^%s^%i", lineInfo.m_File, lineInfo.m_Line));
    }
  }
#else
  UNUSED(address);
#endif
}

//-----------------------------------------------------------------------------
void CaptureWindow::PreRender() {
  if (m_CanHover && m_HoverTimer.QueryMillis() > m_HoverDelayMs) {
    m_IsHovering = true;
    m_Picking = true;
    NeedsRedraw();
  }

  m_NeedsRedraw = m_NeedsRedraw || time_graph_.IsRedrawNeeded();
}

//-----------------------------------------------------------------------------
void CaptureWindow::PostRender() {
  if (m_IsHovering) {
    m_IsHovering = false;
    m_CanHover = false;
    m_Picking = false;
    m_HoverTimer.Reset();

    Hover(m_MousePosX, m_MousePosY);
    NeedsUpdate();
    GlCanvas::Render(m_Width, m_Height);
    m_HoverTimer.Reset();
  }

  if (m_Picking) {
    m_Picking = false;
    Pick(m_ScreenClickX, m_ScreenClickY);
    NeedsRedraw();
    GlCanvas::Render(m_Width, m_Height);
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::Resize(int a_Width, int a_Height) {
  GlCanvas::Resize(a_Width, a_Height);
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RightDown(int a_X, int a_Y) {
  ScreenToWorld(a_X, a_Y, m_WorldClickX, m_WorldClickY);
  m_ScreenClickX = a_X;
  m_ScreenClickY = a_Y;
  Pick();

  m_IsSelecting = true;
  m_SelectStart = Vec2(m_WorldClickX, m_WorldClickY);
  m_SelectStop = m_SelectStart;
  m_TimeStart = time_graph_.GetTickFromWorld(m_WorldClickX);
  m_TimeStop = m_TimeStart;
}

//-----------------------------------------------------------------------------
bool CaptureWindow::RightUp() {
  if (m_IsSelecting && (m_SelectStart[0] != m_SelectStop[0]) &&
      ControlPressed()) {
    float minWorld = std::min(m_SelectStop[0], m_SelectStart[0]);
    float maxWorld = std::max(m_SelectStop[0], m_SelectStart[0]);

    double newMin =
        time_graph_.GetTime((minWorld - m_WorldTopLeftX) / m_WorldWidth);
    double newMax =
        time_graph_.GetTime((maxWorld - m_WorldTopLeftX) / m_WorldWidth);

    time_graph_.SetMinMax(newMin, newMax);
    m_SelectStart = m_SelectStop;
  }

  bool showContextMenu = m_SelectStart[0] == m_SelectStop[0];
  m_IsSelecting = false;
  NeedsRedraw();
  return showContextMenu;
}

//-----------------------------------------------------------------------------
void CaptureWindow::MiddleDown(int a_X, int a_Y) {
  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_IsSelecting = true;
  m_SelectStart = Vec2(worldx, worldy);
  m_SelectStop = m_SelectStart;
}

//-----------------------------------------------------------------------------
void CaptureWindow::MiddleUp(int a_X, int a_Y) {
  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_IsSelecting = false;

  m_SelectStop = Vec2(worldx, worldy);

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Zoom(int a_Delta) {
  if (a_Delta == 0) return;

  a_Delta = -a_Delta;

  float worldx;
  float worldy;

  ScreenToWorld(m_MousePosX, m_MousePosY, worldx, worldy);
  m_MouseRatio = static_cast<double>(m_MousePosX) / getWidth();

  time_graph_.ZoomTime(a_Delta, m_MouseRatio);
  m_WheelMomentum =
      a_Delta * m_WheelMomentum < 0 ? 0 : m_WheelMomentum + a_Delta;

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pan(float a_Ratio) {
  double refTime =
      time_graph_.GetTime(static_cast<double>(m_MousePosX) / getWidth());
  time_graph_.PanTime(m_MousePosX,
                      m_MousePosX + static_cast<int>(a_Ratio * getWidth()),
                      getWidth(), refTime);
  UpdateSceneBox();
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::MouseWheelMoved(int a_X, int a_Y, int a_Delta,
                                    bool a_Ctrl) {
  if (a_Delta == 0) return;

  // Normalize and invert sign, so that delta < 0 is zoom in.
  int delta = a_Delta < 0 ? 1 : -1;

  if (delta < m_MinWheelDelta) m_MinWheelDelta = delta;
  if (delta > m_MaxWheelDelta) m_MaxWheelDelta = delta;

  float mousex = a_X;

  float worldx;
  float worldy;

  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_MouseRatio = static_cast<double>(mousex) / getWidth();

  bool zoomWidth = !a_Ctrl;
  if (zoomWidth) {
    time_graph_.ZoomTime(delta, m_MouseRatio);
    m_WheelMomentum = delta * m_WheelMomentum < 0 ? 0 : m_WheelMomentum + delta;
  } else {
    // TODO: reimplement vertical zoom by scaling track heights.
  }

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(this, -delta);

  m_CanHover = true;

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::MouseWheelMovedHorizontally(int /*a_X*/, int /*a_Y*/,
                                                int a_Delta, bool /*a_Ctrl*/) {
  if (a_Delta == 0) return;

  // Normalize and invert sign, so that delta < 0 is left.
  int delta = a_Delta < 0 ? 1 : -1;

  if (delta < 0) {
    Pan(0.1f);
  } else {
    Pan(-0.1f);
  }

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(this, -delta);
}

//-----------------------------------------------------------------------------
void CaptureWindow::KeyPressed(unsigned int a_KeyCode, bool a_Ctrl,
                               bool a_Shift, bool a_Alt) {
  UpdateSpecialKeys(a_Ctrl, a_Shift, a_Alt);

  ScopeImguiContext state(m_ImGuiContext);

  if (!m_ImguiActive) {
    switch (a_KeyCode) {
      case ' ':
        ZoomAll();
        break;
      case 'A':
        Pan(0.1f);
        break;
      case 'D':
        Pan(-0.1f);
        break;
      case 'W':
        Zoom(1);
        break;
      case 'S':
        Zoom(-1);
        break;
      case 'F':
        m_DrawFilter = !m_DrawFilter;
        break;
      case 'I':
        m_DrawStats = !m_DrawStats;
        break;
      case 'H':
        m_DrawHelp = !m_DrawHelp;
        break;
      case 'X':
        GOrbitApp->ToggleCapture();
        m_DrawHelp = false;
#ifdef __linux__
        ZoomAll();
#endif
        break;
      case 'O':
        if (a_Ctrl) {
          m_TextRenderer.ToggleDrawOutline();
        }
        break;
      case 18:  // Left
        time_graph_.OnLeft();
        break;
      case 20:  // Right
        time_graph_.OnRight();
        break;
      case 19:  // Up
        time_graph_.OnUp();
        break;
      case 21:  // Down
        time_graph_.OnDown();
        break;
    }
  }

  ImGuiIO& io = ImGui::GetIO();
  io.KeyCtrl = a_Ctrl;
  io.KeyShift = a_Shift;
  io.KeyAlt = a_Alt;

  Orbit_ImGui_KeyCallback(this, a_KeyCode, true);

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
const std::string CaptureWindow::MENU_ACTION_GO_TO_CALLSTACK =
    "Go to Callstack";
const std::string CaptureWindow::MENU_ACTION_GO_TO_SOURCE = "Go to Source";

//-----------------------------------------------------------------------------
std::vector<std::string> CaptureWindow::GetContextMenu() {
  std::vector<std::string> menu;
  if (!absl::GetFlag(FLAGS_enable_stale_features)) {
    return menu;
  }

  TextBox* selection = Capture::GSelectedTextBox;
  if (selection != nullptr && !selection->GetTimer().IsCoreActivity() &&
      selection->GetTimer().m_Type != Timer::GPU_ACTIVITY) {
    return menu;
  }

  Append(menu, {MENU_ACTION_GO_TO_CALLSTACK, MENU_ACTION_GO_TO_SOURCE});
  return menu;
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnContextMenu(const std::string& a_Action,
                                  int /*a_MenuIndex*/) {
  if (Capture::GSelectedTextBox) {
    if (a_Action == MENU_ACTION_GO_TO_SOURCE) {
      GOrbitApp->GoToCode(
          Capture::GSelectedTextBox->GetTimer().m_FunctionAddress);
    } else if (a_Action == MENU_ACTION_GO_TO_CALLSTACK) {
      GOrbitApp->GoToCallstack();
    }
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::ToggleSampling() {
  if (Capture::GIsSampling) {
    Capture::StopSampling();
  } else if (!GTimerManager->m_IsRecording) {
    Capture::StartSampling();
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnCaptureStarted() {
  time_graph_.ZoomAll();
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void CaptureWindow::ResetHoverTimer() {
  m_HoverTimer.Reset();
  m_CanHover = true;
}

//-----------------------------------------------------------------------------
void CaptureWindow::Draw() {
  m_WorldMaxY =
      1.5f * ScreenToWorldHeight(static_cast<int>(m_Slider.GetPixelHeight()));

  if (Capture::IsCapturing()) {
    ZoomAll();
  }

  // Reset picking manager before each draw.
  m_PickingManager.Reset();

  time_graph_.Draw(m_Picking);

  if (m_SelectStart[0] != m_SelectStop[0]) {
    TickType minTime = std::min(m_TimeStart, m_TimeStop);
    TickType maxTime = std::max(m_TimeStart, m_TimeStop);

    float from = time_graph_.GetWorldFromTick(minTime);
    float to = time_graph_.GetWorldFromTick(maxTime);

    double micros = MicroSecondsFromTicks(minTime, maxTime);
    float sizex = to - from;
    Vec2 pos(from, m_WorldTopLeftY - m_WorldHeight);
    Vec2 size(sizex, m_WorldHeight);

    std::string time = GetPrettyTime(micros * 0.001);
    TextBox box(pos, size, time, Color(0, 128, 0, 128));
    box.SetTextY(m_SelectStop[1]);
    box.Draw(m_TextRenderer, -FLT_MAX, true, true);
  }

  if (!m_Picking && !m_IsHovering) {
    DrawStatus();
    RenderTimeBar();

    // Vertical line
    glColor4f(0, 1, 0, 0.5f);
    glBegin(GL_LINES);
    glVertex3f(m_MouseX, m_WorldTopLeftY, Z_VALUE_TEXT);
    glVertex3f(m_MouseX, m_WorldTopLeftY - m_WorldHeight, Z_VALUE_TEXT);
    glEnd();
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::DrawScreenSpace() {
  double timeSpan = time_graph_.GetSessionTimeSpanUs();

  Color col = m_Slider.GetBarColor();
  float height = m_Slider.GetPixelHeight();
  float canvasHeight = getHeight();
  float z = GlCanvas::Z_VALUE_TEXT_UI_BG;

  // Time bar
  if (time_graph_.GetSessionTimeSpanUs() > 0) {
    glColor4ub(70, 70, 70, 200);
    glBegin(GL_QUADS);
    glVertex3f(0, height, z);
    glVertex3f(getWidth(), height, z);
    glVertex3f(getWidth(), 2 * height, z);
    glVertex3f(0, 2 * height, z);
    glEnd();
  }

  const TimeGraphLayout& layout = time_graph_.GetLayout();
  float vertical_margin = layout.GetVerticalMargin();

  if (timeSpan > 0) {
    double start = time_graph_.GetMinTimeUs();
    double stop = time_graph_.GetMaxTimeUs();
    double width = stop - start;
    double maxStart = timeSpan - width;
    double ratio =
        Capture::IsCapturing() ? 1 : (maxStart != 0 ? start / maxStart : 0);
    float slider_width = layout.GetSliderWidth();
    m_Slider.SetPixelHeight(slider_width);
    m_Slider.SetSliderRatio(static_cast<float>(ratio));
    m_Slider.SetSliderWidthRatio(static_cast<float>(width / timeSpan));
    m_Slider.Draw(this, m_Picking);

    float verticalRatio = m_WorldHeight / time_graph_.GetThreadTotalHeight();
    if (verticalRatio < 1.f) {
      m_VerticalSlider.SetPixelHeight(slider_width);
      m_VerticalSlider.SetSliderWidthRatio(verticalRatio);
      m_VerticalSlider.Draw(this, m_Picking);
      vertical_margin += slider_width;
    }
  }

  // Draw right vertical margin.
  time_graph_.SetVerticalMargin(vertical_margin);
  const Color kBackgroundColor(70, 70, 70, 255);
  float margin_x1 = getWidth();
  float margin_x0 = margin_x1 - vertical_margin;
  glColor4ubv(&kBackgroundColor[0]);
  glBegin(GL_QUADS);
  glVertex3f(margin_x0, 0, z);
  glVertex3f(margin_x1, 0, z);
  glVertex3f(margin_x1, canvasHeight - height, z);
  glVertex3f(margin_x0, canvasHeight - height, z);
  glEnd();
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnDrag(float a_Ratio) {
  time_graph_.OnDrag(a_Ratio);
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnVerticalDrag(float a_Ratio) {
  float min = m_WorldMaxY;
  float max = m_WorldHeight - time_graph_.GetThreadTotalHeight();
  float range = max - min;
  m_WorldTopLeftY = min + a_Ratio * range;
  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::UpdateVerticalSlider() {
  float min = m_WorldMaxY;
  float max = m_WorldHeight - time_graph_.GetThreadTotalHeight();
  float ratio = (m_WorldTopLeftY - min) / (max - min);
  m_VerticalSlider.SetSliderRatio(ratio);
}

//-----------------------------------------------------------------------------
void CaptureWindow::NeedsUpdate() {
  time_graph_.NeedsUpdate();
  m_NeedsRedraw = true;
}

//-----------------------------------------------------------------------------
float CaptureWindow::GetTopBarTextY() {
  return m_Slider.GetPixelHeight() * 0.5f +
         m_TextRenderer.GetStringHeight("FpjT_H") * 0.5f;
}

//-----------------------------------------------------------------------------
void CaptureWindow::DrawStatus() {
  int s_PosX = 0;
  int s_PosY = static_cast<int>(GetTopBarTextY());
  static int s_IncY = 20;

  static Color s_Color(255, 255, 255, 255);

  int PosX = getWidth() - s_PosX;
  int PosY = s_PosY;
  int LeftY = s_PosY;

  LeftY += s_IncY;

  if (Capture::GInjected) {
    std::string injectStr =
        absl::StrFormat(" %s", Capture::GInjectedProcess.c_str());
    m_ProcessX = m_TextRenderer.AddText2D(injectStr.c_str(), PosX, PosY,
                                          Z_VALUE_TEXT_UI, s_Color, -1, true);
    PosY += s_IncY;
  }

  if (Capture::GIsTesting) {
    m_TextRenderer.AddText2D("TESTING", PosX, PosY, Z_VALUE_TEXT_UI, s_Color,
                             -1, true);
    PosY += s_IncY;
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderUI() {
  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_NewFrame(this);

  if (m_DrawStats) {
    ImGui::ShowDemoWindow();
    if (time_graph_.GetLayout().DrawProperties()) {
      NeedsUpdate();
    }

    m_StatsWindow.Clear();

    m_StatsWindow.AddLine(VAR_TO_STR(m_Width));
    m_StatsWindow.AddLine(VAR_TO_STR(m_Height));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldHeight));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldWidth));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldTopLeftX));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldTopLeftY));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldMinWidth));
    m_StatsWindow.AddLine(VAR_TO_STR(m_MouseX));
    m_StatsWindow.AddLine(VAR_TO_STR(m_MouseY));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GNumContextSwitches));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GNumLinuxEvents));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GNumProfileEvents));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GNumInstalledHooks));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GSelectedFunctionsMap.size()));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GVisibleFunctionsMap.size()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetNumDrawnTextBoxes()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetNumTimers()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetThreadTotalHeight()));

#ifdef WIN32
    for (std::string& line : GTcpServer->GetStats()) {
      m_StatsWindow.AddLine(line);
    }

    bool hasConnection = GTcpServer->HasConnection();
    m_StatsWindow.AddLine(VAR_TO_STR(hasConnection));
#else
    m_StatsWindow.AddLine(
        VAR_TO_STR(GEventTracer.GetEventBuffer().GetCallstacks().size()));
    m_StatsWindow.AddLine(
        VAR_TO_STR(GEventTracer.GetEventBuffer().GetNumEvents()));
#endif

    m_StatsWindow.Draw("Capture Stats", &m_DrawStats);
  }

  if (m_DrawHelp) {
    RenderHelpUi();

    if (m_FirstHelpDraw) {
      // Redraw so that Imgui resizes the
      // window properly on first draw
      NeedsRedraw();
      m_FirstHelpDraw = false;
    }
  }

  RenderToolbars();

  if (m_DrawMemTracker && !m_DrawHelp) {
    RenderMemTracker();
  }

  // Rendering
  glViewport(0, 0, getWidth(), getHeight());
  ImGui::Render();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderText() {
  if (!m_Picking) {
    time_graph_.DrawText();
  }
}

//-----------------------------------------------------------------------------
void ColorToFloat(Color a_Color, float* o_Float) {
  for (size_t i = 0; i < 4; ++i) {
    o_Float[i] = a_Color[i] / 255.f;
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderHelpUi() {
  constexpr float kYOffset = 8.f;
  ImGui::SetNextWindowPos(ImVec2(0, toolbar_height_ + kYOffset));

  ImVec4 color(1.f, 0, 0, 1.f);
  ColorToFloat(m_Slider.GetBarColor(), &color.x);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

  if (!ImGui::Begin("Help Overlay", &m_DrawHelp, ImVec2(0, 0), 1.f,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::PopStyleColor();
    ImGui::End();
    return;
  }

  ImGui::Text("Start/Stop Capture: 'X'");
  ImGui::Text("Pan: 'A','D' or \"Left Click + Drag\"");
  ImGui::Text("Zoom: 'W', 'S', Scroll or \"Ctrl + Right Click + Drag\"");
  ImGui::Text("Select: Left Click");
  ImGui::Text("Measure: \"Right Click + Drag\"");
  ImGui::Text("Toggle Help: 'H'");

  ImGui::End();

  ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
ImTextureID TextureId(uint32_t id) {
  uint64_t texture_id = id;
  return reinterpret_cast<ImTextureID>(texture_id);
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderToolbars() {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  float width = this->getWidth();
  const ImVec4 transparent(0.f, 0, 0, 0.f);
  const ImVec4 popup_color(66.f / 255.f, 150.f / 255.f, 250.f / 255.f, 1.f);
  ImVec4 color(1.f, 0.f, 0.f, 1.f);
  ColorToFloat(m_Slider.GetBarColor(), &color.x);
  float icon_height = time_graph_.GetLayout().GetToolbarIconHeight();
  ImVec2 icon_size(icon_height, icon_height);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
  ImGui::PushStyleColor(ImGuiCol_Button, transparent);
  ImGui::PushStyleColor(ImGuiCol_FrameBg, transparent);
  ImGui::PushStyleColor(ImGuiCol_PopupBg, popup_color);

  // Action Toolbar.
  ImGui::Begin("Toolbar", &m_DrawHelp, ImVec2(0, 0), 1.f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

  // Start Capture.
  if (ImGui::ImageButton(TextureId(start_capture_icon_id_), icon_size)) {
    m_DrawHelp = false;
    GOrbitApp->StartCapture();
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Start Capture");

  // Stop Capture.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(stop_capture_icon_id_), icon_size)) {
    GOrbitApp->StopCapture();
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Stop Capture");

  // Clear Capture.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(clear_capture_icon_id_), icon_size)) {
    Capture::ClearCaptureData();
    Capture::GClearCaptureDataFunc();
    GCurrentTimeGraph->Clear();
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear Capture");

  // Load Capture.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(load_capture_icon_id_), icon_size)) {
    GOrbitApp->SendToUiAsync("opencapture");
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Open Capture");

  // Save Capture.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(save_capture_icon_id_), icon_size)) {
    GOrbitApp->SendToUiAsync("savecapture");
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Save Capture");

  // Help.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(help_icon_id_), icon_size)) {
    m_DrawHelp = !m_DrawHelp;
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Help");

  // Feedback.
  ImGui::SameLine();
  if (ImGui::ImageButton(TextureId(feedback_icon_id_), icon_size)) {
  }
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Feedback");

  const float space_between_toolbars = 0;
  float current_x = ImGui::GetWindowWidth() + space_between_toolbars;
  toolbar_height_ = ImGui::GetWindowHeight();
  ImGui::End();

  // Tracks Filter Toolbar.
  ImGui::SetNextWindowPos(ImVec2(current_x, 0));
  ImGui::Begin("Filters", &m_DrawHelp, ImVec2(0, 0), 1.f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

  ImGui::ImageButton(TextureId(filter_tracks_icon_id_), icon_size);
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filter Tracks");

  ImGui::SameLine();
  ImGui::PushItemWidth(300.f);
  ImGui::InputText("##Track Filter", track_filter_,
                   IM_ARRAYSIZE(track_filter_));
  ImGui::PopItemWidth();
  GCurrentTimeGraph->SetThreadFilter(track_filter_);

  current_x += ImGui::GetWindowWidth() + space_between_toolbars;
  ImGui::End();

  // Search Toolbar.
  ImGui::SetNextWindowPos(ImVec2(current_x, 0));
  ImGui::Begin("Search", &m_DrawHelp, ImVec2(0, 0), 1.f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

  ImGui::ImageButton(TextureId(search_icon_id_), icon_size);
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Search");
  ImGui::SameLine();
  ImGui::PushItemWidth(300.f);
  ImGui::InputText("##Search", find_filter_, IM_ARRAYSIZE(find_filter_));
  ImGui::PopItemWidth();
  GOrbitApp->FilterFunctions(find_filter_);

  current_x += ImGui::GetWindowWidth() + space_between_toolbars;
  ImGui::End();

  // Capture Info.
  ImGui::SetNextWindowPos(ImVec2(current_x, 0));
  ImGui::Begin("CaptureInfo", nullptr, ImVec2(0, 0), 1.f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  ImGui::ImageButton(TextureId(time_icon_id_), icon_size);
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Capture Time");
  ImGui::SameLine();

  double timeSpan = time_graph_.GetSessionTimeSpanUs();
  std::string capture_time = GetPrettyTime(timeSpan * 0.001);
  ImGui::Text("%s", capture_time.c_str());
  current_x += ImGui::GetWindowWidth() + space_between_toolbars;
  ImGui::End();

  // Process Info.
  ImGui::SetNextWindowSize(
      ImVec2(width - current_x - time_graph_.GetVerticalMargin(), -1.f));
  ImGui::SetNextWindowPos(ImVec2(current_x, 0));
  ImGui::Begin("ProcessInfo", nullptr, ImVec2(0, 0), 1.f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
  ImGui::ImageButton(TextureId(info_icon_id_), icon_size);
  if (ImGui::IsItemHovered()) ImGui::SetTooltip("Process Info");
  ImGui::SameLine();
  std::string process_info = Capture::GTargetProcess->GetName();
  if (!process_info.empty()) {
    uint32_t process_id = Capture::GTargetProcess->GetID();
    ImGui::Text("%s [%u]", process_info.c_str(), process_id);
  }
  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderMemTracker() {
  float barHeight = m_Slider.GetPixelHeight();
  ImGui::SetNextWindowPos(ImVec2(0, barHeight * 1.5f));

  ImVec4 color(1.f, 0, 0, 1.f);
  ColorToFloat(m_Slider.GetBarColor(), &color.x);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

  bool dummy = true;
  if (!ImGui::Begin(
          "MemTracker Overlay", &dummy, ImVec2(0, 0), 1.f,
          ImGuiWindowFlags_NoTitleBar /*| ImGuiWindowFlags_NoResize*/ |
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::PopStyleColor();
    ImGui::End();
    return;
  }

  ImGui::Text("=== Memory Tracker ===");

  const MemoryTracker& memTracker = time_graph_.GetMemoryTracker();
  if (memTracker.NumAllocatedBytes() == 0) {
    std::string str = VAR_TO_STR(memTracker.NumAllocatedBytes()) +
                      std::string("            ");
    ImGui::Text("%s", str.c_str());
    ImGui::Text("%s", VAR_TO_STR(memTracker.NumFreedBytes()).c_str());
    ImGui::Text("%s", VAR_TO_STR(memTracker.NumLiveBytes()).c_str());
  } else {
    ImGui::Text("%s", VAR_TO_STR(memTracker.NumAllocatedBytes()).c_str());
    ImGui::Text("%s", VAR_TO_STR(memTracker.NumFreedBytes()).c_str());
    ImGui::Text("%s", VAR_TO_STR(memTracker.NumLiveBytes()).c_str());
  }

  ImGui::End();

  ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
void DrawTexturedSquare(GLuint a_TextureId, float a_Size, float a_X,
                        float a_Y) {
  glUseProgram(0);
  glColor4ub(255, 255, 255, 255);

  glEnable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_MATERIAL);
  glBindTexture(GL_TEXTURE_2D, a_TextureId);

  glBegin(GL_QUADS);
  glTexCoord2f(0.f, 1.f);
  glVertex3f(a_X, a_Y, 0.f);
  glTexCoord2f(0.f, 0.f);
  glVertex3f(a_X, a_Y + a_Size, 0.f);
  glTexCoord2f(1.f, 0.f);
  glVertex3f(a_X + a_Size, a_Y + a_Size, 0.f);
  glTexCoord2f(1.f, 1.f);
  glVertex3f(a_X + a_Size, a_Y, 0.f);
  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
inline double GetIncrementMs(double a_MilliSeconds) {
  const double Day = 24 * 60 * 60 * 1000;
  const double Hour = 60 * 60 * 1000;
  const double Minute = 60 * 1000;
  const double Second = 1000;
  const double Milli = 1;
  const double Micro = 0.001;
  const double Nano = 0.000001;

  std::string res;

  if (a_MilliSeconds < Micro)
    return Nano;
  else if (a_MilliSeconds < Milli)
    return Micro;
  else if (a_MilliSeconds < Second)
    return Milli;
  else if (a_MilliSeconds < Minute)
    return Second;
  else if (a_MilliSeconds < Hour)
    return Minute;
  else if (a_MilliSeconds < Day)
    return Hour;
  else
    return Day;
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderTimeBar() {
  static int numTimePoints = 10;

  if (time_graph_.GetSessionTimeSpanUs() > 0) {
    double millis = time_graph_.GetCurrentTimeSpanUs() * 0.001;
    double incr = millis / float(numTimePoints - 1);
    double unit = GetIncrementMs(incr);
    double normInc = static_cast<int>((incr + unit) / unit) * unit;
    double startMs = time_graph_.GetMinTimeUs() * 0.001;
    double normStartUs = 1000.0 * static_cast<int>(startMs / normInc) * normInc;

    static int pixelMargin = 2;
    int screenY =
        getHeight() - static_cast<int>(m_Slider.GetPixelHeight()) - pixelMargin;
    float dummy, worldY;
    ScreenToWorld(0, screenY, dummy, worldY);

    float height =
        ScreenToWorldHeight(static_cast<int>(GParams.m_FontSize) + pixelMargin);
    float xMargin = ScreenToworldWidth(4);

    for (int i = 0; i < numTimePoints; ++i) {
      double currentMicros = normStartUs + i * 1000 * normInc;
      if (currentMicros < 0) continue;

      double currentMillis = currentMicros * 0.001;
      std::string text = GetPrettyTime(currentMillis);
      float worldX = time_graph_.GetWorldFromUs(currentMicros);
      m_TextRenderer.AddText(text.c_str(), worldX + xMargin, worldY,
                             GlCanvas::Z_VALUE_TEXT_UI,
                             Color(255, 255, 255, 255));

      glColor4f(1.f, 1.f, 1.f, 1.f);
      glBegin(GL_LINES);
      glVertex3f(worldX, worldY, Z_VALUE_UI);
      glVertex3f(worldX, worldY + height, Z_VALUE_UI);
      glEnd();
    }
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnTimerAdded(Timer& a_Timer) {
  time_graph_.ProcessTimer(a_Timer);
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnContextSwitchAdded(const ContextSwitch& a_CS) {
  time_graph_.AddContextSwitch(a_CS);
}

//-----------------------------------------------------------------------------
void CaptureWindow::SendProcess() {
  if (Capture::GTargetProcess) {
    std::string processData =
        SerializeObjectHumanReadable(*Capture::GTargetProcess);
    PRINT_VAR(processData);
    GTcpClient->Send(Msg_RemoteProcess, processData.data(), processData.size());
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::Initialize() {
  GlCanvas::Initialize();
  LoadIcons();
}

//-----------------------------------------------------------------------------
uint32_t LoadIcon(const char* name) {
  std::string icon_path = Path::GetExecutablePath() + "icons/" + name;
  return LoadTextureFromFile(icon_path.c_str());
}

//-----------------------------------------------------------------------------
void CaptureWindow::LoadIcons() {
  start_capture_icon_id_ = LoadIcon("outline_play_arrow_white_48dp.png");
  stop_capture_icon_id_ = LoadIcon("outline_stop_white_48dp.png");
  save_capture_icon_id_ = LoadIcon("outline_save_white_48dp.png");
  load_capture_icon_id_ = LoadIcon("outline_folder_white_48dp.png");
  clear_capture_icon_id_ = LoadIcon("outline_clear_white_48dp.png");
  help_icon_id_ = LoadIcon("outline_help_outline_white_48dp.png");
  filter_tracks_icon_id_ = LoadIcon("outline_filter_list_white_48dp.png");
  search_icon_id_ = LoadIcon("outline_search_white_48dp.png");
  time_icon_id_ = LoadIcon("outline_access_time_white_48dp.png");
  feedback_icon_id_ = LoadIcon("outline_feedback_white_48dp.png");
  info_icon_id_ = LoadIcon("outline_info_white_48dp.png");
  save_capture_icon_id_ = LoadIcon("outline_save_alt_white_48dp.png");
}
