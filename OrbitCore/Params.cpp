//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Params.h"
#include "Core.h"
#include "CoreApp.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

Params GParams;

//-----------------------------------------------------------------------------
Params::Params() : m_LoadTypeInfo( true )
                 , m_SendCallStacks( true )
                 , m_MaxNumTimers( 1000000 )
                 , m_FontSize( 14.f )
                 , m_TrackContextSwitches(true)
                 , m_TrackSamplingEvents(true)
                 , m_UnrealSupport(true)
                 , m_UnitySupport(true)
                 , m_StartPaused(true)
                 , m_AllowUnsafeHooking(false)
                 , m_HookOutputDebugString(false)
                 , m_FindFileAndLineInfo(true)
                 , m_AutoReleasePdb(false)
                 , m_Port(1789)
                 , m_DiffArgs("%1 %2")
                 , m_NumBytesAssembly(1024)
{
    
}

ORBIT_SERIALIZE( Params, 12 )
{
    ORBIT_NVP_VAL( 0, m_LoadTypeInfo );
    ORBIT_NVP_VAL( 0, m_SendCallStacks );
    ORBIT_NVP_VAL( 0, m_MaxNumTimers );
    ORBIT_NVP_VAL( 0, m_FontSize );
    ORBIT_NVP_VAL( 0, m_PdbHistory );
    ORBIT_NVP_VAL( 1, m_TrackContextSwitches );
    ORBIT_NVP_VAL( 3, m_UnrealSupport );
    ORBIT_NVP_VAL( 3, m_UnitySupport );
    ORBIT_NVP_VAL( 3, m_StartPaused );
    ORBIT_NVP_VAL( 4, m_AllowUnsafeHooking );
    ORBIT_NVP_VAL( 5, m_Port );
    ORBIT_NVP_VAL( 6, m_TrackSamplingEvents );
    ORBIT_NVP_VAL( 7, m_DiffExe );
    ORBIT_NVP_VAL( 7, m_DiffArgs );
    ORBIT_NVP_VAL( 8, m_NumBytesAssembly );
    ORBIT_NVP_VAL( 9, m_HookOutputDebugString );
    ORBIT_NVP_VAL( 10, m_ProcessPath );
    ORBIT_NVP_VAL( 10, m_Arguments );
    ORBIT_NVP_VAL( 10, m_WorkingDirectory );
    ORBIT_NVP_VAL( 11, m_FindFileAndLineInfo );
    ORBIT_NVP_VAL( 12, m_AutoReleasePdb );
}

//-----------------------------------------------------------------------------
void Params::Save()
{
    GCoreApp->SendToUiNow(L"UpdateProcessParams");
    std::wstring fileName = Path::GetParamsFileName();
    SCOPE_TIMER_LOG( Format( L"Saving hook params in %s", fileName.c_str() ) );
    std::ofstream file( fileName );
    cereal::XMLOutputArchive archive( file );
    archive( cereal::make_nvp("Params", *this) );
}

//-----------------------------------------------------------------------------
void Params::Load()
{
    std::ifstream file( Path::GetParamsFileName() );
    if( !file.fail() )
    {
        cereal::XMLInputArchive archive( file );
        archive( *this );
    }
    else
    {
        Save();
    }
}

//-----------------------------------------------------------------------------
void Params::AddToPdbHistory( const std::string & a_PdbName )
{
    m_PdbHistory.push_back( a_PdbName );
    auto it = std::unique( m_PdbHistory.begin(), m_PdbHistory.end() );
    m_PdbHistory.resize( std::distance( m_PdbHistory.begin(), it) );
    Save();
}

//-----------------------------------------------------------------------------
void Params::ScanPdbCache()
{
    std::wstring cachePath = Path::GetCachePath();
    SCOPE_TIMER_LOG( Format( L"Scanning cache (%s)", cachePath.c_str() ) );

    for( auto it = std::tr2::sys::directory_iterator( cachePath ); it != std::tr2::sys::directory_iterator(); ++it )
    {
        const auto& file = it->path();
        if( file.extension() == ".bin" )
        {
            std::string fileName(file.filename().string());
            fileName = fileName.substr( 0, fileName.find_first_of('_') );
            m_CachedPdbsMap.insert( std::make_pair( fileName, ws2s(cachePath + file.filename().wstring()) ) );
        }
    }
}
