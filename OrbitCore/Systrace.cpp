#include "Systrace.h"
#include "PrintVar.h"
#include "ScopeTimer.h"
#include "Capture.h"
#include "Profiling.h"
#include <fstream>

//-----------------------------------------------------------------------------
bool ShouldIgnore(const std::string& line)
{
    return StartsWith(line, "#") ||
           Contains(line, "<script class=") ||
           Contains(line, "</script>") ||
           Contains(line, "<!-- ");
}

//-----------------------------------------------------------------------------
bool IsBegin(const std::string& line)
{
    return Contains(line, "tracing_mark_write: B");
}

//-----------------------------------------------------------------------------
bool IsEnd(const std::string& line)
{
    return Contains(line, "tracing_mark_write: E");
}

//-----------------------------------------------------------------------------
bool IsTraceBegin(const std::string& line)
{
    return Contains(line, "<!-- BEGIN TRACE -->");
}

//-----------------------------------------------------------------------------
bool IsTraceEnd(const std::string& line)
{
    return Contains(line, "<!-- END TRACE -->");
}

//-----------------------------------------------------------------------------
uint64_t GetMicros(std::string timestamp)
{
    Replace(timestamp, ":", "");
    std::vector<std::string> tokens = Tokenize(timestamp, ".");
    if (tokens.size() != 2)
    {
        PRINT_FUNC;
        PRINT_VAR(timestamp);
        return 0;
    }

    uint64_t seconds = atoi(tokens[0].c_str());
    uint64_t micros = atoi(tokens[1].c_str());
    return seconds * 1000000 + micros;
}

//-----------------------------------------------------------------------------
DWORD Systrace::GetThreadId(const std::string& a_ThreadName)
{
    auto it = m_ThreadIDs.find(a_ThreadName);
    if (it == m_ThreadIDs.end())
    {
        auto tid = m_ThreadCount++;
        m_ThreadIDs[a_ThreadName] = tid;
        m_ThreadNames[tid] = a_ThreadName;
    }
    return m_ThreadIDs[a_ThreadName];
}

//-----------------------------------------------------------------------------
uint64_t Systrace::ProcessString(const std::string& a_String)
{
    auto hash = StringHash(a_String);
    if (m_StringMap.find(hash) == m_StringMap.end())
    {
        m_StringMap[hash] = a_String;
    }

    return hash;
}

//-----------------------------------------------------------------------------
uint64_t Systrace::ProcessFunctionName(const std::string& a_String)
{
    std::vector<std::string> tokens = Tokenize(a_String, "|");
    if (tokens.size())
    {
        const std::string& function = tokens.back();
        uint64_t hash = ProcessString(function);
        Function func;
        func.m_Address = hash;
        func.m_Name = func.m_PrettyName = s2ws(function);
        func.m_PrettyNameStr = function;
        func.m_PrettyNameLower = ToLower(func.m_Name);
        m_Functions.push_back(func);
        return hash;
    }

    return 0;
}

//-----------------------------------------------------------------------------
const std::string& Systrace::GetFunctionName(uint64_t a_ID) const
{
    auto it = m_StringMap.find(a_ID);
    if (it == m_StringMap.end())
        return "";
    return it->second;
}

//-----------------------------------------------------------------------------
Systrace::Systrace(const char* a_FilePath)
{
    SCOPE_TIMER_LOG(L"Systrace Parsing");
    std::ifstream infile(a_FilePath);
    std::string line;
    bool foundBegin = false;
    while (std::getline(infile, line))
    {
        if (IsTraceBegin(line))
            foundBegin = true;
        if (!foundBegin)
            continue;
        if (IsTraceEnd(line))
            break;

        bool isBegin = IsBegin(line);
        bool isEnd = IsEnd(line);
        if (isBegin || isEnd)
        {
            line = Replace(line, "(", "");
            line = Replace(line, ")", "");
            std::vector<std::string> tokens = Tokenize(line);
           
            if (tokens.size() < 7)
                continue;

            const std::string& threadName = tokens[0];
            const std::string& pid = tokens[1];
            const std::string& timestamp = tokens[4];            

            if (isBegin)
            {
                Timer timer;
                timer.m_TID = GetThreadId(threadName);
                timer.m_Start = TicksFromMicroseconds(GetMicros(timestamp));
                timer.m_Depth = m_TimerStacks[threadName].size();
                const std::string& function = tokens[6];
                timer.m_FunctionAddress = ProcessFunctionName(function);
                m_TimerStacks[threadName].push_back(timer);
            }

            if (isEnd)
            {
                std::vector<Timer>& timers = m_TimerStacks[threadName];
                Timer& timer = timers.back();
                timer.m_End = TicksFromMicroseconds(GetMicros(timestamp));
                m_Timers.push_back(timer);
                timers.pop_back();
            }
        }
    }

    {
        SCOPE_TIMER_LOG(L"Function Map");
        for (auto & function : m_Functions)
        {
            m_FunctionMap[function.m_Address] = &function;
        }
    }

    {
        SCOPE_TIMER_LOG(L"Update Timers");
        for (auto& timer : m_Timers)
        {
            m_FunctionMap[timer.m_FunctionAddress]->m_Stats->Update(timer);
        }
    }
}