//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitProcess.h"
#include "Serialization.h"
#include <unordered_map>

//-----------------------------------------------------------------------------
namespace ProcessUtils
{
    bool Is64Bit(HANDLE hProcess);
}

//-----------------------------------------------------------------------------
struct ProcessList
{
    ProcessList();
    void Refresh();
    void Clear();
    void SortByID();
    void SortByName();
    void SortByCPU();
    void UpdateCpuTimes();
    bool Contains( DWORD a_PID ) const;
    std::vector< std::shared_ptr< Process > > m_Processes;
    std::unordered_map< DWORD, std::shared_ptr< Process > > m_ProcessesMap;

    ORBIT_SERIALIZABLE;
};

