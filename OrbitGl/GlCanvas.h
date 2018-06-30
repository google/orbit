//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "TextRenderer.h"
#include "TimeGraph.h"
#include "ImGuiOrbit.h"
#include "ProcessUtils.h"
#include "GlPanel.h"
#include "PickingManager.h"
#include "GlUtils.h"

#define FREEGLUT_STATIC
#define GLUT_DISABLE_ATEXIT_HACK
#include <freeglut.h>

class GlCanvas : public GlPanel
{
public:
    GlCanvas();
    virtual ~GlCanvas();

    virtual void Initialize();
    virtual void Resize( int a_Width, int a_Height );
    virtual void Render( int a_Width, int a_Height );
    virtual void PostRender(){}

    int getWidth() const;
    int getHeight() const;

    void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void prepareScreenSpaceViewport();
    void drawSquareGrid(float size, float delta);
    void ScreenToWorld( int x, int y, float & wx, float & wy ) const;
    void WorldToScreen( float wx, float wy, int & x, int & y ) const;
    int WorldToScreenHeight( float a_Height ) const;
    float ScreenToWorldHeight( int a_Height ) const;
    float ScreenToworldWidth( int a_Width ) const;

    // events
    virtual void MouseMoved( int a_X, int a_Y, bool a_Left, bool a_Right, bool a_Middle );
    virtual void LeftDown( int a_X, int a_Y );
    virtual void MouseWheelMoved( int a_X, int a_Y, int a_Delta, bool a_Ctrl );
    virtual void LeftUp();
    virtual void LeftDoubleClick();
    virtual void RightDown( int a_X, int a_Y );
    virtual void RightUp();
    virtual void mouseLeftWindow();
    virtual void CharEvent( unsigned int a_Char );
    virtual void KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt );
    virtual void KeyReleased( unsigned int a_KeyCode );

    float GetWorldWidth() const { return m_WorldWidth; }
    void SetWorldWidth( float val ) { m_WorldWidth = val; }
    float GetWorldHeight() const { return m_WorldHeight; }
    void SetWorldHeight( float val ) { m_WorldHeight = val; }
    float GetWorldTopLeftX() const { return m_WorldTopLeftX; }
    void SetWorldTopLeftX( float val ) { m_WorldTopLeftX = val; }
    float GetWorldTopLeftY() const { return m_WorldTopLeftY; }
    void SetWorldTopLeftY( float val ) { m_WorldTopLeftY = val; }

    TextRenderer & GetTextRenderer() { return m_TextRenderer; }
    const TextBox & GetSceneBox() { return m_SceneBox; }
    void SetSceneBox( const TextBox & a_TextBox ) { m_SceneBox = a_TextBox; }
    void SetBackgroundColor( const Vec4 & a_Color ) { m_BackgroundColor = a_Color; }
    
    virtual void UpdateWheelMomentum( float a_DeltaTime );
    void UpdateSceneBox();
    virtual void OnTimer();

    float GetMouseX() const { return m_MouseX; }
    float GetMouseY() const { return m_MouseY; }

    float GetMousePosX() const { return (float)m_MousePosX; }
    float GetMousePosY() const { return (float)m_MousePosY; }

    Vec2 ToScreenSpace(const Vec2 & a_Point);
    Vec2 ToWorldSpace(const Vec2 & a_Point);

    float GetDeltaTimeSeconds() const { return m_DeltaTime; }

    virtual void Draw(){}
    virtual void DrawScreenSpace(){}
    virtual void RenderUI();
    virtual void RenderText(){}
    void RenderSamplingUI();

    ImGuiContext & GetImGuiContext() { return m_ImGuiContext; }

    PickingManager & GetPickingManager() { return m_PickingManager; }

    static float Z_VALUE_UI;
    static float Z_VALUE_TEXT;
    static float Z_VALUE_TEXT_UI;
    static float Z_VALUE_TEXT_UI_BG;
    static float Z_VALUE_CONTEXT_SWITCH;
    static float Z_VALUE_EVENT;
    static float Z_VALUE_BOX_ACTIVE;
    static float Z_VALUE_BOX_INACTIVE;
    static float Z_VALUE_TEXT_BG;

protected:
    int     m_Width;
    int     m_Height;
    float   m_DesiredWorldWidth;
    float   m_DesiredWorldHeight;
    float   m_WorldWidth;
    float   m_WorldHeight;
    float   m_WorldTopLeftX;
    float   m_WorldTopLeftY;
    float   m_WorldMaxY;
    float   m_WorldMinWidth;
    float   m_WorldClickX;
    float   m_WorldClickY;
    float   m_MouseX;
    float   m_MouseY;
    int     m_MousePosX;
    int     m_MousePosY;
    Vec2    m_SelectStart;
    Vec2    m_SelectStop;
    int     m_ScreenClickX;
    int     m_ScreenClickY;
    int     m_WheelDelta;
    int     m_MinWheelDelta;
    int     m_MaxWheelDelta;
    float   m_WheelMomentum;
    float   m_DeltaTime;
    double  m_DeltaTimeMs;
    bool    m_IsSelecting;
    bool    m_IsSelectingMiddle;
    double  m_MouseRatio;
    bool    m_DrawUI;
    bool    m_ImguiActive;
    int     m_ID;
    Vec4    m_BackgroundColor;

    ImGuiContext    m_ImGuiContext;
    TickType        m_RefTimeClick;
    TickType        m_SelectedInterval;
    TextRenderer    m_TextRenderer;
    TextBox         m_SceneBox;
    Timer           m_UpdateTimer;
    PickingManager  m_PickingManager;
    bool            m_Picking;
    bool            m_DoubleClicking;
};

