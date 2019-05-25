//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Track.h"
#include "CallstackTypes.h"
#include <memory>

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

//-----------------------------------------------------------------------------
typedef std::unordered_map< ThreadID, std::shared_ptr<ThreadTrack> > ThreadTrackMap;