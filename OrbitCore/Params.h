//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "SerializationMacros.h"
#include "BaseTypes.h"

using namespace std;

struct Params
{
    Params();
    void Load();
    void Save();
    
    void AddToPdbHistory( const string & a_PdbName );
    void ScanPdbCache();

public:
    bool  m_LoadTypeInfo;
    bool  m_SendCallStacks;
    bool  m_TrackContextSwitches;
    bool  m_TrackSamplingEvents;
    bool  m_UnrealSupport;
    bool  m_UnitySupport;
    bool  m_StartPaused;
    bool  m_AllowUnsafeHooking;
    bool  m_HookOutputDebugString;
    int   m_MaxNumTimers;
    float m_FontSize;
    int   m_Port;
    DWORD64 m_NumBytesAssembly;
    std::string m_DiffExe;
    std::string m_DiffArgs;
    std::vector< string > m_PdbHistory;
    std::unordered_map< string, string > m_CachedPdbsMap;
    std::string m_ProcessPath;
    std::string m_Arguments;
    std::string m_WorkingDirectory;

    ORBIT_SERIALIZABLE;
};

extern Params GParams;

