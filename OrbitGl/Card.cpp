// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Card.h"

#include "Batcher.h"
#include "GlCanvas.h"
#include "ImGuiOrbit.h"
#include "absl/strings/str_format.h"

CardContainer GCardContainer;

//-----------------------------------------------------------------------------
Card::Card()
    : m_Pos(500, 0),
      m_Size(512, 64),
      m_Color(255, 0, 255, 32),
      m_Active(true),
      m_Open(true) {}

//-----------------------------------------------------------------------------
Card::Card(const std::string& a_Name) : Card() { m_Name = a_Name; }

//-----------------------------------------------------------------------------
Card::~Card() {}

//-----------------------------------------------------------------------------
std::map<int, std::string>& Card::GetTypeMap() {
  static std::map<int, std::string> typeMap;
  if (typeMap.size() == 0) {
    typeMap[CARD_FLOAT] = "Float Card";
    typeMap[CARD_2D] = "2D Card";
  }
  return typeMap;
}

//-----------------------------------------------------------------------------
void Card::Draw(GlCanvas* canvas) {
  if (!m_Active) return;
  Batcher* batcher = canvas->GetBatcher();

  Box box(m_Pos, m_Size, 0.f);
  batcher->AddBox(box, m_Color, PickingID::BOX);
}

//-----------------------------------------------------------------------------
void Card::DrawImGui(GlCanvas*) {}

//-----------------------------------------------------------------------------
CardContainer::CardContainer() {}

//-----------------------------------------------------------------------------
CardContainer::~CardContainer() {
  for (auto& it : m_FloatCards) {
    delete it.second;
  }
  m_FloatCards.clear();
}

//-----------------------------------------------------------------------------
void CardContainer::Update(const std::string& a_Name, float a_Value) {
  ScopeLock lock(m_Mutex);

  FloatGraphCard* floatCard = m_FloatCards[a_Name];

  if (floatCard == nullptr) {
    m_FloatCards[a_Name] = floatCard = new FloatGraphCard(a_Name);
  }

  floatCard->Update(a_Value);
}

//-----------------------------------------------------------------------------
void CardContainer::Update(const std::string& /*a_Name*/, double /*a_Value*/) {
  ScopeLock lock(m_Mutex);
  // m_DoubleCards[a_Name].Update(a_Value);
}

//-----------------------------------------------------------------------------
void CardContainer::Update(const std::string& /*a_Name*/, int /*a_Value*/) {
  ScopeLock lock(m_Mutex);
  // m_IntCards[a_Name].Update(a_Value);
}

//-----------------------------------------------------------------------------
void CardContainer::Draw(GlCanvas* a_Canvas) {
  if (!m_Active) return;

  static float Margin = 10.f;
  float YPos = a_Canvas->getHeight();

  for (auto& it : m_FloatCards) {
    FloatGraphCard& card = *it.second;
    YPos -= card.m_Size[1] + Margin;
    card.m_Pos[1] = YPos;
    card.m_Pos[0] = m_Pos[0] + Margin;
    card.Draw(a_Canvas);
  }
}

//-----------------------------------------------------------------------------
void CardContainer::DrawImgui(GlCanvas* a_Canvas) {
  for (auto& card : m_FloatCards) {
    card.second->DrawImGui(a_Canvas);
  }
}

//-----------------------------------------------------------------------------
void FloatGraphCard::Draw(GlCanvas* a_Canvas) {
  if (!m_Active) return;
  Batcher* batcher = a_Canvas->GetBatcher();

  Card::Draw(a_Canvas);

  UpdateMinMax();

  float xIncrement = m_Size[0] / m_Data.Size();
  float YRange = m_Max - m_Min;
  float YRangeInv = 1.f / YRange;
  static float textHeight = 15.f;
  float YGraphSize = m_Size[1] - textHeight;

  for (size_t i = 0; i < m_Data.Size() - 1; ++i) {
    float x0 = m_Pos[0] + i * xIncrement;
    float x1 = x0 + xIncrement;
    float y0 =
        m_Pos[1] + textHeight + (m_Data[i] - m_Min) * YRangeInv * YGraphSize;
    float y1 = m_Pos[1] + textHeight +
               (m_Data[i + 1] - m_Min) * YRangeInv * YGraphSize;

    batcher->AddLine(Vec2(x0, y0), Vec2(x1, y1), 0.f, Color(255, 255, 255, 255), PickingID::LINE);
  }

  Color col(255, 255, 255, 255);
  std::string cardValue = absl::StrFormat(
      "%s: %s  min(%s) max(%s)", m_Name.c_str(),
      std::to_string(m_Data.Latest()).c_str(), std::to_string(m_Min).c_str(),
      std::to_string(m_Max).c_str());
  a_Canvas->GetTextRenderer().AddText2D(
      cardValue.c_str(), static_cast<int>(m_Pos[0]), static_cast<int>(m_Pos[1]),
      GlCanvas::Z_VALUE_TEXT, col, -1.f, false, false);
}

