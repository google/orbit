//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "TextBox.h"
#include "BlockChain.h"
#include "ContextSwitch.h"
#include "TimeGraphLayout.h"
#include "Geometry.h"
#include "Batcher.h"
#include "TextRenderer.h"
#include "MemoryTracker.h"
#include "EventBuffer.h"
#include <unordered_map>

class Systrace;
class ThreadTrack;

class TimeGraph
{
public:
    TimeGraph();

    void Draw( bool a_Picking = false );
    void DrawMainFrame( TextBox & a_Box );
    void DrawEvents( bool a_Picking = false );
    void DrawTime();
    void UpdateThreadIds();
    void GetBounds( Vec2 & a_Min, Vec2 & a_Max );
    
    void DrawLineBuffer( bool a_Picking );
    void DrawBoxBuffer( bool a_Picking );
    void DrawBuffered( bool a_Picking );
    void DrawText();
    void NeedsUpdate();
    void UpdatePrimitives( bool a_Picking );
    void UpdateEvents();
    void SelectEvents( float a_WorldStart, float a_WorldEnd, ThreadID a_TID );

    void ProcessTimer( const Timer & a_Timer );
    void UpdateThreadDepth( int a_ThreadId, int a_Depth );
    void UpdateMaxTimeStamp( TickType a_Time );
    void AddContextSwitch();
    
    int GetThreadDepth( int a_ThreadId ) const;
    int GetThreadDepthTotal() const;
    float GetThreadTotalHeight();
    float GetTextBoxHeight() const { return m_Layout.m_TextBoxHeight; }
    int GetMarginInPixels() const { return m_Margin; }
    float GetWorldFromRawTimeStamp( TickType a_Time ) const;
    float GetWorldFromUs( double a_Micros ) const;
    TickType GetRawTimeStampFromWorld( float a_WorldX );
    TickType GetRawTimeStampFromUs( double a_MicroSeconds ) const;
    void GetWorldMinMax( float & a_Min, float & a_Max ) const;
    bool UpdateSessionMinMaxCounter();

    static Color GetThreadColor( int a_ThreadId );
    
    void Clear();
    void ZoomAll();
    void Zoom( const TextBox* a_TextBox );
    void ZoomTime( float a_ZoomValue, double a_MouseRatio );
    void SetMinMax( double a_MinEpochTime, double a_MaxEpochTime );
    void PanTime( int a_InitialX, int a_CurrentX, int a_Width, double a_InitialTime );
    double GetTime( double a_Ratio );
    double GetTimeIntervalMicro( double a_Ratio );
    void Select( const Vec2 & a_WorldStart, const Vec2 a_WorldStop );
    double GetSessionTimeSpanUs();
    double GetCurrentTimeSpanUs();
    void NeedsRedraw(){ m_NeedsRedraw = true; }

    bool IsVisible( const Timer & a_Timer );
    int GetNumDrawnTextBoxes(){ return m_NumDrawnTextBoxes; }
    void AddTextBox( const TextBox& a_TextBox );
    void AddContextSwitch( const ContextSwitch & a_CS );
    void SetPickingManager( class PickingManager* a_Manager ){ m_PickingManager = a_Manager; }
    void SetCanvas( GlCanvas* a_Canvas );
	void SetFontSize( int a_FontSize );
    void SetSystrace(std::shared_ptr<Systrace> a_Systrace) { m_Systrace = a_Systrace; }
    
public:
    TextRenderer                    m_TextRendererStatic;
    TextRenderer*                   m_TextRenderer;
    GlCanvas*                       m_Canvas;
    TextBox                         m_SceneBox;
    BlockChain<TextBox, 65536>      m_TextBoxes;
    int                             m_NumDrawnTextBoxes;
    
    double                          m_RefEpochTimeUs;
    double                          m_MinEpochTimeUs;
    double                          m_MaxEpochTimeUs;
    TickType                        m_SessionMinCounter;
    TickType                        m_SessionMaxCounter;
    std::map< ThreadID, int >       m_ThreadDepths;
    std::map< ThreadID, uint32_t >  m_EventCount;
    double                          m_TimeWindowUs;
    float                           m_WorldStartX;
    float                           m_WorldWidth;
    int                             m_Margin;

    double                          m_ZoomValue;
    double                          m_MouseRatio;
    double                          m_TimeLeft;
    double                          m_TimeRight;
    double                          m_CurrentTimeWindow;
    double                          m_Scale;
    unsigned int                    m_MainFrameCounter;
    unsigned char                   m_TrackAlpha;

    TimeGraphLayout                 m_Layout;
    std::map< ThreadID, class EventTrack* > m_EventTracks;

    std::map< DWORD/*ThreadId*/, std::map< long long, ContextSwitch > > m_ContextSwitchesMap;
    std::map< DWORD/*CoreId*/, std::map< long long, ContextSwitch > >   m_CoreUtilizationMap;

    std::map< ThreadID, uint32_t >  m_ThreadCountMap;

    std::vector< CallstackEvent >   m_SelectedCallstackEvents;
    bool                            m_NeedsUpdatePrimitives;
    bool                            m_DrawText;
    bool                            m_NeedsRedraw;
    std::vector<TextBox*>           m_VisibleTextBoxes;
    Batcher                         m_Batcher;
    PickingManager*                 m_PickingManager;
    Mutex                           m_Mutex;
    Timer                           m_LastThreadReorder;
    MemoryTracker                   m_MemTracker;
    std::shared_ptr<Systrace>       m_Systrace;
    
    std::unordered_map< ThreadID, std::shared_ptr<ThreadTrack> > m_ThreadTracks;
};

extern TimeGraph* GCurrentTimeGraph;

