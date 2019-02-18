//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include <vector>
#include <string>

class GlPanel
{
public:
    GlPanel();
    ~GlPanel();

    enum Type
    { 
        CAPTURE,
        IMMEDIATE,
        THREADS,
        VISUALIZE,
        RULE_EDITOR,
        PLUGIN,
        DEBUG
    };

    static GlPanel* Create( Type a_Type, void* a_UserData = nullptr );

    virtual void Initialize();
    virtual void Resize( int a_Width, int a_Height );
    virtual void Render( int a_Width, int a_Height );
    virtual void PreRender(){};
    virtual void SetWindowOffset( int a_X, int a_Y ){ m_WindowOffset[0] = a_X; m_WindowOffset[1] = a_Y; }
    virtual void SetMainWindowSize( int a_Width, int a_Height ){ m_MainWindowWidth = a_Width; m_MainWindowHeight = a_Height; }

    virtual void MouseMoved( int /*a_X*/, int /*a_Y*/, bool /*a_Left*/, bool /*a_Right*/, bool /*a_Middle*/ ){}
    virtual void LeftDown ( int /*a_X*/, int /*a_Y*/ ){}
    virtual void LeftUp(){}
    virtual void RightDown( int /*a_X*/, int /*a_Y*/ ){}
    virtual bool RightUp() { return false; }
    virtual void MiddleDown(  int /*a_X*/, int /*a_Y*/ ){}
    virtual void MiddleUp( int /*a_X*/, int /*a_Y*/ ){}
    virtual void LeftDoubleClick(){}
    virtual void MouseWheelMoved( int /*a_X*/, int /*a_Y*/, int /*a_Delta*/, bool /*a_Ctrl*/ ){}
    virtual void CharEvent( unsigned int /*a_Char*/ ){}
    virtual void KeyPressed( unsigned int /*a_KeyCode*/, bool /*a_Ctrl*/, bool /*a_Shift*/, bool /*a_Alt*/ ){}
    virtual void KeyReleased( unsigned int /*a_KeyCode*/, bool /*a_Ctrl*/, bool /*a_Shift*/, bool /*a_Alt*/ ){}
    virtual std::vector<std::wstring> GetContextMenu() { return std::vector<std::wstring>(); }
    virtual void OnContextMenu( const std::wstring & /*a_Action*/, int /*a_MenuIndex*/ ) {}

    Type GetType() const { return m_Type; }
    virtual bool GetNeedsRedraw() const { return m_NeedsRedraw; }
    void NeedsRedraw(){ m_NeedsRedraw = true; }

protected:
    Type m_Type;
    int  m_WindowOffset[2];
    int  m_MainWindowWidth;
    int  m_MainWindowHeight;
    bool m_NeedsRedraw;
};

