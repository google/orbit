//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
struct SessionModule
{
    std::wstring                m_Name;
    std::vector< uint64_t >     m_FunctionHashes;
    std::vector< std::wstring > m_WatchedVariables;

    template <class Archive>
    void serialize( Archive & a_Archive, std::uint32_t const a_Version )
    {
        a_Archive( CEREAL_NVP( m_Name )
                 , CEREAL_NVP( m_FunctionHashes )
                 , CEREAL_NVP( m_WatchedVariables ) );
    }
};

//-----------------------------------------------------------------------------
class Session
{
public:
    Session();
    ~Session();

    template <class Archive>
    void serialize( Archive & a_Archive, std::uint32_t const a_Version )
    {
        a_Archive( CEREAL_NVP(m_ProcessFullPath)
                 , CEREAL_NVP(m_Modules) );
    }

    std::wstring m_FileName;
    std::wstring m_ProcessFullPath;
    std::map< std::wstring, SessionModule > m_Modules;
};
