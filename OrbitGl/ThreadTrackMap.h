//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "CallstackTypes.h"
#include <memory>
#include <unordered_map>

class ThreadTrack;

//-----------------------------------------------------------------------------
typedef std::unordered_map< ThreadID, std::shared_ptr<ThreadTrack> > ThreadTrackMap;