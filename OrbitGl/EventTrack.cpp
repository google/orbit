//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventTrack.h"
#include "GlCanvas.h"
#include "Capture.h"

//-----------------------------------------------------------------------------
EventTrack::EventTrack( TimeGraph* a_TimeGraph ) : m_TimeGraph( a_TimeGraph )
{
    m_MousePos[0] = m_MousePos[1] = Vec2(0, 0);
    m_Picked = false;
}

//-----------------------------------------------------------------------------
void EventTrack::Draw( GlCanvas* a_Canvas, bool a_Picking )
{
    Color col = m_TimeGraph->GetLayout().GetThreadColor(m_ThreadId);

    a_Picking ? PickingManager::SetPickingColor( a_Canvas->GetPickingManager().CreatePickableId( this ) )
              : glColor4ubv( &col[0] );

    float x0 = m_Pos[0];
    float y0 = m_Pos[1];
    float x1 = x0 + m_Size[0];
    float y1 = y0 - m_Size[1];

    glBegin( GL_QUADS );
    glVertex3f( x0, y0, -0.1f );
    glVertex3f( x1, y0, -0.1f );
    glVertex3f( x1, y1, -0.1f );
    glVertex3f( x0, y1, -0.1f );
    glEnd();

    if( a_Canvas->GetPickingManager().GetPicked() == this )
        glColor4ub(  255, 255, 255, 255  );
    else
        glColor4ubv( &col[0] );

    glBegin( GL_LINES );
    glVertex3f( x0, y0, -0.1f );
    glVertex3f( x1, y0, -0.1f );
    glVertex3f( x1, y1, -0.1f );
    glVertex3f( x0, y1, -0.1f );
    glEnd();

    if( m_Picked )
    {
        Vec2 & from = m_MousePos[0];
        Vec2 & to = m_MousePos[1];

        x0 = from[0];
        y0 = m_Pos[1];
        x1 = to[0];
        y1 = y0 - m_Size[1];

        glColor4ub( 0, 128, 255, 128 );
        glBegin( GL_QUADS );
        glVertex3f( x0, y0, -0.f );
        glVertex3f( x1, y0, -0.f );
        glVertex3f( x1, y1, -0.f );
        glVertex3f( x0, y1, -0.f );
        glEnd();
    }

    m_Canvas = a_Canvas;
}

//-----------------------------------------------------------------------------
void EventTrack::SetPos( float a_X, float a_Y )
{
    m_Pos = Vec2( a_X, a_Y );
	m_ThreadName.SetPos(Vec2(a_X, a_Y));
	m_ThreadName.SetSize(Vec2(m_Size[0]*0.3f, m_Size[1]));
}

//-----------------------------------------------------------------------------
void EventTrack::SetSize( float a_SizeX, float a_SizeY )
{
    m_Size = Vec2( a_SizeX, a_SizeY );
}

//-----------------------------------------------------------------------------
void EventTrack::OnPick( int a_X, int a_Y )
{
    Capture::GSelectedThreadId = m_ThreadId;
    Vec2 & mousePos = m_MousePos[0];
    m_Canvas->ScreenToWorld( a_X, a_Y, mousePos[0], mousePos[1] );
    m_MousePos[1] = m_MousePos[0];
    m_Picked = true;
}

//-----------------------------------------------------------------------------
void EventTrack::OnRelease()
{
    if( m_Picked )
    {
        SelectEvents();
    }

    m_Picked = false;
}

//-----------------------------------------------------------------------------
void EventTrack::OnDrag( int a_X, int a_Y )
{
    Vec2 & to   = m_MousePos[1];
    m_Canvas->ScreenToWorld( a_X, a_Y, to[0], to[1] );
}

//-----------------------------------------------------------------------------
void EventTrack::SelectEvents()
{
    Vec2 & from = m_MousePos[0];
    Vec2 & to   = m_MousePos[1];

    m_TimeGraph->SelectEvents( from[0], to[0], m_ThreadId );
}
