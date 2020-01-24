//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PluginCanvas.h"
#include "../OrbitPlugin/OrbitSDK.h"

//-----------------------------------------------------------------------------
PluginCanvas::PluginCanvas(Orbit::Plugin* a_Plugin)
    : GlCanvas(), m_Plugin(a_Plugin) {}

//-----------------------------------------------------------------------------
PluginCanvas::~PluginCanvas() {}

//-----------------------------------------------------------------------------
void PluginCanvas::OnTimer() { GlCanvas::OnTimer(); }

//-----------------------------------------------------------------------------
void PluginCanvas::ZoomAll() {}

//-----------------------------------------------------------------------------
void PluginCanvas::KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                              bool a_Alt) {
  if (!m_ImguiActive) {
    switch (a_KeyCode) {
      case 'A':
        ZoomAll();
        break;
    }
  }

  ImGuiIO& io = ImGui::GetIO();
  io.KeyCtrl = a_Ctrl;
  io.KeyShift = a_Shift;
  io.KeyAlt = a_Alt;

  Orbit_ImGui_KeyCallback(this, a_KeyCode, true);
}

//-----------------------------------------------------------------------------
void PluginCanvas::RenderUI() {
  ScopeImguiContext state(m_ImGuiContext);
  Orbit_ImGui_NewFrame(this);

  if (m_Plugin) {
    m_Plugin->Draw(ImGui::GetCurrentContext(), getWidth(), getHeight());
  }

  // Rendering
  glViewport(0, 0, getWidth(), getHeight());
  ImGui::Render();
}