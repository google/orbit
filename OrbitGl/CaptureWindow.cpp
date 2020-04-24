//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

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

//-----------------------------------------------------------------------------
CaptureWindow::CaptureWindow() {
  GCurrentTimeGraph = &time_graph_;
  time_graph_.SetTextRenderer(&m_TextRenderer);
  time_graph_.SetPickingManager(&m_PickingManager);
  time_graph_.SetCanvas(this);
  m_DrawUI = false;
  m_DrawHelp = false;
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
        m_WorldClickX - (float)mousex / (float)getWidth() * m_WorldWidth;
    m_WorldTopLeftY =
        m_WorldClickY + (float)mousey / (float)getHeight() * m_WorldHeight;

    m_WorldTopLeftX = clamp(m_WorldTopLeftX, worldMin, worldMax - m_WorldWidth);
    m_WorldTopLeftY =
        clamp(m_WorldTopLeftY,
              m_WorldHeight - time_graph_.GetThreadTotalHeight(), m_WorldMaxY);
    UpdateSceneBox();

    time_graph_.PanTime(m_ScreenClickX, a_X, getWidth(),
                        (double)m_RefTimeClick);
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
  m_RefTimeClick =
      (TickType)time_graph_.GetTime((double)a_X / (double)getWidth());

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
  std::vector<unsigned char> pixels(1 * 1 * 4);
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);

  PickingID pickId = PickingID::Get(*((uint32_t*)(&pixels[0])));

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
        TextBox* textBox = (TextBox*)*textBoxPtr;
        SelectTextBox(textBox);
      }
      break;
    }
    case PickingID::LINE: {
      void** textBoxPtr =
          time_graph_.GetBatcher().GetLineBuffer().m_UserData.SlowAt(id);
      if (textBoxPtr) {
        TextBox* textBox = (TextBox*)*textBoxPtr;
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
  std::vector<unsigned char> pixels(1 * 1 * 4);
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);

  PickingID pickId = *((PickingID*)(&pixels[0]));

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
  m_Width = a_Width;
  m_Height = a_Height;
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
  m_MouseRatio = (double)m_MousePosX / (double)getWidth();

  time_graph_.ZoomTime((float)a_Delta, m_MouseRatio);
  m_WheelMomentum =
      a_Delta * m_WheelMomentum < 0 ? 0.f : m_WheelMomentum + (float)a_Delta;

  NeedsUpdate();
}

