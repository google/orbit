// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureWindow.h"

#include "App.h"
#include "Capture.h"
#include "EventTracer.h"
#include "GlUtils.h"
#include "absl/base/casts.h"

using orbit_client_protos::TimerInfo;

CaptureWindow::CaptureWindow() {
  GCurrentTimeGraph = &time_graph_;
  time_graph_.SetTextRenderer(&m_TextRenderer);
  time_graph_.SetPickingManager(&m_PickingManager);
  time_graph_.SetCanvas(this);
  m_DrawUI = false;
  m_DrawHelp = true;
  m_DrawFilter = false;
  m_FirstHelpDraw = true;
  m_DrawStats = false;
  m_Picking = false;
  m_WorldTopLeftX = 0;
  m_WorldTopLeftY = 0;
  m_WorldMaxY = 0;

  slider_ = std::make_shared<GlSlider>();
  vertical_slider_ = std::make_shared<GlSlider>();

  m_HoverDelayMs = 300;
  m_CanHover = false;
  m_IsHovering = false;
  ResetHoverTimer();

  slider_->SetCanvas(this);
  slider_->SetDragCallback([&](float a_Ratio) { this->OnDrag(a_Ratio); });

  vertical_slider_->SetCanvas(this);
  vertical_slider_->SetVertical();
  vertical_slider_->SetDragCallback(
      [&](float a_Ratio) { this->OnVerticalDrag(a_Ratio); });

  GOrbitApp->RegisterCaptureWindow(this);
}

CaptureWindow::~CaptureWindow() {
  if (GCurrentTimeGraph == &time_graph_) GCurrentTimeGraph = nullptr;
}

void CaptureWindow::OnTimer() { GlCanvas::OnTimer(); }

void CaptureWindow::ZoomAll() {
  time_graph_.ZoomAll();
  m_WorldTopLeftY = m_WorldMaxY;
  ResetHoverTimer();
  NeedsUpdate();
}

void CaptureWindow::UpdateWheelMomentum(float a_DeltaTime) {
  GlCanvas::UpdateWheelMomentum(a_DeltaTime);

  bool zoomWidth = true;  // TODO: !wxGetKeyState(WXK_CONTROL);
  if (zoomWidth && m_WheelMomentum != 0.f) {
    time_graph_.ZoomTime(m_WheelMomentum, m_MouseRatio);
  }
}

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
      !GOrbitApp->IsCapturing()) {
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

void CaptureWindow::LeftUp() {
  GlCanvas::LeftUp();
  NeedsRedraw();
}

void CaptureWindow::LeftDoubleClick() {
  GlCanvas::LeftDoubleClick();
  m_DoubleClicking = true;
  m_Picking = true;
}

void CaptureWindow::Pick() {
  m_Picking = true;
  NeedsRedraw();
}

void CaptureWindow::Pick(int a_X, int a_Y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  std::array<uint8_t, 4 * 1 * 1> pixels;
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);
  uint32_t value;
  std::memcpy(&value, &pixels[0], sizeof(uint32_t));
  PickingId pickId = PickingId::FromPixelValue(value);

  Capture::GSelectedTextBox = nullptr;
  Capture::GSelectedThreadId = 0;

  Pick(pickId, a_X, a_Y);

  NeedsUpdate();
}

void CaptureWindow::Pick(PickingId a_PickingID, int a_X, int a_Y) {
  PickingType type = a_PickingID.type;

  Batcher& batcher = GetBatcherById(a_PickingID.batcher_id);
  TextBox* text_box = batcher.GetTextBox(a_PickingID);
  if (text_box) {
    SelectTextBox(text_box);
  } else if (type == PickingType::kPickable) {
    m_PickingManager.Pick(a_PickingID, a_X, a_Y);
  }
}

void CaptureWindow::SelectTextBox(class TextBox* text_box) {
  if (text_box == nullptr) return;
  Capture::GSelectedTextBox = text_box;
  Capture::GSelectedThreadId = text_box->GetTimerInfo().thread_id();

  const TimerInfo& timer_info = text_box->GetTimerInfo();
  uint64_t address = timer_info.function_address();
  FindCode(address);

  if (m_DoubleClicking) {
    time_graph_.Zoom(text_box);
  }
}

