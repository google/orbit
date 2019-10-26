//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "BpfTrace.h"
#include "LinuxUtils.h"
#include "CoreApp.h"
#include "Capture.h"
#include "OrbitProcess.h"
#include <fstream>
#include <sstream>

//-----------------------------------------------------------------------------
BpfTrace::BpfTrace(Callback a_Callback)
{
    m_Callback = a_Callback ? a_Callback : [this](const std::string& a_Buffer)
    {
        CommandCallback(a_Buffer);
    };

    m_ScriptFileName = ws2s(Path::GetBasePath()) + "orbit.bt";
}

//-----------------------------------------------------------------------------
void BpfTrace::Start()
{
#if __linux__
    m_ExitRequested = false;
    m_TimerStacks.clear();
    if( !WriteBpfScript() )
        return;

    m_BpfCommand = std::string("bpftrace ") + m_ScriptFileName;
    m_Thread = std::make_shared<std::thread>
        ( &LinuxUtils::StreamCommandOutput
        , m_BpfCommand.c_str()
        , m_Callback
        , &m_ExitRequested );
    m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
void BpfTrace::Stop()
{
    m_ExitRequested = true;
}

//-----------------------------------------------------------------------------
std::string BpfTrace::GetBpfScript()
{
    if (!m_Script.empty())
    {
        return m_Script;
    }

    std::stringstream ss;

    for (Function *func : Capture::GTargetProcess->GetFunctions())
    {
        if (func->IsSelected())
        {
            uint64_t virtual_address = (uint64_t)func->GetVirtualAddress();
            Capture::GSelectedFunctionsMap[func->m_Address] = func;

            ss << "   uprobe:" << func->m_Probe << R"({ printf("b )" << std::to_string(virtual_address) << R"( %u %lld\n", tid, nsecs); })" << std::endl;
            ss << "uretprobe:" << func->m_Probe << R"({ printf("e )" << std::to_string(virtual_address) << R"( %u %lld\n", tid, nsecs); })" << std::endl;
        }
    }

    return ss.str();
}

//-----------------------------------------------------------------------------
bool BpfTrace::WriteBpfScript()
{
    std::string script = GetBpfScript();
    if (script.empty())
        return false;

    std::ofstream outFile;
    outFile.open(m_ScriptFileName);
    if (outFile.fail())
        return false;

    outFile << script;
    outFile.close();
    return true;
}

//-----------------------------------------------------------------------------
uint64_t BpfTrace::ProcessString(const std::string& a_String)
{
    auto hash = StringHash(a_String);
    if (m_StringMap.find(hash) == m_StringMap.end())
    {
        m_StringMap[hash] = a_String;
    }

    return hash;
}

//-----------------------------------------------------------------------------
void BpfTrace::CommandCallback(const std::string& a_Line)
{
    auto tokens = Tokenize(a_Line);
    const std::string& mode             = tokens[0];
    const std::string& functionAddress  = tokens[1];
    const std::string& threadName       = tokens[2];
    const std::string& timestamp        = tokens[3];

    bool isBegin = mode == "b";
    bool isEnd   = !isBegin;

    if (isBegin)
    {
        Timer timer;
        timer.m_TID = atoi(threadName.c_str());
        uint64_t nanos = std::stoull(timestamp);
        timer.m_Start = nanos;
        timer.m_Depth = (uint8_t)m_TimerStacks[threadName].size();
        timer.m_FunctionAddress = std::stoull(functionAddress);
        m_TimerStacks[threadName].push_back(timer);
    }

    if (isEnd)
    {
        std::vector<Timer>& timers = m_TimerStacks[threadName];
        if (timers.size())
        {
            Timer& timer = timers.back();
            uint64_t nanos = std::stoull(timestamp);
            timer.m_End = nanos;
            GCoreApp->ProcessTimer(&timer, functionAddress);
            timers.pop_back();
        }
    }
}
