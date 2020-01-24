//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <unordered_map>
#include "CallstackTypes.h"

class ThreadTrack;

//-----------------------------------------------------------------------------
typedef std::unordered_map<ThreadID, std::shared_ptr<ThreadTrack> >
    ThreadTrackMap;