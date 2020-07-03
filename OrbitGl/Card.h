// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <unordered_map>

#include "Batcher.h"
#include "../OrbitCore/RingBuffer.h"
#include "../OrbitCore/Threading.h"
#include "CoreMath.h"
#include "ImGuiOrbit.h"

class GlCanvas;

//-----------------------------------------------------------------------------
class Card {
 public:
  Card();
  Card(const std::string& a_Name);
  virtual ~Card();

  enum CardType { CARD_FLOAT, CARD_2D, NUM_CARD_TYPES, INVALID };

  static std::map<int, std::string>& GetTypeMap();

  virtual void Draw(GlCanvas* a_Canvas);
  virtual void DrawImGui(GlCanvas* a_Canvas);

 public:
  Vec2 m_Pos;
  Vec2 m_Size;
  std::string m_Name;
  Color m_Color;
  bool m_Active;
  bool m_Open;
};

//-----------------------------------------------------------------------------
class FloatGraphCard : public Card {
 public:
  FloatGraphCard(const std::string& a_Name) : Card(a_Name) {}

  void AutoZoom();
  void Update(float a_Value) { m_Data.Add(a_Value); }
  void UpdateMinMax();
  void Draw(GlCanvas* a_Canvas) override;
  void DrawImGui(GlCanvas* a_Canvas) override;

 protected:
  RingBuffer<float, 512> m_Data;
  float m_Min;
  float m_Max;
};

//-----------------------------------------------------------------------------
class Vector2DGraphCard : public Card {
 public:
  Vector2DGraphCard(const std::string& a_Name) : Card(a_Name) {}

  void Update(float a_X, float a_Y) { m_Points.push_back(ImVec2(a_X, a_Y)); }
  void DrawImGui(GlCanvas* a_Canvas) override;

 protected:
  ImVector<ImVec2> m_Points;
  ImVec2 m_Min;
  ImVec2 m_Max;
};

//-----------------------------------------------------------------------------
class CardContainer : public Card {
 public:
  CardContainer();
  ~CardContainer();

  void Update(const std::string& a_Name, float a_Value);
  void Update(const std::string& a_Name, double a_Value);
  void Update(const std::string& a_Name, int a_Value);

  void Draw(GlCanvas* a_Canvas);
  void DrawImgui(GlCanvas* a_Canvas);

 protected:
  std::unordered_map<std::string, FloatGraphCard*> m_FloatCards;

  Mutex m_Mutex;
};

extern CardContainer GCardContainer;
