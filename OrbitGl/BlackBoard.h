/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#pragma once

#include "GlCanvas.h"

class BlackBoard : public GlCanvas {
 public:
  BlackBoard();
  ~BlackBoard() override;

  void OnTimer() override;
  void ZoomAll();
  void KeyPressed(unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift,
                  bool a_Alt) override;
  void RenderUI() override;
  bool GetNeedsRedraw() const override;

  // Demo HACK!
  static void AddPos(float x, float y);
};
