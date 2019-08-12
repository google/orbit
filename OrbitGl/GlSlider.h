//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "TextBox.h"
#include "PickingManager.h"
#include "Params.h"
#include <functional>

class GlCanvas;

class GlSlider : public Pickable
{
public:
    GlSlider();
    ~GlSlider(){};

    void OnPick( int a_X, int a_Y ) override;
    void OnDrag( int a_X, int a_Y ) override;
    void Draw( GlCanvas* a_Canvas, bool a_Picking ) override;
    bool Draggable() override { return true; }
    void SetSliderRatio( float a_Start ); // [0,1]
    void SetSliderWidthRatio( float a_Ratio ); // [0,1]
    void SetCanvas( GlCanvas* a_Canvas ) { m_Canvas = a_Canvas; }
    Color GetBarColor() const { return m_SliderColor; }
    float GetPixelHeight() const { return GParams.m_FontSize*1.5f; }
    void SetVertical() { m_Vertical = true; }

    typedef std::function< void(float) > DragCallback;
    void SetDragCallback( DragCallback a_Callback ){ m_DragCallback = a_Callback; }

protected:
    void DrawHorizontal(GlCanvas* a_Canvas, bool a_Picking);
    void DrawVertical(GlCanvas* a_Canvas, bool a_Picking);
    void OnDragHorizontal(int a_X, int a_Y);
    void OnDragVertical(int a_X, int a_Y);
    void OnPickHorizontal(int a_X, int a_Y);
    void OnPickVertical(int a_X, int a_Y);

protected:
    TextBox m_Slider;
    TextBox m_Bar;
    GlCanvas* m_Canvas;
    float m_Ratio;
    float m_Length;
    float m_PickingRatio;
    DragCallback m_DragCallback;
    Color m_SelectedColor;
    Color m_SliderColor;
    Color m_BarColor;
    float m_MinSliderPixelWidth;
    float m_PixelHeight;
    bool  m_Vertical;
};