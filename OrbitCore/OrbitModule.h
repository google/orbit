//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <memory.h>
#include "BaseTypes.h"
#include "SerializationMacros.h"

class Pdb;

//-----------------------------------------------------------------------------
struct Module
{
    Module();

    const std::wstring & GetPrettyName();
    bool IsDll() const;
    bool LoadDebugInfo();

    std::wstring  m_Name;
    std::wstring  m_FullName;
    std::wstring  m_PdbName;
    std::wstring  m_Directory;
    std::wstring  m_PrettyName;
    std::wstring  m_AddressRange;
    std::wstring  m_DebugSignature;
    HMODULE       m_ModuleHandle;
    DWORD64       m_AddressStart;
    DWORD64       m_AddressEnd;
    DWORD64       m_EntryPoint;
    bool          m_FoundPdb;
    bool          m_Selected;
    bool          m_Loaded;
    ULONG64       m_PdbSize;

    mutable std::shared_ptr<Pdb> m_Pdb;

    ORBIT_SERIALIZABLE;
};