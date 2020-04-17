//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "GlCanvas.h"

class ImmediateWindow : public GlCanvas {
 public:
  ImmediateWindow();
  ~ImmediateWindow() override;

  void Draw() override;
  void RenderUI() override;
  void RenderProcessUI();

  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void OnTimer() override;

 protected:
  VizWindow m_ImmediateWindow;
};
