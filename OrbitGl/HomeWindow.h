//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "GlCanvas.h"

class HomeWindow : public GlCanvas {
 public:
  HomeWindow();
  ~HomeWindow() override;

  void VariableTracingCallback(std::vector<std::string>& a_Entries);
  void Draw() override;
  void RenderUI() override;
  void RenderProcessUI();

  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void OnTimer() override;
  void Refresh() {}

 protected:
  bool m_DrawDebugDisplay;
  bool m_DrawTestUI;
  bool m_DrawLog;

  DebugWindow m_DebugWindow;
  WatchWindow m_WatchWindow;
  LogWindow m_LogWindow;
};