//-----------------------------------------------------------------------------
void CaptureWindow::Pan(float a_Ratio) {
  double refTime =
      (TickType)time_graph_.GetTime((double)m_MousePosX / (double)getWidth());
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

  float mousex = (float)a_X;

  float worldx;
  float worldy;

  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_MouseRatio = (double)mousex / (double)getWidth();

  static float zoomRatio = 0.1f;
  bool zoomWidth = !a_Ctrl;

  if (zoomWidth) {
    time_graph_.ZoomTime((float)delta, m_MouseRatio);
    m_WheelMomentum =
        delta * m_WheelMomentum < 0 ? 0.f : m_WheelMomentum + (float)delta;
  } else {
    float zoomInc = zoomRatio * m_DesiredWorldHeight;
    m_DesiredWorldHeight += delta * zoomInc;

    float worldMin, worldMax;
    time_graph_.GetWorldMinMax(worldMin, worldMax);
    m_WorldTopLeftX = clamp(m_WorldTopLeftX, worldMin, worldMax - m_WorldWidth);
    m_WorldTopLeftY = clamp(m_WorldTopLeftY, -FLT_MAX, m_WorldMaxY);

    UpdateSceneBox();
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
      case 'C':
#ifdef __linux__
        LinuxUtils::DumpClocks();
#endif
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
#ifdef __linux__
        ZoomAll();
#endif
        break;
      case 'O':
        if (a_Ctrl) {
          m_TextRenderer.ToggleDrawOutline();
        }
        break;
      case 'P':

#ifdef _WIN32
        GOrbitApp->LoadSystrace(
            "C:/Users/pierr/Downloads/perf_traces/systrace_tutorial.html");
#else
        GOrbitApp->LoadSystrace(
            "/home/pierric/perf_traces/systrace_tutorial.html");
#endif
        break;
      case 'M':
        m_DrawMemTracker = !m_DrawMemTracker;
        break;
      case 'K':
        SendProcess();
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
std::wstring GOTO_CALLSTACK = L"Go to Callstack";
std::wstring GOTO_SOURCE = L"Go to Source";

//-----------------------------------------------------------------------------
std::vector<std::wstring> CaptureWindow::GetContextMenu() {
  static std::vector<std::wstring> menu = {GOTO_CALLSTACK, GOTO_SOURCE};
  static std::vector<std::wstring> emptyMenu;
  TextBox* selection = Capture::GSelectedTextBox;
  return selection != nullptr && !selection->GetTimer().IsCoreActivity() &&
                 selection->GetTimer().m_Type != Timer::GPU_ACTIVITY
             ? menu
             : emptyMenu;
}

//-----------------------------------------------------------------------------
void CaptureWindow::OnContextMenu(const std::wstring& a_Action,
                                  int /*a_MenuIndex*/) {
  if (Capture::GSelectedTextBox) {
    if (a_Action == GOTO_SOURCE) {
      GOrbitApp->GoToCode(
          Capture::GSelectedTextBox->GetTimer().m_FunctionAddress);
    } else if (a_Action == GOTO_CALLSTACK) {
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
  m_DesiredWorldWidth = (float)m_Width;
  m_DesiredWorldHeight = (float)m_Height;
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
  m_WorldMaxY = 1.5f * ScreenToWorldHeight((int)m_Slider.GetPixelHeight());

  if (Capture::IsCapturing()) {
    ZoomAll();
  }

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
  float canvasHeight = (float)getHeight();
  float z = GlCanvas::Z_VALUE_TEXT_UI_BG;

  // Top bar
  glColor4ubv(&col[0]);
  glBegin(GL_QUADS);
  glVertex3f(0, canvasHeight, z);
  glVertex3f((float)getWidth(), canvasHeight, z);
  glVertex3f((float)getWidth(), canvasHeight - height, z);
  glVertex3f(0, canvasHeight - height, z);
  glEnd();

  // Time bar
  if (time_graph_.GetSessionTimeSpanUs() > 0) {
    glColor4ub(70, 70, 70, 200);
    glBegin(GL_QUADS);
    glVertex3f(0, height, z);
    glVertex3f((float)getWidth(), height, z);
    glVertex3f((float)getWidth(), 2 * height, z);
    glVertex3f(0, 2 * height, z);
    glEnd();
  }

  if (timeSpan > 0) {
    double start = time_graph_.GetMinTimeUs();
    double stop = time_graph_.GetMaxTimeUs();
    double width = stop - start;
    double maxStart = timeSpan - width;
    double ratio =
        Capture::IsCapturing() ? 1 : (maxStart != 0 ? start / maxStart : 0);

    m_Slider.SetPixelHeight(time_graph_.GetLayout().GetSliderWidth());
    m_Slider.SetSliderRatio((float)ratio);
    m_Slider.SetSliderWidthRatio(float(width / timeSpan));
    m_Slider.Draw(this, m_Picking);

    float verticalRatio = m_WorldHeight / time_graph_.GetThreadTotalHeight();
    if (verticalRatio < 1.f) {
      m_VerticalSlider.SetPixelHeight(time_graph_.GetLayout().GetSliderWidth());
      m_VerticalSlider.SetSliderWidthRatio(verticalRatio);
      m_VerticalSlider.Draw(this, m_Picking);
    }
  }
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
         (float)m_TextRenderer.GetStringHeight("FpjT_H") * 0.5f;
}

//-----------------------------------------------------------------------------
void CaptureWindow::DrawStatus() {
  int s_PosX = 0;
  float s_PosY = GetTopBarTextY();
  static int s_IncY = 20;

  static Color s_Color(255, 255, 255, 255);

  int PosX = getWidth() - s_PosX;
  int PosY = (int)s_PosY;
  int LeftY = (int)s_PosY;

  m_TextRenderer.AddText2D(" Press 'H' for help", s_PosX, LeftY,
                           Z_VALUE_TEXT_UI, s_Color);
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
    if (time_graph_.GetLayout().DrawProperties()) {
      NeedsUpdate();
    }

    m_StatsWindow.Clear();

    m_StatsWindow.AddLine(VAR_TO_STR(m_Width));
    m_StatsWindow.AddLine(VAR_TO_STR(m_Height));
    m_StatsWindow.AddLine(VAR_TO_STR(m_DesiredWorldHeight));
    m_StatsWindow.AddLine(VAR_TO_STR(m_DesiredWorldWidth));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldHeight));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldWidth));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldTopLeftX));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldTopLeftY));
    m_StatsWindow.AddLine(VAR_TO_STR(m_WorldMinWidth));
    m_StatsWindow.AddLine(VAR_TO_STR(m_MouseX));
    m_StatsWindow.AddLine(VAR_TO_STR(m_MouseY));

    /*
            m_StatsWindow.AddLog( VAR_TO_CHAR(
       GTimerManager->m_NumTimersFromPreviousSession ) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( GTimerManager->m_NumFlushedTimers ) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( Capture::GNumMessagesFromPreviousSession ) );
            m_StatsWindow.AddLog( VAR_TO_CHAR( Capture::GNumTargetFlushedEntries
       ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       Capture::GNumTargetFlushedTcpPackets ) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( GTimerManager->m_NumQueuedEntries ) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( GTimerManager->m_NumQueuedMessages) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( GTimerManager->m_NumQueuedTimers) ); m_StatsWindow.AddLog(
       VAR_TO_CHAR( m_SelectStart[0] ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       m_SelectStop[0] ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       time_graph_.m_CurrentTimeWindow ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       Capture::GMaxTimersAtOnce ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       Capture::GNumTimersAtOnce ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       Capture::GNumTargetQueuedEntries ) ); m_StatsWindow.AddLog( VAR_TO_CHAR(
       m_MousePosX ) ); m_StatsWindow.AddLog( VAR_TO_CHAR( m_MousePosY ) );*/
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

  if (m_DrawFilter) {
    RenderThreadFilterUi();
  }

  if (m_DrawMemTracker && !m_DrawHelp) {
    RenderMemTracker();
  }

  // Rendering
  glViewport(0, 0, getWidth(), getHeight());
  ImGui::Render();

  RenderBar();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderText() {
  if (!m_Picking) {
    time_graph_.DrawText();
  }
}

