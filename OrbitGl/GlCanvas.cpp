//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlCanvas.h"

#include <string>
#include <vector>

#include "App.h"
#include "Capture.h"
#include "Card.h"
#include "Core.h"
#include "GlUtils.h"
#include "ImGuiOrbit.h"
#include "Log.h"
#include "OpenGl.h"
#include "Pdb.h"
#include "SamplingProfiler.h"
#include "TcpServer.h"
#include "TextBox.h"
#include "TextRenderer.h"
#include "VariableTracing.h"
#include "absl/strings/str_format.h"

RingBuffer<float, 512> GDeltaTimeBuffer;

float GlCanvas::Z_VALUE_UI = 0.00f;
float GlCanvas::Z_VALUE_TEXT_UI = 0.0f;
float GlCanvas::Z_VALUE_TEXT_UI_BG = -0.005f;
float GlCanvas::Z_VALUE_TEXT = -0.01f;
float GlCanvas::Z_VALUE_CONTEXT_SWITCH = -0.015f;
float GlCanvas::Z_VALUE_EVENT = -0.015f;
float GlCanvas::Z_VALUE_BOX_ACTIVE = -0.02f;
float GlCanvas::Z_VALUE_BOX_INACTIVE = -0.03f;

//-----------------------------------------------------------------------------
void ClearCaptureData() {
  if (GCurrentTimeGraph) {
    GCurrentTimeGraph->Clear();
  }

  GOrbitApp->FireRefreshCallbacks(DataViewType::LIVE_FUNCTIONS);
}

//-----------------------------------------------------------------------------
GlCanvas::GlCanvas() {
  m_TextRenderer.SetCanvas(this);

  m_DesiredWorldWidth = 1600.f;
  m_DesiredWorldHeight = 1000.f;
  m_WorldWidth = m_DesiredWorldWidth;
  m_WorldHeight = m_DesiredWorldHeight;
  m_WorldTopLeftX = -5.f;
  m_WorldTopLeftY = 5.f;
  m_WorldMinWidth = 1.f;
  m_SelectStart = Vec2(0.f, 0.f);
  m_SelectStop = Vec2(0.f, 0.f);
  m_TimeStart = 0.0;
  m_TimeStop = 0.0;
  m_IsSelecting = false;
  m_Picking = false;
  m_DoubleClicking = false;
  m_ControlKey = false;
  m_ShiftKey = false;
  m_AltKey = false;
  m_NeedsRedraw = true;

  m_MinWheelDelta = INT_MAX;
  m_MaxWheelDelta = INT_MIN;
  m_WheelMomentum = 0.f;
  m_DeltaTime = 0.0f;
  m_DeltaTimeMs = 0;
  m_MouseRatio = 0.0;
  m_DrawUI = true;
  m_ImguiActive = false;
  m_BackgroundColor = Vec4(70.f / 255.f, 70.f / 255.f, 70.f / 255.f, 1.0f);

  static int counter = 0;
  m_ID = counter++;

  m_UpdateTimer.Start();

  Capture::GClearCaptureDataFunc = ClearCaptureData;

  // SetCursor(wxCURSOR_BLANK);

  UpdateSceneBox();

  m_ImGuiContext = ImGui::CreateContext();
  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_Init();
}

//-----------------------------------------------------------------------------
GlCanvas::~GlCanvas() {
  ImGui::DestroyContext(m_ImGuiContext);
  ScopeImguiContext state(m_ImGuiContext);
}

//-----------------------------------------------------------------------------
void GlCanvas::Initialize() {
  static bool firstInit = true;
  if (firstInit) {
    // glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    CheckGlError();
    if (GLEW_OK != err) {
      /* Problem: glewInit failed, something is seriously wrong. */
      ORBIT_LOGV(glewGetErrorString(err));
      exit(EXIT_FAILURE);
    }
    std::string glew =
        absl::StrFormat("Using GLEW %s\n", (char*)glewGetString(GLEW_VERSION));
    PRINT_VAR(glew);
    firstInit = false;
  }
}

