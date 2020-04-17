//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "GlCanvas.h"

namespace Orbit {
class Plugin;
}

class PluginCanvas : public GlCanvas {
 public:
  explicit PluginCanvas(Orbit::Plugin* a_Plugin);
  ~PluginCanvas() override;

  void OnTimer() override;
  void ZoomAll();
  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void RenderUI() override;

  Orbit::Plugin* m_Plugin;
};
