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

    std::wstring GetPrettyName();
    bool IsDll() const;
    bool LoadDebugInfo();
    bool ContainsAddress( uint64_t a_Address ) { return m_AddressStart <= a_Address && m_AddressEnd > a_Address; }
    uint64_t ValidateAddress( uint64_t a_Address );

    std::string   m_Name;
    std::string   m_FullName;
    std::string   m_PdbName;
    std::string   m_Directory;
    std::string   m_PrettyName;
    std::string   m_AddressRange;
    std::string   m_DebugSignature;
    HMODULE       m_ModuleHandle = 0;
    uint64_t      m_AddressStart = 0;
    uint64_t      m_AddressEnd = 0;
    uint64_t      m_EntryPoint = 0;
    bool          m_FoundPdb = false;
    bool          m_Selected = false;
    bool          m_Loaded = false;
    uint64_t      m_PdbSize = 0;

    mutable std::shared_ptr<Pdb> m_Pdb;

    ORBIT_SERIALIZABLE;

    friend class TestRemoteMessages;
};
