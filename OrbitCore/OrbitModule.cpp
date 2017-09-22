//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "OrbitModule.h"
#include "Serialization.h"
#include "Pdb.h"

//-----------------------------------------------------------------------------
Module::Module()
{
    memset( this, 0, sizeof( *this ) );
}

//-----------------------------------------------------------------------------
const wstring & Module::GetPrettyName()
{
    if( m_PrettyName.size() == 0 )
    {
        m_PrettyName = Format( L"%s [%I64x - %I64x] %s\r\n", m_Name.c_str(), m_AddressStart, m_AddressEnd, m_FullName.c_str() );
        m_AddressRange = Format( L"[%I64x - %I64x]", m_AddressStart, m_AddressEnd );
    }

    return m_PrettyName;
}

//-----------------------------------------------------------------------------
bool Module::IsDll() const
{
    return ToLower( Path::GetExtension( m_FullName ) ) == std::wstring( L".dll" );
}

//-----------------------------------------------------------------------------
bool Module::LoadDebugInfo()
{
    m_Pdb = std::make_shared<Pdb>( m_PdbName.c_str() );
    m_Pdb->SetMainModule( (HMODULE)m_AddressStart );

    if( m_FoundPdb )
    {
        return m_Pdb->LoadDataFromPdb();
    }

    return false;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Module, 0 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_FullName );
    ORBIT_NVP_VAL( 0, m_PdbName );
    ORBIT_NVP_VAL( 0, m_Directory );
    ORBIT_NVP_VAL( 0, m_PrettyName );
    ORBIT_NVP_VAL( 0, m_AddressRange );
    ORBIT_NVP_VAL( 0, m_DebugSignature );
    ORBIT_NVP_VAL( 0, m_AddressStart );
    ORBIT_NVP_VAL( 0, m_AddressEnd );
    ORBIT_NVP_VAL( 0, m_EntryPoint );
    ORBIT_NVP_VAL( 0, m_FoundPdb );
    ORBIT_NVP_VAL( 0, m_Selected );
    ORBIT_NVP_VAL( 0, m_Loaded );
    ORBIT_NVP_VAL( 0, m_PdbSize );
}