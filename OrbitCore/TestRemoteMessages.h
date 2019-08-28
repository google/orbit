//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <memory.h>
#include "BaseTypes.h"
#include "SerializationMacros.h"

class TestRemoteMessages
{
public:
    static TestRemoteMessages& Get();

    TestRemoteMessages();
    ~TestRemoteMessages();

    void Init();
    void Run();

protected:
    void SetupMessageHandlers();
};
