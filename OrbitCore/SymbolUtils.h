//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "OrbitType.h"
#include "Log.h"
#include "ProcessUtils.h"
#include "OrbitDbgHelp.h"
#include "SamplingProfiler.h"

//-----------------------------------------------------------------------------
struct SymUtils
{
    static void ListModules( HANDLE a_ProcessHandle, std::map< DWORD64, std::shared_ptr< Module > > & o_ModuleMap );
    static bool GetLineInfo( DWORD64 a_Address, LineInfo & o_LineInfo );
};

//-----------------------------------------------------------------------------
struct ScopeSymCleanup
{
    ScopeSymCleanup( HANDLE a_Handle );
    ~ScopeSymCleanup();
    HANDLE m_Handle;
};

//-----------------------------------------------------------------------------
inline bool SymInit( HANDLE a_Handle )
{
    return true;

//    SCOPE_TIMER_LOG( L"SymInit" );
//
//    DWORD Options = SymGetOptions();
//#ifdef _WIN64
//    if ( !ProcessUtils::Is64Bit(a_Handle) )
//    {
//        Options |= SYMOPT_INCLUDE_32BIT_MODULES;
//    }
//#endif
//    Options |= SYMOPT_DEBUG;
//    Options |= SYMOPT_LOAD_LINES;
//    SymSetOptions(Options);
//
//    if (!SymInitialize(a_Handle, L"", FALSE))
//    {
//        ORBIT_ERROR;
//        return false;
//    }
//
//    //SymRefreshModuleList( a_Handle );
//
//    return true;
}

//-----------------------------------------------------------------------------
inline void OrbitSymCleanup(HANDLE a_Handle)
{
    return;
    /*if( !SymCleanup( a_Handle ) )
    {
        ORBIT_ERROR;
    }*/
}

//-----------------------------------------------------------------------------
template<class Archive>
void serialize( Archive & archive, SYMBOL_INFO & a_SymbolInfo, std::uint32_t const version )
{
    archive( cereal::binary_data( &a_SymbolInfo, sizeof(SYMBOL_INFO) ) );
}

//-----------------------------------------------------------------------------
template<class Archive>
void serialize( Archive & archive, IMAGEHLP_MODULE64 & a_Module, std::uint32_t const version )
{
    archive( cereal::binary_data( &a_Module, sizeof(IMAGEHLP_MODULE64) ) );
}

