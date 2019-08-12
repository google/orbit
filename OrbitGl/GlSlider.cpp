//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "GlSlider.h"
#include "GlCanvas.h"
#include <algorithm>

//-----------------------------------------------------------------------------
GlSlider::GlSlider() : m_Canvas(nullptr)
                     , m_Ratio(0)
                     , m_Length(0)
                     , m_PickingRatio(0)
                     , m_SelectedColor(40, 40, 40, 255)
                     , m_SliderColor(50, 50, 50, 255)
                     , m_BarColor(60, 60, 60, 255)
                     , m_MinSliderPixelWidth(20)
                     , m_PixelHeight(20)
                     , m_Vertical(false)

{
}

//-----------------------------------------------------------------------------
void GlSlider::SetSliderRatio( float a_Ratio ) // [0,1]
{
    m_Ratio = a_Ratio;
}

//-----------------------------------------------------------------------------
void GlSlider::SetSliderWidthRatio( float a_WidthRatio ) // [0,1]
{
    float minWidth = m_MinSliderPixelWidth/(float)m_Canvas->getWidth();
    m_Length = std::max( a_WidthRatio, minWidth );
}

//-----------------------------------------------------------------------------
void GlSlider::OnPick( int a_X, int a_Y )
{
    m_Vertical ? OnPickVertical(a_X, a_Y) : OnPickHorizontal(a_X, a_Y);
}

//-----------------------------------------------------------------------------
void GlSlider::OnPickHorizontal(int a_X, int a_Y)
{
    float canvasWidth = (float)m_Canvas->getWidth();
    float sliderWidth = m_Length * canvasWidth;
    float nonSliderWidth = canvasWidth - sliderWidth;

    float x = (float)a_X;
    float sliderX = m_Ratio * nonSliderWidth;
    m_PickingRatio = (x - sliderX) / sliderWidth;
}

//-----------------------------------------------------------------------------
void GlSlider::OnPickVertical(int a_X, int a_Y)
{
    float canvasHeight = (float)m_Canvas->getHeight();
    float sliderHeight = m_Length * canvasHeight;
    float nonSliderHeight = canvasHeight - sliderHeight;

    float y = (float)a_Y;
    float sliderY = m_Ratio * nonSliderHeight;
    m_PickingRatio = (y - sliderY) / sliderHeight;
}

//-----------------------------------------------------------------------------
void GlSlider::OnDrag(int a_X, int a_Y)
{
    m_Vertical ? OnDragVertical(a_X, a_Y) : OnDragHorizontal(a_X, a_Y);
}

//-----------------------------------------------------------------------------
void GlSlider::OnDragVertical( int a_X, int a_Y )
{
    float canvasHeight = (float)m_Canvas->getHeight();
    float sliderHeight = m_Length * canvasHeight;
    float nonSliderHeight = canvasHeight - sliderHeight;

    float sliderTopHeight = m_PickingRatio*m_Length*canvasHeight;
    float newY = (float)a_Y - sliderTopHeight;
    float ratio = newY / nonSliderHeight;

    m_Ratio = clamp( ratio, 0.f, 1.f );

    if( m_DragCallback )
    {
        m_DragCallback( m_Ratio );
    }
}

//-----------------------------------------------------------------------------
void GlSlider::OnDragHorizontal(int a_X, int a_Y)
{
    float canvasWidth = (float)m_Canvas->getWidth();
    float sliderWidth = m_Length * canvasWidth;
    float nonSliderWidth = canvasWidth - sliderWidth;

    float sliderLeftWidth = m_PickingRatio * m_Length * canvasWidth;
    float newX = (float)a_X - sliderLeftWidth;
    float ratio = newX / nonSliderWidth;

    m_Ratio = clamp(ratio, 0.f, 1.f);

    if (m_DragCallback)
    {
        m_DragCallback(m_Ratio);
    }
}

//-----------------------------------------------------------------------------
void GlSlider::Draw(GlCanvas* a_Canvas, bool a_Picking)
{
    m_Vertical ? DrawVertical(a_Canvas, a_Picking) : DrawHorizontal(a_Canvas, a_Picking);
}

//-----------------------------------------------------------------------------
void GlSlider::DrawHorizontal( GlCanvas* a_Canvas, bool a_Picking )
{
    m_Canvas = a_Canvas;

    static float y = 0;

    float canvasWidth = (float)m_Canvas->getWidth();
    float sliderWidth = m_Length * canvasWidth;
    float nonSliderWidth = canvasWidth - sliderWidth;
    
    // Bar
    if( !a_Picking )
    {
        glColor4ubv( &m_BarColor[0] );
        glBegin( GL_QUADS );
        glVertex3f( 0, y, 0 );
        glVertex3f( canvasWidth, y, 0 );
        glVertex3f( canvasWidth, y + GetPixelHeight(), 0 );
        glVertex3f( 0, y + GetPixelHeight(), 0 );
        glEnd();
    }

    float start = m_Ratio * nonSliderWidth;
    float stop = start + sliderWidth;

    a_Picking ? PickingManager::SetPickingColor( a_Canvas->GetPickingManager().CreatePickableId(this) ) :
    a_Canvas->GetPickingManager().GetPicked() == this ? glColor4ubv( &m_SelectedColor[0] ) : glColor4ubv( &m_SliderColor[0] );

    glBegin( GL_QUADS );
    glVertex3f( start, y, 0 );
    glVertex3f( stop, y, 0 );
    glVertex3f( stop, y + GetPixelHeight(), 0 );
    glVertex3f( start, y + GetPixelHeight(), 0 );
    glEnd();
}

//-----------------------------------------------------------------------------
void GlSlider::DrawVertical(GlCanvas* a_Canvas, bool a_Picking)
{
    m_Canvas = a_Canvas;

    static float offset = 0;
    float x = (float)m_Canvas->getWidth() - GetPixelHeight() - offset;

    float canvasHeight = (float)m_Canvas->getHeight();
    float sliderHeight = m_Length * canvasHeight;
    float nonSliderHeight = canvasHeight - sliderHeight;

    // Bar
    if (!a_Picking)
    {
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

    a_Picking ? PickingManager::SetPickingColor(a_Canvas->GetPickingManager().CreatePickableId(this)) :
        a_Canvas->GetPickingManager().GetPicked() == this ? glColor4ubv(&m_SelectedColor[0]) : glColor4ubv(&m_SliderColor[0]);

    glBegin(GL_QUADS);
    glVertex3f(x, start, 0);
    glVertex3f(x, stop, 0);
    glVertex3f(x + GetPixelHeight(), stop, 0);
    glVertex3f(x + GetPixelHeight(), start, 0);
    glEnd();
}
