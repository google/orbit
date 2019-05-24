#include "ThreadTrack.h"

//-----------------------------------------------------------------------------
ThreadTrack::ThreadTrack()
{

}

//-----------------------------------------------------------------------------
void ThreadTrack::Draw( GlCanvas* a_Canvas, bool a_Picking )
{
    Track::Draw( a_Canvas, a_Picking );
}

//-----------------------------------------------------------------------------
void ThreadTrack::OnDrag( int a_X, int a_Y )
{
    Track::OnDrag( a_X, a_Y );

}