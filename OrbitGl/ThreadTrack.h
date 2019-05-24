//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Track.h"

class TextBox;

//-----------------------------------------------------------------------------
class ThreadTrack : public Track
{
public:
    ThreadTrack();
    ~ThreadTrack(){}

    // Pickable
    void Draw( GlCanvas* a_Canvas, bool a_Picking ) override;
    void OnDrag( int a_X, int a_Y ) override;
};