void CaptureWindow::Hover(int a_X, int a_Y) {
  // 4 bytes per pixel (RGBA), 1x1 bitmap
  uint8_t pixels[1 * 1 * 4];
  glReadPixels(a_X, m_MainWindowHeight - a_Y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
               &pixels[0]);

  PickingId pickId = absl::bit_cast<PickingId>(pixels);
  Batcher& batcher = GetBatcherById(pickId.batcher_id);

  std::string tooltip = "";

  if (pickId.type == PickingType::kPickable) {
    auto pickable = GetPickingManager().GetPickableFromId(pickId).lock();
    if (pickable) {
      tooltip = pickable->GetTooltip();
    }
  } else {
    PickingUserData* user_data = batcher.GetUserData(pickId);

    if (user_data && user_data->generate_tooltip_) {
      tooltip = user_data->generate_tooltip_(pickId);
    }
  }

  GOrbitApp->SendTooltipToUi(tooltip);
}

void CaptureWindow::FindCode(uint64_t /*address*/) {}

void CaptureWindow::PreRender() {
  if (is_mouse_over_ && m_CanHover &&
      m_HoverTimer.QueryMillis() > m_HoverDelayMs) {
    m_IsHovering = true;
    m_Picking = true;
    NeedsRedraw();
  }

  m_NeedsRedraw = m_NeedsRedraw || time_graph_.IsRedrawNeeded();
}

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

void CaptureWindow::Resize(int a_Width, int a_Height) {
  GlCanvas::Resize(a_Width, a_Height);
  NeedsUpdate();
}

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

void CaptureWindow::MiddleDown(int a_X, int a_Y) {
  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_IsSelecting = true;
  m_SelectStart = Vec2(worldx, worldy);
  m_SelectStop = m_SelectStart;
}

void CaptureWindow::MiddleUp(int a_X, int a_Y) {
  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_IsSelecting = false;

  m_SelectStop = Vec2(worldx, worldy);

  NeedsRedraw();
}

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

void CaptureWindow::Pan(float a_Ratio) {
  double refTime =
      time_graph_.GetTime(static_cast<double>(m_MousePosX) / getWidth());
  time_graph_.PanTime(m_MousePosX,
                      m_MousePosX + static_cast<int>(a_Ratio * getWidth()),
                      getWidth(), refTime);
  UpdateSceneBox();
  NeedsUpdate();
}

void CaptureWindow::MouseWheelMoved(int a_X, int a_Y, int a_Delta,
                                    bool a_Ctrl) {
  if (a_Delta == 0) return;

  // Normalize and invert sign, so that delta < 0 is zoom in.
  const int delta = a_Delta < 0 ? 1 : -1;
  const auto delta_float = static_cast<float>(delta);

  if (delta < m_MinWheelDelta) m_MinWheelDelta = delta;
  if (delta > m_MaxWheelDelta) m_MaxWheelDelta = delta;

  float worldx;
  float worldy;

  ScreenToWorld(a_X, a_Y, worldx, worldy);

  bool zoomWidth = !a_Ctrl;
  if (zoomWidth) {
    m_MouseRatio = static_cast<double>(a_X) / getWidth();
    time_graph_.ZoomTime(delta_float, m_MouseRatio);
    m_WheelMomentum =
        delta_float * m_WheelMomentum < 0 ? 0 : m_WheelMomentum + delta_float;
  } else {
    float mouse_relative_y_position =
        static_cast<float>(a_Y) / static_cast<float>(getHeight());
    time_graph_.VerticalZoom(delta_float, mouse_relative_y_position);
  }

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(this, -delta);

  m_CanHover = true;

  NeedsUpdate();
}

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

