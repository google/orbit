//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "ScopeTimer.h"
#include "OrbitFunction.h"
#include <vector>
#include <string>
#include <map>
#include <unordered_map>;

class BpfTrace
{
public:
    typedef std::function<void(const std::string& a_Data)> Callback;
    BpfTrace(Callback a_Callback = nullptr);
    
    void Start();
    void Stop();
    void Toggle() { IsRunning() ? Stop() : Start(); }
    bool IsRunning() const { return !m_ExitRequested; }
    std::string GetBpfScript();

protected:
    uint64_t ProcessString(const std::string& a_String);
    void CommandCallback(const std::string& a_Line);
    std::string WriteBpfScript();

private:    
    std::map<std::string, std::vector<Timer>> m_TimerStacks;
    std::unordered_map<uint64_t, std::string> m_StringMap;
    std::string                               m_BpfCommand;

    std::shared_ptr<std::thread> m_Thread;
    bool m_ExitRequested = true;
    uint32_t m_PID = 0;
    Callback m_Callback;
};