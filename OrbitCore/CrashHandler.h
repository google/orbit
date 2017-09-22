//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

class CrashHandler
{
public:
    CrashHandler();
    static void SendMiniDump();
};