void CaptureWindow::KeyPressed(unsigned int a_KeyCode, bool a_Ctrl,
                               bool a_Shift, bool a_Alt) {
  UpdateSpecialKeys(a_Ctrl, a_Shift, a_Alt);

  ScopeImguiContext state(m_ImGuiContext);

  if (!m_ImguiActive) {
    switch (a_KeyCode) {
      case ' ':
        if (!a_Shift) {
          ZoomAll();
        }
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
        if (a_Shift) {
          time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                        TimeGraph::JumpDirection::kPrevious,
                                        TimeGraph::JumpScope::kSameFunction);
        } else if (a_Alt) {
          time_graph_.JumpToNeighborBox(
              Capture::GSelectedTextBox, TimeGraph::JumpDirection::kPrevious,
              TimeGraph::JumpScope::kSameThreadSameFunction);
        } else {
          time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                        TimeGraph::JumpDirection::kPrevious,
                                        TimeGraph::JumpScope::kSameDepth);
        }
        break;
      case 20:  // Right
        if (a_Shift) {
          time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                        TimeGraph::JumpDirection::kNext,
                                        TimeGraph::JumpScope::kSameFunction);
        } else if (a_Alt) {
          time_graph_.JumpToNeighborBox(
              Capture::GSelectedTextBox, TimeGraph::JumpDirection::kNext,
              TimeGraph::JumpScope::kSameThreadSameFunction);
        } else {
          time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                        TimeGraph::JumpDirection::kNext,
                                        TimeGraph::JumpScope::kSameDepth);
        }
        break;
      case 19:  // Up
        time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                      TimeGraph::JumpDirection::kTop,
                                      TimeGraph::JumpScope::kSameThread);
        break;
      case 21:  // Down
        time_graph_.JumpToNeighborBox(Capture::GSelectedTextBox,
                                      TimeGraph::JumpDirection::kDown,
                                      TimeGraph::JumpScope::kSameThread);
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

std::vector<std::string> CaptureWindow::GetContextMenu() {
  return std::vector<std::string>{};
}

void CaptureWindow::OnContextMenu(const std::string& /*a_Action*/,
                                  int /*a_MenuIndex*/) {}

void CaptureWindow::OnCaptureStarted() {
  time_graph_.ZoomAll();
  NeedsRedraw();
}

void CaptureWindow::ResetHoverTimer() {
  m_HoverTimer.Reset();
  m_CanHover = true;
}

void CaptureWindow::Draw() {
  m_WorldMaxY =
      1.5f * ScreenToWorldHeight(static_cast<int>(slider_->GetPixelHeight()));

  if (GOrbitApp->IsCapturing()) {
    ZoomAll();
  }

  // Reset picking manager before each draw.
  m_PickingManager.Reset();

  time_graph_.Draw(this, GetPickingMode());

  if (!m_Picking && m_SelectStart[0] != m_SelectStop[0]) {
    TickType minTime = std::min(m_TimeStart, m_TimeStop);
    TickType maxTime = std::max(m_TimeStart, m_TimeStop);

    float from = time_graph_.GetWorldFromTick(minTime);
    float to = time_graph_.GetWorldFromTick(maxTime);

    float sizex = to - from;
    Vec2 pos(from, m_WorldTopLeftY - m_WorldHeight);
    Vec2 size(sizex, m_WorldHeight);

    std::string time = GetPrettyTime(TicksToDuration(minTime, maxTime));
    TextBox box(pos, size, time, Color(0, 128, 0, 128));
    box.SetTextY(m_SelectStop[1]);
    box.Draw(&ui_batcher_, m_TextRenderer, -FLT_MAX, true, true);
  }

  if (!m_Picking && !m_IsHovering) {
    RenderTimeBar();

    Vec2 pos(m_MouseX, m_WorldTopLeftY);
    ui_batcher_.AddVerticalLine(pos, -m_WorldHeight, Z_VALUE_TEXT,
                                Color(0, 255, 0, 127), PickingType::kLine);
  }
}