//-----------------------------------------------------------------------------
void FloatGraphCard::DrawImGui(GlCanvas*) {
  UpdateMinMax();

  bool p_opened = true;
  ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin(m_Name.c_str(), &p_opened);

  bool copy = ImGui::Button("Copy");
  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false,
                    ImGuiWindowFlags_HorizontalScrollbar);
  if (copy) ImGui::LogToClipboard();

  ImGui::PlotLines("Lines", reinterpret_cast<float*>(&m_Data), m_Data.Size(),
                   m_Data.GetCurrentIndex(), "avg 0.0", m_Min, m_Max,
                   ImVec2(0, 80));

  ImGui::EndChild();
  ImGui::End();
}

//-----------------------------------------------------------------------------
void FloatGraphCard::UpdateMinMax() {
  m_Min = FLT_MAX;
  m_Max = FLT_MIN;

  for (size_t i = 0; i < m_Data.Size(); ++i) {
    float val = m_Data[i];
    if (val < m_Min) m_Min = val;
    if (val > m_Max) m_Max = val;
  }
}

//-----------------------------------------------------------------------------
void Vector2DGraphCard::DrawImGui(GlCanvas*) {
  m_Min = ImVec2(FLT_MAX, FLT_MAX);
  m_Max = ImVec2(-FLT_MAX, -FLT_MAX);
  for (ImVec2 p : m_Points) {
    if (p.x < m_Min.x) m_Min.x = p.x;
    if (p.y < m_Min.y) m_Min.y = p.y;
    if (p.x > m_Max.x) m_Max.x = p.x;
    if (p.y > m_Max.y) m_Max.y = p.y;
  }
  ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin(m_Name.c_str(), &m_Active)) {
    ImGui::End();
    return;
  }

  // Tip: If you do a lot of custom rendering, you probably want to use your own
  // geometrical types and benefit of overloaded operators, etc. Define
  // IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between
  // your types and ImVec2/ImVec4. ImGui defines overloaded operators but they
  // are internal to imgui.cpp and not exposed outside (to avoid messing with
  // your types) In this example we are not using the maths operators!
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImGui::Separator();
  {
    // ImGui::Text("Canvas example");
    if (ImGui::Button("Clear")) m_Points.clear();
    /*if (points.Size >= 2)
    { ImGui::SameLine();
        if (ImGui::Button("Undo"))
        { points.pop_back(); points.pop_back();
        }
    }*/
    // ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

    // Here we are using InvisibleButton() as a convenience to 1) advance the
    // cursor and 2) allows us to use IsItemHovered() However you can draw
    // directly and poll mouse/keyboard by yourself. You can manipulate the
    // cursor using GetCursorPos() and SetCursorPos(). If you only use the
    // ImDrawList API, you can notify the owner window of its extends by using
    // SetCursorPos(max).
    ImVec2 canvas_pos =
        ImGui::GetCursorScreenPos();  // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size =
        ImGui::GetContentRegionAvail();  // Resize canvas to what's available
    if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
    if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
    draw_list->AddRectFilledMultiColor(
        canvas_pos,
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
        ImColor(50, 50, 50), ImColor(50, 50, 60), ImColor(60, 60, 70),
        ImColor(50, 50, 60));
    draw_list->AddRect(
        canvas_pos,
        ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
        ImColor(255, 255, 255));

    bool adding_preview = false;
    ImGui::InvisibleButton("canvas", canvas_size);
    /*if (ImGui::IsItemHovered())
    {
        ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x -
    canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y); if (!adding_line &&
    ImGui::IsMouseClicked(0))
        {
            points.push_back(mouse_pos_in_canvas);
            adding_line = true;
        }
        if (adding_line)
        {
            adding_preview = true;
            points.push_back(mouse_pos_in_canvas);
            if (!ImGui::GetIO().MouseDown[0])
                adding_line = adding_preview = false;
        }
        if (ImGui::IsMouseClicked(1) && !points.empty())
        {
            adding_line = adding_preview = false;
            points.pop_back();
            points.pop_back();
        }
    }*/
    draw_list->PushClipRect(
        ImVec2(canvas_pos.x, canvas_pos.y),
        ImVec2(canvas_pos.x + canvas_size.x,
               canvas_pos.y + canvas_size.y));  // clip lines within the canvas
                                                // (if we resize it, etc.)

    float xsize = m_Max.x - m_Min.x;
    float ysize = m_Max.y - m_Min.y;
    float size = std::max(ysize, xsize);
    if (size == 0.f) size = 1.f;
    for (int i = 0; i < m_Points.Size - 1; i += 2) {
      ImVec2 p0 = m_Points[i];
      ImVec2 p1 = m_Points[i + 1];

      p0.x = ((p0.x - m_Min.x) / size) * canvas_size.x;
      p1.x = ((p1.x - m_Min.x) / size) * canvas_size.x;

      p0.y = ((p0.y - m_Min.y) / size) * canvas_size.y;
      p1.y = ((p1.y - m_Min.y) / size) * canvas_size.y;

      draw_list->AddLine(ImVec2(canvas_pos.x + p0.x, canvas_pos.y + p0.y),
                         ImVec2(canvas_pos.x + p1.x, canvas_pos.y + p1.y),
                         0xFF00FFFF, 2.0f);
    }
    draw_list->PopClipRect();
    if (adding_preview) m_Points.pop_back();
  }
  ImGui::End();
}
