//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "OrbitModule.h"
#include "Serialization.h"
#include "Pdb.h"
#include <string>

#ifndef WIN32
#include "LinuxUtils.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitUnreal.h"
#include "Params.h"
#include "OrbitProcess.h"
#include "Path.h"
#endif

//-----------------------------------------------------------------------------
Module::Module()
{
}

//-----------------------------------------------------------------------------
const std::wstring & Module::GetPrettyName()
{
    if( m_PrettyName.size() == 0 )
    {
        #ifdef WIN32
        m_PrettyName = Format( L"%s [%I64x - %I64x] %s\r\n", m_Name.c_str(), m_AddressStart, m_AddressEnd, m_FullName.c_str() );
        m_AddressRange = Format( L"[%I64x - %I64x]", m_AddressStart, m_AddressEnd );
        #else
        m_PrettyName = m_FullName;
        m_AddressRange = Format( L"[%016llx - %016llx]", m_AddressStart, m_AddressEnd );
        m_PdbName = m_FullName;
        m_FoundPdb = true;
        #endif
    }

    return m_PrettyName;
}

//-----------------------------------------------------------------------------
bool Module::IsDll() const
{
    return ToLower( Path::GetExtension( m_FullName ) ) == std::wstring( L".dll" ) ||
           Contains(m_Name, L".so" );
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

#ifndef WIN32

//-----------------------------------------------------------------------------
void Pdb::LoadPdbAsync( const wchar_t* a_PdbName, std::function<void()> a_CompletionCallback )
{
    m_LoadingCompleteCallback = a_CompletionCallback;
    std::string command = std::string("nm ") + ws2s(a_PdbName) + std::string(" -n -l");
    std::string result = LinuxUtils::ExecuteCommand( command.c_str() );

    m_FileName = a_PdbName;
    m_Name = Path::GetFileName( m_FileName );

    std::stringstream ss(result);
    std::string line;
    while(std::getline(ss,line,'\n'))
    {
        std::vector<std::string> tokens = Tokenize(line);
        if( tokens.size() == 3 )
        {
            std::stringstream addrStream(tokens[0]);
            uint64_t address;
            addrStream >> address;
            Function func;
            func.m_Name = s2ws(tokens[2]);
            func.m_Address = address;
            func.m_PrettyName = func.m_Name;
            func.m_Module = Path::GetFileName(a_PdbName);
            func.m_Pdb = this;
            this->AddFunction(func);
        }
    }

    ProcessData();
    
    a_CompletionCallback();
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData()
{
    ScopeLock lock( Capture::GTargetProcess->GetDataMutex() );

    auto & functions = Capture::GTargetProcess->GetFunctions();
    auto & globals   = Capture::GTargetProcess->GetGlobals();

    functions.reserve( functions.size() + m_Functions.size() );

    for( Function & func : m_Functions )
    {
        func.m_Pdb = this;
        functions.push_back( &func );
        GOrbitUnreal.OnFunctionAdded( &func );
    }

    if( GParams.m_FindFileAndLineInfo )
    {
        SCOPE_TIMER_LOG(L"Find File and Line info");
        for( Function & func : m_Functions )
        {
            func.FindFile();
        }
    }

    for( Type & type : m_Types )
    {
        type.m_Pdb = this;
        Capture::GTargetProcess->AddType( type );
        GOrbitUnreal.OnTypeAdded( &type );
    }

    for( Variable & var : m_Globals )
    {
        var.m_Pdb = this;
        globals.push_back( &var );
    }

    for( auto & it : m_TypeMap )
    {
        it.second.m_Pdb = this;
    }

    PopulateFunctionMap();
    //PopulateStringFunctionMap();
    // TODO: parallelize: PopulateStringFunctionMap();
}

//-----------------------------------------------------------------------------
void Pdb::PopulateFunctionMap()
{
    m_IsPopulatingFunctionMap = true;
    
    SCOPE_TIMER_LOG( Format( L"Pdb::PopulateFunctionMap for %s", m_FileName.c_str() ) );

    for( Function & Function : m_Functions )
    {
        m_FunctionMap.insert( std::make_pair( Function.m_Address, &Function ) );
    }

    m_IsPopulatingFunctionMap = false;
}

#endif