//-----------------------------------------------------------------------------
void ColorToFloat(Color a_Color, float* o_Float) {
  for (int i = 0; i < 4; ++i) {
    o_Float[i] = (float)a_Color[i] / 255.f;
  }
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderHelpUi() {
  float barHeight = m_Slider.GetPixelHeight();
  ImGui::SetNextWindowPos(ImVec2(0, barHeight * 1.5f));

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

  ImGui::Text("Start/Stop capture: 'X'");
  ImGui::Text("Time zoom: scroll or CTRL+right-click/drag");
  ImGui::Text("Y axis zoom: CTRL+scroll");
  ImGui::Text("Zoom last 2 seconds: 'A'");
  ImGui::Separator();
  ImGui::Text("Icons:");

  extern GLuint GTextureInjected;
  extern GLuint GTextureTimer;
  extern GLuint GTextureHelp;
  extern GLuint GTextureRecord;
  float size = m_Slider.GetPixelHeight();

  ImGui::Image((void*)(DWORD64)GTextureInjected, ImVec2(size, size),
               ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255),
               ImColor(255, 255, 255, 128));
  ImGui::SameLine();
  ImGui::Text("Injected in process");

  ImGui::Image((void*)(DWORD64)GTextureTimer, ImVec2(size, size), ImVec2(0, 0),
               ImVec2(1, 1), ImColor(255, 255, 255, 255),
               ImColor(255, 255, 255, 128));
  ImGui::SameLine();
  ImGui::Text("Capture time");

  ImGui::Image((void*)(DWORD64)GTextureRecord, ImVec2(size, size), ImVec2(0, 0),
               ImVec2(1, 1), ImColor(255, 255, 255, 255),
               ImColor(255, 255, 255, 128));
  ImGui::SameLine();
  ImGui::Text("Recording");

  ImGui::Image((void*)(DWORD64)GTextureHelp, ImVec2(size, size), ImVec2(0, 0),
               ImVec2(1, 1), ImColor(255, 255, 255, 255),
               ImColor(255, 255, 255, 128));
  ImGui::SameLine();
  ImGui::Text("Help");

  ImGui::End();

  ImGui::PopStyleColor();
}

