//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CoreMath.h"
#include "PickingManager.h"
#include "TextBox.h"

class GlCanvas;
class TimeGraph;

//-----------------------------------------------------------------------------
class Track : public Pickable {
 public:
  Track();
  virtual ~Track() = default;

  // Pickable
  void Draw(GlCanvas* a_Canvas, bool a_Picking) override;
  void OnPick(int a_X, int a_Y) override;
  void OnRelease() override;
  void OnDrag(int a_X, int a_Y) override;
  bool Draggable() override { return true; }
  bool Movable() override { return true; }

  virtual float GetHeight() const { return 0.f; };

  bool IsMoving() const { return m_Moving; }
  Vec2 GetMoveDelta() const {
    return m_Moving ? m_MousePos[1] - m_MousePos[0] : Vec2(0, 0);
  }
  void SetName(const std::string& a_Name) { m_Name = a_Name; }
  const std::string& GetName() const { return m_Name; }
  void SetTimeGraph(TimeGraph* a_TimeGraph) { m_TimeGraph = a_TimeGraph; }
  void SetPos(float a_X, float a_Y);
  Vec2 GetPos() const { return m_Pos; }
  void SetSize(float a_SizeX, float a_SizeY);
  void SetID(uint32_t a_ID) { m_ID = a_ID; }
  uint32_t GetID() const { return m_ID; }
  void SetColor(Color a_Color) { m_Color = a_Color; }

 protected:
  GlCanvas* m_Canvas;
  TimeGraph* m_TimeGraph;
  Vec2 m_Pos;
  Vec2 m_Size;
  Vec2 m_MousePos[2];
  Vec2 m_PickingOffset;
  bool m_Picked;
  bool m_Moving;
  std::string m_Name;
  uint32_t m_ID;
  Color m_Color;
};
