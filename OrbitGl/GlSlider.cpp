//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlSlider.h"

#include <algorithm>

#include "GlCanvas.h"

//-----------------------------------------------------------------------------
GlSlider::GlSlider()
    : m_Canvas(nullptr),
      m_Ratio(0),
      m_Length(0),
      m_PickingRatio(0),
      m_SelectedColor(40, 40, 40, 255),
      m_SliderColor(50, 50, 50, 255),
      m_BarColor(60, 60, 60, 255),
      m_MinSliderPixelWidth(20),
      m_PixelHeight(20),
      m_Vertical(false)

{}

//-----------------------------------------------------------------------------
void GlSlider::SetSliderRatio(float a_Ratio)  // [0,1]
{
  m_Ratio = a_Ratio;
}

//-----------------------------------------------------------------------------
void GlSlider::SetSliderWidthRatio(float a_WidthRatio)  // [0,1]
{
  float minWidth =
      m_MinSliderPixelWidth / static_cast<float>(m_Canvas->getWidth());
  m_Length = std::max(a_WidthRatio, minWidth);
}

//-----------------------------------------------------------------------------
void GlSlider::OnPick(int a_X, int a_Y) {
  m_Vertical ? OnPickVertical(a_X, a_Y) : OnPickHorizontal(a_X, a_Y);
}

//-----------------------------------------------------------------------------
void GlSlider::OnPickHorizontal(int a_X, int /*a_Y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float x = static_cast<float>(a_X);
  float sliderX = m_Ratio * nonSliderWidth;
  m_PickingRatio = (x - sliderX) / sliderWidth;
}

//-----------------------------------------------------------------------------
void GlSlider::OnPickVertical(int /*a_X*/, int a_Y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float y = a_Y;
  float sliderY = m_Ratio * nonSliderHeight;
  m_PickingRatio = (y - sliderY) / sliderHeight;
}

//-----------------------------------------------------------------------------
void GlSlider::OnDrag(int a_X, int a_Y) {
  m_Vertical ? OnDragVertical(a_X, a_Y) : OnDragHorizontal(a_X, a_Y);
}

//-----------------------------------------------------------------------------
void GlSlider::OnDragVertical(int /*a_X*/, int a_Y) {
  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  float sliderTopHeight = m_PickingRatio * m_Length * canvasHeight;
  float newY = static_cast<float>(a_Y) - sliderTopHeight;
  float ratio = newY / nonSliderHeight;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

//-----------------------------------------------------------------------------
void GlSlider::OnDragHorizontal(int a_X, int /*a_Y*/) {
  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  float sliderLeftWidth = m_PickingRatio * m_Length * canvasWidth;
  float newX = static_cast<float>(a_X) - sliderLeftWidth;
  float ratio = newX / nonSliderWidth;

  m_Ratio = clamp(ratio, 0.f, 1.f);

  if (m_DragCallback) {
    m_DragCallback(m_Ratio);
  }
}

//-----------------------------------------------------------------------------
void GlSlider::Draw(GlCanvas* a_Canvas, bool a_Picking) {
  m_Vertical ? DrawVertical(a_Canvas, a_Picking)
             : DrawHorizontal(a_Canvas, a_Picking);
}

//-----------------------------------------------------------------------------
void GlSlider::DrawHorizontal(GlCanvas* canvas, bool picking) {
  m_Canvas = canvas;

  static float y = 0;

  float canvasWidth = m_Canvas->getWidth();
  float sliderWidth = m_Length * canvasWidth;
  float nonSliderWidth = canvasWidth - sliderWidth;

  // Bar
  if (!picking) {
    glColor4ubv(&m_BarColor[0]);
    glBegin(GL_QUADS);
    glVertex3f(0, y, 0);
    glVertex3f(canvasWidth, y, 0);
    glVertex3f(canvasWidth, y + GetPixelHeight(), 0);
    glVertex3f(0, y + GetPixelHeight(), 0);
    glEnd();
  }

  float start = m_Ratio * nonSliderWidth;
  float stop = start + sliderWidth;

  Color color = m_SliderColor;
  if (picking) {
    color = canvas->GetPickingManager().GetPickableColor(this);
  } else if (canvas->GetPickingManager().GetPicked() == this) {
    color = m_SelectedColor;
  }

  glColor4ubv(&color[0]);
  glBegin(GL_QUADS);
  glVertex3f(start, y, 0);
  glVertex3f(stop, y, 0);
  glVertex3f(stop, y + GetPixelHeight(), 0);
  glVertex3f(start, y + GetPixelHeight(), 0);
  glEnd();
}

//-----------------------------------------------------------------------------
void GlSlider::DrawVertical(GlCanvas* canvas, bool picking) {
  m_Canvas = canvas;

  float x = m_Canvas->getWidth() - GetPixelHeight();

  float canvasHeight = m_Canvas->getHeight();
  float sliderHeight = m_Length * canvasHeight;
  float nonSliderHeight = canvasHeight - sliderHeight;

  // Bar
  if (!picking) {
    glColor4ubv(&m_BarColor[0]);
    glBegin(GL_QUADS);
    glVertex3f(x, 0, 0);
    glVertex3f(x, canvasHeight, 0);
    glVertex3f(x + GetPixelHeight(), canvasHeight, 0);
    glVertex3f(x + GetPixelHeight(), 0, 0);
    glEnd();
  }

  float start = canvasHeight - m_Ratio * nonSliderHeight;
  float stop = start - sliderHeight;

  Color color = m_SliderColor;
  if (picking) {
    color = canvas->GetPickingManager().GetPickableColor(this);
  } else if (canvas->GetPickingManager().GetPicked() == this) {
    color = m_SelectedColor;
  }

  glColor4ubv(&color[0]);

  glBegin(GL_QUADS);
  glVertex3f(x, start, 0);
  glVertex3f(x, stop, 0);
  glVertex3f(x + GetPixelHeight(), stop, 0);
  glVertex3f(x + GetPixelHeight(), start, 0);
  glEnd();
}
