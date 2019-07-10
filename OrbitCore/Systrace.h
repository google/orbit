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
#include <unordered_map>

class Systrace
{
public:
    Systrace(const char* a_FilePath);
    const std::vector<Timer>& GetTimers() const { return m_Timers; }
    const std::unordered_map<uint64_t, std::string>& GetStrings() const { return m_StringMap; }
    const std::string& GetFunctionName(uint64_t a_ID) const;
    const std::unordered_map<DWORD, std::string>& GetThreadNames() const { return m_ThreadNames; }
    std::vector<Function>& GetFunctions() { return m_Functions; }


protected:
    DWORD GetThreadId(const std::string& a_ThreadName);
    uint64_t ProcessString(const std::string& a_String);
    uint64_t ProcessFunctionName(const std::string& a_String);

private:    
    std::vector<Timer>                          m_Timers;
    std::map<std::string, std::vector<Timer>>   m_TimerStacks;
    std::map<std::string, DWORD>                m_ThreadIDs;
    std::unordered_map<DWORD, std::string>      m_ThreadNames;
    std::unordered_map<uint64_t, std::string>   m_StringMap;
    std::vector<Function>                       m_Functions;
    std::unordered_map<uint64_t, Function*>     m_FunctionMap;
    
    DWORD m_ThreadCount = 0;
    DWORD m_StringCount = 0;
};