//-----------------------------------------------------------------------------
void CaptureWindow::RenderThreadFilterUi() {
  float barHeight = m_Slider.GetPixelHeight();
  ImGui::SetNextWindowPos(ImVec2(0, barHeight * 1.5f));
  ImGui::SetNextWindowSize(ImVec2(barHeight * 50.f, barHeight * 3.f));

  ImVec4 color(1.f, 0, 0, 1.f);
  ColorToFloat(m_Slider.GetBarColor(), &color.x);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

  if (!ImGui::Begin("Thread Filter", &m_DrawHelp, ImVec2(0, 0), 1.f,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::PopStyleColor();
    ImGui::End();
    return;
  }

  static char filter[128] = "";
  ImGui::Text("Thread Filter");
  ImGui::InputText("", filter, IM_ARRAYSIZE(filter));
  GCurrentTimeGraph->SetThreadFilter(filter);

  ImGui::End();

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
void CaptureWindow::RenderBar() {
  extern GLuint GTextureTimer;
  extern GLuint GTextureRecord;

  float barHeight = m_Slider.GetPixelHeight();
  float size = barHeight;
  float h = (float)getHeight();
  float y = h - barHeight;
  float timerX = getWidth() / 2.f;

  DrawTexturedSquare(GTextureTimer, size, timerX, y);

  if (Capture::IsCapturing()) {
    DrawTexturedSquare(GTextureRecord, size, timerX - size, y);
  }

  double timeSpan = time_graph_.GetSessionTimeSpanUs();
  timerX += size;
  m_TextRenderer.AddText2D(
      absl::StrFormat("%s", GetPrettyTime(timeSpan * 0.001).c_str()).c_str(),
      (int)timerX, (int)GetTopBarTextY(), GlCanvas::Z_VALUE_TEXT_UI,
      Color(255, 255, 255, 255));

  /*m_TextRenderer.AddText2D(Format("%s",
     GetPrettyTime(time_graph_.GetCurrentTimeSpanUs()*0.001).c_str()).c_str() ,
     timerX , 100 , GlCanvas::Z_VALUE_TEXT_UI , Color(255, 255, 255, 255));*/
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
    double normInc = (double((int)((incr + unit) / unit))) * unit;
    double startMs = time_graph_.GetMinTimeUs() * 0.001;
    double normStartUs = 1000.0 * (double(int(startMs / normInc))) * normInc;

    static int pixelMargin = 2;
    int screenY = getHeight() - (int)m_Slider.GetPixelHeight() - pixelMargin;
    float dummy, worldY;
    ScreenToWorld(0, screenY, dummy, worldY);

    float height = ScreenToWorldHeight((int)GParams.m_FontSize + pixelMargin);
    float xMargin = ScreenToworldWidth(4);

    for (int i = 0; i < numTimePoints; ++i) {
      double currentMicros = normStartUs + double(i) * 1000 * normInc;
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
    GTcpClient->Send(Msg_RemoteProcess, (void*)processData.data(),
                     processData.size());
  }
}