void CaptureWindow::DrawScreenSpace() {
  double timeSpan = time_graph_.GetCaptureTimeSpanUs();

  Color col = slider_->GetBarColor();
  float height = slider_->GetPixelHeight();
  float canvasHeight = getHeight();
  float z = GlCanvas::Z_VALUE_TEXT_UI_BG;

  const TimeGraphLayout& layout = time_graph_.GetLayout();
  float vertical_margin = layout.GetVerticalMargin();

  const auto picking_mode = GetPickingMode();

  if (timeSpan > 0) {
    double start = time_graph_.GetMinTimeUs();
    double stop = time_graph_.GetMaxTimeUs();
    double width = stop - start;
    double maxStart = timeSpan - width;
    double ratio =
        GOrbitApp->IsCapturing() ? 1 : (maxStart != 0 ? start / maxStart : 0);
    float slider_width = layout.GetSliderWidth();
    slider_->SetPixelHeight(slider_width);
    slider_->SetSliderRatio(static_cast<float>(ratio));
    slider_->SetSliderWidthRatio(static_cast<float>(width / timeSpan));
    slider_->Draw(this, picking_mode);

    float verticalRatio = m_WorldHeight / time_graph_.GetThreadTotalHeight();
    if (verticalRatio < 1.f) {
      vertical_slider_->SetPixelHeight(slider_width);
      vertical_slider_->SetSliderWidthRatio(verticalRatio);
      vertical_slider_->Draw(this, picking_mode);
      vertical_margin += slider_width;
    }
  }

  // Right vertical margin.
  time_graph_.SetVerticalMargin(vertical_margin);
  const Color kBackgroundColor(70, 70, 70, 255);
  float margin_x1 = getWidth();
  float margin_x0 = margin_x1 - vertical_margin;

  Box box(Vec2(margin_x0, 0),
          Vec2(margin_x1 - margin_x0, canvasHeight - height), z);
  ui_batcher_.AddBox(box, kBackgroundColor, PickingType::kBox);

  // Time bar background
  if (time_graph_.GetCaptureTimeSpanUs() > 0) {
    Box box(Vec2(0, time_graph_.GetLayout().GetSliderWidth()),
            Vec2(getWidth(), time_graph_.GetLayout().GetTimeBarHeight()),
            GlCanvas::Z_VALUE_TEXT_UI_BG);
    ui_batcher_.AddBox(box, Color(70, 70, 70, 200), PickingType::kBox);
  }
}

void CaptureWindow::OnDrag(float a_Ratio) {
  time_graph_.OnDrag(a_Ratio);
  NeedsUpdate();
}

void CaptureWindow::OnVerticalDrag(float a_Ratio) {
  float min = m_WorldMaxY;
  float max = m_WorldHeight - time_graph_.GetThreadTotalHeight();
  float range = max - min;
  m_WorldTopLeftY = min + a_Ratio * range;
  NeedsUpdate();
}

void CaptureWindow::UpdateVerticalSlider() {
  float min = m_WorldMaxY;
  float max = m_WorldHeight - time_graph_.GetThreadTotalHeight();
  float ratio = (m_WorldTopLeftY - min) / (max - min);
  vertical_slider_->SetSliderRatio(ratio);
}

void CaptureWindow::ToggleDrawHelp() {
  m_DrawHelp = !m_DrawHelp;
  NeedsRedraw();
}

Batcher& CaptureWindow::GetBatcherById(BatcherId batcher_id) {
  switch (batcher_id) {
    case BatcherId::kTimeGraph:
      return time_graph_.GetBatcher();
    case BatcherId::kUi:
      return ui_batcher_;
  }

  UNREACHABLE();
}

[[nodiscard]] PickingMode CaptureWindow::GetPickingMode() {
  PickingMode picking_mode = PickingMode::kNone;
  if (m_Picking) {
    picking_mode = PickingMode::kClick;
  }
  if (m_IsHovering) {
    picking_mode = PickingMode::kHover;
  }

  return picking_mode;
}

void CaptureWindow::NeedsUpdate() {
  time_graph_.NeedsUpdate();
  m_NeedsRedraw = true;
}

float CaptureWindow::GetTopBarTextY() {
  return slider_->GetPixelHeight() * 0.5f +
         m_TextRenderer.GetStringHeight("FpjT_H") * 0.5f;
}

template <class T>
static std::string VariableToString(std::string_view name, const T& value) {
  std::stringstream string_stream{};
  string_stream << name << " = " << value;
  return string_stream.str();
}

void CaptureWindow::RenderUI() {
  // Don't draw ImGui when picking.
  if (m_Picking || m_IsHovering) {
    return;
  }

  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_NewFrame(this);

#define VAR_TO_STR(var) VariableToString(#var, var)

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
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GSelectedFunctionsMap.size()));
    m_StatsWindow.AddLine(VAR_TO_STR(Capture::GVisibleFunctionsMap.size()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetNumDrawnTextBoxes()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetNumTimers()));
    m_StatsWindow.AddLine(VAR_TO_STR(time_graph_.GetThreadTotalHeight()));