//-----------------------------------------------------------------------------
void GlCanvas::MouseMoved(int a_X, int a_Y, bool a_Left, bool /*a_Right*/,
                          bool /*a_Middle*/) {
  int mousex = a_X;
  int mousey = a_Y;

  float worldx, worldy;
  ScreenToWorld(mousex, mousey, worldx, worldy);

  m_MouseX = worldx;
  m_MouseY = worldy;
  m_MousePosX = mousex;
  m_MousePosY = mousey;

  // Pan
  if (a_Left && !m_ImguiActive) {
    m_WorldTopLeftX =
        m_WorldClickX - (float)mousex / (float)getWidth() * m_WorldWidth;
    m_WorldTopLeftY =
        m_WorldClickY + (float)mousey / (float)getHeight() * m_WorldHeight;
    UpdateSceneBox();
  }

  if (m_IsSelecting) {
    m_SelectStop = Vec2(worldx, worldy);
  }

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::LeftDown(int a_X, int a_Y) {
  // Store world clicked pos for panning
  ScreenToWorld(a_X, a_Y, m_WorldClickX, m_WorldClickY);
  m_ScreenClickX = a_X;
  m_ScreenClickY = a_Y;
  m_IsSelecting = false;

  Orbit_ImGui_MouseButtonCallback(this, 0, true);

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::MouseWheelMoved(int a_X, int a_Y, int a_Delta, bool a_Ctrl) {
  // Normalize and invert sign, so that delta < 0 is zoom in.
  int delta = a_Delta < 0 ? 1 : -1;

  if (delta < m_MinWheelDelta) m_MinWheelDelta = delta;
  if (delta > m_MaxWheelDelta) m_MaxWheelDelta = delta;

  float mousex = (float)a_X;
  float mousey = (float)a_Y;

  float worldx;
  float worldy;

  ScreenToWorld(a_X, a_Y, worldx, worldy);
  m_MouseRatio = (double)mousex / (double)getWidth();

  static float zoomRatio = 0.1f;
  bool zoomWidth = !a_Ctrl;

  if (zoomWidth) {
    m_WheelMomentum =
        delta * m_WheelMomentum < 0 ? 0.f : m_WheelMomentum + (float)delta;
  } else {
    float zoomInc = zoomRatio * m_DesiredWorldHeight;
    m_DesiredWorldHeight += delta * zoomInc;
    m_WorldTopLeftY =
        worldy + (float)mousey / (float)getHeight() * m_DesiredWorldHeight;
    UpdateSceneBox();
  }

  // Use the original sign of a_Delta here.
  Orbit_ImGui_ScrollCallback(this, -delta);

  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::LeftUp() {
  m_PickingManager.Release();
  Orbit_ImGui_MouseButtonCallback(this, 0, false);
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::LeftDoubleClick() {
  ScopeImguiContext state(m_ImGuiContext);
  m_DoubleClicking = true;
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::RightDown(int a_X, int a_Y) {
  float worldx, worldy;
  ScreenToWorld(a_X, a_Y, worldx, worldy);

  m_SelectStart = m_SelectStop = Vec2(worldx, worldy);
  m_IsSelecting = true;

  Orbit_ImGui_MouseButtonCallback(this, 1, true);
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
bool GlCanvas::RightUp() {
  Orbit_ImGui_MouseButtonCallback(this, 1, false);
  m_IsSelecting = true;
  NeedsRedraw();
  return false;
}

//-----------------------------------------------------------------------------
void GlCanvas::mouseLeftWindow() {}

//-----------------------------------------------------------------------------
void GlCanvas::CharEvent(unsigned int a_Char) {
  Orbit_ImGui_CharCallback(this, a_Char);
}

//-----------------------------------------------------------------------------
void GlCanvas::KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                          bool a_Alt) {
  UpdateSpecialKeys(a_Ctrl, a_Shift, a_Alt);
  ScopeImguiContext state(m_ImGuiContext);
  ImGuiIO& io = ImGui::GetIO();
  io.KeyCtrl = a_Ctrl;
  io.KeyShift = a_Shift;
  io.KeyAlt = a_Alt;

  Orbit_ImGui_KeyCallback(this, a_KeyCode, true);
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::KeyReleased(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                           bool a_Alt) {
  UpdateSpecialKeys(a_Ctrl, a_Shift, a_Alt);
  Orbit_ImGui_KeyCallback(this, a_KeyCode, false);
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::UpdateSpecialKeys(bool a_Ctrl, bool a_Shift, bool a_Alt) {
  m_ControlKey = a_Ctrl;
  m_ShiftKey = a_Shift;
  m_AltKey = a_Alt;
}

//-----------------------------------------------------------------------------
bool GlCanvas::ControlPressed() { return m_ControlKey; }

//-----------------------------------------------------------------------------
bool GlCanvas::ShiftPressed() { return m_ShiftKey; }

//-----------------------------------------------------------------------------
bool GlCanvas::AltPressed() { return m_AltKey; }

//-----------------------------------------------------------------------------
void GlCanvas::UpdateWheelMomentum(float a_DeltaTime) {
  float sign = m_WheelMomentum > 0 ? 1.f : -1.f;
  static float inc = 15;
  float newMomentum = m_WheelMomentum - sign * inc * a_DeltaTime;
  m_WheelMomentum = newMomentum * m_WheelMomentum > 0.f ? newMomentum : 0.f;
}

//-----------------------------------------------------------------------------
void GlCanvas::OnTimer() {
  m_UpdateTimer.Stop();
  m_DeltaTime = (float)m_UpdateTimer.ElapsedSeconds();
  m_DeltaTimeMs = m_UpdateTimer.ElapsedMillis();
  m_UpdateTimer.Start();
  UpdateWheelMomentum(m_DeltaTime);
}

/** Inits the OpenGL viewport for drawing in 3D. */
void GlCanvas::prepare3DViewport(int topleft_x, int topleft_y,
                                 int bottomrigth_x, int bottomrigth_y) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Black Background
  glClearDepth(1.0f);                    // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);               // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);                // The Type Of Depth Testing To Do
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_COLOR_MATERIAL);

  glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x,
             bottomrigth_y - topleft_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float ratio_w_h =
      (float)(bottomrigth_x - topleft_x) / (float)(bottomrigth_y - topleft_y);
  gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/,
                 200 /*clip far*/);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/** Inits the OpenGL viewport for drawing in 2D. */
void GlCanvas::prepare2DViewport(int topleft_x, int topleft_y,
                                 int bottomrigth_x, int bottomrigth_y) {
  glClearColor(m_BackgroundColor[0], m_BackgroundColor[1], m_BackgroundColor[2],
               m_BackgroundColor[3]);
  if (m_Picking) glClearColor(0.f, 0.f, 0.f, 0.f);

  // glEnable(GL_DEBUG_OUTPUT);

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_COLOR_MATERIAL);
  m_Picking ? glDisable(GL_BLEND) : glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);  // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);   // The Type Of Depth Testing To Do
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x,
             bottomrigth_y - topleft_y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Text renderer
  // mat4_set_orthographic( &m_TextRenderer.GetProjection(), topleft_x,
  // bottomrigth_x, topleft_y, bottomrigth_y, -1, 1);

  m_WorldWidth = m_DesiredWorldWidth;
  m_WorldHeight = m_DesiredWorldHeight;

  UpdateSceneBox();

  Capture::DisplayStats();

  // extern TcpServer* GTcpServer;
  // TRACE_VAR( m_ScreenClickY );
  // TRACE_VAR( GTcpServer->HasConnection( Server_LocalTarget ) );
  // TRACE_VAR( GTcpServer->HasConnection( Server_RemoteTarget ) );
  // TRACE_VAR( GTcpServer->HasConnection( Server_RemoteServer ) );
  // TRACE_VAR( GPdbDbg->GetFunctions().size() );
  // TRACE_VAR( GPdbDbg->GetTypes().size() );
  // TRACE_VAR( GPdbDbg->GetGlobals().size() );
  // TRACE_VAR( GPdbDbg->GetLoadTime() );
  // TRACE_VAR( m_WorldTopLeftX );
  // TRACE_VAR( m_WorldTopLeftY );
  // TRACE_VAR( m_WorldWidth );
  // TRACE_VAR( m_WorldHeight );
  // TRACE_VAR( m_DesiredWorldWidth );
  // TRACE_VAR( m_DesiredWorldHeight );/**/
  // TRACE_VAR( GTimerManager->m_NumQueuedEntries );
  // TRACE_VAR( GTimerManager->m_NumQueuedMessages );
  // TRACE_VAR( GTimerManager->m_NumQueuedTimers );
  // TRACE_VAR( GPdbDbg->GetHModule() );
  // TRACE_VAR( m_MinWheelDelta );
  // TRACE_VAR( m_MaxWheelDelta );
  // TRACE_VAR( m_WheelMomentum );
  // TRACE_VAR( m_DeltaTime );
  // TRACE_VAR( GDeltaTimeBuffer.Size() );
  // TRACE_VAR( GDeltaTimeBuffer[0] );
  // TRACE_VAR( GDeltaTimeBuffer[512] );
  // TRACE_VAR( Capture::GNumContextSwitches );

  if (m_WorldWidth <= 0) m_WorldWidth = 1.f;
  if (m_WorldHeight <= 0) m_WorldHeight = 1.f;

  gluOrtho2D(m_WorldTopLeftX, m_WorldTopLeftX + m_WorldWidth,
             m_WorldTopLeftY - m_WorldHeight, m_WorldTopLeftY);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

//-----------------------------------------------------------------------------
void GlCanvas::prepareScreenSpaceViewport() {
  glViewport(0, 0, getWidth(), getHeight());
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, getWidth(), 0, getHeight(), -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

//-----------------------------------------------------------------------------
void GlCanvas::ScreenToWorld(int x, int y, float& wx, float& wy) const {
  wx = m_WorldTopLeftX + ((float)x / (float)getWidth()) * m_WorldWidth;
  wy = m_WorldTopLeftY - ((float)y / (float)getHeight()) * m_WorldHeight;
}

//-----------------------------------------------------------------------------
void GlCanvas::WorldToScreen(float wx, float wy, int& x, int& y) const {
  x = (int)((wx - m_WorldTopLeftX) / m_WorldWidth) * getWidth();

  float bottomY = m_WorldTopLeftY - m_WorldHeight;
  y = (int)((1.f - ((wy - bottomY) / m_WorldHeight)) * getHeight());
}

//-----------------------------------------------------------------------------
int GlCanvas::WorldToScreenHeight(float a_Height) const {
  return (int)((a_Height / m_WorldHeight) * (float)getHeight());
}

//-----------------------------------------------------------------------------
float GlCanvas::ScreenToWorldHeight(int a_Height) const {
  return ((float)a_Height / (float)getHeight()) * m_WorldHeight;
}

//-----------------------------------------------------------------------------
float GlCanvas::ScreenToworldWidth(int a_Width) const {
  return ((float)a_Width / (float)getWidth()) * m_WorldWidth;
}

//-----------------------------------------------------------------------------
void GlCanvas::drawSquareGrid(float size, float delta) {
  int halfIncrements = ((int)(size * 0.5f / delta));
  float halfSize = delta * halfIncrements;

  glBegin(GL_LINES);

  glColor4f(0.5f, 0.5f, 0.5f, 1);
  for (float x = -halfSize; x <= halfSize; x += delta) {
    for (float y = -halfSize; y <= halfSize; y += delta) {
      glVertex3f(-x, y, 0);
      glVertex3f(x, y, 0);
      glVertex3f(x, -y, 0);
      glVertex3f(x, y, 0);
    }
  }

  glColor4f(1, 0, 0, 1);
  glVertex3f(-halfSize, 0, 0);
  glVertex3f(halfSize, 0, 0);

  glColor4f(0, 1, 0, 1);
  glVertex3f(0, -halfSize, 0);
  glVertex3f(0, halfSize, 0);

  glEnd();
}

//-----------------------------------------------------------------------------
int GlCanvas::getWidth() const { return m_Width; }

//-----------------------------------------------------------------------------
int GlCanvas::getHeight() const { return m_Height; }

//-----------------------------------------------------------------------------
void GlCanvas::Render(int a_Width, int a_Height) {
  m_Width = a_Width;
  m_Height = a_Height;

  if (!m_NeedsRedraw) {
    return;
  }

  m_NeedsRedraw = false;

  ScopeImguiContext state(m_ImGuiContext);

  Timer timer;
  timer.Start();

  prepare2DViewport(0, 0, getWidth(), getHeight());
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  // Clear text renderer
  m_TextRenderer.Init();
  m_TextRenderer.Clear();

  Draw();

  prepareScreenSpaceViewport();
  DrawScreenSpace();

  RenderUI();

  m_TextRenderer.Display();
  RenderText();

  glFlush();

  timer.Stop();

  m_ImguiActive = ImGui::IsAnyItemActive();

  PostRender();

  m_Picking = false;
  m_DoubleClicking = false;
}

//-----------------------------------------------------------------------------
void GlCanvas::Resize(int a_Width, int a_Height) {
  m_Width = a_Width;
  m_Height = a_Height;
  NeedsRedraw();
}

//-----------------------------------------------------------------------------
void GlCanvas::RenderUI() {
  if (!m_DrawUI) return;

  ImGui::PushFont(GOrbitImguiFont);

  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_NewFrame(this);
  RenderSamplingUI();
  glViewport(0, 0, getWidth(), getHeight());
  ImGui::Render();

  ImGui::PopFont();
}

//-----------------------------------------------------------------------------
void GlCanvas::RenderSamplingUI() {
  if (!Capture::GIsSampling) return;

  ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);
  bool opened = true;
  ImGui::Begin("Sampling target application", &opened);

  //// Typically we would use ImVec2(-1.0f,0.0f) to use all available width, or
  /// ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses
  /// ItemWidth.
  // ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
  ////ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
  // ImGui::Text("Progress Bar");

  float curTime = Capture::GSamplingProfiler->GetSampleTime();
  float totTime = Capture::GSamplingProfiler->GetSampleTimeTotal();
  std::string prog = absl::StrFormat("%f/%f", curTime, totTime);
  ImGui::ProgressBar(std::min(curTime / totTime, 1.f), ImVec2(0.f, 0.f),
                     prog.c_str());

  ImGui::End();
}

//-----------------------------------------------------------------------------
void GlCanvas::UpdateSceneBox() {
  Vec2 pos;
  pos[0] = m_WorldTopLeftX;
  pos[1] = m_WorldTopLeftY - m_WorldHeight;

  Vec2 size(m_WorldWidth, m_WorldHeight);

  m_SceneBox = TextBox(pos, size);
}

//-----------------------------------------------------------------------------
Vec2 GlCanvas::ToScreenSpace(const Vec2& a_Point) {
  float x = (a_Point[0] / m_WorldMinWidth) * (float)m_Width;
  float y = (a_Point[1] / m_WorldHeight) * (float)m_Height;

  return Vec2(x, y);
}

//-----------------------------------------------------------------------------
Vec2 GlCanvas::ToWorldSpace(const Vec2& a_Point) {
  float x = (a_Point[0] / (float)m_Width) * m_WorldMinWidth;
  float y = (a_Point[1] / (float)m_Height) * m_WorldHeight;

  return Vec2(x, y);
}

//-----------------------------------------------------------------------------
void GlCanvas::AddText(const char* a_Text, float a_X, float a_Y, float a_Z,
                       const Color& a_Color, float a_MaxSize,
                       bool a_RightJustified) {
  m_TextRenderer.AddText(a_Text, a_X, a_Y, a_Z, a_Color, a_MaxSize,
                         a_RightJustified);
}

//-----------------------------------------------------------------------------
int GlCanvas::AddText2D(const char* a_Text, int a_X, int a_Y, float a_Z,
                        const Color& a_Color, float a_MaxSize,
                        bool a_RightJustified, bool a_InvertY) {
  return m_TextRenderer.AddText2D(a_Text, a_X, a_Y, a_Z, a_Color, a_MaxSize,
                                  a_RightJustified, a_InvertY);
}
