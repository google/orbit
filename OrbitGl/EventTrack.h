//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "PickingManager.h"
#include "CallstackTypes.h"
#include "CoreMath.h"

class GlCanvas;
class TimeGraph;

//-----------------------------------------------------------------------------
class EventTrack : public Pickable
{
public:
    EventTrack( TimeGraph* a_TimeGraph );
    ~EventTrack(){}

    void Draw( GlCanvas* a_Canvas, bool a_Picking ) override;
    void OnPick( int a_X, int a_Y ) override;
    void OnRelease() override;
    void OnDrag( int a_X, int a_Y ) override;
    bool Draggable() override { return true; }
    void SetThreadId( ThreadID a_ThreadId ) { m_ThreadId = a_ThreadId; }
    void SetTimeGraph( TimeGraph* a_TimeGraph ) { m_TimeGraph = a_TimeGraph; }
    void SetPos( float a_X, float a_Y );
    void SetSize( float a_SizeX, float a_SizeY );
    void SelectEvents();

protected:
    GlCanvas*   m_Canvas;
    ThreadID    m_ThreadId;
    TimeGraph*  m_TimeGraph;
    Vec2        m_Pos;
    Vec2        m_Size;
    Vec2        m_MousePos[2];
    bool        m_Picked;
};