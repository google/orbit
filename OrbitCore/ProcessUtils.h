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
    bool Contains(uint32_t a_PID ) const;
    std::shared_ptr<Process> GetProcess(uint32_t a_PID);
    std::vector< std::shared_ptr< Process > > m_Processes;
    std::unordered_map< uint32_t, std::shared_ptr< Process > > m_ProcessesMap;

    ORBIT_SERIALIZABLE;
};