#ifndef WIN32
    m_StatsWindow.AddLine(
        VAR_TO_STR(GEventTracer.GetEventBuffer().GetCallstacks().size()));
    m_StatsWindow.AddLine(
        VAR_TO_STR(GEventTracer.GetEventBuffer().GetNumEvents()));
#endif

    m_StatsWindow.Draw("Capture Stats", &m_DrawStats);
  }

#undef VAR_TO_STR

  if (m_DrawHelp) {
    RenderHelpUi();

    if (m_FirstHelpDraw) {
      // Redraw so that Imgui resizes the
      // window properly on first draw
      NeedsRedraw();
      m_FirstHelpDraw = false;
    }
  }

  // Rendering
  glViewport(0, 0, getWidth(), getHeight());
  ImGui::Render();
}

void CaptureWindow::RenderText() {
  if (!m_Picking) {
    time_graph_.DrawText(this);
  }
}

void ColorToFloat(Color a_Color, float* o_Float) {
  for (size_t i = 0; i < 4; ++i) {
    o_Float[i] = a_Color[i] / 255.f;
  }
}

void CaptureWindow::RenderHelpUi() {
  constexpr float kYOffset = 8.f;
  ImGui::SetNextWindowPos(ImVec2(0, kYOffset));

  ImVec4 color(1.f, 0, 0, 1.f);
  ColorToFloat(slider_->GetBarColor(), &color.x);
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

ImTextureID TextureId(uint64_t id) {
  return absl::bit_cast<ImTextureID>(static_cast<uintptr_t>(id));
}

bool IconButton(uint64_t texture_id, const char* tooltip, ImVec2 size,
                bool enabled) {
  ImTextureID imgui_texture_id = TextureId(texture_id);

  if (!enabled) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.4f);
  }

  bool clicked = ImGui::ImageButton(imgui_texture_id, size);

  if (tooltip != nullptr &&
      ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
    ImGui::SetTooltip("%s", tooltip);
  }

  if (!enabled) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
  }

  return clicked;
}

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

void CaptureWindow::RenderTimeBar() {
  static int numTimePoints = 10;

  if (time_graph_.GetCaptureTimeSpanUs() > 0) {
    const float time_bar_height = time_graph_.GetLayout().GetTimeBarHeight();

    double millis = time_graph_.GetCurrentTimeSpanUs() * 0.001;
    double incr = millis / float(numTimePoints - 1);
    double unit = GetIncrementMs(incr);
    double normInc = static_cast<int>((incr + unit) / unit) * unit;
    double startMs = time_graph_.GetMinTimeUs() * 0.001;
    double normStartUs = 1000.0 * static_cast<int>(startMs / normInc) * normInc;

    static int pixelMargin = 2;
    int screenY = getHeight() - static_cast<int>(time_bar_height) - pixelMargin;
    float dummy, worldY;
    ScreenToWorld(0, screenY, dummy, worldY);

    float height =
        ScreenToWorldHeight(static_cast<int>(GParams.font_size) + pixelMargin);
    float xMargin = ScreenToworldWidth(4);

    for (int i = 0; i < numTimePoints; ++i) {
      double current_micros = normStartUs + i * 1000 * normInc;
      if (current_micros < 0) continue;

      std::string text = GetPrettyTime(absl::Microseconds(current_micros));
      float worldX = time_graph_.GetWorldFromUs(current_micros);
      m_TextRenderer.AddText(text.c_str(), worldX + xMargin, worldY,
                             GlCanvas::Z_VALUE_TEXT_UI,
                             Color(255, 255, 255, 255));

      Vec2 pos(worldX, worldY);
      ui_batcher_.AddVerticalLine(pos, height, GlCanvas::Z_VALUE_UI,
                                  Color(255, 255, 255, 255),
                                  PickingType::kLine);
    }
  }
}

void CaptureWindow::Initialize() { GlCanvas::Initialize(); }
