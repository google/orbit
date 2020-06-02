// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


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
